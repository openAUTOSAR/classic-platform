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

/* ----------------------------[information]----------------------------------*/
/*
 * Author: mahi
 *
 * Part of Release:
 *   3.0 (R3.0 V002)
 *
 * Description:
 *   Implements the Fls module (flash driver)
 *
 * Support:
 *   General				  Have Support
 *   -------------------------------------------
 *   FLS_AC_LOAD_ON_JOB_START	 Y
 *   FLS_BASE_ADDRESS			 Y, taken from FlashInfo
 *   FLS_CANCEL_API				 N
 *   FLS_COMPARE_API			 Y
 *   FLS_DEV_ERROR_DETECT		 Y
 *   FLS_GET_JOB_RESULT_API		 Y
 *   FLS_GET_STATUS_API			 Y
 *   FLS_SET_MODE_API            Y
 *   FLS_TOTAL_SIZE				 Y, taken from FlashInfo
 *   FLS_USE_INTERRUPTS			 N, no hardware support
 *   FLS_VERSION_INFO_API		 Y
 *
 *   FlsConfigSet			  Have Support
 *   -------------------------------------------
 *   FLS_AC_ERASE				 N
 *   FLS_AC_WRITE			 	 N
 *   FLS_CALL_CYCLE				 N
 *   FLS_JOB_END_NOTIFICATION	 Y
 *   FLS_JOB_ERROR_NOTIFICATION  Y
 *   FLS_MAX_READ_FAST_MODE      N
 *   FLS_MAX_READ_NORMAL_MODE    N
 *   FLS_MAX_WRITE_FAST_MODE	 N
 *   FLS_MAX_WRITE_NORMAL_MODE	 N
 *   FLS_PROTECTION				 N
 *
 *
 *   Device
 *   - MPC5668  , No support for shadow flash
 *   - MPC5606S , Support for dataflash only
 *
 * Implementation Notes:
 *   Affected files:
 *   - Fls.c
 *   - Fls_Cfg.c/.h
 *   - flash_h7f_c90.c     middle level driver
 *   - flash_ll_h7f_c90.c  low level driver
 *   - flash.h             interface for flash_xxx
 *
 *   Can't really tell if FlsMaxWriteFastMode, FlsMaxWriteNormalMode, etc is only
 *   for SPI flashes or not. Is it?
 *
 * Things left:
 *   - Virtual addresses, FLS209 is not done (code assumes FLS_BASE_ADDRESS is 0)
 *
 */

/* ----------------------------[requirements]--------------------------------*/

/* FlsGeneral, Complete for 3.0 */
/** @req FLS172 */
// TODO: #warning ENABLE THESE AGAIN

/** !req FLS169 */
/** !req FLS285 */
/** !req FLS286 */
/** !req FLS287 */
/** !req FLS288 */
/** !req FLS289 */
/** !req FLS290 */
/** !req FLS291 */
/** !req FLS170 */
/** !req FLS292 */
/** !req FLS293 */

/* FlsConfigSet, Complete for 3.0 */
/** !req FLS174 */
/** !req FLS270 */
/** !req FLS271 */
/** !req FLS272 */
/** @req FLS273 */
/** @req FLS274 */
/** !req FLS275 */
/** !req FLS276 */
/** @req FLS277 */
/** @req FLS278 */
/** !req FLS279  N/A in core */

/* FlsPublishedInformation, Complete for 3.0 */
/** !req FLS294 */
/** !req FLS295 */
/** !req FLS296 */
/** !req FLS297 */
/** !req FLS298 */
/** !req FLS299 */
/** !req FLS300 */
/** !req FLS198 */
/** !req FLS301 */

/* FlsSectorList and FlsSector , Complete for 3.0 */
/** !req FLS201 N/A in core since we use own format */
/** !req FLS202 N/A in core since we use own format */
/** !req FLS280 N/A in core since we use own format */
/** !req FLS281 N/A in core since we use own format */
/** !req FLS282 N/A in core since we use own format */
/** !req FLS283 N/A in core since we use own format */


/* ----------------------------[includes]------------------------------------*/
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "Fls.h"
#include "flash.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "Cpu.h"
#include "mpc55xx.h"
#include "Mcu.h"
#if (FLS_BASE_ADDRESS != 0)
#error Virtual addresses not supported
#endif



/* ----------------------------[private define]------------------------------*/

