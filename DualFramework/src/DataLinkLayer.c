/**
 * @file    DataLinkLayer.c
 * @brief   DataLink Layer of the DualFramework code.
 *
 * @addtogroup DUALFRAMEWORK
 * @{
 */

#include "DataLinkLayer.h"

#if DUALFRAMEWORK_USE_WIFI || defined(__DOXYGEN__)

/**
 * DataLinkLayer Serial Driver structure
 */
DLLDriver DLLS1;

/**
 * @brief  A single frame which represents a sync frame.
 */
char DLLSyncFrame[FRAME_SIZE_BYTE];


/*===========================================================================*/
/* Receiving and Sync functions                                              */
/*===========================================================================*/

/**
 * @brief   The Main receiving function.
 * @details The SDReceiving thread responsible for receiving frames
 *          continuously from the serial.
 */
static THD_FUNCTION(SDReceiving, arg) {
  chRegSetThreadName("Main Receiving Func");
  DLLDriver *driver = arg;
  while(true)
  {
    int i;
    for(i = 0; i < FRAME_SIZE_BYTE; i++)
      sdRead(driver->config->SDriver, &driver->DLLTempBuffer[i], 1);

    if(CheckCRC(&driver->DLLTempBuffer) == 0)
    {
      driver->DLLStats.ReceivedFrames++;
    }else
    {
      chMtxLock(&driver->DLLSerialSendMutex);
      DLLSyncProcedure(driver);
      chMtxUnlock(&driver->DLLSerialSendMutex);
    }
  }
}

/**
 * @brief   Send a single SYNC Frame via serial.
 *
 */
void DLLSendSyncFrame(DLLDriver *driver){
  driver->DLLStats.SyncFrameSentCounter++;
  sdWrite(driver->config->SDriver, driver->DLLSyncFrame, FRAME_SIZE_BYTE);
}

/**
 * @brief   Sync procedure.
 * @details This function implements the synchronization protocol
 *          between the STM and ESP/RPi
 *
 * @param[in] driver  pointer to the DataLinkLayer driver object
 */
void DLLSyncProcedure(DLLDriver *driver)
{
  int FFs = 0;
  char c;
  DLLSendSyncFrame(driver);
  driver->DLLStats.SyncCounter++;

  while(FFs != FRAME_SIZE_BYTE)
  {
    driver->DLLStats.SyncTimeout++;
    sdReadTimeout(driver->config->SDriver, &c, 1, US2ST(1000));
    if(c == 0xFF)
    {
      FFs++;
      c = 0x00;
    }
    if(driver->DLLStats.SyncTimeout >= SYNC_TIMEOUT_THRS){
      DLLSendSyncFrame(driver);
      driver->DLLStats.SyncTimeout = 0;
    }
  }
  driver->DLLStats.SyncTimeout = 0;
  return;
}


/**
 * @brief   Create a single SYNC Frame into the SyncFrame array.
 * @details It creates a sync frame dynamically
 *
 * @param[in] dllp  pointer to the DataLinkLayer driver object
 */
void DLLCreateSyncFrame(DLLDriver *dllp){
  int i;
  for(i = 0; i < FRAME_SIZE_BYTE; i++)
    dllp->DLLSyncFrame[i] = 0xFF;
}
/*===========================================================================*/
/* Sending functions                                                         */
/*===========================================================================*/

/**
 * @brief   Continuous serial sending thread.
 * @details The SDSending thread responsible for the continuous frame sending
 *          via serial. It receives the frames from the application through a
 *          mailbox.
 */
static THD_FUNCTION(SDSending, arg) {
  chRegSetThreadName("Sending Thread");
  DLLDriver *dllp = arg;
  void *pbuf;
  FrameStruct *Temp;
  while(true)
  {
      dllp->DLLStats.FreeFilledBuffer = chMBGetFreeCountI(&dllp->DLLBuffers.DLLFilledOutputBuffer);
      dllp->DLLStats.FreeFreeBuffer = chMBGetFreeCountI(&dllp->DLLBuffers.DLLFreeOutputBuffer);
      msg_t msg = chMBFetch(&dllp->DLLBuffers.DLLFilledOutputBuffer, (msg_t *)&pbuf, TIME_INFINITE);

      if(msg == MSG_OK)
      {
        Temp = pbuf;
        if(DLLSendSingleFrameSerial(dllp, Temp))
          dllp->DLLStats.SentFrames++;
        else
          dllp->DLLStats.LostFrames++;
        (void)chMBPost(&dllp->DLLBuffers.DLLFreeOutputBuffer, (msg_t)pbuf, TIME_INFINITE);
      }
  }
}

/**
 * @brief   Send a 'FrameStruct' type pointer via serial
 * @details If a sync happens the serial sending blocked by a mutex variable
 *          and the frame which need to be sent will lost.
 *
 * @param[in] driver    DataLinkLayer driver structure
 * @param[in] frame     The frame which need to be sent
 *
 */
