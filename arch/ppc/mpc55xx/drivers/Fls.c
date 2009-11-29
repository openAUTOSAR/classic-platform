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








/**
 * @file Fls.c
 * @breif Autosar Flash driver for Freescale MPC55xx.
 */


/*
 * IMPLEMENTATION NOTES
 * - It seems that the Autosar specification is not consistent. For
 *   FLS_AC_LOAD_ON_JOB_START == STD_ON it seems that the driver suddenly
 *   becomes blocking.
 *
 */

/*
 * Use cases:
 * 1. Bootloader, self replace
 * 2. Bootloader, application loading
 *
 * In case 2 it's very straight forward and you can just use all the functions
 * as intended. In case 1. there are some problems understanding how Autosar
 * want to implement it. What is given from spec is.
 * - If FLS_AC_LOAD_ON_JOB_START == STD_ON we copy the flash access routines
 *   to RAM( to FlsAcErase and FlsAcWrite ).
 *
 * Strange things:
 * - What happens to all the other functions that is needed to get the status
 *   for the flash driver. Did the driver just get blocking ??
 *
 */

#include "Fls.h"
/* Freescale driver */
#include "ssd_types.h"
#include "ssd_h7f.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Det.h"
#include "h7f_types.h"
#include "Cpu.h"
#include "mpc55xx.h"
#include "Fls_H7F.h"


/* Flash layout for MPC5516 */
/*
 * Low:  8x16K + 2x64k
 * Mid:  2x128K
 * High: 8x128K
 */

#define H7F_REG_BASE 			0xFFFF8000
#define MAIN_ARRAY_BASE 		0x00000000
#define SHADOW_ROW_BASE  	0x00FF8000
#define SHADOW_ROW_SIZE 		0x00008000
#define FLASH_PAGE_SIZE    H7FB_PAGE_SIZE

#if 0
#define VFLAGS_ADDR_SECT		(1<<0)
#define VFLAGS_ADDR_PAGE		(1<<1)
#define VFLAGS_LEN_SECT			(1<<2)
#define VFLAGS_LEN_PAGE			(1<<3)

static inline int Fls_Validate( uint32 addr,uint32 length, uint32 api,uint32 rv ) {
  int i;
  int addrOk=0;
  uint32 flags_ok;
  const Fls_SectorType* sector;
  Fls_ConfigType *cfg = Fls_Global.config;

  // Pre checks.
	if( flags & VFLAGS_LEN_SECT ) {
		if( (addr + length) > FLS_TOTAL_SIZE ) {
			return (-1);
		}
	}

  for(i=0;i<cfg->FlsSectorListSize;i++) {
  	sector = &cfg->FlsSectorList[sectorIndex];
  	if( addr > (sector->FlsSectorStartaddress + sector->FlsNumberOfSectors * sector->FlsNumberOfSectors) ) {
  		continue;
  	}
  	if( flags & VFLAGS_ADDR_SECT ) {
  		if( (addr % sector->FlsSectorSize) == 0) {
  			flags &= ~VFLAGS_ADDR_SECT;
  		}
  	}
  	if( flags & VFLAGS_ADDR_PAGE ) {
  		if( (addr % sector->FlsPageSize) == 0) {
  			flags &= ~VFLAGS_ADDR_PAGE;
  		}
  	}
  	if( flags & VFLAGS_LEN_SECT ) {
  		// Check
  		if( (0!= length) && (length < sectorPtr->FlsSectorSize) ) {
  			flags &= ~VFLAGS_ADDR_SECT;
  		}
  	}
  	if( flags & VFLAGS_LEN_PAGE ) {
  		if( (0!= length) && (length < sectorPtr->FlsPageSize)) {
  			flags &= ~VFLAGS_ADDR_PAGE;
  		}
  	}
  }
}
#endif

