/**
 * @file    NetworkLayer.h
 * @brief   Network Layer header.
 *
 * @addtogroup DUALFRAMEWORK
 * @{
 */

#ifndef DUALFRAMEWORK_INCLUDE_NETWORKLAYER_H_
#define DUALFRAMEWORK_INCLUDE_NETWORKLAYER_H_

#include "ch.h"
#include "hal.h"
#include "FrameworkConf.h"
#include "DataLinkLayer.h"

#if DUALFRAMEWORK_USE_WIFI || defined(__DOXYGEN__)

/**
 * @brief   Network Layer Statistics struct.
 */
typedef struct{
  char FrameNumber;
  long SentPacket;
}NetworkStatistics;

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  WIFI_UNINIT = 0,                   /**< Not initialized.                   */
  WIFI_STOP = 1,                     /**< Stopped.                           */
  WIFI_ACTIVE = 2,                   /**< Active.                            */
} WIFI_state_t;


/**
 * @brief   WiFi driver struct
 */
typedef struct {
  /**
   * @brief Driver state.
   */
  WIFI_state_t         state;

  /**
   * @brief Network Layer Statistics.
   */
  NetworkStatistics NWLStats;

  /**
   * @brief Pointer to the DataLinkLayer driver structure
   */
  DLLDriver *DLLObject;

  /**
   * @brief Memory space declaration for the packets
   */
  memory_pool_t PacketPool;

} WIFIDriver;

/*
 * @brief   Frame type constants
 * @details These constants determines the type of a single frame
 */
#define FTYPE_USERDATA 0x00
#define FTYPE_UDPSEND 0x20

/**
 * @brief 'IPAddress' structure represents a data type which can store a whole
 *        IP address
 */
typedef struct{
  char oct1;
  char oct2;
  char oct3;
  char oct4;
}IPAddress;

/**
 * @brief   'PacketStruct' structure represents a data type which can store
 *          'MAX_FRAME_PER_PACKET' piece of frames.
 * @details 'length' contains the how many frames are inside the packet
 */
typedef struct{
  uint8_t length;
  FrameStruct FrameSlot[MAX_FRAME_PER_PACKET];
}PacketStruct;

/**
 * @brief   'PacketBuffer' is the memory space (buffer) of the memory_pool
 *          object which declared in the 'WiFiDriver' structure
 */
static PacketStruct PacketBuffer[MAX_AVAILABLE_PACKET] __attribute__((aligned(sizeof(stkalign_t))));

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/**
 * @brief Declaration of the driver itself
 */
extern WIFIDriver WIFID1;


void wifiInit(void);
void wifiStart(WIFIDriver *wifip, DLLDriver *dllp, DLLSerialConfig *config);
void NWLAssignFNtoPacket(WIFIDriver *wifip, PacketStruct *Packet);
void NWLAddFrameToPacket(PacketStruct *Packet, FrameStruct *Frame);
PacketStruct *NWLCreatePacket(WIFIDriver *wifip);
void NWLSendPacketUDP(WIFIDriver *wifip, PacketStruct *Packet, IPAddress ipaddr, int portnum);

/*===========================================================================*/
/* Function macros (NWL APIs).                                               */
/*===========================================================================*/
#define NWLSendSingleFramePacket(X, Y) wifiSendSFP(X, Y)
#define NWLSendPacketUDP(X, Y, Z, I) wifiSendUDP(X, Y, Z, I)


#endif /* DUALFRAMEWORK_USE_WIFI */
#endif /* DUALFRAMEWORK_INCLUDE_NETWORKLAYER_H_ */
