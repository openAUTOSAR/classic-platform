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








#ifndef FEE_H_
#define FEE_H_

#include "Fee_Cfg.h"


#define FEE_MODULE_ID			MODULE_ID_FEE
#define FEE_VENDOR_ID			1

#define FEE_SW_MAJOR_VERSION	1
#define FEE_SW_MINOR_VERSION	0
#define FEE_SW_PATCH_VERSION	0
#define FEE_AR_MAJOR_VERSION	3
#define FEE_AR_MINOR_VERSION	0
#define FEE_AR_PATCH_VERSION	1

#if ( FEE_VERSION_INFO_API == STD_ON )
#define Fee_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi, FEE)	/** @req FEE093 */
#endif /* FEE_VERSION_INFO_API */

void Fee_MainFunction(void);	/** @req FEE097 */

void Fee_Init(void);	/** @req FEE085 */
void Fee_SetMode(MemIf_ModeType Mode);	/** @req FEE086 */
Std_ReturnType Fee_Read(uint16 BlockNumber, uint16 BlockOffset, uint8* DataBufferPtr, uint16 Length); /** @req FEE087 */
Std_ReturnType Fee_Write(uint16 BlockNumber, uint8* DataBufferPtr); /** @req FEE088 */
void Fee_Cancel(void);	/** @req FEE089 */
MemIf_StatusType Fee_GetStatus(void);	/** @req FEE090 */
MemIf_JobResultType Fee_GetJobResult(void);	/** @req FEE091 */
Std_ReturnType Fee_InvalidateBlock(uint16 BlockNumber);	/** @req FEE092 */
Std_ReturnType Fee_EraseImmediateBlock(uint16 BlockNumber);	/** @req FEE094 */



#endif /*FEE_H_*/