#define FLASH_NON_CORRECTABLE_ERROR 0x1

/* Enable check:
 * - Check that the destination is actually 0xff
 */
//#define CFG_FLS_EXTRA_CHECKS
/* Add ramlog prints at each read/write/erase */
//#define CFG_FLS_DEBUG

#if defined(CFG_FLS_DEBUG)
#include "Ramlog.h"
#define LOG_HEX1(_str,_arg1) \
	ramlog_str(_str);ramlog_hex(_arg1);ramlog_str("\n")

#define LOG_HEX2(_str,_arg1,_str2,_arg2) \
	ramlog_str(_str);ramlog_hex(_arg1);ramlog_str(_str2);ramlog_hex(_arg2);ramlog_str("\n")

#define LOG_STR(_str) 	ramlog_str(_str)
#else
#define LOG_HEX1(_str,_arg1)
#define LOG_HEX2(_str,_arg1,_str2,_arg2)
#define LOG_STR(_str)
#endif


/* ----------------------------[private macro]-------------------------------*/


#if ( FLS_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE_CONFIG(_x) assert(_x)

#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(MODULE_ID_FLS, _y, _z, _q)

#define FEE_JOB_END_NOTIFICATION() \
  if( Fls_Global.config->FlsJobEndNotification != NULL ) { \
    Fls_Global.config->FlsJobEndNotification(); \
  }

#define FEE_JOB_ERROR_NOTIFICATION() \
  if( Fls_Global.config->FlsJobErrorNotification != NULL ) { \
    Fls_Global.config->FlsJobErrorNotification(); \
  }

#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_FLS,0,_api,_err); \
          return E_NOT_OK; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_FLS,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_FLS,0,_api,_err); \
          return (_rv); \
        }


#else
#define VALIDATE_CONFIG(_x)
#define DET_REPORTERROR(_x,_y,_z,_q)
#define FEE_JOB_END_NOTIFICATION()
#define FEE_JOB_ERROR_NOTIFICATION()
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

/* ----------------------------[private typedef]-----------------------------*/

typedef enum {
	FLS_JOB_NONE, FLS_JOB_COMPARE, FLS_JOB_ERASE, FLS_JOB_READ, FLS_JOB_WRITE,
} Fls_Arc_JobType;

typedef struct {
	uint32_t dest;
//	uint32_t size;
	uint32_t source;
	uint32_t left;

    uint32_t pDest;
    uint32_t pLeft;
    uint32_t chunkSize;
} Fls_ProgInfoType;


typedef struct {
	const Fls_ConfigType * config;
	MemIf_StatusType 	status;
	MemIf_JobResultType jobResultType;
	Fls_Arc_JobType 	jobType;
	Fls_AddressType 	flashAddr; //sourceAddr
	uint8 *  			ramAddr;    // targetAddr
	Fls_LengthType 		length;
	Fls_ProgInfoType 	flashWriteInfo;
	bool				mustCheck;
	MemIf_ModeType		mode;
	uint32_t 			readChunkSize;
} Fls_GlobalType;

Fls_GlobalType Fls_Global = {
	.status = MEMIF_UNINIT,
	.jobResultType = MEMIF_JOB_OK,
	.jobType = FLS_JOB_NONE,
	.mustCheck = 0,
	.mode = MEMIF_MODE_SLOW
};



/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
static Std_VersionInfoType _Fls_VersionInfo = {
		.vendorID = (uint16) 1,
		.moduleID = (uint16) MODULE_ID_FLS,
		.instanceID = (uint8) 1,
		/* Vendor numbers */
		.sw_major_version = (uint8) FLS_SW_MAJOR_VERSION,
		.sw_minor_version = (uint8) FLS_SW_MINOR_VERSION,
		.sw_patch_version = (uint8) FLS_SW_PATCH_VERSION,
		.ar_major_version = (uint8) FLS_AR_MAJOR_VERSION,
		.ar_minor_version = (uint8) FLS_AR_MINOR_VERSION,
		.ar_patch_version = (uint8) FLS_AR_PATCH_VERSION, };

/* ----------------------------[private functions]---------------------------*/

