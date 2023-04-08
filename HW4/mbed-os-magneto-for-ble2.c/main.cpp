/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include "ble/services/HeartRateService.h"
#include "pretty_printer.h"
#include "mbed-trace/mbed_trace.h"

#include "mbed.h"

// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "Heartrate";

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

class MagnetoService {
	public:
	const static uint16_t MAGNETO_SERVICE_UUID = 0xA000;
	const static uint16_t PDATAX_CHARACTERISTIC_UUID = 0xA001;
	const static uint16_t PDATAY_CHARACTERISTIC_UUID = 0xA002;
	const static uint16_t PDATAZ_CHARACTERISTIC_UUID = 0xA003;

	MagnetoService(BLE &_ble, int16_t pDataXinitial, int16_t pDataYinitial, int16_t pDataZinitial) :
		ble(_ble),
		pDataX(PDATAX_CHARACTERISTIC_UUID,
				&pDataXinitial,
				GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
		pDataY(PDATAY_CHARACTERISTIC_UUID,
				&pDataXinitial,
				GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
		pDataZ(PDATAZ_CHARACTERISTIC_UUID,
				&pDataXinitial,
				GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {

		GattCharacteristic *charTable[] = {&pDataX, &pDataY, &pDataZ};
		GattService magnetoService(MagnetoService::MAGNETO_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
		ble.gattServer().addService(magnetoService);
	}

	void update_pDataXYZ(int16_t pDataXYZ[]) {
		ble.gattServer().write(pDataX.getValueHandle(), (uint8_t *)&pDataXYZ[0], sizeof(int16_t));
		ble.gattServer().write(pDataY.getValueHandle(), (uint8_t *)&pDataXYZ[1], sizeof(int16_t));
		ble.gattServer().write(pDataZ.getValueHandle(), (uint8_t *)&pDataXYZ[2], sizeof(int16_t));
	}

	private:
	BLE &ble;
	ReadOnlyGattCharacteristic<int16_t> pDataX;
	ReadOnlyGattCharacteristic<int16_t> pDataY;
	ReadOnlyGattCharacteristic<int16_t> pDataZ;
};

class HeartrateDemo : ble::Gap::EventHandler {
	public:
	HeartrateDemo(BLE &ble, events::EventQueue &event_queue) :
		_ble(ble),
		_event_queue(event_queue),
		_heartrate_uuid(GattService::UUID_HEART_RATE_SERVICE),
		_heartrate_value(100),
		_heartrate_service(ble, _heartrate_value, HeartRateService::LOCATION_FINGER),
		_pDataXYZ{0, 0, 0},
		_magneto_service(ble, 0, 0, 0),
		_adv_data_builder(_adv_buffer) {
	}

	void start() {
		_ble.init(this, &HeartrateDemo::on_init_complete);
		BSP_MAGNETO_Init();

		_event_queue.dispatch_forever();
	}

	private:
	/** Callback triggered when the ble initialization process has finished */
	void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
		if (params->error != BLE_ERROR_NONE) {
			printf("Ble initialization failed.");
			return;
		}

		print_mac_address();

		/* this allows us to receive events like onConnectionComplete() */
		_ble.gap().setEventHandler(this);

		/* heart rate value updated every second */
		_event_queue.call_every(
			1000ms,
			[this] {
			update_sensor_value();
			}
			);

		start_advertising();
	}

	void start_advertising() {
		/* Create advertising parameters and payload */

		ble::AdvertisingParameters adv_parameters(
			ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
			ble::adv_interval_t(ble::millisecond_t(100))
			);

		_adv_data_builder.setFlags();
		_adv_data_builder.setAppearance(ble::adv_data_appearance_t::GENERIC_HEART_RATE_SENSOR);
		_adv_data_builder.setLocalServiceList({&_heartrate_uuid, 1});
		_adv_data_builder.setName(DEVICE_NAME);

		/* Setup advertising */

		ble_error_t error = _ble.gap().setAdvertisingParameters(
			ble::LEGACY_ADVERTISING_HANDLE,
			adv_parameters
			);

		if (error) {
			printf("_ble.gap().setAdvertisingParameters() failed\r\n");
			return;
		}

		error = _ble.gap().setAdvertisingPayload(
			ble::LEGACY_ADVERTISING_HANDLE,
			_adv_data_builder.getAdvertisingData()
			);

		if (error) {
			printf("_ble.gap().setAdvertisingPayload() failed\r\n");
			return;
		}

		/* Start advertising */

		error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

		if (error) {
			printf("_ble.gap().startAdvertising() failed\r\n");
			return;
		}

		printf("Heart rate sensor advertising, please connect\r\n");
	}

	void update_sensor_value() {
		/* you can read in the real value but here we just simulate a value */
		_heartrate_value++;

		/*  60 <= bpm value < 110 */
		if (_heartrate_value == 110) {
			_heartrate_value = 60;
		}

		_heartrate_service.updateHeartRate(_heartrate_value);


		BSP_MAGNETO_GetXYZ(_pDataXYZ);
		/*
		printf("\nMAGNETO_X = %d\n", _pDataXYZ[0]);
		printf("MAGNETO_Y = %d\n", _pDataXYZ[1]);
		printf("MAGNETO_Z = %d\n", _pDataXYZ[2]);
		*/

		_magneto_service.update_pDataXYZ(_pDataXYZ);
	}

	/* these implement ble::Gap::EventHandler */
	private:
	/* when we connect we stop advertising, restart advertising so others can connect */
	virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event) {
		if (event.getStatus() == ble_error_t::BLE_ERROR_NONE) {
			printf("Client connected, you may now subscribe to updates\r\n");
		}
	}

	/* when we connect we stop advertising, restart advertising so others can connect */
	virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) {
		printf("Client disconnected, restarting advertising\r\n");

		ble_error_t error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

		if (error) {
			printf("_ble.gap().startAdvertising() failed\r\n");
			return;
		}
	}

	private:
	BLE &_ble;
	events::EventQueue &_event_queue;

	UUID _heartrate_uuid;
	uint16_t _heartrate_value;
	HeartRateService _heartrate_service;

	int16_t _pDataXYZ[3];
	MagnetoService _magneto_service;

	uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
	ble::AdvertisingDataBuilder _adv_data_builder;
};


/* Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
	event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

int main() {
	mbed_trace_init();

	BLE &ble = BLE::Instance();
	ble.onEventsToProcess(schedule_ble_events);

	HeartrateDemo demo(ble, event_queue);
	demo.start();

	return 0;
}
