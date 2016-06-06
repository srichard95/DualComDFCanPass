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

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for the DualCom board.
 */

/*
 * Board identifier.
 */
#define BOARD_DUALCOM
#define BOARD_NAME              "DualCom"

/*
 * Board frequencies.
 */
#define STM32_LSECLK            0
#define STM32_HSECLK            8000000

/*
 * MCU type, supported types are defined in ./os/hal/platforms/hal_lld.h.
 */
#define STM32F103xB

/*
 * IO pins assignments.
 */

#define GPIOA_UBAT              0
#define GPIOA_IN0               1
#define GPIOA_TXD2              2
#define GPIOA_RXD2              3
#define GPIOA_SPI_CS            4
#define GPIOA_SPI_CLK           5
#define GPIOA_SPI_MISO          6
#define GPIOA_SPI_MOSI          7
#define GPIOA_LOAD              8
#define GPIOA_TXD1              9
#define GPIOA_RXD1             10
#define GPIOA_CAN_RX           11
#define GPIOA_CAN_TX           12
#define GPIOA_SWDIO            13
#define GPIOA_SWCLK            14
#define GPIOA_ONEWIRE          15

#define GPIOB_IN1               0
#define GPIOB_IN2               1
#define GPIOB_BOOT1             2
#define GPIOB_5V_EN             3
#define GPIOB_LED2              4
#define GPIOB_LED1              5
#define GPIOB_I2C_SCL           6
#define GPIOB_I2C_SDA           7

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 *
 * The digits have the following meaning:
 *   0 - Analog input.
 *   1 - Push Pull output 10MHz.
 *   2 - Push Pull output 2MHz.
 *   3 - Push Pull output 50MHz.
 *   4 - Digital input.
 *   5 - Open Drain output 10MHz.
 *   6 - Open Drain output 2MHz.
 *   7 - Open Drain output 50MHz.
 *   8 - Digital input with PullUp or PullDown resistor depending on ODR.
 *   9 - Alternate Push Pull output 10MHz.
 *   A - Alternate Push Pull output 2MHz.
 *   B - Alternate Push Pull output 50MHz.
 *   C - Reserved.
 *   D - Alternate Open Drain output 10MHz.
 *   E - Alternate Open Drain output 2MHz.
 *   F - Alternate Open Drain output 50MHz.
 * Please refer to the STM32 Reference Manual for details.
 */

/*
 * Port A setup.
 * Everything input with pull-up except:
 * PA1  - Push-pull output, initially low state.    (LED4)
 * PA2  - Analog input.                             (Output Voltage)
 * PA3  - Analog input.                             (NTC)
 * PA4  - Push Pull output 50MHz.                   (SPI_CS2)
 * PA5  - Alternate Push Pull output 50MHz.         (SPI_CLK)
 * PA6  - Digital input.                            (SPI_MISO)
 * PA7  - Alternate Push Pull output 50MHz.         (SPI_MOSI)
 * PA8  - Digital input.                            (DIP_6)
 * PA10 - Push Pull output 50MHz                    (RXD1 - CS1)
 * PA11 - Alternate Push Pull output 10MHz.         (CAN_RX)
 * PA12 - Alternate Push Pull output 10MHz.         (CAN_TX)
 */
#define VAL_GPIOACRL            0xB4B34B48      /*  PA7...PA0 */
#define VAL_GPIOACRH            0x888944B4      /* PA15...PA8 */
#define VAL_GPIOAODR            0xFFFFFFFF

/*
 * Port B setup.
 * Everything input with pull-up except:
 * PB0  - Analog input.                             (Input Current)
 * PB1  - Analog input.                             (Output Current)
 * PB3  - Digital input.                            (DIP_5)
 * PB4  - Digital input.                            (DIP_4)
 * PB5  - Digital input.                            (DIP_3)
 * PB6  - Digital input.                            (DIP_2)
 * PB7  - Digital input.                            (DIP_1)
 */
#define VAL_GPIOBCRL            0x44333844      /*  PB7...PB0 */
#define VAL_GPIOBCRH            0x88888888      /* PB15...PB8 */
#define VAL_GPIOBODR            0xFFFFFFFF

/*
 * USB bus activation macro, required by the USB driver.
 */
#define usb_lld_connect_bus(usbp) palClearPad(GPIOC, GPIOC_USB_DISC)

/*
 * USB bus de-activation macro, required by the USB driver.
 */
#define usb_lld_disconnect_bus(usbp) palSetPad(GPIOC, GPIOC_USB_DISC)

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