#if ( FLS_DEV_ERROR_DETECT == STD_ON )
#define FLS_VALIDATE_PARAM_ADDRESS_SECTOR_W_RV(_addr, _api, _rv)\
  int sectorIndex;\
  int addrOk=0;\
  const Fls_SectorType* sector;\
  for (sectorIndex=0; sectorIndex<Fls_Global.config->FlsSectorListSize;sectorIndex++) {\
    sector = &Fls_Global.config->FlsSectorList[sectorIndex];\
    if((((uint32)_addr-sector->FlsSectorStartaddress) / sector->FlsSectorSize)<sector->FlsNumberOfSectors){\
      /* Within the right adress space */\
      if (!(((uint32)_addr-sector->FlsSectorStartaddress) % sector->FlsSectorSize)){\
        /* Address is correctly aligned */\
        addrOk=1;\
        break;\
      }\
    }\
  }\
  if (1!=addrOk){\
  Det_ReportError(MODULE_ID_FLS,0,_api,FLS_E_PARAM_ADDRESS ); \
  return _rv; \
  }

#define FLS_VALIDATE_PARAM_ADDRESS_PAGE_W_RV(_addr, _api, _rv)\
  int sectorIndex;\
  int addrOk=0;\
  const Fls_SectorType* sector;\
  for (sectorIndex=0; sectorIndex<Fls_Global.config->FlsSectorListSize;sectorIndex++) {\
    sector = &Fls_Global.config->FlsSectorList[sectorIndex];\
    if((((uint32)_addr-sector->FlsSectorStartaddress) / sector->FlsSectorSize)<sector->FlsNumberOfSectors){\
      /* Within the right adress space */\
      if (!(((uint32)_addr-sector->FlsSectorStartaddress) % sector->FlsPageSize)){\
        /* Address is correctly aligned */\
        addrOk=1;\
        break;\
      }\
    }\
  }\
  if (1!=addrOk){\
  Det_ReportError(MODULE_ID_FLS,0,_api,FLS_E_PARAM_ADDRESS ); \
  return _rv; \
  }

#define FLS_VALIDATE_PARAM_LENGTH_PAGE_W_RV(_addr, _length, _api, _rv)\
  int i;\
  int lengthOk=0;\
  const Fls_SectorType* sectorPtr= &Fls_Global.config->FlsSectorList[0];\
  for (i=0; i<Fls_Global.config->FlsSectorListSize;i++) {\
    if ((sectorPtr->FlsSectorStartaddress + (sectorPtr->FlsNumberOfSectors * sectorPtr->FlsSectorSize))>=(uint32_t)(_addr+(_length))){\
      if ((0!=_length)&&!(_length % sectorPtr->FlsPageSize)){\
        lengthOk=1;\
        break;\
      }\
    }\
    sectorPtr++;\
  }\
  if (!lengthOk){\
    Det_ReportError(MODULE_ID_FLS,0,_api,FLS_E_PARAM_LENGTH ); \
    return _rv; \
  }

#define FLS_VALIDATE_PARAM_LENGTH_SECTOR_W_RV(_addr, _length, _api, _rv)\
  int i;\
  int lengthOk=0;\
  const Fls_SectorType* sectorPtr= &Fls_Global.config->FlsSectorList[0];\
  for (i=0; i<Fls_Global.config->FlsSectorListSize;i++) {\
    if ((sectorPtr->FlsSectorStartaddress + (sectorPtr->FlsNumberOfSectors * sectorPtr->FlsSectorSize))>=(uint32_t)(_addr+(_length))){\
      if ((0!=_length)&& !(_length % sectorPtr->FlsSectorSize)){\
        lengthOk=1;\
        break;\
      }\
    }\
    sectorPtr++;\
  }\
  if (!lengthOk){\
    Det_ReportError(MODULE_ID_FLS,0,_api,FLS_E_PARAM_LENGTH ); \
    return _rv; \
  }

#define FLS_VALIDATE_STATUS_UNINIT_W_RV(_status, _api, _rv)\
  if (MEMIF_UNINIT == _status){\
    Det_ReportError(MODULE_ID_FLS,0,_api,FLS_E_UNINIT); \
    return _rv; \
  }

#define FLS_VALIDATE_STATUS_BUSY(_status, _api)\
  if (MEMIF_BUSY == _status){\
    Det_ReportError(MODULE_ID_FLS,0,_api,FLS_E_BUSY); \
    return; \
  }

#define FLS_VALIDATE_STATUS_BUSY_W_RV(_status, _api, _rv)\
  if (MEMIF_BUSY == _status){\
    Det_ReportError(MODULE_ID_FLS,0,_api,FLS_E_BUSY); \
    return _rv; \
  }

