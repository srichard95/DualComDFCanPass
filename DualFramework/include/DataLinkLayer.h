/**
 * @file    DataLinkLayer.h
 * @brief   DataLink Layer header.
 *
 * @addtogroup DUALFRAMEWORK
 * @{
 */

#ifndef DUALFRAMEWORK_INCLUDE_DATALINKLAYER_H_
#define DUALFRAMEWORK_INCLUDE_DATALINKLAYER_H_

#include "ch.h"
#include "hal.h"
#include "FrameworkConf.h"


#if DUALFRAMEWORK_USE_WIFI || defined(__DOXYGEN__)

/**
 * @brief  Define the a single frame size in byte
 */
#define FRAME_SIZE_BYTE 15

/**
 * @brief  Define the number of Sync Timeout
 */
#define SYNC_TIMEOUT_THRS 100

/**
 * @brief  Represents a Frame.
 */
typedef struct{
  char Id;
  char FrameNumber;
  char data[12];
  char CrcHex;
}FrameStruct;

/**
 * @brief  Represents a Statistics structure.
 */
typedef struct{
  long SentFrames;
  long ReceivedFrames;
  long LostFrames;
  long SyncTimeout;
  long SyncCounter;
  long SyncFrameSentCounter;
  int FreeFilledBuffer;
  int FreeFreeBuffer;
}DataLinkStatistics;

/*
 * @brief The 'DLLBufferPark' contains all the buffers (array) which used by the
 *        DataLinkLayer
 */
typedef struct{
  FrameStruct DLLOutputBuffer[OUTPUT_FRAME_BUFFER];

  msg_t DLLFreeOutputBufferQueue[OUTPUT_FRAME_BUFFER];
  mailbox_t DLLFreeOutputBuffer;

  msg_t DLLFilledOutputBufferQueue[OUTPUT_FRAME_BUFFER];
  mailbox_t DLLFilledOutputBuffer;

  FrameStruct DLLInputBuffer[INPUT_FRAME_BUFFER];
}DLLBufferPark;

/*
 * Contains the states of the DataLinkLayer
 */
typedef enum {
  DLL_UNINIT = 0,                   /**< Not initialized.                   */
  DLL_STOP = 1,                     /**< Stopped.                           */
  DLL_ACTIVE = 2,                   /**< Active.                            */
} DLLSerial_state_t;

/**
 * @brief   DataLinkLayer config.
 * @details Contains the driver ID and the speed of the serial communication
 */
typedef struct{
  SerialDriver *SDriver;
  uint32_t baudrate;
}DLLSerialConfig;

/**
 * @brief   DataLinkLayer structure
 * @details Represents the whole DataLinkLayer serial communication driver
 */
typedef struct {
  /**
   * @brief Driver state.
   */
  DLLSerial_state_t state;

  /**
   * @brief Current configuration data.
   */
  const DLLSerialConfig *config;

  /**
   * @brief DataLinkLayer Statistics.
   */
  DataLinkStatistics DLLStats;

  /**
   * @brief The bytes from the serial line write into DLLTempBuffer firstly
   */
  char DLLTempBuffer[FRAME_SIZE_BYTE];

  /**
   * @brief DLLSyncFrame contains a SyncFrame which generated in the DLLStartFunction
   */
  char DLLSyncFrame[FRAME_SIZE_BYTE];

  /**
   * @brief This mutex used to sync the datastream between the sync protocol and
   *        the user data.
   */
  mutex_t DLLSerialSendMutex;

  /**
   * @brief Declaration of the buffers used in the DLL driver
   */
  DLLBufferPark DLLBuffers;

  /**
   * @brief Pointers of the SDReceiving and SDSending thread
   */
  thread_t *SendingThread;
  thread_t *ReceivingThread;

} DLLDriver;

/**
 * @brief   DLL serial line cfg
 */
static SerialConfig SerialDCfg =
{
DEFAULT_BAUDRATE, // bit rate
0,
0,
0
};

/**
 * @brief Declaration of the DataLinkLayer
 */
extern DLLDriver DLLS1;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
void DLLInit(void);
void DLLStart(DLLDriver *dllp, DLLSerialConfig *config);
void DLLCreateSyncFrame(DLLDriver *dllp);
void DLLSyncProcedure(DLLDriver *driver);
DataLinkStatistics *DLLGetStats(DLLDriver *dllp);
msg_t DLLPutFrameInQueue(DLLDriver *dllp, FrameStruct *Frame);
bool DLLSendSingleFrameSerial(DLLDriver *driver, FrameStruct *Frame);


#endif /* DUALFRAMEWORK_USE_WIFI */
#endif /* DUALFRAMEWORK_INCLUDE_DATALINKLAYER_H_ */