bool DLLSendSingleFrameSerial(DLLDriver *driver, FrameStruct *Frame){
  bool IsLocked = chMtxTryLock(&driver->DLLSerialSendMutex);
  if(IsLocked){
    sdWrite(driver->config->SDriver, Frame, FRAME_SIZE_BYTE);
    palTogglePad(GPIOB, GPIOB_LED1);
    chMtxUnlock(&driver->DLLSerialSendMutex);
  }
  return IsLocked;
}

/**
 * @brief   Put one 'FrameStruct' type pointer into the mailbox
 * @details This function is one of the APIs between the NetworkLayer and
 *          the DataLinkLayer. When a packet need to be sent the NWL will
 *          call this function individually.
 *
 * @param[in] driver    DataLinkLayer driver structure
 * @param[in] frame     The frame which need to be put into the mailbox
 *
 */
msg_t DLLPutFrameInQueue(DLLDriver *dllp, FrameStruct *Frame){
  void *pbuf;
  msg_t ReturnValue = chMBFetch(&dllp->DLLBuffers.DLLFreeOutputBuffer, (msg_t *)&pbuf, TIME_INFINITE);
  if (ReturnValue == MSG_OK) {
    FrameStruct *Temp = pbuf;
    int i;
    for(i = 0; i < FRAME_SIZE_BYTE; i++)
      ((char *)Temp)[i] = ((char *)Frame)[i];

    Temp->CrcHex = CreateCRC(Temp);

    (void)chMBPost(&dllp->DLLBuffers.DLLFilledOutputBuffer, (msg_t)pbuf, TIME_INFINITE);
  }
  return ReturnValue;
}

/**
 * @brief  Gives back the statistics of the DataLink Layer
 */
DataLinkStatistics *DLLGetStats(DLLDriver *dllp){
  return &dllp->DLLStats;
}

/**
 * @brief Init the DataLinkLayer structure
 */
void DLLObjectInit(DLLDriver *dllp){

  dllp->state  = DLL_STOP;
  dllp->config = NULL;
}

void DLLInit(void) {

  /* Object initialization */
  DLLObjectInit(&DLLS1);
}

/**
 * @brief   Initialize the Data Link Layer object.
 * @details The function starts the DataLinkLayer serial driver
 *          - Check the actual state of the driver
 *          - Configure and start the sdSerial driver
 *          - Init the mutex variable used by the 'DLLSendSingleFrameSerial' function
 *          - Init the mailboxes which are work like a buffer
 *          - Creates a SyncFrame
 *          - Starts the 'SDReceiving' and 'SDSending' threads which are provide
 *            the whole DLL functionality
 *          - Set the DLL state to ACTIVE
 *
 * @param[in] dllp    DataLinkLayer driver structure
 * @param[in] config  The config contains the speed and the ID of the serial driver
 */
void DLLStart(DLLDriver *dllp, DLLSerialConfig *config){
  osalDbgCheck((dllp != NULL) && (config != NULL));

  osalDbgAssert((dllp->state == DLL_UNINIT) || (dllp->state == DLL_ACTIVE),
              "DLLInit(), invalid state");

  dllp->config = config;
  SerialDCfg.speed = dllp->config->baudrate;       //Set the data rate to the given rate
  sdStart(dllp->config->SDriver, &SerialDCfg);     //Start the serial driver for the ESP8266


  chMtxObjectInit(&dllp->DLLSerialSendMutex);


  chMBObjectInit(&dllp->DLLBuffers.DLLFilledOutputBuffer,
                 dllp->DLLBuffers.DLLFilledOutputBufferQueue, OUTPUT_FRAME_BUFFER);

  chMBObjectInit(&dllp->DLLBuffers.DLLFreeOutputBuffer,
                 dllp->DLLBuffers.DLLFreeOutputBufferQueue, OUTPUT_FRAME_BUFFER);

  int i;
  for (i = 0; i < OUTPUT_FRAME_BUFFER; i++)
    (void)chMBPost(&dllp->DLLBuffers.DLLFreeOutputBuffer,
                   (msg_t)&dllp->DLLBuffers.DLLOutputBuffer[i], TIME_INFINITE);

  DLLCreateSyncFrame(dllp);

  dllp->SendingThread = chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(128), NORMALPRIO+1, SDSending, (void *)dllp);
  if (dllp->SendingThread == NULL)
    chSysHalt("DualFramework: Starting 'SendingThread' failed - out of memory");

  dllp->ReceivingThread = chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(128), NORMALPRIO+1, SDReceiving, (void *)dllp);
  if (dllp->ReceivingThread == NULL)
    chSysHalt("DualFramework: Starting 'ReceivingThread' failed - out of memory");

  dllp->state = DLL_ACTIVE;
}

#endif /* DUALFRAMEWORK_USE_WIFI */