static Std_ReturnType fls_SectorAligned( Fls_AddressType SourceAddress ) {
	Std_ReturnType rv = E_NOT_OK;
    const FlashType *bPtr;

    for (int bank = 0; bank < FLASH_BANK_CNT; bank++) {
        bPtr = &Fls_Global.config->FlsInfo[bank];

        /* In range of bank */
        if( (SourceAddress >= bPtr->sectAddr[0]) &&
            (SourceAddress <= (bPtr->sectAddr[bPtr->sectCnt])) )
        {
            for (int sector = 0; sector < bPtr->sectCnt + 1; sector++)
            {
                if( SourceAddress == bPtr->sectAddr[sector] ) {
                    rv = E_OK;
                    break;
                }
            }
            break;
        }
    }
    return rv;
}

static Std_ReturnType fls_CheckValidAddress( Fls_AddressType SourceAddress ) {
	Std_ReturnType rv = E_NOT_OK;
    const FlashType *bPtr;

    for (int bank = 0; bank < FLASH_BANK_CNT; bank++) {
        bPtr = &Fls_Global.config->FlsInfo[bank];

		for (int sector = 0; sector < bPtr->sectCnt; sector++)
		{
			if( (SourceAddress >= bPtr->sectAddr[sector]) &&
				(SourceAddress < bPtr->sectAddr[sector+1]) )
			{
				rv = E_OK;
				break;
			}
		}
    }
    return rv;
}

/**
 * Get PC.
 * Since you can't read the PC on PPC, do the next best thing.
 * Ensure that the function is not inlined
 */
#if 0
static uint32 fls_GetPc(void) __attribute__ ((noinline));

static uint32 fls_GetPc(void) {
	return get_spr(SPR_LR);
}
#endif