#define FLS_VALIDATE_PARAM_DATA_W_RV(_ptr,_api, _rv) \
  if( (_ptr)==((void *)0)) { \
    Det_ReportError(MODULE_ID_FLS,0,_api,FLS_E_PARAM_DATA); \
    return _rv; \
  }
#else
  #define FLS_VALIDATE_PARAM_ADDRESS_SECTOR_W_RV(_addr, _api, _rv)
  #define FLS_VALIDATE_PARAM_ADDRESS_PAGE_W_RV(_addr, _api, _rv)
  #define FLS_VALIDATE_PARAM_LENGTH_SECTOR_W_RV(_addr, _length, _api, _rv)
  #define FLS_VALIDATE_PARAM_LENGTH_PAGE_W_RV(_addr, _length, _api, _rv)
  #define FLS_VALIDATE_STATUS_UNINIT_W_RV(_status, _api, _rv)
  #define FLS_VALIDATE_STATUS_BUSY(_status, _api)
  #define FLS_VALIDATE_STATUS_BUSY_W_RV(_status, _api, _rv)
  #define FLS_VALIDATE_PARAM_DATA_W_RV(_ptr,_api,_rv)
#endif

#if ( FLS_DEV_ERROR_DETECT == STD_ON )
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(MODULE_ID_FLS, _y, _z, _q)
#else
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#if ( FLS_GET_JOB_RESULT_API == STD_ON)
#define FEE_JOB_END_NOTIFICATION() \
  if( Fls_Global.config->FlsJobEndNotification != NULL ) { \
    Fls_Global.config->FlsJobEndNotification(); \
  }
#define FEE_JOB_ERROR_NOTIFICATION() \
  if( Fls_Global.config->FlsJobErrorNotification != NULL ) { \
    Fls_Global.config->FlsJobErrorNotification(); \
  }
#else
#define FEE_JOB_END_NOTIFICATION()
#define FEE_JOB_ERROR_NOTIFICATION()
#endif



/**
 * Get PC.
 * Since you can't read the PC on PPC, do the next best thing.
 * Ensure that the function is not inlined
 */
static uint32 Fls_GetPc( void ) __attribute__ ((noinline));

static uint32 Fls_GetPc( void )
{
  return get_spr(SPR_LR);
}


typedef struct {
  uint32 addr;
  uint32 size;
} Fls_InternalSectorType;


SSD_CONFIG ssdConfig = {
    H7F_REG_BASE,           /* H7F control register base */
    MAIN_ARRAY_BASE,        /* base of main array */
    0,                      /* size of main array */
    SHADOW_ROW_BASE,        /* base of shadow row */
    SHADOW_ROW_SIZE,        /* size of shadow row */
    0,                      /* block number in low address space */
    0,                      /* block number in middle address space */
    0,                      /* block number in high address space */
    8,        				/* page size */
    FALSE,                   /* debug mode selection */
};

static Std_VersionInfoType _Fls_VersionInfo = {
    .vendorID 			= (uint16)1,
    .moduleID 			= (uint16) MODULE_ID_FLS,
    .instanceID			= (uint8)1,
    /* Vendor numbers */
    .sw_major_version	= (uint8)FLS_SW_MAJOR_VERSION,
    .sw_minor_version	= (uint8)FLS_SW_MINOR_VERSION,
    .sw_patch_version	= (uint8)FLS_SW_PATCH_VERSION,
    .ar_major_version	= (uint8)FLS_AR_MAJOR_VERSION,
    .ar_minor_version	= (uint8)FLS_AR_MINOR_VERSION,
    .ar_patch_version	= (uint8)FLS_AR_PATCH_VERSION,
};

//
typedef enum {
  FLS_JOB_NONE,
  FLS_JOB_COMPARE,
  FLS_JOB_ERASE,
  FLS_JOB_READ,
  FLS_JOB_WRITE,
} Fls_Arc_JobType;

#if 0
typedef struct {
  MemIf_StatusType    status;
  MemIf_JobResultType jobResultType;
  Fls_Arc_JobType	jobType;
  MemIf_AddressType   sourceAddr;
  uint8 *targetAddr;
  MemIf_LengthType length;

  Fls_ProgInfoType flashWriteInfo;

} FlsUnit_t;
#endif


