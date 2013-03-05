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








#if 1


/* CONFIGURATION NOTES
 *   The configuration is simple, use the template supplied.
 *   Changing the configuration is NOT recommended.
 */

/* REQUIREMENTS:
 * - Variant PB is supported not PC ( FLS203,FLS204 )
 * - Since DEM is NOT supported all those requirements are not supported.
 * - AC is not supported since it makes no sense for a SPI flash.
 *
 */

/* IMPLEMENTATION NOTES
 * - The only SPI flash supported is the SST25VF016B although the
 *   entire SST25XX should work through configuration changes
 * - Commands that are used by this module are:
 *   WREN,WRDI,WRSR,byte write and erase 4K
 * - AC is not supported since the there's no use for it.
 * - Supports 64 bytes read, byte write and 4K erase
 * - The implementation very much dependent on the configuration
 *   of the SPI( Spi_Cfg.c )
 * - Calls from SPI are not checked( Only makes sense if DEM is supported )
 *
 */


#define FLS_INCLUDE_FILE "Fls_SST25xx.h"
#include "Fls.h"
#include "Spi.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include <stdlib.h>
#include <assert.h>
//#include <stdio.h>
#include <string.h>

//#define USE_LDEBUG_PRINTF
#include "debug.h"
#define MODULE_NAME 	"/driver/Fls_25"


/* RDID data for S25FL040A */
#define DEVICE_RDID		0x010212  /* 0x12 - Uniform, 0x25- Top boot, 0x28-bottom boot */

#define FLASH_READ_25			0x03
#define FLASH_READ_50			0x0B
#define FLASH_RDSR				0x05
#define FLASH_JEDEC_ID			0x9f
#define FLASH_RDID				0x90
#define FLASH_BYTE_WRITE 		0x02
#define FLASH_AI_WORD_WRITE 	0xad
#define FLASH_WREN				0x06
#define FLASH_WRDI				0x04
#define FLASH_WRSR				0x01
#define FLASH_ERASE_4K			0xd8


/* The width in bytes used by this flash */
#define ADDR_LENGTH		3

/* Helper macro for the process function */
#define SET_STATE(_done,_state) done=(_done);job->state=(_state)

/* How many loops to wait for SPI to go back to "normal" state after
 * read/write/erase */
 #define TIMER_BUSY_WAIT  100000