static void fls_EraseFail( void ) {
	Fls_Global.jobResultType = MEMIF_JOB_FAILED;
	Fls_Global.jobType = FLS_JOB_NONE;
	Fls_Global.status = MEMIF_IDLE;
#if defined(USE_DEM)
	Dem_ReportErrorStatus(FLS_E_ERASE_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
	FEE_JOB_ERROR_NOTIFICATION();
}

static void fls_WriteFail( void ) {
	Fls_Global.jobResultType = MEMIF_JOB_FAILED;
	Fls_Global.jobType = FLS_JOB_NONE;
	Fls_Global.status = MEMIF_IDLE;
#if defined(USE_DEM)
	Dem_ReportErrorStatus(FLS_E_WRITE_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
	FEE_JOB_ERROR_NOTIFICATION();

}
static void fls_ReadFail( void ) {
	Fls_Global.jobResultType = MEMIF_BLOCK_INCONSISTENT;
	Fls_Global.jobType = FLS_JOB_NONE;
	Fls_Global.status = MEMIF_IDLE;
#if defined(USE_DEM)
	Dem_ReportErrorStatus(FLS_E_READ_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
	FEE_JOB_ERROR_NOTIFICATION();

}
/* ----------------------------[public functions]----------------------------*/

/**
 *
 * @param ConfigPtr
 */

void Fls_Init(const Fls_ConfigType *ConfigPtr) {
	/** @req FLS249 3.0 */
	/** @req FLS191 3.0 */
	/** @req FLS014 3.0 */
	/** @req FLS086 3.0 */
	/** @req FLS015 3.0 */
	/** !req FLS048 TODO, true?    */
	/** !req FLS271 NO_SUPPORT 3.0 */
	/** !req FLS325 NO_SUPPORT 4.0 */
	/** !req FLS326 NO_SUPPORT 4.0 */

	/** @req FLS268 */
	VALIDATE_NO_RV(Fls_Global.status!=MEMIF_BUSY,FLS_INIT_ID, FLS_E_BUSY );

	VALIDATE_CONFIG(ConfigPtr->FlsMaxReadFastMode != 0 );
	VALIDATE_CONFIG(ConfigPtr->FlsMaxReadNormalMode != 0 );
	VALIDATE_CONFIG(ConfigPtr->FlsMaxWriteFastMode != 0 );
	VALIDATE_CONFIG(ConfigPtr->FlsMaxWriteNormalMode != 0 );

	Fls_Global.status = MEMIF_UNINIT;
	Fls_Global.jobResultType = MEMIF_JOB_PENDING;
	// TODO: FLS_E_PARAM_CONFIG

	/** @req FLS191 */
	Fls_Global.config = ConfigPtr;

	Flash_Init();

	/** @req FLS016 3.0 *//** @req FLS323 4.0 *//** @req FLS324 4.0*/
	Fls_Global.status = MEMIF_IDLE;
	Fls_Global.jobResultType = MEMIF_JOB_OK;
	return;
}

/**
 * Erase flash sectors
 *
 * @param TargetAddress Always from 0 to FLS_TOTAL_SIZE
 * @param Length
 * @return
 */
Std_ReturnType Fls_Erase(Fls_AddressType TargetAddress, Fls_LengthType Length) {
	TargetAddress += FLS_BASE_ADDRESS;

	/** @req FLS250 3.0/4.0 */
	/** @req FLS218 3.0/4.0 */
	/** @req FLS220 3.0/4.0 */
	/** @req FLS327 4.0     */

	/** @req FLS065 */
	VALIDATE_W_RV( Fls_Global.status != MEMIF_UNINIT, FLS_ERASE_ID, FLS_E_UNINIT, E_NOT_OK );
	/** @req FLS023 */
	VALIDATE_W_RV( Fls_Global.status != MEMIF_BUSY, FLS_ERASE_ID, FLS_E_BUSY, E_NOT_OK );
    /** @req FLS020 3.0/4.0 */
	VALIDATE_W_RV( E_OK == fls_SectorAligned( TargetAddress ),
	        FLS_ERASE_ID, FLS_E_PARAM_ADDRESS, E_NOT_OK );
    /** @req FLS021 3.0/4.0 */
    VALIDATE_W_RV( (Length != 0) && (EE_OK == fls_SectorAligned( TargetAddress + Length)),
            FLS_ERASE_ID, FLS_E_PARAM_LENGTH, E_NOT_OK );

	// Check if we trying to erase a partition that we are executing in
#if 0
	pc = fls_GetPc();
#endif


	Fls_Global.status = MEMIF_BUSY;				    /** @req FLS219 3.0 */ /** @req FLS328 4.0 */
	Fls_Global.jobResultType = MEMIF_JOB_PENDING;   /** @req FLS329 4.0 */
	Fls_Global.jobType = FLS_JOB_ERASE;
	Fls_Global.flashAddr = TargetAddress;
	Fls_Global.length = Length;

	LOG_HEX2("Fls_Erase() ",TargetAddress," ", Length);

	/* Unlock */
	Flash_Lock(Fls_Global.config->FlsInfo,FLASH_OP_UNLOCK,TargetAddress, Length );

	/** @req FLS145 */
	Flash_Erase(Fls_Global.config->FlsInfo,TargetAddress, Length, NULL );

	return E_OK;	/** @req FLS330 4.0 */
}


/**
 * Programs flash sectors
 *
 * @param TargetAddress
 * @param SourceAddressPtr
 * @param Length
 * @return
 */

Std_ReturnType Fls_Write(Fls_AddressType TargetAddress,
		const uint8 *SourceAddressPtr, Fls_LengthType Length) {
	TargetAddress += FLS_BASE_ADDRESS;

	/** @req FLS251 3.0 */
	/** @req FLS223 3.0 */
	/** @req FLS225 3.0/4.0 */
	/** @req FLS226 3.0/4.0 */

	/** @req FLS066 3.0/4.0 */
	/** @req FLS030 3.0/4.0 */
	/** @req FLS157 3.0/4.0 */
	/** @req FLS026 3.0/4.0 */
	/** @req FLS027 3.0/4.0 */
	VALIDATE_W_RV(Fls_Global.status != MEMIF_UNINIT,FLS_WRITE_ID, FLS_E_UNINIT,E_NOT_OK );
	VALIDATE_W_RV(Fls_Global.status != MEMIF_BUSY,FLS_WRITE_ID, FLS_E_BUSY,E_NOT_OK );
	VALIDATE_W_RV(SourceAddressPtr != ((void *)0),FLS_WRITE_ID, FLS_E_PARAM_DATA,E_NOT_OK );
	VALIDATE_W_RV( (TargetAddress % FLASH_PAGE_SIZE == 0) && (E_OK == fls_CheckValidAddress(TargetAddress)),
			FLS_WRITE_ID, FLS_E_PARAM_ADDRESS, E_NOT_OK );
	VALIDATE_W_RV( (Length != 0) && (((TargetAddress + Length) % FLASH_PAGE_SIZE) == 0 && (E_OK == fls_CheckValidAddress(TargetAddress + Length))),
			FLS_WRITE_ID, FLS_E_PARAM_LENGTH, E_NOT_OK );

	// Destination is FLS_BASE_ADDRESS + TargetAddress
	/** @req FLS224 3.0 */ /** @req FLS333 4.0 */
	Fls_Global.jobResultType = MEMIF_JOB_PENDING;
	/** @req FLS332 4.0 */
	Fls_Global.status = MEMIF_BUSY;
	Fls_Global.jobType = FLS_JOB_WRITE;

	// Fill in the required fields for programming...
	/** @req FLS331 4.0 */
	Fls_Global.flashWriteInfo.source = (uint32) SourceAddressPtr;
	Fls_Global.flashWriteInfo.dest = TargetAddress;
	Fls_Global.flashWriteInfo.left = Length;

	if( Fls_Global.mode == MEMIF_MODE_FAST ) {
		Fls_Global.flashWriteInfo.chunkSize = Fls_Global.config->FlsMaxWriteFastMode;
	} else {
		Fls_Global.flashWriteInfo.chunkSize = Fls_Global.config->FlsMaxWriteNormalMode;
	}

	// unlock flash for the entire range.
	Flash_Lock(Fls_Global.config->FlsInfo,FLASH_OP_UNLOCK, TargetAddress, Length );

	LOG_HEX2("Fls_Write() ",Fls_Global.flashWriteInfo.dest," ", Fls_Global.flashWriteInfo.left);

#if defined(CFG_FLS_EXTRA_CHECKS)
	for( int i=0;i<Fls_Global.flashWriteInfo.left;i++) {
		if( *((uint8 *)Fls_Global.flashWriteInfo.dest + i) != 0xff ) {
			while(1) {};
		}
	}
#endif

    /* Save to original request */
    Fls_Global.flashWriteInfo.pDest = TargetAddress;
    Fls_Global.flashWriteInfo.pLeft = Length;

    /** @req FLS146 3.0/4.0 */
    Flash_ProgramPageStart(	Fls_Global.config->FlsInfo,
							&Fls_Global.flashWriteInfo.dest,
							&Fls_Global.flashWriteInfo.source,
							&Fls_Global.flashWriteInfo.left,
							NULL);

	return E_OK; /** @req FLS334 4.0 */
}

#if ( FLS_CANCEL_API == STD_ON )
void Fls_Cancel( void )
{
	/* API NOT SUPPORTED */
}
#endif

#if ( FLS_GET_STATUS_API == STD_ON )
MemIf_StatusType Fls_GetStatus( void )
{
	return Fls_Global.status;
}
#endif

#if ( FLS_GET_JOB_RESULT_API == STD_ON )
MemIf_JobResultType Fls_GetJobResult( void )
{
	return Fls_Global.jobResultType;
}
#endif

void Fls_MainFunction(void) {
	/** @req FLS255 */
	/** @req FLS266 */
	/** @req FLS038 */
	/** !req FLS040  No support for Fls_ConfigSetType.FlsMaxXXXX */
	/** !req FLS104 */
	/** !req FLS105 */
	/** !req FLS106 */
	/** !req FLS154 */
	/** !req FLS200 */
	/** !req FLS022 */
	/** !req FLS055 */
	/** !req FLS056 */
	/** !req FLS052 */
	/** !req FLS232 */
	/** !req FLS233 */
	/** !req FLS234 */
	/** !req FLS235 */
	/** !req FLS272 */
	/** !req FLS196 */



	uint32 flashStatus;
	int result;
	uint32 eccErrReg = 0;

	uint32 chunkSize;

	/** @req FLS117 */
	VALIDATE_NO_RV(Fls_Global.status != MEMIF_UNINIT,FLS_MAIN_FUNCTION_ID, FLS_E_UNINIT );

	/** @req FLS039 */
	if ( Fls_Global.jobResultType == MEMIF_JOB_PENDING) {
		switch (Fls_Global.jobType) {
		case FLS_JOB_COMPARE:
		    /** @req FLS243 */

			// NOT implemented. Hardware error = FLS_E_COMPARE_FAILED
			// ( we are reading directly from flash so it makes no sense )

			chunkSize = MIN( Fls_Global.length, Fls_Global.readChunkSize );

		    /** @req FLS244 */
			result = memcmp((void *)Fls_Global.ramAddr,
					        (void *)Fls_Global.flashAddr, chunkSize );

			Fls_Global.ramAddr += chunkSize;
			Fls_Global.flashAddr += chunkSize;
			Fls_Global.length -= chunkSize;

			McuE_GetECCError(&eccErrReg);
			if( eccErrReg & FLASH_NON_CORRECTABLE_ERROR ){
				fls_ReadFail();
			} else {
				if( 0 != Fls_Global.length ) {
					if (result == 0) {
						Fls_Global.jobResultType = MEMIF_JOB_OK;
					} else {
						Fls_Global.jobResultType = MEMIF_BLOCK_INCONSISTENT;
					}
					Fls_Global.status = MEMIF_IDLE;
					Fls_Global.jobType = FLS_JOB_NONE;
				} else {
					/* Do nothing, wait for next loop */
				}
			}

			break;
		case FLS_JOB_ERASE: {

			flashStatus = Flash_CheckStatus(Fls_Global.config->FlsInfo, (uint32_t *)Fls_Global.flashAddr, Fls_Global.length );

			if (flashStatus == EE_OK ) {
				Fls_Global.jobResultType = MEMIF_JOB_OK;
				Fls_Global.jobType = FLS_JOB_NONE;
				Fls_Global.status = MEMIF_IDLE;
				FEE_JOB_END_NOTIFICATION();
			} else if (flashStatus == EE_INFO_HVOP_INPROGRESS) {
				/* Busy, Do nothing */
			} else {
				// Error
				fls_EraseFail();
			}
            break;
		}
		case FLS_JOB_READ:
			/** @req FLS238 */
			/** @req FLS239 */

			// NOT implemented. Hardware error = FLS_E_READ_FAILED
			// ( we are reading directly from flash so it makes no sense )
			// Read ECC-error to clear it
			McuE_GetECCError(&eccErrReg);


			chunkSize = MIN( Fls_Global.length, Fls_Global.readChunkSize );

			memcpy( (void *)Fls_Global.ramAddr, (void *) Fls_Global.flashAddr, chunkSize );

			Fls_Global.ramAddr += chunkSize;
			Fls_Global.flashAddr += chunkSize;
			Fls_Global.length -= chunkSize;

			McuE_GetECCError(&eccErrReg);
			if( eccErrReg & FLASH_NON_CORRECTABLE_ERROR ){
				fls_ReadFail();
			} else {
				if( 0 == Fls_Global.length ) {
					Fls_Global.jobResultType = MEMIF_JOB_OK;
					Fls_Global.status = MEMIF_IDLE;
					Fls_Global.jobType = FLS_JOB_NONE;
					FEE_JOB_END_NOTIFICATION();
					LOG_STR("Fls_RP() OK\n");
				}
			}
			break;

		case FLS_JOB_WRITE:
		{
			/* We are writing in chunks. If we want to write 6 chunks in total but
			 * only 2 at a time:
			 *
			 * Call
			 *  #1   The Fls_Write
			 *  #2   Wait for Flash_CheckStatus(), Flash_ProgramPageStart().. function return
			 *      -> 1 verified write, 1 pending
			 *  #3  Wait for Flash_CheckStatus(), Flash_ProgramPageStart()
			 *      Wait for Flash_CheckStatus(), Flash_ProgramPageStart() .. function return
			 *      -> 3 verified writes, 1 pending
			 *  #4  Wait for Flash_CheckStatus(), Flash_ProgramPageStart()
			 *      Wait for Flash_CheckStatus(), Flash_ProgramPageStart() .. function return
			 *      -> 5 verified writes, 1 pending
			 *  #5  Wait for Flash_CheckStatus(), ...function return
			 *      -> 6 verified writes,
			 */



			int32_t chunkSize = MIN(Fls_Global.flashWriteInfo.chunkSize, Fls_Global.flashWriteInfo.left);

		    do {
				flashStatus = Flash_CheckStatus(
										Fls_Global.config->FlsInfo,
										(uint32_t *) Fls_Global.flashWriteInfo.pDest,
										Fls_Global.flashWriteInfo.pLeft - Fls_Global.flashWriteInfo.left);



				if (flashStatus == EE_OK) {

					LOG_HEX1("Fls_CS() OK ",Fls_Global.flashWriteInfo.pDest);

					if (Fls_Global.flashWriteInfo.left == 0) {
						/* Done! */
						Fls_Global.jobResultType = MEMIF_JOB_OK;
						Fls_Global.status = MEMIF_IDLE;
						Fls_Global.jobType = FLS_JOB_NONE;
						FEE_JOB_END_NOTIFICATION();
						break;
					}

					/* Write more */
					Fls_Global.flashWriteInfo.pDest = Fls_Global.flashWriteInfo.dest;
					Fls_Global.flashWriteInfo.pLeft = Fls_Global.flashWriteInfo.left;

					/* Double word programming */
					LOG_HEX2("Fls_PP() ",Fls_Global.flashWriteInfo.dest," ", Fls_Global.flashWriteInfo.left);

					flashStatus = Flash_ProgramPageStart(
											Fls_Global.config->FlsInfo,
											&Fls_Global.flashWriteInfo.dest,
											&Fls_Global.flashWriteInfo.source,
											&Fls_Global.flashWriteInfo.left, NULL);
					if (flashStatus != EE_OK) {
						fls_WriteFail();
						break;
					}

					chunkSize = chunkSize - (int32_t)(Fls_Global.flashWriteInfo.pLeft - Fls_Global.flashWriteInfo.left);

				} else if (flashStatus == EE_INFO_HVOP_INPROGRESS) {
					/* Wait for it */
				} else {
					fls_WriteFail();
					/* Nothing to do, quit loop */
					break;
				}

			} while (chunkSize > 0 );

			break;
		}
		case FLS_JOB_NONE:
			assert(0);
			break;

		default:
		    break;
		} /* switch */

	}   /* if */
}


/**
 * Read from flash memory
 *
 * @param SourceAddress
 * @param TargetAddressPtr
 * @param Length
 * @return
 */
Std_ReturnType Fls_Read(	Fls_AddressType SourceAddress,
							uint8 *TargetAddressPtr,
							Fls_LengthType Length)
{
	SourceAddress += FLS_BASE_ADDRESS;
	/** @req FLS256 */
	/** @req FLS236 */
	/** !req FLS239 TODO */
	/** !req FLS240 Have no idea what the requirement means*/

	/** @req FLS099 */
	VALIDATE_W_RV(Fls_Global.status != MEMIF_UNINIT,FLS_READ_ID, FLS_E_UNINIT,E_NOT_OK );
	/** @req FLS100 */
	VALIDATE_W_RV( Fls_Global.status != MEMIF_BUSY, FLS_READ_ID, FLS_E_BUSY, E_NOT_OK );
	/** @req FLS158 */
	VALIDATE_W_RV( TargetAddressPtr != NULL , FLS_READ_ID, FLS_E_PARAM_DATA, E_NOT_OK );
	/** @req FLS097  */
	VALIDATE_W_RV( E_OK == fls_CheckValidAddress(SourceAddress), FLS_READ_ID, FLS_E_PARAM_ADDRESS, E_NOT_OK );
	/** @req FLS098  */
	VALIDATE_W_RV( (Length != 0) && (E_OK == fls_CheckValidAddress(SourceAddress + Length)), FLS_READ_ID, FLS_E_PARAM_LENGTH, E_NOT_OK );

	// Always check if status is not busy
	if (Fls_Global.status == MEMIF_BUSY)
		return E_NOT_OK;

	Fls_Global.status = MEMIF_BUSY;
	Fls_Global.jobResultType = MEMIF_JOB_PENDING;
	Fls_Global.jobType = FLS_JOB_READ;

	LOG_HEX2("Fls_Read() ",(uint32)TargetAddressPtr," ", Length);

	/** @req FLS237 */
	Fls_Global.flashAddr = SourceAddress;
	Fls_Global.ramAddr = TargetAddressPtr;
	Fls_Global.length = Length;

	if( Fls_Global.mode == MEMIF_MODE_FAST ) {
		Fls_Global.readChunkSize = Fls_Global.config->FlsMaxReadFastMode;
	} else {
		Fls_Global.readChunkSize = Fls_Global.config->FlsMaxReadNormalMode;
	}

	return E_OK;
}

#if ( FLS_COMPARE_API == STD_ON )
Std_ReturnType Fls_Compare( Fls_AddressType SourceAddress,
							uint8 *TargetAddressPtr,
							Fls_LengthType Length )
{
	SourceAddress += FLS_BASE_ADDRESS;
    /** @req FLS257 */
    /** @req FLS241 */
    /** @req FLS186 */

    /** @req FLS152 */
    VALIDATE_W_RV(Fls_Global.status != MEMIF_UNINIT,FLS_COMPARE_ID, FLS_E_UNINIT,E_NOT_OK );
    /** @req FLS153 */
    VALIDATE_W_RV( Fls_Global.status != MEMIF_BUSY, FLS_COMPARE_ID, FLS_E_BUSY, E_NOT_OK );
    /** @req FLS273 */
    VALIDATE_W_RV( TargetAddressPtr != NULL , FLS_COMPARE_ID, FLS_E_PARAM_DATA, E_NOT_OK );
	/** @req FLS150  */
	VALIDATE_W_RV( E_OK == fls_CheckValidAddress(SourceAddress), FLS_COMPARE_ID, FLS_E_PARAM_ADDRESS, E_NOT_OK );
	/** @req FLS151  */
	VALIDATE_W_RV( (Length != 0) && (E_OK == fls_CheckValidAddress(SourceAddress + Length)),
			FLS_COMPARE_ID, FLS_E_PARAM_LENGTH, E_NOT_OK );


	// Always check if status is not busy
	if (Fls_Global.status == MEMIF_BUSY )
	    return E_NOT_OK;

	Fls_Global.status = MEMIF_BUSY;
	Fls_Global.jobResultType = MEMIF_JOB_PENDING;
	Fls_Global.jobType = FLS_JOB_COMPARE;

	if( Fls_Global.mode == MEMIF_MODE_FAST ) {
		Fls_Global.readChunkSize = Fls_Global.config->FlsMaxReadFastMode;
	} else {
		Fls_Global.readChunkSize = Fls_Global.config->FlsMaxReadNormalMode;
	}

	/* @req FLS242 */
	Fls_Global.flashAddr = SourceAddress;
	Fls_Global.ramAddr = TargetAddressPtr;
	Fls_Global.length = Length;

	return E_OK;
}
#endif

/** @req 3.1.5/FLS258 */
/** @req 3.1.5/FLS187 */
#if ( FLS_SET_MODE_API == STD_ON )
void Fls_SetMode(MemIf_ModeType Mode) {

	VALIDATE_NO_RV( ( Fls_Global.status != MEMIF_UNINIT ), FLS_SET_MODE_ID, FLS_E_UNINIT);
	/** @req 3.1.5/FLS156 */
	VALIDATE_NO_RV( ( Fls_Global.status != MEMIF_BUSY ), FLS_SET_MODE_ID, FLS_E_BUSY);

	/** @req 3.1.5/FLS155 */
	Fls_Global.mode = Mode;
}
#endif

#if ( FLS_VERSION_INFO_API == STD_ON )
void Fls_GetVersionInfo( Std_VersionInfoType *VersioninfoPtr )
{
    /** @req FLS259 */
    /** @req FLS165 */
    /** @req FLS166 */
    /** !req FLS166 Change if moved to macro */

	memcpy(VersioninfoPtr, &_Fls_VersionInfo, sizeof(Std_VersionInfoType));
}
#endif

#if 0
void Fls_Check(uint32 flsBaseAddress, uint32 flsTotalSize) {
	// ECC checking is always on by default.
	// If a non correctable error is discovered
	// we will get an IVOR2 exception.

	// Enable Flash Non_Correctible Reporting,
	// Not really necessary but makes more information
	// available in the MCM registers if an error occurs.
#if defined(CFG_MPC560X) || defined(CFG_MPC5567) || defined(CFG_MPC563XM)
	ECSM.ECR.B.EFNCR = 1;
#elif defined (CFG_MPC5516)
	MCM.ECR.B.EFNCR = 1;
#else
#warning "Non supported processor"
#endif

	// Read flash in 32bit chunks, it's most efficient.
	uint32* memoryChunkPtr = (uint32*) flsBaseAddress;
	uint32* flsTotalSizePtr = (uint32*) flsTotalSize;
	uint32 memoryChunk = *memoryChunkPtr; // The first read

	// Read the rest of the flash, chunk by chunk
	while (memoryChunkPtr < flsTotalSizePtr) {
		memoryChunk = *(memoryChunkPtr++);
	}
}
#endif

