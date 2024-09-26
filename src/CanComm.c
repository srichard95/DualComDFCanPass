/*
 * CanComm.c
 *
 *  Created on: 2016 jún. 6
 *      Author: srich
 */

#include "ch.h"
#include "hal.h"
#include "CanComm.h"

#include "NetworkLayer.h"
#include "DataLinkLayer.h"

PacketStruct *packet;
IPAddress ipcim = {192, 168, 4, 255};
#define PORTNUMBER 4000

static binary_semaphore_t SendSync;


static const CANConfig cancfg = {
 CAN_MCR_ABOM,
 CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
 CAN_BTR_TS1(8) | CAN_BTR_BRP(11)
};
/*
static const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_LBKM | CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};*/
/*
static const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(5)
};*/


static THD_WORKING_AREA(waSendingThread, 256);
static THD_FUNCTION(SendingThread, arg) {
  chRegSetThreadName("Sending Thread");
  systime_t time;
  time = chVTGetSystemTime();
  int divider = 0;
  while(true)
  {
    time += MS2ST(DATAFREQ);
    chBSemWait(&SendSync);

    if(packet->length > 0){

      wifiSendUDP(&WIFID1, packet, ipcim, PORTNUMBER);
      packet = NWLCreatePacket(&WIFID1);

      divider++;
      if(divider == 5)
      {
        divider = 0;
        palTogglePad(GPIOB, GPIOB_LED1);
      }

    }

    chBSemSignal(&SendSync);
    chThdSleepUntil(time);
  }
}

/*
 * Receiver thread.
 */
static THD_WORKING_AREA(can_rx_wa, 256);
static THD_FUNCTION(can_rx, p) {
  event_listener_t el;
  CANRxFrame rxmsg;

  (void)p;
  chRegSetThreadName("receiver");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);
  while(!chThdShouldTerminateX()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
      continue;
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == MSG_OK) {
      chBSemWait(&SendSync);
      FrameStruct frame;
      frame.Id = FTYPE_USERDATA;

      int i;
      for(i = 0; i < 8; i++)
        frame.data[i] = rxmsg.data8[i];

      frame.data[8] = (uint8_t)rxmsg.EID;
      frame.data[9] = rxmsg.EID >> 8;
      frame.data[10] = rxmsg.EID >> 16;

      NWLAddFrameToPacket(packet, &frame);

      rxmsg.EID = 0x00;
      chBSemSignal(&SendSync);
    }
  }
  chEvtUnregister(&CAND1.rxfull_event, &el);
}

/*
 * Transmitter thread.
 *//*
static THD_WORKING_AREA(can_tx_wa, 256);
static THD_FUNCTION(can_tx, p) {
  CANTxFrame txmsg;

  (void)p;
  chRegSetThreadName("transmitter");
  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  while (!chThdShouldTerminateX()) {
    txmsg.EID = 0x01234567;
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
    txmsg.EID++;
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
    txmsg.EID++;
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
    chThdSleepMilliseconds(10);
  }
}*/


void CanCommInit(){
  packet = NWLCreatePacket(&WIFID1);
  chBSemSignal(&SendSync);
  /*
   * Activates the CAN driver 1.
   */
  canStart(&CAND1, &cancfg);

  /*
   * Starting the transmitter and receiver threads.
   */
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO+7, can_rx, NULL);
  chThdCreateStatic(waSendingThread, sizeof(waSendingThread), NORMALPRIO + 7, SendingThread, NULL);
  //chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);
}
