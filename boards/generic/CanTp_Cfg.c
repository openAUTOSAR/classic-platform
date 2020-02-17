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


#warning "This default file may only be used as an example!"

#include "CanTp_Types.h"

CanTp_GeneralType CanTpGeneralConfig =
{
  .main_function_period = 20,
};


//NSa
CanTp_NSaType CanTpNSaConfig_UdsPhys_RxNSdu = 
{	
    .CanTpNSa = 0,
};
CanTp_NSaType CanTpNSaConfig_UdsFunc_RxNSdu = 
{	
    .CanTpNSa = 0,
};
CanTp_NSaType CanTpNSaConfig_UdsPhys_TxNSdu = 
{	
    .CanTpNSa = 0,
};
CanTp_NSaType CanTpNSaConfig_UdsFunc_TxNSdu = 
{	
    .CanTpNSa = 0,
};		

//NTa
CanTp_NTaType CanTpNTaConfig_UdsPhys_RxNSdu = 
{
  .CanTpNTa = 0,
};
CanTp_NTaType CanTpNTaConfig_UdsFunc_RxNSdu = 
{
  .CanTpNTa = 0,
};
CanTp_NTaType CanTpNTaConfig_UdsPhys_TxNSdu = 
{
  .CanTpNTa = 0,
};
CanTp_NTaType CanTpNTaConfig_UdsFunc_TxNSdu = 
{
  .CanTpNTa = 0,
};

CanTp_NSduType CanTpNSduConfigList[] =
{
    {
        .direction = IS015765_TRANSMIT,
        .configData.CanTpTxNSdu.CanIf_PduId = 1, //CANIF_PDU_ID_UDS_PHYS_TX,
        .configData.CanTpTxNSdu.PduR_PduId = 0, //PDUR_PDU_ID_UDS_PHYS_TX,
        .configData.CanTpTxNSdu.CanTpTxChannel = 2,
        .configData.CanTpTxNSdu.CanTpAddressingMode = CANTP_STANDARD,
        .configData.CanTpTxNSdu.CanTpNas = 2,
        .configData.CanTpTxNSdu.CanTpNbs = 2,
        .configData.CanTpTxNSdu.CanTpNcs = 2,
        .configData.CanTpTxNSdu.CanTpTxDI = 6,
        .configData.CanTpTxNSdu.CanTpTxPaddingActivation = CANTP_OFF,
        .configData.CanTpTxNSdu.CanTpTxTaType = CANTP_FUNCTIONAL,
        .configData.CanTpTxNSdu.CanTpNSa = &CanTpNSaConfig_UdsPhys_TxNSdu,
        .configData.CanTpTxNSdu.CanTpNTa = &CanTpNTaConfig_UdsPhys_TxNSdu,
        .listItemType = CANTP_NOT_LAST_ENTRY,
    },
    {
        .direction = IS015765_TRANSMIT,
        .configData.CanTpTxNSdu.CanIf_PduId = 0, //CANIF_PDU_ID_UDS_FUNC_TX,
        .configData.CanTpTxNSdu.PduR_PduId = 1, //PDUR_PDU_ID_UDS_FUNC_TX,
        .configData.CanTpTxNSdu.CanTpTxChannel = 3,
        .configData.CanTpTxNSdu.CanTpAddressingMode = CANTP_STANDARD,
        .configData.CanTpTxNSdu.CanTpNas = 2,
        .configData.CanTpTxNSdu.CanTpNbs = 2,
        .configData.CanTpTxNSdu.CanTpNcs = 2,
        .configData.CanTpTxNSdu.CanTpTxDI = 6,
        .configData.CanTpTxNSdu.CanTpTxPaddingActivation = CANTP_OFF,
        .configData.CanTpTxNSdu.CanTpTxTaType = CANTP_FUNCTIONAL,
        .configData.CanTpTxNSdu.CanTpNSa = &CanTpNSaConfig_UdsFunc_TxNSdu,
        .configData.CanTpTxNSdu.CanTpNTa = &CanTpNTaConfig_UdsFunc_TxNSdu,
        .listItemType = CANTP_END_OF_LIST,
    },
    {
        .direction = ISO15765_RECEIVE,
        .configData.CanTpRxNSdu.CanIf_FcPduId = 0, //CANIF_PDU_ID_UDS_PHYS_RX,
        .configData.CanTpRxNSdu.PduR_PduId = 0, //PDUR_PDU_ID_UDS_PHYS_RX,
        .configData.CanTpRxNSdu.CanTpRxChannel = 0,	
        .configData.CanTpRxNSdu.CanTpAddressingFormant = CANTP_STANDARD,
        .configData.CanTpRxNSdu.CanTpBs = 30,
        .configData.CanTpRxNSdu.CanTpNar = 5000,
        .configData.CanTpRxNSdu.CanTpNbr = 1000,
        .configData.CanTpRxNSdu.CanTpNcr = 1000,
        .configData.CanTpRxNSdu.CanTpRxDI = 6,
        .configData.CanTpRxNSdu.CanTpRxPaddingActivation = CANTP_OFF,
        .configData.CanTpRxNSdu.CanTpRxTaType = CANTP_FUNCTIONAL,
        .configData.CanTpRxNSdu.CanTpWftMax = 5,
        .configData.CanTpRxNSdu.CanTpSTmin = 0,
        .configData.CanTpRxNSdu.CanTpNSa = &CanTpNSaConfig_UdsPhys_RxNSdu,
        .configData.CanTpRxNSdu.CanTpNTa = &CanTpNTaConfig_UdsPhys_RxNSdu,
        .listItemType = CANTP_NOT_LAST_ENTRY,	
    },
    {
        .direction = ISO15765_RECEIVE,
        .configData.CanTpRxNSdu.CanIf_FcPduId = 0, //CANIF_PDU_ID_UDS_FUNC_RX,
        .configData.CanTpRxNSdu.PduR_PduId = 1, //PDUR_PDU_ID_UDS_FUNC_RX,
        .configData.CanTpRxNSdu.CanTpRxChannel = 1,	
        .configData.CanTpRxNSdu.CanTpAddressingFormant = CANTP_STANDARD,
        .configData.CanTpRxNSdu.CanTpBs = 30,
        .configData.CanTpRxNSdu.CanTpNar = 5000,
        .configData.CanTpRxNSdu.CanTpNbr = 1000,
        .configData.CanTpRxNSdu.CanTpNcr = 1000,
        .configData.CanTpRxNSdu.CanTpRxDI = 6,
        .configData.CanTpRxNSdu.CanTpRxPaddingActivation = CANTP_OFF,
        .configData.CanTpRxNSdu.CanTpRxTaType = CANTP_FUNCTIONAL,
        .configData.CanTpRxNSdu.CanTpWftMax = 0,
        .configData.CanTpRxNSdu.CanTpSTmin = 0,
        .configData.CanTpRxNSdu.CanTpNSa = &CanTpNSaConfig_UdsFunc_RxNSdu,
        .configData.CanTpRxNSdu.CanTpNTa = &CanTpNTaConfig_UdsFunc_RxNSdu,
        .listItemType = CANTP_NOT_LAST_ENTRY,	
    },
};

CanTp_ConfigType CanTpConfig =
{
  .CanTpNSduList 	= 	CanTpNSduConfigList,
  .CanTpGeneral 	= 	&CanTpGeneralConfig,
};
    
