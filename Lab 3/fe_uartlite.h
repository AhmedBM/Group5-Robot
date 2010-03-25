/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2010 Farkas Engineering                                       *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
******************************************************************************/

#ifndef __FE_UARTLITE_H__
#define __FE_UARTLITE_H__

#include <io.h>

/* Basic address, read and write macros. */

#define IORD_FE_UART_TXDATA(base)           IORD(base, 0)
#define IOWR_FE_UART_TXDATA(base, data)     IOWR(base, 0, data)

#define IORD_FE_UART_RXDATA(base)           IORD(base, 1)
//#define IOWR_FE_UART_RXDATA(base, data)     IOWR(base, 1, data)     // Should not be allowed

#define IORD_FE_UART_BAUDRATE(base)         IORD(base, 2)
#define IOWR_FE_UART_BAUDRATE(base, data)   IOWR(base, 2, data)

#define IORD_FE_UART_STATUS(base)           IORD(base, 6)
//#define IOWR_FE_UART_STATUS(base, data)     IOWR(base, 6, data)     // Should not be allowed

#define IORD_FE_UART_CONTROL(base)          IORD(base, 4)
#define IOWR_FE_UART_CONTROL(base, data)    IOWR(base, 4, data)

/* Masks. */

#define FE_UART_RESET                       (0x2)
#define FE_UART_EN_RX                       (0x4)
#define FE_UART_EN_TX                       (0x8)

#define ASB_STATUS_TX_FULL_MSK              (0x1)
#define ASB_STATUS_RX_EMPTY_MSK             (0x2)
#define ASB_STATUS_SLOTRDY_MSK              (0x4)

/* Baud rates, only valid if the system clock is 50 MHz */
#define BAUD_57600      219
#define BAUD_115200     108
#define BAUD_250000      50
#define BAUD_500000      25
#define BAUD_1250000     10
#define BAUD_2500000      5
#define BAUD_3125000      4
#define BAUD_6250000      2

#endif /* __FE_UARTLITE_H__ */
