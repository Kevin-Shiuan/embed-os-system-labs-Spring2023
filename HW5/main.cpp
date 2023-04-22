#include "mbed.h"

// use PWM capable pin, and see generated signal on logical analyzer.
PwmOut led(PA_15);

int main() {
	int i;
	const float duty[] = {
		0.75,
		0.933,
		1,
		0.933,
		0.75,
		0.5,
		0.25,
		0.066,
		0,
		0.066,
		0.25,
		0.5
	};
	// specify period first
	led.period(1.0f);      // 1 second period

	for(i = 0; 1; i = (i + 1) % 12) {
		led.write(duty[i]);	// set duty cycle
		ThisThread::sleep_for(1s);
	}
}
