/*
 * Copyright (c) 2014-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

// Adjust pin name to your board specification.
// You can use LED1/LED2/LED3/LED4 if any is connected to PWM capable pin,
// or use any PWM capable pin, and see generated signal on logical analyzer.
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
	led.period(1.0f);      // 4 second period
	//led.write(0.50f);      // 50% duty cycle, relative to period
	for(i = 0; 1; i = (i + 1) % 12) {
		led.write(duty[i]);
		ThisThread::sleep_for(1s);
	}
	//led = 0.5f;          // shorthand for led.write()
	//led.pulsewidth(2);   // alternative to led.write, set duty cycle time in seconds
	while (1);
}
