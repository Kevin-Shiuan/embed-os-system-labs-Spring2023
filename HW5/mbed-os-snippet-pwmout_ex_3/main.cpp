/*
 * Copyright (c) 2014-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <math.h>

#define PI 3.14159265358979323846

// Adjust pin name to your board specification.
// You can use LED1/LED2/LED3/LED4 if any is connected to PWM capable pin,
// or use any PWM capable pin, and see generated signal on logical analyzer.
PwmOut led(LED1);
// PwmOut led(PA_15);

int main() {
  // specify period first
  // led.period(2.0f);      // 4 second period
  // led.write(0.5f);      // 50% duty cycle, relative to period
  // while (1);

  float frequency = 1.0f; // Set the frequency of the sine wave
  float amplitude = 0.5f; // Set the amplitude of the sine wave
  //   float offset = 0.5f;    // Set the offset of the sine wave

  led.period(frequency); // Set the period of the PwmOut object to 1 second

  while (1) {
    for (float i = 0; i < 2 * PI; i += 0.1f) {
      //   led.write(amplitude * sin(frequency * i) + offset);
      led.write(amplitude * sin(frequency * i));
      wait_us(100); // wait for 100 microseconds to control the frequency of the output signal
    }
  }
}
