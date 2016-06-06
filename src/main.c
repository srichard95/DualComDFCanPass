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

#include "NetworkLayer.h"
#include "DataLinkLayer.h"

static DLLSerialConfig WIFICfg = {
  &SD1,
  921600
};

bool StopTest = false;
FrameStruct ToSendCorrect = { 0x78, 0x1C, {0x60, 0x7D, 0xDC, 0x69, 0x3B, 0x56, 0xDB, 0xB5, 0xF1, 0xD7, 0xC4, 0xA5}, 0x00};
FrameStruct ToSendWrong[] = { 0x50, 0x4D, 0xF4, 0xEB, 0xF8, 0xB1, 0x6F, 0x75, 0x43, 0x82, 0xAB, 0x23, 0x42, 0xE9 };
int SendingFreq = 10000;
int SendingPiece = 1;

static THD_WORKING_AREA(waUartTest, 128);
static THD_FUNCTION(UartTestThread, arg) {
  chRegSetThreadName("Driver Test");
  systime_t time;
  time = chVTGetSystemTime();
  int i;
  PacketStruct *csomag;
  DataLinkStatistics *Stats = &DLLS1.DLLStats;

  IPAddress ipcim = {192, 168, 4, 255};
  while(!StopTest)
  {
    time += US2ST(SendingFreq);
    Stats = &DLLS1.DLLStats;
    csomag = NWLCreatePacket(&WIFID1);


      for(i=0; i<SendingPiece; i++)
        NWLAddFrameToPacket(csomag, &ToSendCorrect);

      wifiSendUDP(&WIFID1, csomag, ipcim, 4000);
      /*if((Stats->SentFrames % 10000)==0)
        SendFrame(&ToSendWrong);*/
    chThdSleepUntil(time);
  }
}

void esp_send(BaseSequentialStream *chp, int argc, char *argv[]) {
  IPAddress ipcim = {192, 168, 4, 255};
  PacketStruct *csomag = NWLCreatePacket(&WIFID1);
  if (csomag == NULL)
    chprintf(chp, "null\r\n");


  int i;
  for(i = 0; i<MAX_FRAME_PER_PACKET; i++){
    NWLAddFrameToPacket(csomag, &ToSendCorrect);
  }

  wifiSendUDP(&WIFID1, csomag, ipcim, 4000);

  DataLinkStatistics *Stats = &DLLS1.DLLStats;

  chprintf(chp, "SentFrames: %d\r\n", Stats->SentFrames);
}


void start_driver_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  StopTest = false;
  SendingFreq = atoi(argv[0]);
  SendingPiece = atoi(argv[1]);
  DataLinkStatistics *Stats = &DLLS1.DLLStats;
  NetworkStatistics *NWLStats = &WIFID1.NWLStats;
  chThdCreateStatic(waUartTest, sizeof(waUartTest), NORMALPRIO+1, UartTestThread, NULL);
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "\x1B\x63");
    chprintf(chp, "\x1B[2J");
    chprintf(chp, "DRIVER STRESS TEST\r\n");

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
  StopTest = true;
}

void GetDllStats(BaseSequentialStream *chp, int argc, char *argv[]) {

  DataLinkStatistics *Stats = &DLLS1.DLLStats;
  chprintf(chp, "DataLinkLayer Statistics\r\n");

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
  //StartUart();


  wifiInit();
  wifiStart(&WIFID1, &DLLS1,&WIFICfg);


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
