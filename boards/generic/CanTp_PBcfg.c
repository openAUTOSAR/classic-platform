/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


#include "CanTp_Types.h"


CanTp_GeneralType 	CanTpGeneralConfig =
{
  .main_function_period = 1000,
};


CanTp_NSaType CanTpNSaConfig =
{
  .CanTpNSa = 100,
};


CanTp_NTaType CanTpNTaConfig =
{
  .CanTpNTa = 100,
};


CanTp_RxNPduType CanTpRxNPduConfig =
{
	.CanTpRxNPduId = 100, /** req: CanTp258: */
	.CanTpRxNPduRef = 100, /** req: CanTp257: */
};

CanTp_TxNPduType CanTpTxNPduConfig =
{
	.CanTpTxNPduId = 100, /** req: CanTp258: */
	.CanTpTxNPduRef = 100, /** req: CanTp257: */
};


CanTp_TxFcNPduType CanTpTxFcNPduConfig =
{
	.CanTpTxFcNPduRef = 100,
};

CanTp_RxFcNPduType CanTpRxFcNPduConfig =
{
	.CanTpRxFcNPduRef = 100,
};

CanTp_NSduType CanTpNSduConfigList[] =
{
	{
		.direction = IS015765_TRANSMIT,
		.configData.CanTpTxNSdu.CanIf_CanTxPduId = 0, // The polite CanIf index.
		.configData.CanTpTxNSdu.PduR_CanTpTxPduId = 0, // The polite PduR index.
		.configData.CanTpTxNSdu.CanTpTxChannel = 5, // Runtime array index.
		.configData.CanTpTxNSdu.CanTpAddressingMode = CANTP_STANDARD,
		//.configData.CanTpTxNSdu.CanTpTxPduId = 200,
		.configData.CanTpTxNSdu.CanTpNas = 100,
		.configData.CanTpTxNSdu.CanTpNbs = 100,
		.configData.CanTpTxNSdu.CanTpNcs = 100,
		.configData.CanTpTxNSdu.CanTpTxDI = 100,
		.configData.CanTpTxNSdu.CanTpTxPaddingActivation = CANTP_ON,
		.configData.CanTpTxNSdu.CanTpTxTaType = CANTP_FUNCTIONAL,
		.configData.CanTpTxNSdu.CanTpNSa = &CanTpNSaConfig,
		.configData.CanTpTxNSdu.CanTpNTa = &CanTpNTaConfig,
		//.configData.CanTpTxNSdu.CanTpTxNPdu = &CanTpTxNPduConfig,
		//.configData.CanTpTxNSdu.CanTpRxFcNPdu = &CanTpRxFcNPduConfig,
		.listItemType = CANTP_NOT_LAST_ENTRY,
	},
	{
		.direction = IS015765_TRANSMIT,
		.configData.CanTpTxNSdu.CanIf_CanTxPduId = 1, // The polite CanIf index.
		.configData.CanTpTxNSdu.PduR_CanTpTxPduId = 1, // The polite PduR index.
		.configData.CanTpTxNSdu.CanTpTxChannel = 4, // Runtime array index.
		.configData.CanTpTxNSdu.CanTpAddressingMode = CANTP_STANDARD,
		//.configData.CanTpTxNSdu.CanTpTxPduId = 201,
		.configData.CanTpTxNSdu.CanTpNas = 100,
		.configData.CanTpTxNSdu.CanTpNbs = 100,
		.configData.CanTpTxNSdu.CanTpNcs = 100,
		.configData.CanTpTxNSdu.CanTpTxDI = 100,
		.configData.CanTpTxNSdu.CanTpTxPaddingActivation = CANTP_ON,
		.configData.CanTpTxNSdu.CanTpTxTaType = CANTP_FUNCTIONAL,
		.configData.CanTpTxNSdu.CanTpNSa = &CanTpNSaConfig,
		.configData.CanTpTxNSdu.CanTpNTa = &CanTpNTaConfig,
		//.configData.CanTpTxNSdu.CanTpTxNPdu = &CanTpTxNPduConfig,
		//.configData.CanTpTxNSdu.CanTpRxFcNPdu = &CanTpRxFcNPduConfig,
		.listItemType = CANTP_NOT_LAST_ENTRY,
	},
	{
		.direction = IS015765_TRANSMIT,
		.configData.CanTpTxNSdu.CanIf_CanTxPduId = 2, // The polite CanIf index.
		.configData.CanTpTxNSdu.PduR_CanTpTxPduId = 2, // The polite PduR index.
		.configData.CanTpTxNSdu.CanTpTxChannel = 3, // Runtime array index.
		.configData.CanTpTxNSdu.CanTpAddressingMode = CANTP_STANDARD,
		//.configData.CanTpTxNSdu.CanTpTxPduId = 202,
		.configData.CanTpTxNSdu.CanTpNas = 100,
		.configData.CanTpTxNSdu.CanTpNbs = 100,
		.configData.CanTpTxNSdu.CanTpNcs = 100,
		.configData.CanTpTxNSdu.CanTpTxDI = 100,
		.configData.CanTpTxNSdu.CanTpTxPaddingActivation = CANTP_ON,
		.configData.CanTpTxNSdu.CanTpTxTaType = CANTP_FUNCTIONAL,
		.configData.CanTpTxNSdu.CanTpNSa = &CanTpNSaConfig,
		.configData.CanTpTxNSdu.CanTpNTa = &CanTpNTaConfig,
		//.configData.CanTpTxNSdu.CanTpTxNPdu = &CanTpTxNPduConfig,
		//.configData.CanTpTxNSdu.CanTpRxFcNPdu = &CanTpRxFcNPduConfig,
		.listItemType = CANTP_NOT_LAST_ENTRY,
	},
	{
		.direction = ISO15765_RECEIVE,
		.configData.CanTpRxNSdu.CanIf_CanTxPduId = 3, // The polite CanIf index.
		.configData.CanTpRxNSdu.PduR_CanTpRxPduId = 3, // The polite PduR index.
		.configData.CanTpRxNSdu.CanTpRxChannel = 2, // Runtime array index.
		.configData.CanTpRxNSdu.CanTpAddressingFormant = CANTP_STANDARD,
		//.configData.CanTpRxNSdu.CanTpRxPduId = 100,
		.configData.CanTpRxNSdu.CanTpBs = 5,
		.configData.CanTpRxNSdu.CanTpNar = 100,
		.configData.CanTpRxNSdu.CanTpNbr = 100,
		.configData.CanTpRxNSdu.CanTpNcr = 100,
		.configData.CanTpRxNSdu.CanTpRxDI = 100,
		.configData.CanTpRxNSdu.CanTpRxPaddingActivation = CANTP_ON,
		.configData.CanTpRxNSdu.CanTpRxTaType = CANTP_FUNCTIONAL,
		.configData.CanTpRxNSdu.CanTpWftMax = 100,
		.configData.CanTpRxNSdu.CanTpSTmin = 100,
		.configData.CanTpRxNSdu.CanTpNSa = &CanTpNSaConfig,
		.configData.CanTpRxNSdu.CanTpNTa = &CanTpNTaConfig,
		//.configData.CanTpRxNSdu.CanTpRxNPdu = &CanTpRxNPduConfig,
		//.configData.CanTpRxNSdu.CanTpTxFcNPdu = &CanTpTxFcNPduConfig,
		.listItemType = CANTP_NOT_LAST_ENTRY,
	},
	{
		.direction = ISO15765_RECEIVE,
		.configData.CanTpRxNSdu.CanIf_CanTxPduId = 4, // The polite CanIf index.
		.configData.CanTpRxNSdu.PduR_CanTpRxPduId = 4, // The polite PduR index.
		.configData.CanTpRxNSdu.CanTpRxChannel = 1, // Runtime array index.
		.configData.CanTpRxNSdu.CanTpAddressingFormant = CANTP_STANDARD,
		//.configData.CanTpRxNSdu.CanTpRxPduId = 101,
		.configData.CanTpRxNSdu.CanTpBs = 5,
		.configData.CanTpRxNSdu.CanTpNar = 100,
		.configData.CanTpRxNSdu.CanTpNbr = 100,
		.configData.CanTpRxNSdu.CanTpNcr = 100,
		.configData.CanTpRxNSdu.CanTpRxDI = 100,
		.configData.CanTpRxNSdu.CanTpRxPaddingActivation = CANTP_ON,
		.configData.CanTpRxNSdu.CanTpRxTaType = CANTP_FUNCTIONAL,
		.configData.CanTpRxNSdu.CanTpWftMax = 100,
		.configData.CanTpRxNSdu.CanTpSTmin = 100,
		.configData.CanTpRxNSdu.CanTpNSa = &CanTpNSaConfig,
		.configData.CanTpRxNSdu.CanTpNTa = &CanTpNTaConfig,
		//.configData.CanTpRxNSdu.CanTpRxNPdu = &CanTpRxNPduConfig,
		//.configData.CanTpRxNSdu.CanTpTxFcNPdu = &CanTpTxFcNPduConfig,
		.listItemType = CANTP_NOT_LAST_ENTRY,
	},
	{
		.direction = ISO15765_RECEIVE,
		.configData.CanTpRxNSdu.CanIf_CanTxPduId = 5, // The polite CanIf index.
		.configData.CanTpRxNSdu.PduR_CanTpRxPduId = 5, // The polite PduR index.
		.configData.CanTpRxNSdu.CanTpRxChannel = 0, // Runtime array index.
		.configData.CanTpRxNSdu.CanTpAddressingFormant = CANTP_STANDARD,
		//.configData.CanTpRxNSdu.CanTpRxPduId = 102,
		.configData.CanTpRxNSdu.CanTpBs = 5,
		.configData.CanTpRxNSdu.CanTpNar = 100,
		.configData.CanTpRxNSdu.CanTpNbr = 100,
		.configData.CanTpRxNSdu.CanTpNcr = 100,
		.configData.CanTpRxNSdu.CanTpRxDI = 100,
		.configData.CanTpRxNSdu.CanTpRxPaddingActivation = CANTP_ON,
		.configData.CanTpRxNSdu.CanTpRxTaType = CANTP_FUNCTIONAL,
		.configData.CanTpRxNSdu.CanTpWftMax = 100,
		.configData.CanTpRxNSdu.CanTpSTmin = 100,
		.configData.CanTpRxNSdu.CanTpNSa = &CanTpNSaConfig,
		.configData.CanTpRxNSdu.CanTpNTa = &CanTpNTaConfig,
		//.configData.CanTpRxNSdu.CanTpRxNPdu = &CanTpRxNPduConfig,
		//.configData.CanTpRxNSdu.CanTpTxFcNPdu = &CanTpTxFcNPduConfig,
		.listItemType = CANTP_END_OF_LIST,
	},

};

CanTp_ConfigType CanTpConfig =
{
  .CanTpNSduList 	= 	CanTpNSduConfigList,
  .CanTpGeneral 	= 	&CanTpGeneralConfig,
};