#if 0
static FlsUnit_t privData = {
    .status = MEMIF_UNINIT,
    .jobResultType = MEMIF_JOB_OK,
    .jobType = FLS_JOB_NONE,

};
#endif

// Default Config
#if 0
const Fls_ConfigType * configDataPtr = &FlsConfigSet[0];
#endif

// TODO: Comment and cleanup
typedef struct {
  const Fls_ConfigType * config;
  Fls_EraseBlockType lockBits;

  MemIf_StatusType    status;
  MemIf_JobResultType jobResultType;
  Fls_Arc_JobType	jobType;
  MemIf_AddressType   sourceAddr;
  uint8 *targetAddr;
  MemIf_LengthType length;

  Fls_ProgInfoType flashWriteInfo;
} Fls_GlobalType;

Fls_GlobalType Fls_Global = {
    .status = MEMIF_UNINIT,
    .jobResultType = MEMIF_JOB_OK,
    .jobType = FLS_JOB_NONE,
};


#if 0
static inline uint32 rlwimi(uint32 val, uint16 sh, uint16 mb,uint16 me)
{
  uint32 result;
  asm volatile("rlwimi %0,%1,8,16,23"
          : "=r" (result)
          : "r" (val),"g" (sh), "g" (mb), "g" (me) );
  return result;
}

#define CREATE_MASK(_start,_stop) rlwimi(0xffffffff,0x0,0x0,0x10)
#endif

/**
 * Converts an address to a freescale erase block.
 * Assumes addr is located from FLS_BASE_ADDRESS
 *
 * @param addr address to convert
 * @param rem pointer to reminder that gets filled in by the function
 * @return A block number
 */

// TODO: This have hardcoded limits. Get from config instead
static uint32 address_to_block( uint32 addr, uint32 *rem ) {
  uint32 block;

  if( addr < 0x20000) {
    // Low range, 8x16K
    block = addr / 0x4000;
    *rem   = addr % 0x4000;
  } else if ( addr < 0x40000) {
    // Low range, 2x64k range
    block = 8 + ( addr - 0x20000 ) / 0x10000;
    *rem   = addr % 0x10000;
  } else if( addr < 0x80000 ) {
    // mid range
    block = 10 + ( addr - 0x40000 ) / 0x20000;
    *rem = addr % 0x20000;
  } else if( addr < 0x180000 ) {
    // high range
    block = 12 + ( addr - 0x80000 ) / 0x20000;
    *rem = addr % 0x20000;
  } else {
    block = (-1);
    *rem = (-1);
  }
  return block;
}


/**
 * Converts an address range( addr to addr + size) to freescale bit erase
 * blocks. The function adds the erase block information to eraseBlocks ptr.
 *
 * @param eraseBlocks Ptr to an erase structure
 * @param addr The start-address to convert
 * @param size The size of the block
 * @return The test results
 */
static void address_to_erase_blocks( Fls_EraseBlockType *eraseBlocks, uint32 addr, uint32 size ) {
//	EraseBlock_t eraseBlocks;
  uint32 startBlock;
  uint32 endBlock;
  uint32 mask1;
  uint32 mask2;
  uint32 mask;
  uint32 rem;

  /* Create a mask with continuous set of 1's */
  startBlock = address_to_block( addr,&rem );
  endBlock = address_to_block( addr + size - 1,&rem );

  // Check so our implementation holds..
  assert( endBlock<=32 );

#define BLOCK_MASK 0x0003ffffUL

  // create the mask
  mask1 = ((-1UL)<<(31-endBlock))>>(31-endBlock);
  mask2 = ((-1UL)>>startBlock)<<startBlock;
  mask = mask1 & mask2;


  // shift things in to make freescale driver happy
  eraseBlocks->lowEnabledBlocks = mask&0x3f; // ????
  eraseBlocks->midEnabledBlocks = (mask>>10)&3; // ????
  eraseBlocks->highEnabledBlocks = mask>>12;


  return ;
}


