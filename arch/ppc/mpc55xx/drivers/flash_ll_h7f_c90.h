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

/* Inital code by:
 * COPYRIGHT :(c) 2009, Freescale & STMicroelectronics
 *
 * Modified to fit purpose:
 * Copyright (C) ArcCore AB <contact@arccore.com>
 */

#ifndef FLASH_H7F_C90_H_
#define FLASH_H7F_C90_H_

#include "Std_Types.h"
#include "flash.h"

/* Offsets of C90FL Control Registers*/
#define C90FL_MCR                    0x0000        /* Module Configuration Register */
#define C90FL_LML                    0x0004        /* Low/Mid Address Sapce Block Locking Register */
#define C90FL_HBL                    0x0008        /* High Address Sapce Block Locking Register */
#define C90FL_SLL                    0x000C        /* Secondary Low/Mid Address Space Block Locking Register */
#define C90FL_LMS                    0x0010        /* Low/Mid Address Space Block Select Register */
#define C90FL_HBS                    0x0014        /* High Address Space Block Select Register */

/* macros for scheduling*/
//#define NUMBER_OF_SEARCHING_RECORD_IN_BLOCK     0x2000
#define DATA_VERIFY_NUMBER                      0x5000
#define DATA_READ_NUMBER                        0x5000

/* macros for CallBack period controlling*/
#define CALLBACK_READ_BYTES_NUMBER              0x0020
//#define CALLBACK_SEARCHING_BYTES_NUMBER         0x001A
#define CALLBACK_VERIFY_BYTES_NUMBER            0x0020
//#define CALLBACK_FILL_BUFFER                    0x0060

/* NULL callback */
#define NULL_CALLBACK             ((void *) 0xFFFFFFFF)

/* return code definition*/
#define EE_OK                        0x00000000
#define EE_FIRST_TIME_INITIALIZATION 0x00000001
#define EE_INFO_HVOP_INPROGRESS      0x00000002
#define EE_INFO_PROGRAM_SUSPEND      0x00000004
#define EE_INFO_ERASE_SUSPEND        0x00000010
#define EE_ERROR_WRITE_IN_PROGRESS   0x00000020
#define EE_ERROR_PE_OPT              0x00000040
#define EE_ERROR_MISMATCH            0x00000080
#define EE_ERROR_BLOCK_STATUS        0x00000100
#define EE_ERROR_RECORD_STATUS       0x00000200
#define EE_ERROR_BLOCK_CONFIG        0x00000400
#define EE_ERROR_DATA_NOT_FOUND      0x00000800
#define EE_ERROR_NOT_IN_CACHE        0x00001000
#define EE_ERROR_NO_ENOUGH_SPACE     0x00002000

/* macros for Flash suspend and resume*/
#define C90FLMCR_EED_BIT_SET(MCRAddress, mask)      \
    WRITE32(MCRAddress, ((mask | READ32(MCRAddress)) & (~(C90FL_MCR_EER | C90FL_MCR_RWE))))

#define C90FLMCR_EED_BIT_CLEAR(MCRAddress, mask)    \
    WRITE32(MCRAddress, (((~mask) & READ32(MCRAddress)) & (~(C90FL_MCR_EER | C90FL_MCR_RWE))))

#define FLASH_RESUME_WAIT           15

/* structure declaration*/
typedef struct
{
    uint64_t  dataStatus;                 /* the data record status*/
    uint16_t  dataID;                     /* the unique data ID*/
    uint16_t  dataSize;                   /* the data size*/
    uint32_t  wordData;                   /* the first 4 bytes data*/
}DATA_RECORD_HEAD;


typedef struct
{
    uint32_t  enabledBlock;               /* the block bit map in specific space*/
    uint32_t  blockStartAddr;             /* the block start address*/
    uint32_t  blockSize;                  /* the block size*/
    uint32_t  blankSpace;                 /* the address pointer to the blank space*/
    uint8_t   blockSpace;                 /* the space (low, middle or high) for the block*/
}BLOCK_CONFIG;


uint32_t FSL_FlashProgramStart (uint32_t c90flRegBase, uint32_t* dest, uint32_t* size, uint32_t* source);
uint32_t FSL_FlashEraseStart (uint32_t c90flRegBase, uint32_t interlockWriteAddress, uint32_t lowEnabledBlock, uint32_t midEnabledBlock, uint32_t highEnabledBlock);
uint32_t FSL_FlashCheckStatus (uint32_t c90flRegBase);
uint32_t FSL_DataVerify (uint32_t c90flRegBase, bool blankCheck, uint32_t* dest, uint32_t* size, uint32_t* source, uint32_t *compareAddress, uint64_t *compareData, void(*CallBack)(void));
uint32_t FSL_FlashSuspend (uint32_t c90flRegBase);
uint32_t FSL_FlashResume (uint32_t c90flRegBase, uint32_t resumeType);
uint32_t FSL_FlashRead (uint32_t c90flRegBase, uint32_t* dest, uint32_t* size, uint32_t* buffer, void(*CallBack)(void));
uint32_t FSL_FlashEraseAbort (uint32_t c90flRegBase);


uint32_t FSL_SearchRecordInBlock (BLOCK_CONFIG* blockConf, uint32_t c90flRegBase, uint32_t bufferAddress, uint32_t bufferSize, uint16_t startID, uint32_t* startAddrInBlock, uint16_t* nextStartID, void(*CallBack)(void));
uint8_t  FSL_FlashRead8(uint32_t address);
uint64_t FSL_FlashRead64(uint32_t address);
void   FSL_ReadRecordHead(uint32_t address, DATA_RECORD_HEAD *pLocRecHead);
uint32_t readAndClearEei(void);
void   restoreEei(uint32_t value);
void   EER_exception_handler(void);

#endif /* FLASH_H7F_C90_H_ */
