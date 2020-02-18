/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

/*
 * DESCRIPTION
 *   Uses the linFlex module that contains a UART.
 *   The HW features 4 RX and 4 TX buffers
 *
 *   UARTCR[WL1,WL0] is not clear but should be for "normal" 8-bit transfers
 *     WL0 - 0 - 7 bits
 *           1 - 8 bits
 *     WL1 - 0 - since we don't really care about the 15-16 bits
 *
 */

/* ----------------------------[includes]------------------------------------*/
#include <stdint.h>
#include "device_serial.h"
#include "sys/queue.h"
#include "MemMap.h"
#include "mpc55xx.h"
#include "Mcu.h"

/* ----------------------------[private define]------------------------------*/

#define UARTCR_UART 			0x00000001
#define UARTCR_TDFL(_size)		((_size)<<(31-18))
#define UARTCR_RDFL(_size)		((_size)<<(31-21))
#define UARTCR_RXEN				(1<<(31-26))
#define UARTCR_TXEN				(1<<(31-27))
#define UARTCR_WL_8BIT			(1<<(31-30))

#define SCI_BAUD                57600   // 4800 || 9600 || 19200 || 57600 || 115200
#define SCI_UNIT                0       // 0    || 1    || 2

#if defined(CFG_MPC5645S)
#define DTFTFF DTF
#define DRFRFE DRF

#if (SCI_UNIT==0)
#define LINFLEX LINFLEX_0
#elif (SCI_UNIT==1)
#define LINFLEX LINFLEX_1
#elif (SCI_UNIT==2)
#define LINFLEX LINFLEX_2
#endif
#else
#if (SCI_UNIT==0)
#define LINFLEX LINFlexD_0
#elif (SCI_UNIT==1)
#define LINFLEX LINFlexD_1
#elif (SCI_UNIT==2)
#define LINFLEX LINFlexD_2
#else
#error Not a valid unit
#endif
#endif

uint8 isOpen = 0;

#define SCI_RX_FIFO     0
#define SCI_RX_BUFFER   1
#define SCI_RX_MODE    SCI_FIFO

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

/* ----------------------------[private functions]---------------------------*/

static int SCI_Write(uint8_t *data, size_t nbytes);
static int SCI_Read(uint8_t *data, size_t nbytes);
static int SCI_Open(const char *path, int oflag, int mode);
/* ----------------------------[public functions]----------------------------*/

static int SCI_Open(const char *path, int oflag, int mode) {
#if (SCI_RX_MODE == SCI_RX_BUFFER)
    uint32 rxFifo = 0ul;
#else
    uint32 rxFifo = 1ul;
#endif

    uint32_t pClk;

    /* Set to init mode (NO, were are not initialized after this ) */
    LINFLEX.LINCR1.R = 1;

    /* Set to UART mode, FIFO=4 for both RX, TX */
    LINFLEX.UARTCR.R = (1UL << 0U); /* Enable UART mode */

    // 0x0233;* Enable UART mode */
    LINFLEX.UARTCR.R = ((rxFifo << (31U - 22U)) | /* 0 - RFBM (RX FIFO mode)*/
                        (0UL << (31U - 23U)) | /* 0 - TFBM (TX FIFO mode)*/
                        (0UL << (31U - 24U)) | /* 0 - WL1 (always 8-bits transfers)*/
                        (0UL << (31U - 25U)) | /* 0 - parity (ignore) */
                        (1UL << (31U - 26U)) | /* 1 - Receiver enabled */
                        (1UL << (31U - 27U)) | /* 1 - Transmitter enabled */
                        (0UL << (31U - 29U)) | /* 0 - Parity disabled */
                        (1UL << (31U - 30U)) | /* 1 - WL0 (word length = 8) */
                        (1UL << (31U - 31U))); /* 1 - UART mode */

    /**
     * Baud = Fper / ( 16 *LFDIV )
     *
     */

    pClk = Mcu_Arc_GetPeripheralClock(PERIPHERAL_CLOCK_LIN_A); // Choose clock based on architecture (irq_mpc*.h)

#if (SCI_BAUD == 57600) || (SCI_BAUD == 9600) || (SCI_BAUD == 19200) || (SCI_BAUD == 4800)
    LINFLEX.LINIBRR.B.DIV_M = pClk / (16 * SCI_BAUD);
    LINFLEX.LINFBRR.B.DIV_F = (16 * (pClk % (16 * SCI_BAUD))) / (16 * SCI_BAUD);
#elif (SCI_BAUD == 115200)
    LINFLEX.LINIBRR.B.DIV_M = pClk / (16 * SCI_BAUD);
    LINFLEX.LINFBRR.B.DIV_F = (16 * (pClk % (16 * SCI_BAUD))) / (16 * SCI_BAUD);
#else
#error BAD baudrate
#endif

    /* Set to normal mode */
    LINFLEX.LINCR1.R = 0;

    isOpen = 1;
    return 0;
}

/**
 * Blocking write to serial port
 */
int SCI_Write(uint8_t *data, size_t nbytes) {

    if (isOpen == 0) {
        SCI_Open(NULL, 0, 0);
    }

    int i = 0;

    while (nbytes--) {
        LINFLEX.BDRL.B.DATA0 = data[i];
        i++;

        /* Once the programmed number of bytes (halfwords) has been transmitted,
         the DTFTFF flag is set in UARTSR. */
        while (LINFLEX.UARTSR.B.DTFTFF == 0) {}

        /* Clear the DTFTFF (must be done in software) by writing 1 */
        LINFLEX.UARTSR.B.DTFTFF = 1;
    }
    return i;
}

static int SCI_Read(uint8_t *data, size_t nbytes) {
    if (isOpen == 0) {
        SCI_Open(NULL, 0, 0);
    }

    int i = 0;

    while (nbytes--) {
        /* DRFRFE is set by hardware and indicates that the number of bytes
         programmed in RDFL have been received.  */
#if (SCI_RX_MODE == SCI_RX_BUFFER)
        while (LINFLEX.UARTSR.B.DRFRFE == 0) {}
#else
        while (LINFLEX.UARTSR.B.DRFRFE == 1) {}
#endif

        data[i] = LINFLEX.BDRM.B.DATA4;
        i++;

        /* Clear the RFE (must be done in software) */
        LINFLEX.UARTSR.B.DRFRFE = 1;
    }
    return i;
}

DeviceSerialType SCI_Device = {
        .device.type = DEVICE_TYPE_CONSOLE,
        .device.name = "serial_sci",
        .name = "serial_sci",
        .read = SCI_Read,
        .write = SCI_Write,
        .open = SCI_Open,
        };

