/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "console.h"
#include "EspUart.h"
#include "at_mode.h"
#include "CanComm.h"

#include "NetworkLayer.h"
#include "DataLinkLayer.h"

static DLLSerialConfig WIFICfg = {
  &SD1,
  921600
};


void GetDllStats(BaseSequentialStream *chp, int argc, char *argv[]) {
  DataLinkStatistics *Stats = &DLLS1.DLLStats;
  NetworkStatistics *NWLStats = &WIFID1.NWLStats;

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B\x63");
    chprintf(chp, "\x1B[2J");
    chprintf(chp, "DUALFRAMEWORK STATISTICS\r\n");

    Stats = &DLLS1.DLLStats;
    int lost = Stats->SentFrames - Stats->ReceivedFrames;
    chprintf(chp, "Sent: %d\r\n", Stats->SentFrames);
    chprintf(chp, "Received: %d\r\n", Stats->ReceivedFrames);
    chprintf(chp, "LostFrames: %d\r\n", Stats->LostFrames);
    chprintf(chp, "Sync: %d\r\n", Stats->SyncCounter);
    chprintf(chp, "SyncFrameSentCounter: %d\r\n", Stats->SyncFrameSentCounter);
    chprintf(chp, "SyncTimeout: %d\r\n", Stats->SyncTimeout);
    chprintf(chp, "FreeFilledBuffer: %d\r\n", Stats->FreeFilledBuffer);
    chprintf(chp, "FreeFreeBuffer: %d\r\n", Stats->FreeFreeBuffer);
    chprintf(chp, "CalculatedLostFrames: %d\r\n", lost);

    chprintf(chp, "\r\n");
    chprintf(chp, "SentPacket: %d\r\n", NWLStats->SentPacket);
    chprintf(chp, "FrameNumber: %d\r\n", NWLStats->FrameNumber);

    chThdSleepMilliseconds(100);
  }
}


/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  if (palReadPad(GPIOA, GPIOA_IN0) == PAL_HIGH)
      init_atmode();

  chThdSleepMilliseconds(100);
  consoleInit();


  wifiInit();
  wifiStart(&WIFID1, &DLLS1,&WIFICfg);

  CanCommInit();

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state, when the button is
   * pressed the test procedure is launched.
   */
  while (true) {
    consoleStart();
    chThdSleepMilliseconds(500);
  }
}
