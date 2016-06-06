/*
 * crc.h
 *
 *  Created on: 2015 j�l. 6
 *      Author: Rich�rd
 */

#ifndef INCLUDE_CRC_H_
#define INCLUDE_CRC_H_

#include "ch.h"
#include "hal.h"
#include "DataLinkLayer.h"

uint8_t CreateCRC(volatile FrameStruct *s);
uint8_t CheckCRC(volatile uint8_t *s);

#endif /* INCLUDE_CRC_H_ */
