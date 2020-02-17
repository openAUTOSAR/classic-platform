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

#ifndef OSEKNM_OSEKINDIRECTNM_H_
#define OSEKNM_OSEKINDIRECTNM_H_
#include "Std_Types.h"


/* OsekNm Indirect Internal types
 * Encoding of the network status as per Table 3 and page 17 */
/* @req OSEKNM052 */
#define OSEKNM_NO_BUS_ERROR         0u
#define ERROR_BUSBLOCKED            1u
#define ERROR_BUS_COMM_NOT_POSSIBLE 2u
#define RESERVED                    3u

#define NM_ON                   0u
#define NM_OFF                  1u

#define NMLIMPHOME_OFF          0u
#define NMLIMPHOME_ON           1u

#define NMBUSSLEEP_OFF          0u
#define NMBUSSLEEP_ON           1u

#define NMWAITBUSSLEEP_OFF      0u
#define NMWAITBUSSLEEP_ON       1u


/* Internal node status of indirect NM */
typedef struct
{
    volatile TickType ToBTimeLeft;
    volatile TickType TwbsTimeLeft;  /* time out counter for wait bus sleep  */
    volatile TickType TErrorTimeLeft; /* time out counter for bus error */
    NetworkStatusType networkStatus;
    OsekNmIndirectNmStateType nmState;
    OsekNm_PduType nmTxPdu;
    /* Node monitor counter for slaves if configured as master and monitoring node index is 0
     * Considering monitored node index is always 0 */
    uint8 indirectSlaveNodeCounter[OSEKNM_INDIRECT_NODES+1];
    uint8 IndirectMasterNodeCount;
    OsekNm_CmaskParamsType NmInDirectMask;
    OsekNm_ConfigParamsType indirectConfig;
    uint8 indirectPrivateConfig[OSEKNM_NODE_MAX];  /* internal private config */
    boolean TxOk;
    boolean RxOk;
}OsekIndNm_InternalNetType;


/******* Function Declerations ***********/


/* ********* Services Support for Config ************* */


/* StartNM starts the local Network Management. This causes the state transition from NMOff to NMOn*/
StatusType OsekInDirectNm_StartNM(NetIdType netId);

/* StopNM stops the local Network Management. This causes the state transition from NMOn
 * to NMShutDown and after processing of the shutdown activities to NMOff*/
/** @req OSEKNM004 */
StatusType OsekInDirectNm_StopNM(NetIdType netId);

/* GotoMode serves to set the NM operating mode specified by <NewMode> */
StatusType OsekInDirectNm_GotoMode(NetIdType netHandle,NMModeName newMode);

StatusType OsekInDirectNm_GetStatus(NetIdType netHandle,StatusRefType networkStatus);/*lint !e9018 'NetworkStatus' with union based type 'StatusRefType */

void OsekInDirectNm_FatalBusError_Handle(NetIdType netId);

void OsekInDirectNm_comMsg_TimeoutHandle(NetIdType netid,NodeIdType nodeid);

void OsekInDirectNm_Processing(NetIdType netId);

void OsekInDirectNm_Com_MsgNotification(NetIdType netId, NodeIdType nodeId);

StatusType OsekInDirectNm_CmpConfig(NetIdType netHandleId,ConfigRefType testConfig,ConfigRefType RefConfig,ConfigRefType cMask);

StatusType OsekInDirectNm_GetConfig(NetIdType netHandleId,ConfigRefType Config,ConfigKindName configKind);
#if (ISO_OSEK_NM == STD_ON)
void OsekInDirectNm_WakeupIndication(NetIdType netId);
#endif
void OsekInDirectNm_InitCMaskTable(NetIdType netHandleId,ConfigKindName configKind, ConfigRefType cMask);
void OsekInDirectNm_InitTargetConfigTable(NetIdType netHandleId,ConfigKindName configKind,ConfigRefType targetConfig);

#endif /* OSEKNM_OSEKINDIRECTNM_H_ */
