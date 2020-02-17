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
#ifndef ETHTSYN_TYPES_H_
#define ETHTSYN_TYPES_H_

/* @req 4.2.2/SWS_EthTSyn_00001 */
#include "ComStack_Types.h"


/* @req 4.2.2/SWS_EthTSyn_00033 */
typedef enum {
    ETHTSYN_TX_OFF,   /* Transmission Disabled */
    ETHTSYN_TX_ON,    /* Transmission Enabled */
}EthTSyn_TransmissionModeType;

/* @req 4.2.2/SWS_EthTSyn_00034 */
typedef enum {
    ETHTSYN_SYNC,        /* Ethernet time synchronous */
    ETHTSYN_UNSYNC,      /* Ethernet not time synchronous */
    ETHTSYN_UNCERTAIN,   /* Ethernet Sync state uncertain */
    ETHTSYN_NEVERSYNC,   /* No Sync message received between EthTSyn_Init() and current requested state. */
}EthTSyn_SyncStateType;

typedef enum {
    PTP_SLAVE,
    PTP_MASTER,
}EthTSyn_PtpType;

typedef struct {
     uint32  EthTSynGlobalTimeTxFollowUpOffset;      /* TX offset for Follow_Up / Pdelay_Resp_Follow_Up messages */
     uint32  EthTSynGlobalTimeTxPdelayReqPeriod;     /* TX period for Pdelay_Req messages */
     uint32  EthTSynGlobalTimeTxPeriod;              /* TX period */
     uint32  EthTSynTimeHardwareCorrectionThreshold; /* maximum deviation between the local time and the time obtained from SYNC message */
     uint8   EthTSynGlobalTimeEthIfId;               /* reference to the Ethernet interface taken to fetch the global time information */
     boolean EthTSynEthTrcvEnable;                   /* Flag indicating if transceiver is enabled for the related EthIf controller */
}EthTSyn_GlobalTimeType;

typedef struct{
     uint32                       EthTSynGlobalTimeFollowUpTimeout;  /* EthTSyn Global Time FollowUp Timeout */
     uint8                        EthTSynSynchronizedTimeBaseRef;    /* reference to the required synchronized time-base */
    const EthTSyn_GlobalTimeType* EthTSynTimeDomain;                /* Reference to configured time domains */
     EthTSyn_PtpType              EthTSynMasterFlag;                 /* Flag to differentiate Master and Slave */
}EthTSyn_GlobalTimeDomainType;

/* @req 4.2.2/SWS_EthTSyn_00032 */
typedef struct{
    const EthTSyn_GlobalTimeDomainType* EthTSynGlobalTimeDomain;         /* Refernce to global time domain */
    uint8                         EhtTSyn_GlobalTimeDomainCount;   /* number of global time domain configuresd */
}EthTSyn_ConfigType;
#endif /* ETHTSYN_TYPES_H_ */
