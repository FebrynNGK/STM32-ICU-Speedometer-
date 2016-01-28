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
#include "test.h"
#include "shell.h"
#include "chprintf.h"
#include "usbcfg.h"
#include <string.h>

extern const USBConfig usbcfg;
extern const SerialUSBConfig serusbcfg;

int data_periode;
int kecepatan;
//diberi usb cdc
// Serial over USB Driver structure.
SerialUSBDriver SDU1;
BaseSequentialStream* chp =(BaseSequentialStream*) &SDU1;
#define usb_lld_connect_bus(usbp)
#define usb_lld_disconnect_bus(usbp)
//
//mengkatifkan shell command dan menghitung kecepatan mobil
void send_kecepatan(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argc;
  (void)argv;
  (void)chp;
  chprintf((BaseSequentialStream *)&SDU1, "kecepatan=%d\r\n", data_periode);

}
void send_periode(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)argc;
    (void)argv;
    (void)chp;
    chprintf((BaseSequentialStream *)&SDU1, "periode=%d\r\n",data_periode);
}
#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
static const ShellCommand commands[] = {
  {"kecepatanl", send_kecepatan},//ketik ul+enter pada serial terminal(bray terminal)
  {"periode", send_periode},
  {NULL, NULL}
};
static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};

/*tidak menggunakan PWM seperti contoh di testhal*/
/*menggunakan input Capture dan mengaktifkan ICUdriver*/
/*menghitung pulsa masuk pada pin P...*/
icucnt_t last_width, last_period;

static void icuwidthcb(ICUDriver *icup) {

  palSetPad(GPIOE, GPIOE_LED9_BLUE);
  last_width = icuGetWidth(icup);
}

static void icuperiodcb(ICUDriver *icup) {

  palClearPad(GPIOE, GPIOE_LED9_BLUE);
  last_period = icuGetPeriod(icup);
  data_periode=last_period;
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

/*belajar membuat thread, thread ini untuk menghitung data*/

static WORKING_AREA(waThread_hitung, 1024);
static msg_t Thread_hitung(void *arg) {
  (void)arg;
  chRegSetThreadName("hitung");
  kecepatan=0;//kecepatan awal
  int data_periode1;
  while (TRUE)
  {
      data_periode1=data_periode*4;//angka 4 menunjukan berapa high dalam 1 lingkaran roda
    kecepatan = (((2*3.14)/data_periode1)*0.3);

    chThdSleepMilliseconds(100);//jarak antar perhitungan
  };
  return 0;
}
/*
 * progam utama
 */
int main(void) {
    Thread *shelltp = NULL;//deklarasi thread shell comand

  /*
   mengaktifkan chibios dan hal
   */
  halInit();
  chSysInit();
  /*inisiasi usb*/
  sduObjectInit(&SDU1);
  sduStart(&SDU1,&serusbcfg);
/*mengaktifkan usb*/
 usbDisconnectBus(serusbcfg.usbp);
 chThdSleepMilliseconds(200);
 usbStart(serusbcfg.usbp, &usbcfg);
 usbConnectBus(serusbcfg.usbp);
 /*mengaktifkan shell*/
 shellInit();

  /*
   * Initializes the PWM driver 2 and ICU driver 3.
   * GPIOA15 is the PWM output.
   * GPIOC6 is the ICU input.
   * The two pins have to be externally connected together.
   * diaktifkan juga di halconf dan mcuconf diganti true
   */
  icuStart(&ICUD3, &icucfg);
  palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(2));
  icuEnable(&ICUD3);
  chThdSleepMilliseconds(2000);

  icuDisable(&ICUD3);
  icuStop(&ICUD3);
  palClearPad(GPIOE, GPIOE_LED4_BLUE);
  palClearPad(GPIOE, GPIOE_LED9_BLUE);
//memanggil thread yang dibuat diatas

   chThdCreateStatic(waThread_hitung, sizeof(waThread_hitung), NORMALPRIO+10, Thread_hitung, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (TRUE) {

    //khusus USB Comand
        if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
            {
                shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);//mengaktifkan shell comand
            }
        else if (chThdTerminated(shelltp))
            {
                chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
                shelltp = NULL;           /* Triggers spawning of a new shell.        */
            }
    chThdSleepMilliseconds(500);//delay mainthread
       \
      }
      return 0;
  }