void Fls_Init( const Fls_ConfigType *ConfigPtr )
{
  FLS_VALIDATE_STATUS_BUSY(Fls_Global.status, FLS_INIT_ID);
  Fls_Global.status 		= MEMIF_UNINIT;
  Fls_Global.jobResultType 	= MEMIF_JOB_PENDING;
  uint32 returnCode;
  Fls_EraseBlockType eraseBlocks;
  // TODO: FLS_E_PARAM_CONFIG
  Fls_Global.config = ConfigPtr;

#if (FLS_AC_LOAD_ON_JOB_START == STD_ON )
  /* Copy fls routines to RAM */
  memcpy(__FLS_ERASE_RAM__,__FLS_ERASE_ROM__, (size_t)&__FLS_SIZE__);

#endif


  returnCode = FlashInit( &ssdConfig );

  // Lock shadow row..
  eraseBlocks.lowEnabledBlocks = 0;
  eraseBlocks.midEnabledBlocks = 0;
  eraseBlocks.highEnabledBlocks = 0;
  eraseBlocks.shadowBlocks = 1;

  Fls_H7F_SetLock(&eraseBlocks,1);

  Fls_Global.status 		= MEMIF_IDLE;
  Fls_Global.jobResultType 	= MEMIF_JOB_OK;
  return;
}

