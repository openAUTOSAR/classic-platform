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

#ifndef CANTRCV_CONFIGTYPES_H_
#define CANTRCV_CONFIGTYPES_H_

#include "EcuM_Types.h"

/*
 * Defines use to identify the chosen device.
 * Parameter: CanTrcvGeneral/ArcCanTrcvDevice
 *
 * Values must be unique.
 */
#define TJA1145 0x1145


/** PnFrame Data Mask and Byte index for WUF */
typedef struct {
    uint8 CanTrcvPnFrameDataMask;
    uint8 CanTrcvPnFrameDataMaskIndex;
}CanTrcv_PnFrameDataMaskConfigType;

/** SpiSequence config type */
typedef struct {
    uint8		CanTrcvSpiSequenceName;
    uint8 		CanTrcvSpiCmdChannel;
    uint8		CanTrcvSpiDataChannel;
}CanTrcv_SpiSequenceConfigType;


/** Config structure for Partial network frame */
typedef struct {
    const CanTrcv_PnFrameDataMaskConfigType*    CanTrcvPnFrameDataMaskConfig; /* Mask for payload data & byte index in PN WUF */
    uint32                              CanTrcvPnFrameCanIdMask; /* Mask for CAN Ids, if multiple WUFs possible */
    uint32                              CanTrcvPnFrameCanId; /* CAN Id of PN WUF */
	uint16              		        CanTrcvBaudRate; /* Baud rate value */
    uint8                               CanTrcvPnFrameDlc; /* DLC of PN WUF */
    uint8                               CanTrcvPnFrameDataMaskSize;
    boolean                             CanTrcvPnEnabled; /* PN Enable status */
	boolean					            CanTrcvPnCanIdIsExtended; /* Whether channel uses extended CAN Id */
	boolean                             CanTrcv_Arc_EOL; /* End of Line Indication */
} CanTrcv_PartialNetworkConfigType;


/** Config structure for a CanTrcv Channel */
typedef struct {
    const CanTrcv_PartialNetworkConfigType*     CanTrcvPartialNetworkConfig; /* Reference to PN struct */
    const CanTrcv_SpiSequenceConfigType*        CanTrcvSpiSequenceConfig; /* Reference to SPI config struct */
	CanTrcv_TrcvModeType     		            CanTrcvInitState; /* Initial mode of CanTrcv */
	EcuM_WakeupSourceType                     	CanTrcvWakeupSourceRef; /* Wake up source reference */
	EcuM_WakeupSourceType                       CanTrcvPorWakeupSourceRef; /* wake up source reference for Power on */
	EcuM_WakeupSourceType                       CanTrcvSyserrWakeupSourceRef; /* wake up source reference for trcv errors */
	uint8                                       CanTrcvChannelId; /* Symbolic name of the channel */
	boolean                                     CanTrcvEnableStatus; /* Channel Enable/Disable */
	boolean                                     CanTrcvPnSupport; /* Hw supports selective wakeup by PN WUF */
	boolean                                     CanTrcvWakeupByBus; /* Hw supports wakeup on bus (wakeup patterns) */
	boolean                                     CanTrcv_Arc_EOL; /* End of Line Indication */
} Can_TrcvChannelType;

typedef struct {
    const Can_TrcvChannelType*          CanTrcvChannel;
} CanTrcv_ConfigType;

#endif /* CANTRCV_CONFIGTYPES_H_ */
