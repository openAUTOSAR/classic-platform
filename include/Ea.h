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


#ifndef EA_H_
#define EA_H_

#define EA_SW_MAJOR_VERSION     1
#define EA_SW_MINOR_VERSION     0
#define EA_SW_PATCH_VERSION     0
#define EA_AR_MAJOR_VERSION     4   // Implemented after the 4.0.2 specification due to the poor quality of 3.1 specification
#define EA_AR_MINOR_VERSION     0
#define EA_AR_PATCH_VERSION     2

#include "Eep.h"
#include "Ea_Cfg.h"
#include "Modules.h"

#define EA_MODULE_ID            MODULE_ID_EA
#define EA_VENDOR_ID            VENDOR_ID_ARCCORE



/*
 *  API parameter checking
 */
/** @req EA139 */
#define EA_E_NOT_INITIALIZED		0x01
/** @req EA140 */
#define EA_E_INVALID_BLOCK_NO		0x02

/*
 * EA Module Service ID Macro Collection
*/
typedef enum {
	 EA_INIT_ID = 0x00,
	 EA_SETMODE_ID,
	 EA_READ_ID,
	 EA_WRITE_ID,
	 EA_CANCEL_ID,
	 EA_GETSTATUS_ID,
	 EA_GETJOBRESULT_ID,
	 EA_INVALIDATEBLOCK_ID,
	 EA_GETVERSIONINFO_ID,
	 EA_ERASEIMMEDIATEBLOCK_ID,
	 EA_MAIN_ID = 0x12
} Ea_APIServiceIDType;

/*@req <EA061> */
/*@req <EA062> */
/*@req <EA082> */
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
