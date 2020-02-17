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
#include "Spi.h"
#include "CanTrcv.h"

#define CAN_FRAME_MAX_DLC   8u

#define SPI_GET_RESULT_MAX_TRY 5000u /* Maximum SPI Async transmit tries to read Transceiver status registers */

/* Macros for baud rate configuration */
#define CANTRCV_BAUD_RATE_50KBPS 50u
#define CANTRCV_BAUD_RATE_100KBPS 100u
#define CANTRCV_BAUD_RATE_125KBPS 125u
#define CANTRCV_BAUD_RATE_250KBPS 250u
#define CANTRCV_BAUD_RATE_500KBPS 500u
#define CANTRCV_BAUD_RATE_1000KBPS 1000u

struct CanTrcv_Internal_FrameBufType {
    uint8 FrameData[CAN_FRAME_MAX_DLC];
};

typedef struct {
    CanTrcv_TrcvModeType            CanTrcv_TrcvCurMode; /* Channel mode */
#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    CanTrcv_TrcvWakeupReasonType    CanTrcv_TrcvWakeupReason; /* Wake up reason */
    CanTrcv_TrcvWakeupModeType      CanTrcv_CurWakeupMode; /* Wake up mode */
    boolean                         CanTrcv_CurWakeupFlag; /* Wake up flag*/
#endif
} CanTrcv_Internal_ChannelRunTimeType;

typedef struct {

    CanTrcv_Internal_ChannelRunTimeType    CanTrcv_RuntimeData[CANTRCV_CHANNEL_COUNT];
    boolean                         initRun; /* Init status */

}CanTrcv_InternalType;


/* Internal Function definition */
#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
/* Set baud rate */
Std_ReturnType CanTrcv_Hw_SetBaudRate(uint16 BaudRate,const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Enable Partial networking selective wake up */
Std_ReturnType CanTrcv_Hw_EnablePN(const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Disable partial networking selective wake up */
Std_ReturnType CanTrcv_Hw_DisablePN(const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Configure partial networking selective wake up */
Std_ReturnType CanTrcv_Hw_SetupPN(const CanTrcv_PartialNetworkConfigType* partialNwConfig,const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Read status of partial network configuration = success or failure */
Std_ReturnType CanTrcv_Hw_ReadPNConfigurationStatus(boolean * errConfigSts,const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Clear wake up events */
Std_ReturnType CanTrcv_Hw_ClearWakeUpEventStatus(const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Read wake up events and this function implicitly clears the function */
Std_ReturnType CanTrcv_Hw_ReadWakeupEventStatus(CanTrcv_TrcvWakeupReasonType* reason,boolean * wakeupDetected,const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Enable transceiver wake up events */
Std_ReturnType CanTrcv_Hw_EnableWakeUpEvent(const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Disable transceiver wake up events */
Std_ReturnType CanTrcv_Hw_DisableWakeUpEvent(const CanTrcv_SpiSequenceConfigType *spiSeq);
#endif

/* Set transceiver mode */
Std_ReturnType CanTrcv_Hw_SetupMode(CanTrcv_TrcvModeType state,const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Read current transceiver mode */
Std_ReturnType CanTrcv_Hw_ReadCurrMode(CanTrcv_TrcvModeType* mode,const CanTrcv_SpiSequenceConfigType *spiSeq);
/* Read current transceiver bus activity status */
Std_ReturnType CanTrcv_Hw_ReadSilenceFlag(CanTrcv_TrcvFlagStateType *flag,const CanTrcv_SpiSequenceConfigType *spiSeq);


