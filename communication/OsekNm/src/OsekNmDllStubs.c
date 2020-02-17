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

#include "OsekNm_ConfigTypes.h"
#include "OsekDirectNm.h"
#include "OsekNm.h"
#include "CanIf.h"
#include <string.h>
#include "OsekNm_Cfg.h"

extern const OsekNm_ConfigType* OsekNm_ConfigPtr;

/**
 * @brief function is used to init five service ex businit, bussleep etc..
 * @param netId
 * @param InitRoutine
 */
void D_Init(NetIdType netId,RoutineRefType initRoutine)
{
    initRoutine(netId);
}

/**
 *
 * @param netId
 */
void D_Online(NetIdType netId) {
    (void)netId;
}

void D_Offline(NetIdType netId) {
    (void)netId;
}

/**
 * @brief Function needed by DLL to initialize the bus hardware once at the start of the network
 * @param netId
 */
void BusInit(NetIdType netId) {
    (void)netId;
}

/**
 * @brief Function needed by DLL to reinitialize the bus hardware to leave the power down mode
 * @param netId
 */
void BusAwake(NetIdType netId) {
    (void)netId;
}

/**
 * @brief Function needed by DLL to shut down the bus hardware
 * @param netId
 */
void BusShutdown(NetIdType netId) {
    (void)netId;
}
/**
 * @brief Function needed by DLL to initialize the power down mode of the bus hardware
 * @param netId
 */
void BusSleep(NetIdType netId) {
    (void)netId;
}

/**
 * @brief Function needed by DLL to restart the bus hardware in the case of a fatal bus error
 * @param netId
 */
void BusRestart(NetIdType netId) {
    (void)netId;
}

/**
 * @brief Function is transmit message by decoding nmPdu data to the network
 * @param netId
 * @param nmPdu
 * @param dlc
 * @return
 */
StatusType D_Window_Data_req(NetIdType netId, const OsekNm_PduType* nmPdu,uint8 dlc)
{
    StatusType status;
    NetIdType netHandleId;
    PduInfoType pdu;
    uint8 data[8];

    netHandleId = OsekNm_Internal_CheckNetId(netId);
    pdu.SduLength = dlc;
    pdu.SduDataPtr = data;

    data[0] = nmPdu->destination;
    data[1] = nmPdu->OpCode.b;
    memcpy(&data[2], nmPdu->ringData, (dlc-2));
    status = CanIf_Transmit(OsekNm_ConfigPtr->osekNmNetwork[netHandleId].canIfTxLPduRef,&pdu);

    return status;
}