#if FLS_SST25XX_DEV_ERROR_DETECT
#define FLS_VALIDATE_PARAM_ADDRESS_SECTOR_W_RV(_addr, _api, _rv)\
  int sectorIndex;\
  int addrOk=0;\
  Fls_SectorType sector;\
  for (sectorIndex=0; sectorIndex<Fls_SST25xx_Global.config->FlsSectorListSize;sectorIndex++) {\
    sector = Fls_SST25xx_Global.config->FlsSectorList[sectorIndex];\
    if((((uint32)_addr-sector.FlsSectorStartaddress) / sector.FlsSectorSize)<sector.FlsNumberOfSectors){\
      /* Within the right adress space */\
      if (!(((uint32)_addr-sector.FlsSectorStartaddress) % sector.FlsSectorSize)){\
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
  Fls_SectorType sector;\
  for (sectorIndex=0; sectorIndex<Fls_SST25xx_Global.config->FlsSectorListSize;sectorIndex++) {\
    sector = Fls_SST25xx_Global.config->FlsSectorList[sectorIndex];\
    if((((uint32)_addr-sector.FlsSectorStartaddress) / sector.FlsSectorSize)<sector.FlsNumberOfSectors){\
      /* Within the right adress space */\
      if (!(((uint32)_addr-sector.FlsSectorStartaddress) % sector.FlsPageSize)){\
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
  const Fls_SectorType* sectorPtr= &Fls_SST25xx_Global.config->FlsSectorList[0];\
  for (i=0; i<Fls_SST25xx_Global.config->FlsSectorListSize;i++) {\
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
  const Fls_SectorType* sectorPtr= &Fls_SST25xx_Global.config->FlsSectorList[0];\
  for (i=0; i<Fls_SST25xx_Global.config->FlsSectorListSize;i++) {\
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
#define VALIDATE_CONFIG(_x) assert(_x)
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(MODULE_ID_FLS, _y, _z, _q)
#else
#define VALIDATE_CONFIG(_x)
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#if ( FLS_GET_JOB_RESULT_API == STD_ON )
#define FEE_JOB_END_NOTIFICATION() \
  if( Fls_SST25xx_Global.config->FlsJobEndNotification != NULL ) { \
    Fls_SST25xx_Global.config->FlsJobEndNotification(); \
  }
#define FEE_JOB_ERROR_NOTIFICATION() \
  if( Fls_SST25xx_Global.config->FlsJobErrorNotification != NULL ) { \
    Fls_SST25xx_Global.config->FlsJobErrorNotification(); \
  }
#else
#define FEE_JOB_END_NOTIFICATION()
#define FEE_JOB_ERROR_NOTIFICATION()
#endif



#if ( FLS_SST25XX_DEV_ERROR_DETECT == STD_ON ) // Report DEV errors
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_FLS,0,_api,_err); \
          return; \
        }
#endif

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_FLS,0,_api,_err); \
          return (_rv); \
        }

#define VALID_CHANNEL(_ch)    ( Gpt_Global.configured & (1<<(_ch)) )

#else // Validate but do not report
#define VALIDATE(_exp,_api,_err )\
        if( !(_exp) ) { \
          return; \
        }
#define VALIDATE_W_RV(_exp,_api,_err,_rv )\
        if( !(_exp) ) { \
          return (_rv); \
        }
#endif

const Fls_ConfigType* _Fls_SST25xx_ConfigPtr;

#if ( FLS_SST25XX_VERSION_INFO_API == STD_ON )
static Std_VersionInfoType Fls_SST25XX_VersionInfo = {
    .vendorID 			= (uint16)1,
    .moduleID 			= (uint16) MODULE_ID_FLS,
    .instanceID			= (uint8)1,
    /* Vendor numbers */
    .sw_major_version	= (uint8)FLS_SST25XX_SW_MAJOR_VERSION,
    .sw_minor_version	= (uint8)FLS_SST25XX_SW_MINOR_VERSION,
    .sw_patch_version	= (uint8)FLS_SST25XX_SW_PATCH_VERSION,
    .ar_major_version	= (uint8)FLS_SST25XX_AR_MAJOR_VERSION,
    .ar_minor_version	= (uint8)FLS_SST25XX_AR_MINOR_VERSION,
    .ar_patch_version	= (uint8)FLS_SST25XX_AR_PATCH_VERSION,
};
#endif

/* Job state */
typedef enum {
  FLS_SST25XX_NONE,
  FLS_SST25XX_COMPARE,
  FLS_SST25XX_ERASE,
  FLS_SST25XX_READ,
  FLS_SST25XX_WRITE,
} Fls_SST25xx_Arc_JobType;

/* Spi job state */
typedef enum {
	JOB_MAIN,
	JOB_READ_STATUS,
	JOB_READ_STATUS_RESULT,
} Job_StateType;

/* Information about a job */
typedef struct {
	uint8 *targetAddr;
	Fls_AddressType flsAddr;
	uint32 left;
	Job_StateType state;
	Fls_SST25xx_Arc_JobType mainState;
	Spi_SequenceType currSeq;
	uint32 chunkSize;
	boolean initialOp;
} Fls_SST25xx_JobInfoType;

#define JOB_SET_STATE(_x,_y)		job->state=(_x);job->mainState=(_y)


typedef struct {
  const Fls_ConfigType *config;

  // Status of driver
  MemIf_StatusType status;
  MemIf_JobResultType jobResultType;
  Fls_SST25xx_Arc_JobType jobType;

  // Saved information from API calls.
  Fls_AddressType flsAddr;
  uint8 *targetAddr;
  Fls_LengthType length;

  // Data containers for EB buffers
  Spi_DataType ebCmd;
  Spi_DataType ebReadStatus;
  Spi_DataType ebFlsAddr[ADDR_LENGTH];

  // What mode we are in ( normal/fast )
  MemIf_ModeType mode;

  // Hold job information
  Fls_SST25xx_JobInfoType job;

} Fls_SST25xx_GlobalType;

Fls_SST25xx_GlobalType Fls_SST25xx_Global;

#if 0
#define SPI_TRANSMIT_FUNC(_x)	Spi_SyncTransmit(_x)
#else
#define SPI_TRANSMIT_FUNC(_x,_y)	Fls_SST25xx_AsyncTransmit(_x,_y)

Std_ReturnType Fls_SST25xx_AsyncTransmit(Spi_SequenceType Sequence,Fls_SST25xx_JobInfoType *job) {
	Std_ReturnType rv;
	job->currSeq = Sequence;
	rv = Spi_AsyncTransmit(Sequence);
	return rv;
}
#endif


/**
 * Convert Fls_AddressType to something used by SPI
 *
 * @param spiAddr Address to convert to SPI address
 * @param addr Pointer to the SPI address to be written
 *
 */
static void Spi_ConvertToSpiAddr(Spi_DataType *spiAddr, Fls_AddressType addr ) {

	spiAddr[0] = (addr>>16)&0xff;		// MSB first
	spiAddr[1] = (addr>>8)&0xff;
	spiAddr[2] = (addr)&0xff;

}

/**
 * Get configuration sector information from a flash address
 *
 * @param addr The address
 */

static const Fls_SectorType * Fls_SST25xx_GetSector( Fls_AddressType addr ) {
  int sectorIndex;
  const Fls_SectorType *sector;

  for (sectorIndex=0; sectorIndex<Fls_SST25xx_Global.config->FlsSectorListSize;sectorIndex++) {
    sector = &Fls_SST25xx_Global.config->FlsSectorList[sectorIndex];
    if((((uint32)addr-sector->FlsSectorStartaddress) / sector->FlsSectorSize)<sector->FlsNumberOfSectors){
      return sector;
    }
  }
  assert(0);
  return NULL;
 }

void Fls_SST25xx_Init( const Fls_ConfigType* ConfigPtr ){

	FLS_VALIDATE_STATUS_BUSY(Fls_SST25xx_Global.status, FLS_INIT_ID);

#if ( FLS_SST25XX_VARIANT_PB == STD_ON )
  VALIDATE( (ConfigPtr != NULL) , FLS_INIT_ID, FLS_E_PARAM_CONFIG );
#endif

  Fls_SST25xx_Global.config = ConfigPtr;
  Spi_DataType data = 0;
  int timer = 0;
  Std_ReturnType rv = E_OK;
  Spi_DataType jedecId[3];



  // Do some basic testing of configuration data, FLS205
  VALIDATE_CONFIG(ConfigPtr->FlsMaxReadFastMode != 0 );
  VALIDATE_CONFIG(ConfigPtr->FlsMaxReadNormalMode != 0 );
  VALIDATE_CONFIG(ConfigPtr->FlsMaxWriteFastMode != 0 );
  VALIDATE_CONFIG(ConfigPtr->FlsMaxWriteNormalMode != 0 );

  VALIDATE_CONFIG(ConfigPtr->FlsAcWrite == NULL );   // NOT supported
  VALIDATE_CONFIG(ConfigPtr->FlsAcErase == NULL );   // NOT supported

  // Setup External buffers for jobs and sequences
  Spi_SetupEB( SPI_CH_FLASH_CMD,  &Fls_SST25xx_Global.ebCmd,NULL,sizeof(Fls_SST25xx_Global.ebCmd)/sizeof(Fls_SST25xx_Global.ebCmd));
  Spi_SetupEB( SPI_CH_FLASH_ADDR,  Fls_SST25xx_Global.ebFlsAddr,NULL,sizeof(Fls_SST25xx_Global.ebFlsAddr)/sizeof(Fls_SST25xx_Global.ebFlsAddr[0]));
  Spi_SetupEB( SPI_CH_FLASH_WREN,  NULL,NULL,1);
  Spi_SetupEB( SPI_CH_FLASH_WRDI,  NULL,NULL,1);
  Spi_SetupEB( SPI_CH_FLASH_WRSR,  NULL,NULL,1);

  /* Check that the JEDEC ID can be read */
  Spi_SetupEB( SPI_CH_FLASH_DATA, NULL ,jedecId,3);

  Fls_SST25xx_Global.ebCmd = FLASH_JEDEC_ID;
  Spi_SyncTransmit(SPI_SEQ_FLASH_CMD_DATA );

  if( ((jedecId[0]<<16) + (jedecId[1]<<8) + jedecId[2]) != DEVICE_RDID ) {
   LDEBUG_PRINTF("JEDEC: %02x %02x %02x\n",jedecId[0],jedecId[1],jedecId[2]);
  }

  /* The flash comes locked from factory so it must be unlocked.
   * The unlock is done in here instead before each write to reduce overhead.
   * ( The flash is still protected by WREN )
   */

  /* Unlock flash with sync API. */
  Spi_SetupEB( SPI_CH_FLASH_DATA, &data, NULL,1);
  rv = Spi_SyncTransmit(SPI_SEQ_FLASH_WRSR);

	// Busy wait
	Spi_SetupEB( SPI_CH_FLASH_DATA, NULL, &Fls_SST25xx_Global.ebReadStatus, 1);
	do {
		Fls_SST25xx_Global.ebCmd = FLASH_RDSR;
		Spi_SyncTransmit(SPI_SEQ_FLASH_CMD2);
		timer++;
	} while( (Fls_SST25xx_Global.ebReadStatus != 0) && (timer < TIMER_BUSY_WAIT ));

	assert(timer!=TIMER_BUSY_WAIT);

  Fls_SST25xx_Global.status     = MEMIF_IDLE;
  Fls_SST25xx_Global.jobResultType  = MEMIF_JOB_PENDING;

  // Set currSeq to any sequence we use
  Fls_SST25xx_Global.job.currSeq = SPI_SEQ_FLASH_WRSR;

}

#if ( FLS_SST25XX_SET_MODE_API == STD_ON )
void Fls_SST25xx_SetMode( MemIf_ModeType Mode ){
  VALIDATE( ( Fls_SST25xx_Global.status != MEMIF_UNINIT ), FLS_SET_MODE_ID, FLS_E_UNINIT );
  VALIDATE( ( Fls_SST25xx_Global.status != MEMIF_BUSY ), FLS_SET_MODE_ID, FLS_E_BUSY );

  Fls_SST25xx_Global.mode = Mode;
}
#endif

Std_ReturnType Fls_SST25xx_Read ( Fls_AddressType SourceAddress,
                           uint8 *TargetAddressPtr,
                           Fls_LengthType Length )
{
	Fls_SST25xx_JobInfoType *job = &Fls_SST25xx_Global.job;

  FLS_VALIDATE_STATUS_UNINIT_W_RV(Fls_SST25xx_Global.status, FLS_READ_ID, E_NOT_OK);
  FLS_VALIDATE_STATUS_BUSY_W_RV(Fls_SST25xx_Global.status, FLS_READ_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_ADDRESS_PAGE_W_RV(SourceAddress, FLS_READ_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_LENGTH_PAGE_W_RV(SourceAddress, Length, FLS_READ_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_DATA_W_RV((void*)TargetAddressPtr, FLS_READ_ID, E_NOT_OK)

  Fls_SST25xx_Global.status = MEMIF_BUSY;
  Fls_SST25xx_Global.jobResultType = MEMIF_JOB_PENDING;
  Fls_SST25xx_Global.jobType = FLS_SST25XX_READ;

  if( Fls_SST25xx_Global.mode == MEMIF_MODE_FAST ) {
     job->chunkSize = Fls_SST25xx_Global.config->FlsMaxReadFastMode;
  } else {
     job->chunkSize = Fls_SST25xx_Global.config->FlsMaxReadNormalMode;
  }

  job->initialOp = true;
  job->currSeq = SPI_SEQ_FLASH_READ;
  job->flsAddr = SourceAddress;
  job->targetAddr = TargetAddressPtr;
  job->left = Length;

  JOB_SET_STATE(JOB_MAIN,FLS_SST25XX_READ);

  return E_OK;

}

Std_ReturnType Fls_SST25xx_Erase( Fls_AddressType   TargetAddress, Fls_LengthType    Length ){
	Fls_SST25xx_JobInfoType *job = &Fls_SST25xx_Global.job;

  FLS_VALIDATE_STATUS_UNINIT_W_RV(Fls_SST25xx_Global.status, FLS_ERASE_ID, E_NOT_OK);
  FLS_VALIDATE_STATUS_BUSY_W_RV(Fls_SST25xx_Global.status, FLS_ERASE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_ADDRESS_SECTOR_W_RV(TargetAddress, FLS_ERASE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_LENGTH_SECTOR_W_RV(TargetAddress, Length, FLS_ERASE_ID, E_NOT_OK);

  Fls_SST25xx_Global.status = MEMIF_BUSY;
  Fls_SST25xx_Global.jobResultType = MEMIF_JOB_PENDING;
  Fls_SST25xx_Global.jobType = FLS_SST25XX_ERASE;

  job->initialOp = true;
  job->currSeq = SPI_SEQ_FLASH_WRITE;

  job->flsAddr = TargetAddress;
  // Not used, so set to illegal value
  job->targetAddr = (uint8 *)0;
  job->left = Length;
  job->chunkSize = Fls_SST25xx_GetSector(TargetAddress)->FlsSectorSize;

  JOB_SET_STATE(JOB_MAIN,FLS_SST25XX_ERASE);

  return E_OK;
}

Std_ReturnType Fls_SST25xx_Write( Fls_AddressType TargetAddress, const uint8* SourceAddressPtr, Fls_LengthType Length ){

	Fls_SST25xx_JobInfoType *job = &Fls_SST25xx_Global.job;

  FLS_VALIDATE_STATUS_UNINIT_W_RV(Fls_SST25xx_Global.status, FLS_WRITE_ID, E_NOT_OK);
  FLS_VALIDATE_STATUS_BUSY_W_RV(Fls_SST25xx_Global.status, FLS_WRITE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_ADDRESS_PAGE_W_RV(TargetAddress, FLS_WRITE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_LENGTH_PAGE_W_RV(TargetAddress, Length, FLS_WRITE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_DATA_W_RV(SourceAddressPtr, FLS_WRITE_ID, E_NOT_OK)

	Fls_SST25xx_Global.jobResultType = MEMIF_JOB_PENDING;
	Fls_SST25xx_Global.status = MEMIF_BUSY;
	Fls_SST25xx_Global.jobType = FLS_SST25XX_WRITE;

	if( Fls_SST25xx_Global.mode == MEMIF_MODE_FAST ) {
		job->chunkSize = Fls_SST25xx_Global.config->FlsMaxWriteFastMode;
	} else {
		job->chunkSize = Fls_SST25xx_Global.config->FlsMaxWriteNormalMode;
	}

  job->initialOp = true;
  job->currSeq = SPI_SEQ_FLASH_WRITE;
  job->flsAddr = TargetAddress;
  job->targetAddr = (uint8 *)SourceAddressPtr;
  job->left = Length;

  JOB_SET_STATE(JOB_MAIN,FLS_SST25XX_WRITE);

  return E_OK;
}


Std_ReturnType Fls_SST25xx_Compare( Fls_AddressType SourceAddress, uint8 *TargetAddressPtr, Fls_LengthType Length )
{
	Fls_SST25xx_JobInfoType *job = &Fls_SST25xx_Global.job;

  FLS_VALIDATE_STATUS_UNINIT_W_RV(Fls_SST25xx_Global.status, FLS_COMPARE_ID, E_NOT_OK);
  FLS_VALIDATE_STATUS_BUSY_W_RV(Fls_SST25xx_Global.status, FLS_COMPARE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_ADDRESS_PAGE_W_RV(SourceAddress, FLS_COMPARE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_LENGTH_PAGE_W_RV(SourceAddress, Length, FLS_COMPARE_ID, E_NOT_OK);
  FLS_VALIDATE_PARAM_DATA_W_RV((void*)SourceAddress,FLS_COMPARE_ID, E_NOT_OK)

  Fls_SST25xx_Global.status = MEMIF_BUSY;
  Fls_SST25xx_Global.jobResultType = MEMIF_JOB_PENDING;
  Fls_SST25xx_Global.jobType = FLS_SST25XX_COMPARE;

  /* This is a compare job but the compare jobs really issues read in portions
   * big enough to fit it's static buffers
   */
  if( Fls_SST25xx_Global.mode == MEMIF_MODE_FAST ) {
     job->chunkSize = Fls_SST25xx_Global.config->FlsMaxReadFastMode;
  } else {
     job->chunkSize = Fls_SST25xx_Global.config->FlsMaxReadNormalMode;
  }

  job->flsAddr = SourceAddress;
  job->targetAddr = TargetAddressPtr;
  job->left = Length;

  JOB_SET_STATE(JOB_MAIN,FLS_SST25XX_COMPARE);

  return E_OK;
}


#if ( FLS_SST25XX_CANCEL_API == STD_ON )

/* API NOT SUPPORTED */

void Fls_SST25xx_Cancel( void ){
  if (Fls_SST25xx_Global.config->FlsJobEndNotification!=NULL) Fls_SST25xx_Global.config->FlsJobEndNotification();

  if (MEMIF_JOB_PENDING==Fls_SST25xx_Global.jobResultType) {
  	Fls_SST25xx_Global.jobResultType=MEMIF_JOB_CANCELLED;
  }

  Fls_SST25xx_Global.status = MEMIF_IDLE;
}
#endif


#if ( FLS_SST25XX_GET_STATUS_API == STD_ON )
MemIf_StatusType Fls_SST25xx_GetStatus( void ){
  return Fls_SST25xx_Global.status;
}
#endif

MemIf_JobResultType Fls_SST25xx_GetJobResult( void ){
  return Fls_SST25xx_Global.jobResultType;
}


/**
 * Function that process read/write/erase requests to the SPI
 *
 * @param job The present job
 */

static Spi_SeqResultType Fls_SST25xx_ProcessJob( Fls_SST25xx_JobInfoType *job ) {
  Spi_SeqResultType rv;
  _Bool done = 0;

	rv = Spi_GetSequenceResult(job->currSeq);

	if( job->initialOp ) {
		assert( rv != SPI_SEQ_PENDING );
		assert( job->state == JOB_MAIN );
		job->initialOp = false;
	} else {
		if( rv != SPI_SEQ_OK ) {
			return rv;
		}
	}
	rv = SPI_SEQ_PENDING;

  do {
		switch(job->state ) {
		case JOB_READ_STATUS:
			DEBUG(DEBUG_LOW,"%s: READ_STATUS\n",MODULE_NAME);
			/* Check status from erase cmd, read status from flash */
			Spi_SetupEB( SPI_CH_FLASH_DATA, NULL, &Fls_SST25xx_Global.ebReadStatus, 1);
			Fls_SST25xx_Global.ebCmd = FLASH_RDSR;
			if( SPI_TRANSMIT_FUNC(SPI_SEQ_FLASH_CMD2,job ) != E_OK ) {
				assert(0);
			}
			SET_STATE(1,JOB_READ_STATUS_RESULT);
			break;

		case JOB_READ_STATUS_RESULT:
			DEBUG(DEBUG_LOW,"%s: READ_STATUS_RESULT\n",MODULE_NAME);
			if( Fls_SST25xx_Global.ebReadStatus&1 ) {
				SET_STATE(0,JOB_READ_STATUS);
			} else {
				SET_STATE(0,JOB_MAIN);
			}
			break;

		case JOB_MAIN:
			if( job->left != 0 ) {
				if( job->left <= job->chunkSize ) {
					job->chunkSize = job->left;
				}

				Spi_ConvertToSpiAddr(Fls_SST25xx_Global.ebFlsAddr,job->flsAddr);

				switch(job->mainState) {
				case FLS_SST25XX_ERASE:
					DEBUG(DEBUG_LOW,"%s: Erase 4K s:%04x\n",MODULE_NAME,job->flsAddr);
					Fls_SST25xx_Global.ebCmd = FLASH_ERASE_4K;
					SPI_TRANSMIT_FUNC(SPI_SEQ_FLASH_ERASE,job );
					break;

				case FLS_SST25XX_READ:
				case FLS_SST25XX_COMPARE:
				  DEBUG(DEBUG_LOW,"%s: READ s:%04x d:%04x l:%04x\n",MODULE_NAME,job->flsAddr, job->targetAddr, job->left);
					Fls_SST25xx_Global.ebCmd = FLASH_READ_25;
					Spi_SetupEB( SPI_CH_FLASH_DATA, NULL ,job->targetAddr,job->chunkSize);
					SPI_TRANSMIT_FUNC(SPI_SEQ_FLASH_READ,job );
					break;

				case FLS_SST25XX_WRITE:
					DEBUG(DEBUG_LOW,"%s: WRITE d:%04x s:%04x first data:%02x\n",MODULE_NAME,job->flsAddr,job->targetAddr,*job->targetAddr);
					Fls_SST25xx_Global.ebCmd = FLASH_BYTE_WRITE;
					Spi_ConvertToSpiAddr(Fls_SST25xx_Global.ebFlsAddr,job->flsAddr);
					Spi_SetupEB( SPI_CH_FLASH_DATA, job->targetAddr, NULL, job->chunkSize);
					SPI_TRANSMIT_FUNC(SPI_SEQ_FLASH_WRITE,job );
					break;

				default:
					assert(0);
					break;
				}

				job->flsAddr += job->chunkSize;
				job->targetAddr += job->chunkSize;
				job->left -= job->chunkSize;
				SET_STATE(1,JOB_READ_STATUS);

			} else {
				/* We are done :) */
				SET_STATE(1,JOB_MAIN);
				job->mainState = FLS_SST25XX_NONE;
				rv = SPI_SEQ_OK;
			}
			break;

		default:
			assert(0);
			break;

		}
	} while(!done);
  return rv;
}


#define CMP_BUFF_SIZE SPI_EB_MAX_LENGTH

void Fls_SST25xx_MainFunction( void )
{
  Spi_SeqResultType jobResult;

  if( Fls_SST25xx_Global.jobResultType == MEMIF_JOB_PENDING ) {
  	switch (Fls_SST25xx_Global.jobType) {
		case FLS_SST25XX_COMPARE: {
			static Fls_SST25xx_JobInfoType readJob;
			static uint8 Fls_SST25xx_CompareBuffer[SPI_EB_MAX_LENGTH];
			Fls_SST25xx_JobInfoType *gJob = &Fls_SST25xx_Global.job;
			static _Bool firstTime = 1;
			static uint32 readSize;

			/* Compare jobs must use a local buffer to hold one portion
			 * of the job. Since Fls_SST25xx_ProcessJob() also manipulates the
			 * job structure we need to create a new local job each time.
			 * The global job updates is updated for each process job.
			 */

			if (firstTime == 1) {
				readJob = *gJob;

				if ( gJob->left <= CMP_BUFF_SIZE ) {
					readSize = gJob->left;
				} else {
					readSize = CMP_BUFF_SIZE;
				}
				readJob.left = readSize;
				readJob.targetAddr = Fls_SST25xx_CompareBuffer;
				firstTime = 0;
			}

			jobResult = Fls_SST25xx_ProcessJob(&readJob);

			if( jobResult == SPI_SEQ_PENDING ) {
				/* Do nothing */
			} else if( jobResult == SPI_SEQ_OK ) {

				if( memcmp(Fls_SST25xx_CompareBuffer,gJob->targetAddr, readSize) != 0 ) {
#if defined(USE_DEM)
					Dem_ReportErrorStatus(FLS_E_COMPARE_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
					FEE_JOB_ERROR_NOTIFICATION();
					return;
				}
				// Update the global comare job
				gJob->targetAddr += readSize;
				gJob->flsAddr += readSize;
				gJob->left -= readSize;

				// Check if we are done
				if( gJob->left == 0 ) {
					Fls_SST25xx_Global.jobResultType = MEMIF_JOB_OK;
					Fls_SST25xx_Global.jobType = FLS_SST25XX_NONE;
					Fls_SST25xx_Global.status = MEMIF_IDLE;
					FEE_JOB_END_NOTIFICATION();
					firstTime = 1;
					return;
				}
				// Calculate new readSize
				if ( gJob->left <= CMP_BUFF_SIZE ) {
					readSize = gJob->left;
				} else {
					readSize = CMP_BUFF_SIZE;
				}

				// Update the readjob for next session
				readJob = *gJob;
				readJob.left = readSize;
				readJob.targetAddr = Fls_SST25xx_CompareBuffer;
			} else {
			  // all other cases are bad
			  firstTime = 1;
        Fls_SST25xx_Global.jobResultType = MEMIF_JOB_FAILED;
        Fls_SST25xx_Global.jobType = FLS_SST25XX_NONE;
        Fls_SST25xx_Global.status = MEMIF_IDLE;

#if defined(USE_DEM)
				Dem_ReportErrorStatus(FLS_E_COMPARE_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
				FEE_JOB_ERROR_NOTIFICATION();
			}
		}
		break;

    case FLS_SST25XX_ERASE:
    case FLS_SST25XX_READ:
    case FLS_SST25XX_WRITE:

    	jobResult =  Fls_SST25xx_ProcessJob(&Fls_SST25xx_Global.job);

    	if( jobResult == SPI_SEQ_OK ) {

    		Fls_SST25xx_Global.jobResultType = MEMIF_JOB_OK;
    		Fls_SST25xx_Global.jobType = FLS_SST25XX_NONE;
    		Fls_SST25xx_Global.status = MEMIF_IDLE;
        FEE_JOB_END_NOTIFICATION();
      } else if( jobResult == SPI_SEQ_PENDING )  {
        /* Busy, Do nothing */
      } else {
        // Error
      	Fls_SST25xx_Global.jobResultType = MEMIF_JOB_FAILED;

      	switch(Fls_SST25xx_Global.jobType) {
      	case FLS_SST25XX_ERASE:
#if defined(USE_DEM)
					Dem_ReportErrorStatus(FLS_E_ERASED_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
					break;
      	case FLS_SST25XX_READ:
#if defined(USE_DEM)
					Dem_ReportErrorStatus(FLS_E_READ_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
					break;
      	case FLS_SST25XX_WRITE:
#if defined(USE_DEM)
					Dem_ReportErrorStatus(FLS_E_WRITE_FAILED, DEM_EVENT_STATUS_FAILED);
#endif
					break;
      	default:
      		assert(0);
      	}

        FEE_JOB_ERROR_NOTIFICATION();
      }
    	break;
    case FLS_SST25XX_NONE:
      assert(0);
      break;
    }
  }
}


#if ( FLS_SST25XX_VERSION_INFO_API == STD_ON )
void Fls_SST25XX_GetVersionInfo( Std_VersionInfoType *VersioninfoPtr )
{
  memcpy(VersioninfoPtr, &Fls_SST25XX_VersionInfo, sizeof(Std_VersionInfoType));
}

#endif

