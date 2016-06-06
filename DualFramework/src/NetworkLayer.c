/**
 * @file    NetworkLayer.c
 * @brief   Network Layer of the DualFramework code.
 *
 * @addtogroup DUALFRAMEWORK
 * @{
 */

#include "NetworkLayer.h"


#if DUALFRAMEWORK_USE_WIFI

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
/**
 * @brief   WIFID1 driver identifier.
 */
WIFIDriver WIFID1;



/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   WiFi Driver initialization.
 * @note    This function is init the wifi driver.
 *
 * @init
 */
void wifiInit(void) {

  /* Object initialization */
  wifiObjectInit(&WIFID1);
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] wifid         pointer to the @p WIFIDriver object
 *
 * @init
 */
void wifiObjectInit(WIFIDriver *wifid){

  wifid->state  = WIFI_STOP;
  wifid->NWLStats.FrameNumber = 0x00;
  wifid->NWLStats.SentPacket = 0x00;
}

/**
 * @brief   Configures and activates the Wifi Driver  peripheral.
 *
 * @details The function starts the whole FrameWork (NWL, DLL)
 *          - Check the actual status of the driver
 *          - Initialize and Start the DataLinkLayer
 *          - Initialize the memory pool for the packets
 *          - Set the WiFiDriver active
 *
 * @param[in] wifip   pointer to the @p WIFIDriver object
 * @param[in] dllp    pointer to the @p DLLDriver object
 * @param[in] config  pointer to the @p WIFIConfig object
 *
 * @api
 */
void wifiStart(WIFIDriver *wifip, DLLDriver *dllp, DLLSerialConfig *config) {

  osalDbgCheck((wifip != NULL) && (config != NULL));

  osalDbgAssert((wifip->state == WIFI_UNINIT) || (wifip->state == WIFI_ACTIVE),
              "wifiStart(), invalid state");

  DLLInit();

  wifip->DLLObject = dllp;
  DLLStart(wifip->DLLObject, config);

  chPoolObjectInit(&wifip->PacketPool, sizeof(PacketStruct), NULL);

  chPoolLoadArray(&wifip->PacketPool, &PacketBuffer[0], MAX_AVAILABLE_PACKET);

  wifip->state = WIFI_ACTIVE;
}

/**
 * @brief  Increase and return with the next FrameNumber
 *
 * @param[in] wifip   pointer to the @p WIFIDriver object
 */
char NWLGetNextFrameNumber(WIFIDriver *wifip){
  char next = wifip->NWLStats.FrameNumber;
  wifip->NWLStats.FrameNumber++;
  return next;
}

/**
 * @brief  Assign the next FrameNumber to the frames which are inside the packet
 *
 * @param[in]  wifip          pointer to the @p WIFIDriver object
 * @param[out] PacketStruct   pointer to the @p PacketStruct object
 */
void NWLAssignFNtoPacket(WIFIDriver *wifip, PacketStruct *Packet){
  char FN = NWLGetNextFrameNumber(wifip);
  int i;

  for(i = 0; i < Packet->length; i++)
    Packet->FrameSlot[i].FrameNumber = FN;
}

/**
 * @brief   The function creates an UDP controller frame based on a predefined
 *          pattern. The pattern is defined by the protocol (below).
 *
 * @details This controller frame means that the receiver device will send
 *          received frames to the given IP and port number. The 'FTYPE_UDPSEND'
 *          frame ID constant carry the 'send' information.
 *
 * @note    Data Fields:
 *          |_____4byte integer portnumber_____|____IP address_____|
 *          portnum4|portnum3|portnum2|portnum1|oct4|oct3|oct2|oct1|
 *
 *
 * @param[in] frame     pointer to the @p FrameStruct object
 * @param[in] ipaddr    pointer to the @p IPAddress object
 * @param[in] portnum   pointer to the @p portnum variable
 */
void NWLCreateControlFrameUDP(FrameStruct *frame, IPAddress *ipaddr, int *portnum){
  frame->Id = FTYPE_UDPSEND;
  memcpy(frame->data, ipaddr, sizeof(IPAddress));

  frame->data[4] = (char)*portnum;
  frame->data[5] = (char)(*portnum >> 8);
  frame->data[6] = (char)(*portnum >> 16);
  frame->data[7] = (char)(*portnum >> 24);
}

/**
 * @brief   The function execute the sending procedure as a result in the UDP
 *          packet will be sent.
 *
 * @details The following steps defines the sending procedure:
 *          - Check the driver state
 *          - Assign the proper frame number to the frames which are in the packet
 *          - Assign the proper frame id to the frames which are in the packet
 *          - Put the frames each by each into the mailbox
 *          - Create and send the control frame with the proper ID
 *          - Free the memory space of the packet
 *          - Increase the 'SentPackets' statistics
 *
 * @param[in] wifip    pointer to the @p WIFIDriver variable
 * @param[in] packet   pointer to the @p PacketStruct variable
 * @param[in] ipaddr   IPAddress struct which contains the ip address of the packet
 * @param[in] portnum  Port number of the packet
 */
void NWLSendPacketUDP(WIFIDriver *wifip, PacketStruct *Packet, IPAddress ipaddr, int portnum){
  osalDbgCheck((wifip != NULL));

  osalDbgAssert((wifip->state == WIFI_STOP) || (wifip->state == WIFI_UNINIT),
              "NWLSendPacket(), invalid state");

  NWLAssignFNtoPacket(wifip, Packet);

  int i;
  for(i=0; i < Packet->length; i++)
  {
    Packet->FrameSlot[i].Id = FTYPE_USERDATA;
    DLLPutFrameInQueue(wifip->DLLObject, &Packet->FrameSlot[i]);
  }

  FrameStruct ControlFrame;
  NWLCreateControlFrameUDP(&ControlFrame, &ipaddr, &portnum);
  DLLPutFrameInQueue(wifip->DLLObject, &ControlFrame);

  chPoolFree(&wifip->PacketPool, (void*)Packet);
  wifip->NWLStats.SentPacket++;
}

/**
 * @brief   Return the pointer of a packet
 *
 * @details The function serves pointer from the packet memory pool
 *
 * @param[in] wifip    pointer to the @p WIFIDriver variable
 */
PacketStruct *NWLCreatePacket(WIFIDriver *wifip)
{
  PacketStruct *Temp = chPoolAlloc(&wifip->PacketPool);
  if(Temp == NULL)
    return NULL;
  Temp->length = 0;
  return Temp;
}

/**
 * @brief   The function adds a single frame to a packet
 *
 * @param[out] packet     pointer to the @p PacketStruct object
 * @param[in]  frame     pointer to the @p FrameStruct object
 */
void NWLAddFrameToPacket(PacketStruct *Packet, FrameStruct *Frame){
  uint8_t l = Packet->length;
  FrameStruct *Temp = &Packet->FrameSlot[l];
  memcpy(Temp, Frame, sizeof(FrameStruct));
  Packet->length++;
}




#endif /* DUALFRAMEWORK_USE_WIFI */
