/*
 * uart.h
 *
 *  Created on: 2016 febr. 9
 *      Author: srich
 */

#ifndef INCLUDE_ESPUART_H_
#define INCLUDE_ESPUART_H_

#define FRAME_SIZE_BYTE 15
#define SYNC_TIMEOUT_THRS 100

void StartUart(void);
void esp_send(BaseSequentialStream *chp, int argc, char *argv[]);
void esp_send_sync(BaseSequentialStream *chp, int argc, char *argv[]);
void start_driver_test(BaseSequentialStream *chp, int argc, char *argv[]);
void GetDllStats(BaseSequentialStream *chp, int argc, char *argv[]);
#endif /* INCLUDE_ESPUART_H_ */
