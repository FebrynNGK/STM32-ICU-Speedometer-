/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/* dikerjakan dengan Chibi Os 2.6.9*/
#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"
#include "usbcfg.h"
#include <string.h>
#include "test.h"
/*tidak menggunakan PWM*/
/*menggunakan input Capture dan mengaktifkan ICUdriver*/
icucnt_t last_width, last_period;

static void icuwidthcb(ICUDriver *icup) {

  palSetPad(GPIOE, GPIOE_LED9_BLUE);
  last_width = icuGetWidth(icup);
}

static void icuperiodcb(ICUDriver *icup) {

  palClearPad(GPIOE, GPIOE_LED9_BLUE);
  last_period = icuGetPeriod(icup);
}

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  10000,                                    /* 10kHz ICU clock frequency.   */
  icuwidthcb,
  icuperiodcb,
  NULL,
  ICU_CHANNEL_1,
  0
};

/*
 * Application entry point.
 */
int main(void) {

  /*
   mengaktifkan chibios dan hal
   */
  halInit();
  chSysInit();

  /*
   * Initializes the PWM driver 2 and ICU driver 3.
   * GPIOA15 is the PWM output.
   * GPIOC6 is the ICU input.
   * The two pins have to be externally connected together.
   */
  //pwmStart(&PWMD2, &pwmcfg);
  icuStart(&ICUD3, &icucfg);
  palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(2));
  icuEnable(&ICUD3);
  chThdSleepMilliseconds(2000);

  icuDisable(&ICUD3);
  icuStop(&ICUD3);
  palClearPad(GPIOE, GPIOE_LED4_BLUE);
  palClearPad(GPIOE, GPIOE_LED9_BLUE);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
  return 0;
}
