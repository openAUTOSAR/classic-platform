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








#ifndef EA_H_
#define EA_H_

#include "Ea_Cfg.h"


#define EA_MODULE_ID			MODULE_ID_EA
#define EA_VENDOR_ID			1

#define EA_SW_MAJOR_VERSION	1
#define EA_SW_MINOR_VERSION	0
#define EA_SW_PATCH_VERSION	0
#define EA_AR_MAJOR_VERSION	3
#define EA_AR_MINOR_VERSION	0
#define EA_AR_PATCH_VERSION	1

#if ( EA_VERSION_INFO_API == STD_ON )
#define Ea_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi, EA)	/** @req EA092 */
#endif /* EA_VERSION_INFO_API */

void Ea_MainFunction(void);	/** @req EA096 */

void Ea_Init(void);	/** @req EA084 */
void Ea_SetMode(MemIf_ModeType Mode);	/** @req EA085 */
Std_ReturnType Ea_Read(uint16 BlockNumber, uint16 BlockOffset, uint8* DataBufferPtr, uint16 Length); /** @req EA086 */
Std_ReturnType Ea_Write(uint16 BlockNumber, uint8* DataBufferPtr); /** @req EA087 */
void Ea_Cancel(void);	/** @req EA088 */
MemIf_StatusType Ea_GetStatus(void);	/** @req EA089 */
MemIf_JobResultType Ea_GetJobResult(void);	/** @req EA090 */
Std_ReturnType Ea_InvalidateBlock(uint16 BlockNumber);	/** @req EA091 */
Std_ReturnType Ea_EraseImmediateBlock(uint16 BlockNumber);	/** @req EA093 */



#endif /*EA_H_*/
