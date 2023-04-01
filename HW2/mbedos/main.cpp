#include "mbed.h"
#include "wifi_helper.h"
#include "mbed-trace/mbed_trace.h"

// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"


void wifi_scan(NetworkInterface *net) {
	static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
	WiFiInterface *wifi = net->wifiInterface();

	WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

	/* scan call returns number of access points found */
	int result = wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

	if (result <= 0) {
		printf("WiFiInterface::scan() failed with return value: %d\r\n", result);
		return;
	}

	printf("%d networks available:\r\n", result);

	for (int i = 0; i < result; i++) {
		printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n",
		ap[i].get_ssid(), get_security_string(ap[i].get_security()),
		ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
		ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5],
		ap[i].get_rssi(), ap[i].get_channel());
	}
	printf("\r\n");
}

void print_network_info(NetworkInterface *net) {
	/* print the network info */
	SocketAddress a;
	net->get_ip_address(&a);
	printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
	net->get_netmask(&a);
	printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
	net->get_gateway(&a);
	printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
}

bool resolve_hostname(NetworkInterface *net, SocketAddress &address) {
	const char hostname[] = MBED_CONF_APP_HOSTNAME;

	/* get the host address */
	printf("\nResolve hostname %s\r\n", hostname);
	nsapi_size_or_error_t result = net->gethostbyname(hostname, &address);
	if(result != 0) {
		printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
		return false;
	}

	printf("%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None") );

	return true;
}

bool send(TCPSocket *socket, const char buffer[], int len) {
	/* loop until whole buffer sent */

	nsapi_size_t bytes_to_send = strlen(buffer);
	nsapi_size_or_error_t bytes_sent = 0;

	printf("\r\nSending message: \r\n%s", buffer);

	while(bytes_to_send) {
		bytes_sent = socket->send(buffer + bytes_sent, bytes_to_send);
		if (bytes_sent < 0) {
			printf("Error! socket->send() returned: %d\r\n", bytes_sent);
			return false;
		} else {
			printf("sent %d bytes\r\n", bytes_sent);
		}

		bytes_to_send -= bytes_sent;
	}

	printf("Complete message sent\r\n");

	return true;
}

int setup(NetworkInterface *net, TCPSocket *socket) {
	static constexpr size_t REMOTE_PORT = 65431; // custom port
	nsapi_size_or_error_t result;
	SocketAddress address;

	if(!net) {
		printf("Error! No network interface found.\r\n");
		return 1;
	}

	if(net->wifiInterface()) {
		wifi_scan(net);
	}

	printf("Connecting to the network...\r\n");

	result = net->connect();
	if(result != 0) {
		printf("Error! net->connect() returned: %d\r\n", result);
		return 1;
	}

	print_network_info(net);

	/* opening the socket only allocates resources */
	result = socket->open(net);
	if(result != 0) {
		printf("Error! socket->open() returned: %d\r\n", result);
		return 1;
	}

	if (!resolve_hostname(net, address)) {
		return 1;
	}

	address.set_port(REMOTE_PORT);

	/* we are connected to the network but since we're using a connection oriented
	* protocol we still need to open a connection on the socket */

	printf("Opening connection to remote port %d\r\n", REMOTE_PORT);

	result = socket->connect(address);
	if (result != 0) {
		printf("Error! socket->connect() returned: %d\r\n", result);
		return 1;
	}

	return 0;
}


int main() {
	static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;
	NetworkInterface *net = NetworkInterface::get_default_instance();
	TCPSocket *socket = new TCPSocket;
	int16_t pDataXYZ[3] = {0};
	char buffer[1024];

	printf("\r\nStarting program\r\n\r\n");

#ifdef MBED_CONF_MBED_TRACE_ENABLE
	mbed_trace_init();
#endif

	if(setup(net, socket)) {
		if(net) {
			net->disconnect();
		}
	} else {
		BSP_ACCELERO_Init();
		while(true) {
			BSP_ACCELERO_AccGetXYZ(pDataXYZ);
			sprintf(buffer, "%d\t%d\t%d\n", pDataXYZ[0], pDataXYZ[1], pDataXYZ[2]);
			send(socket, buffer, strlen(buffer));
			ThisThread::sleep_for(100ms);
		}
	}

	return 0;
}
