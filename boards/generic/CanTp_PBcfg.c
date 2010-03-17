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


#include "CanTp_PBcfg.h"


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


CanTp_TxFcNPduType CanTpTxFcNPduConfig =
{
	.CanTpTxFcNPduRef = 100,
};


const CanTp_RxNSduType 	CanTpRxNSduConfigList[] =  // qqq: Maybe macro is not required.
{
  {
	.CanTpAddressingFormant = CANTP_STANDARD,
	.CanTpRxPduId = 100,
	.CanTpBs = 5,
	.CanTpNar = 100,
	.CanTpNbr = 100,
	.CanTpNcr = 100,
	.CanTpRxDI = 100,
	.CanTpRxPaddingActivation = CANTP_ON,
	.CanTpRxTaType = CANTP_FUNCTIONAL,
	.CanTpWftMax = 100,
	.CanTpSTmin = 100,
	.CanTpNSa = &CanTpNSaConfig,
	.CanTpNTa = &CanTpNTaConfig,
	.CanTpRxNPdu = &CanTpRxNPduConfig,
	.CanTpTxFcNPdu = &CanTpTxFcNPduConfig,
	.CanTpListItemType = CANTP_NOT_LAST_ENTRY,
  },
  {
	.CanTpAddressingFormant = CANTP_EXTENDED,
	.CanTpRxPduId = 101,
	.CanTpBs = 5,
	.CanTpNar = 100,
	.CanTpNbr = 100,
	.CanTpNcr = 100,
	.CanTpRxDI = 100,
	.CanTpRxPaddingActivation = CANTP_ON,
	.CanTpRxTaType = CANTP_FUNCTIONAL,
	.CanTpWftMax = 100,
	.CanTpSTmin = 100,
	.CanTpNSa = &CanTpNSaConfig,
	.CanTpNTa = &CanTpNTaConfig,
	.CanTpRxNPdu = &CanTpRxNPduConfig,
	.CanTpTxFcNPdu = &CanTpTxFcNPduConfig,
	.CanTpListItemType = CANTP_END_OF_LIST,
  },
};


const CanTp_TxNSduType    CanTpTxNSduConfigList[] =
{
  {
	.CanTpAddressingMode = CANTP_STANDARD,
	.CanTpTxPduId = 100,
	.CanTpNas = 100,
	.CanTpNbs = 100,
	.CanTpNcs = 100,
	.CanTpTxDI = 100,
	.CanTpTxPaddingActivation = CANTP_ON,
	.CanTpTxTaType = CANTP_FUNCTIONAL,
	.CanTpNSa = &CanTpNSaConfig,
	.CanTpNTa = &CanTpNTaConfig,
	.CanTpTxNPdu = &CanTpRxNPduConfig,
	.CanTpRxFcNPdu = &CanTpTxFcNPduConfig,
	.CanTpListItemType = CANTP_NOT_LAST_ENTRY
  },
  {
	.CanTpAddressingMode = CANTP_EXTENDED,
	.CanTpTxPduId = 101,
	.CanTpNas = 100,
	.CanTpNbs = 100,
	.CanTpNcs = 100,
	.CanTpTxDI = 100,
	.CanTpTxPaddingActivation = CANTP_ON,
	.CanTpTxTaType = CANTP_FUNCTIONAL,
	.CanTpNSa = &CanTpNSaConfig,
	.CanTpNTa = &CanTpNTaConfig,
	.CanTpTxNPdu = &CanTpRxNPduConfig,
	.CanTpRxFcNPdu = &CanTpTxFcNPduConfig,
	.CanTpListItemType = CANTP_END_OF_LIST
  },

};

CanTp_ConfigType CanTpConfig =
{
  .CanTpGeneral 	= 	&CanTpGeneralConfig,
  .CanTpRxNSduList 	= 	CanTpRxNSduConfigList,
  .CanTpTxNSduList 	= 	CanTpTxNSduConfigList,
};



