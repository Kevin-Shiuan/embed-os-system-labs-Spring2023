#include "mbed.h"

PwmOut fan(PA_15); // pin D9
volatile int _count = 0;
#define LENGTH 10
#define TIMEOUT 1s

AnalogIn ain(PC_5); // pin A0

void increment() {
	_count++;
}

float rpm(void) {
	InterruptIn tach(PB_2); // pin D8
	float rpm;
	auto start = Kernel::Clock::now();
	std::chrono::milliseconds time;

	_count = 0;
	tach.rise(&increment);
	start = Kernel::Clock::now();
	while((_count < LENGTH) && (Kernel::Clock::now() - start < TIMEOUT));
	time = Kernel::Clock::now() - start;
	if(_count < LENGTH) {
		rpm = 0;
	} else {
		rpm = (LENGTH / 2) / (int(time.count()) / 60000.0);
	}

	return rpm;
}

int main() {
	int i;
	float P0, P, I, D;
	const float Kp = -10, Ki = -0.0001, Kd = -0.01;
	float set = 0.65;
	float rawduty, duty;

	auto start = Kernel::Clock::now();

	// specify period first
	fan.period_us(40.0f);      // 25kHz

	P0 = 0;
	I = 0;
	while(1) {
		P = set - ain.read();
		I += P;
		D = P - P0;
		rawduty = (set - 0.6) / 0.2 + Kp * P + Ki * I + Kd * D;
		if(rawduty < 0) {
			duty = 0;
			I = 0;
		} else if(rawduty > 1)
			duty = 1;
		else
			duty = rawduty;
		fan.write(duty);	// set duty cycle
		if(Kernel::Clock::now() - start > 5s) {
			printf("------------------------\n");
			printf("duty: %f\n", duty);
			printf("raw duty: %f\n", rawduty);
			printf("rpm: %f\n", rpm());
			printf("adc: %f\n", ain.read());
			start = Kernel::Clock::now();
		}
		P0 = P;
	}
}