/* TargetAddress always from 0 to FLS_TOTAL_SIZE */
Std_ReturnType Fls_Erase(	MemIf_AddressType   TargetAddress,
                          MemIf_LengthType    Length )
{
  uint32 block;
  uint32 sBlock;
  uint32 rem;
  Fls_EraseBlockType eraseBlock;
  Fls_EraseInfoType eraseInfo;
  uint32 pc;

  FLS_VALIDATE_STATUS_UNINIT_W_RV(Fls_Global.status, FLS_ERASE_ID, E_NOT_OK);
  FLS_VALIDATE_STATUS_BUSY_W_RV(Fls_Global.status, FLS_ERASE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_ADDRESS_SECTOR_W_RV(TargetAddress, FLS_ERASE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_LENGTH_SECTOR_W_RV(TargetAddress, Length, FLS_ERASE_ID, E_NOT_OK);

  // Always check if status is not busy
  if (Fls_Global.status == MEMIF_BUSY )
     return E_NOT_OK;

  // TargetAddress
  sBlock = address_to_block(TargetAddress,&rem);

  if( (sBlock == (-1)) || (rem!=0) ) {
    DET_REPORTERROR(MODULE_ID_FLS,0,0x0,FLS_E_PARAM_ADDRESS );
    return E_NOT_OK;
  }

  block = address_to_block(TargetAddress+Length,&rem);

  // Check if we trying to erase a partition that we are executing in
  pc = Fls_GetPc();
  if( (pc >= FLS_BASE_ADDRESS) && ( pc <= (FLS_BASE_ADDRESS + FLS_TOTAL_SIZE) ) ) {
    // In flash erase
  	uint32 pcBlock = address_to_block(pc,&rem);
  	uint8 *partMap = Fls_Global.config->FlsBlockToPartitionMap;

  	if( (partMap[pcBlock] >= partMap[sBlock]) && (partMap[pcBlock] <= partMap[block]) ) {
//    if( address_to_block(pc,&rem) == Fls_Global.config->FlsBlockToPartitionMap[block] ) {
        // Can't erase and in the same partition we are executing
        assert(0);
    }
  }

  Fls_Global.status = MEMIF_BUSY;
  Fls_Global.jobResultType = MEMIF_JOB_PENDING;
  Fls_Global.jobType = FLS_JOB_ERASE;

  address_to_erase_blocks(&eraseBlock,TargetAddress,Length);

  eraseBlock.shadowBlocks = 0;
  // Unlock
  Fls_H7F_SetLock(&eraseBlock,0);

  eraseInfo.state  = 0; // Always set this



  Fls_H7F_FlashErase ( 	&ssdConfig ,
          0, // shadowFlag...
          eraseBlock.lowEnabledBlocks,
          eraseBlock.midEnabledBlocks,
          eraseBlock.highEnabledBlocks,
          &eraseInfo
                  );
  return E_OK;
}


Std_ReturnType Fls_Write (    MemIf_AddressType   TargetAddress,
                        const uint8         *SourceAddressPtr,
                        MemIf_LengthType    Length )
{
  Fls_EraseBlockType eraseBlock;

  FLS_VALIDATE_STATUS_UNINIT_W_RV(Fls_Global.status, FLS_WRITE_ID, E_NOT_OK);
  FLS_VALIDATE_STATUS_BUSY_W_RV(Fls_Global.status, FLS_WRITE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_ADDRESS_PAGE_W_RV(TargetAddress, FLS_WRITE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_LENGTH_PAGE_W_RV(TargetAddress, Length, FLS_WRITE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_DATA_W_RV(SourceAddressPtr, FLS_WRITE_ID, E_NOT_OK)

  // Always check if status is not busy
  if (Fls_Global.status == MEMIF_BUSY )
     return E_NOT_OK;

  // Destination is FLS_BASE_ADDRESS + TargetAddress
  Fls_Global.jobResultType = MEMIF_JOB_PENDING;
  Fls_Global.status = MEMIF_BUSY;
  Fls_Global.jobType = FLS_JOB_WRITE;

  // Fill in the required fields for programming...
  Fls_Global.flashWriteInfo.source = (uint32)SourceAddressPtr;
  Fls_Global.flashWriteInfo.dest = TargetAddress;
  Fls_Global.flashWriteInfo.size = Length;

  // unlock flash....
  address_to_erase_blocks(&eraseBlock,TargetAddress,Length);
  eraseBlock.shadowBlocks = 0;
  Fls_H7F_SetLock(&eraseBlock,0);

  return E_OK;
}

#if ( FLS_CANCEL_API == STD_ON )
void Fls_Cancel( void )
{
	/* API NOT SUPPORTED */
}
#endif


#if ( FLS_GET_STATUS_API == STD_ON )
MemIf_StatusType Fls_GetStatus(	void )
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

void Fls_MainFunction( void )
{
  uint32 flashStatus;
  int result;
  if( Fls_Global.jobResultType == MEMIF_JOB_PENDING ) {
    switch(Fls_Global.jobType) {
    case FLS_JOB_COMPARE:
      // NOT implemented. Hardware error = FLS_E_COMPARE_FAILED
      // ( we are reading directly from flash so it makes no sense )

      result = memcmp(Fls_Global.targetAddr,(void *)Fls_Global.sourceAddr,Fls_Global.length);
      if( result == 0 ) {
        Fls_Global.jobResultType = MEMIF_JOB_OK;
      } else {
        Fls_Global.jobResultType = MEMIF_JOB_FAILED;
      }
      Fls_Global.status = MEMIF_IDLE;
      Fls_Global.jobType = FLS_JOB_NONE;

      break;
    case FLS_JOB_ERASE:
    {
//      uint32 failAddress;
//      uint32 failData;

      flashStatus = Fls_H7F_EraseStatus(&ssdConfig);
      if( flashStatus == H7F_OK ) {
        Fls_EraseBlockType blocks;
        // Lock all.
        blocks.highEnabledBlocks = (-1UL);
        blocks.midEnabledBlocks = (-1UL);
        blocks.highEnabledBlocks = (-1UL);
        blocks.shadowBlocks = (-1UL);

        Fls_H7F_SetLock(&blocks,1);

        Fls_Global.jobResultType = MEMIF_JOB_OK;
        Fls_Global.jobType = FLS_JOB_NONE;
        Fls_Global.status = MEMIF_IDLE;
        FEE_JOB_END_NOTIFICATION();
      } else if( flashStatus == H7F_BUSY )  {
        /* Busy, Do nothing */
      } else {
        // Error
        Fls_Global.jobResultType = MEMIF_JOB_FAILED;
        Fls_Global.jobType = FLS_JOB_NONE;
        Fls_Global.status = MEMIF_IDLE;
        DET_REPORTERROR(MODULE_ID_FLS,0, 0x6, FLS_E_WRITE_FAILED );
        FEE_JOB_ERROR_NOTIFICATION();
      }
      break;
    }
    case FLS_JOB_READ:

      // NOT implemented. Hardware error = FLS_E_READ_FAILED
      // ( we are reading directly from flash so it makes no sense )
      memcpy(Fls_Global.targetAddr,(void *)Fls_Global.sourceAddr,Fls_Global.length);
      Fls_Global.jobResultType = MEMIF_JOB_OK;
      Fls_Global.status = MEMIF_IDLE;
      Fls_Global.jobType = FLS_JOB_NONE;
      break;
    case FLS_JOB_WRITE:
    {
      // NOT implemented. Hardware error = FLS_E_READ_FAILED

      flashStatus = Fls_H7F_Program( &ssdConfig,&Fls_Global.flashWriteInfo);

      if( flashStatus == H7F_OK ) {
        Fls_EraseBlockType blocks;
        blocks.highEnabledBlocks = (-1UL);
        blocks.midEnabledBlocks = (-1UL);
        blocks.highEnabledBlocks = (-1UL);
        blocks.shadowBlocks = (-1UL);

        // Lock all
        Fls_H7F_SetLock(&blocks,1);

        Fls_Global.jobResultType = MEMIF_JOB_OK;
        Fls_Global.jobType = FLS_JOB_NONE;
        Fls_Global.status = MEMIF_IDLE;
        FEE_JOB_END_NOTIFICATION();
      } else if( flashStatus == H7F_BUSY )  {
        /* Busy, Do nothing */
      } else {
        // Error
        Fls_Global.jobResultType = MEMIF_JOB_FAILED;
        Fls_Global.jobType = FLS_JOB_NONE;
        Fls_Global.status = MEMIF_IDLE;
        DET_REPORTERROR(MODULE_ID_FLS,0, 0x6, FLS_E_WRITE_FAILED );
        FEE_JOB_ERROR_NOTIFICATION();
      }

      break;
    }
    case FLS_JOB_NONE:
      assert(0);
      break;
    }
  }
}

Std_ReturnType Fls_Read (	MemIf_AddressType SourceAddress,
              uint8 *TargetAddressPtr,
              MemIf_LengthType Length )
{
  FLS_VALIDATE_STATUS_UNINIT_W_RV(Fls_Global.status, FLS_READ_ID, E_NOT_OK);
  FLS_VALIDATE_STATUS_BUSY_W_RV(Fls_Global.status, FLS_READ_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_ADDRESS_PAGE_W_RV(SourceAddress, FLS_READ_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_LENGTH_PAGE_W_RV(SourceAddress, Length, FLS_READ_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_DATA_W_RV((void*)SourceAddress, FLS_READ_ID, E_NOT_OK)

  // Always check if status is not busy
  if (Fls_Global.status == MEMIF_BUSY )
     return E_NOT_OK;

  Fls_Global.status = MEMIF_BUSY;
  Fls_Global.jobResultType = MEMIF_JOB_PENDING;
  Fls_Global.jobType = FLS_JOB_READ;

  Fls_Global.sourceAddr = SourceAddress;
  Fls_Global.targetAddr = TargetAddressPtr;
  Fls_Global.length = Length;

  return E_OK;
}

#if ( FLS_COMPARE_API == STD_ON )
Std_ReturnType Fls_Compare( MemIf_AddressType SourceAddress,
              uint8 *TargetAddressPtr,
              MemIf_LengthType Length )
{
  FLS_VALIDATE_STATUS_UNINIT_W_RV(Fls_Global.status, FLS_COMPARE_ID, E_NOT_OK);
  FLS_VALIDATE_STATUS_BUSY_W_RV(Fls_Global.status, FLS_COMPARE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_ADDRESS_PAGE_W_RV(SourceAddress, FLS_COMPARE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_LENGTH_PAGE_W_RV(SourceAddress, Length, FLS_COMPARE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_DATA_W_RV((void*)SourceAddress,FLS_COMPARE_ID, E_NOT_OK)

  // Always check if status is not busy
  if (Fls_Global.status == MEMIF_BUSY )
     return E_NOT_OK;

  Fls_Global.status = MEMIF_BUSY;
  Fls_Global.jobResultType = MEMIF_JOB_PENDING;
  Fls_Global.jobType = FLS_JOB_COMPARE;

  Fls_Global.sourceAddr = SourceAddress;
  Fls_Global.targetAddr = TargetAddressPtr;
  Fls_Global.length = Length;

  return E_OK;
}
#endif

#if ( FLS_SET_MODE_API == STD_ON )
void Fls_SetMode(		MemIf_ModeType Mode )
{
	/* API NOT SUPPORTED */
}
#endif

void Fls_GetVersionInfo( Std_VersionInfoType *VersioninfoPtr )
{
  memcpy(VersioninfoPtr, &_Fls_VersionInfo, sizeof(Std_VersionInfoType));
}



