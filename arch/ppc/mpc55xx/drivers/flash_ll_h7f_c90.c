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

/*
 * Inital driver by:
 * COPYRIGHT :(c) 2009, Freescale & STMicroelectronics
 *
 * Modified to fit purpose:
 * Copyright (C) ArcCore AB <contact@arccore.com>
 *
 */
/* ----------------------------[includes]------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "typedefs.h"
#include "flash_ll_h7f_c90.h"
#include "Cpu.h"
#include "io.h"

/* ----------------------------[private define]------------------------------*/
/* Module Configuration Register */
#define C90FL_MCR_EER                0x00008000   /* ECC Event Error */
#define C90FL_MCR_RWE                0x00004000   /* Read While Write Event Error */
#define C90FL_MCR_DONE               0x00000400   /* State Machine Status */
#define C90FL_MCR_PEG                0x00000200   /* Program/Erase Good */
#define C90FL_MCR_PGM                0x00000010   /* Program */
#define C90FL_MCR_PSUS               0x00000008   /* Program Suspend */
#define C90FL_MCR_ERS                0x00000004   /* Erase */
#define C90FL_MCR_ESUS               0x00000002   /* Erase Suspend */
#define C90FL_MCR_EHV                0x00000001   /* Enable High Voltage */

/* const definition*/
#define DATAID_SIZE                 2
#define CTABLE_ITEM_SIZE            4
#define PROGRAMMABLE_SIZE           8
#define WORD_SIZE                   4
#define DWORD_SIZE                  8
#define PAGE_SIZE                   16	/* OK */
#if defined(C90_DFO)
  #define PAGE_SIZE_BPS             4   /* Page buffer size for C90DFO flash */
#else
  #define PAGE_SIZE_BPS             8   /* Page buffer size for C90LC flash*/
#endif

/* macro for 64-bit data comparison*/
#define CREATE_WORD64(high, low)    (vuint64_t)((vuint64_t)(high) << 32 | (low))

/* data record status state*/
#define ERASED_HIGH                     0xFFFFFFFF
#define ERASED_LOW                      0xFFFFFFFF
#define RECORD_STATUS_ERASED            CREATE_WORD64(ERASED_HIGH, ERASED_LOW)

/* ----------------------------[private macro]-------------------------------*/

/* read and write macros*/
#define SET8(address, value)        (*(uint8_t*)(address) |= (value))
#define CLEAR8(address, value)      (*(uint8_t*)(address) &= ~(value))

#define SET16(address, value)       (*(vuint16_t*)(address) |= (value))
#define CLEAR16(address, value)     (*(vuint16_t*)(address) &= ~(value))

#define SET64(address, value)       (*(vuint64_t*)(address) |= (value))
#define CLEAR64(address, value)     (*(vuint64_t*)(address) &= ~(value))

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/



/* Flag to keep track of ECC Error Module*/
typedef enum
{
  NONE = 0,
  EE_MODULE = 1,
  OTHER_MODULE = 2
}EE_ModuleType;

extern volatile EE_ModuleType eccErrorModule_Flag;

/* Flag to keep track of ECC Error Status*/
extern uint8_t volatile eccErrorStatus_Flag;


/**
 * Program a page
 *
 * @param c90flRegBase
 * @param dest
 * @param size
 * @param source
 * @return
 */
uint32_t FSL_FlashProgramStart (uint32_t c90flRegBase, uint32_t* dest, uint32_t* size, uint32_t* source)
{
    uint32_t counter;                 /* loop counter*/
    uint32_t sourceIndex;             /* source address index*/
    uint32_t temp;                    /* temporary variable*/
    uint64_t buffer[4];               /* internal source data buffer*/

    /* Anything to program?*/
    if ( *size == 0 )
    {
        return (EE_OK);
    }

    /* check the high voltage operation*/
    if (READ32(c90flRegBase) & (C90FL_MCR_ERS | C90FL_MCR_PGM))
    {
        /* if any P/E operation in progress, return error*/
        return (EE_INFO_HVOP_INPROGRESS);
    }

    /* calculate the size to be programmed within the page boundary*/
    temp = *dest;
    temp = ((temp / PAGE_SIZE_BPS) + 1) * PAGE_SIZE_BPS - temp;

    /* there are remained data need programming*/
    /* padding for not alignment data*/
    for ( counter = 0; counter < (sizeof(buffer) / DWORD_SIZE); counter++)
    {
        buffer[counter] = CREATE_WORD64(ERASED_HIGH, ERASED_LOW);
    }

    /* copy one page to page buffer*/
    for (sourceIndex = 0; (sourceIndex < temp) && (*size > 0); sourceIndex++)
    {
        /* copy data*/
        WRITE8(((uint32_t)buffer + sourceIndex), FSL_FlashRead8(*source));

        /* update source data pointer*/
        *source += 1;

        *size -= 1;
    }

    /* program this page*/
    temp = ((sourceIndex + DWORD_SIZE - 1) / DWORD_SIZE);

    /* Set MCR PGM bit*/
    SET32(c90flRegBase, C90FL_MCR_PGM);

    /* Program data within one page*/
    for (sourceIndex = 0; sourceIndex < temp; sourceIndex++)
    {
#if defined(C90_DFO)
        /* Programming interlock write*/
        WRITE32(*dest, (uint32_t)(buffer[sourceIndex]>>32));

        *dest += DWORD_SIZE/2;
#else
        /* Programming interlock write*/
        WRITE64(*dest, buffer[sourceIndex]);

        *dest += DWORD_SIZE;
#endif
    }

    /* Set MCR EHV bit*/
    SET32(c90flRegBase, C90FL_MCR_EHV);

    return (EE_OK);
}

uint32_t FSL_FlashEraseStart (uint32_t c90flRegBase, uint32_t interlockWriteAddress, uint32_t lowEnabledBlock, uint32_t midEnabledBlock, uint32_t highEnabledBlock)
{

    /* check the high voltage operation*/
    if (READ32(c90flRegBase) & (C90FL_MCR_ERS | C90FL_MCR_PGM))
    {
        /* if any P/E operation in progress, return error*/
        return (EE_INFO_HVOP_INPROGRESS);
    }

    /* Set MCR ERS bit*/
    SET32(c90flRegBase, C90FL_MCR_ERS);

    /* prepare low enabled blocks*/
    lowEnabledBlock &= 0x0000FFFF;

    /* prepare middle enabled blocks*/
    midEnabledBlock &= 0xF;
    midEnabledBlock = midEnabledBlock << 16;

    /* prepare high enabled blocks*/
    highEnabledBlock &= 0x0FFFFFFF;

    /* write the block selection registers*/
    WRITE32 ((c90flRegBase + C90FL_LMS), (lowEnabledBlock | midEnabledBlock));
    WRITE32 ((c90flRegBase + C90FL_HBS), highEnabledBlock);

    /* Interlock write*/
    WRITE32(interlockWriteAddress, 0xFFFFFFFF);

    /* Set MCR EHV bit*/
    SET32(c90flRegBase, C90FL_MCR_EHV);

    return (EE_OK);
}


uint32_t FSL_FlashCheckStatus (uint32_t c90flRegBase)
{
    uint32_t returnCode;          /* return code*/

    /* initialize return code*/
    returnCode = EE_OK;

    /* Check if MCR DONE is set*/
    if (READ32(c90flRegBase) & C90FL_MCR_DONE)
    {
        /* check the operation status*/
        if(!(READ32(c90flRegBase) & C90FL_MCR_PEG))
        {
            /* high voltage operation failed*/
            returnCode = EE_ERROR_PE_OPT;
        }

        /* end the high voltage operation*/
        CLEAR32(c90flRegBase, C90FL_MCR_EHV);

        /* check for program operation*/
        if (READ32(c90flRegBase) & C90FL_MCR_PGM)
        {
            /* finish the program operation*/
            CLEAR32(c90flRegBase, C90FL_MCR_PGM);
        }
        else
        {
            /* finish the erase operation*/
            CLEAR32(c90flRegBase, C90FL_MCR_ERS);
        }

        return (returnCode);
    }

    return (EE_INFO_HVOP_INPROGRESS);
}

#if 0
uint32_t FSL_DataVerify (uint32_t c90flRegBase, bool blankCheck,
		uint32_t* dest, uint32_t* size, uint32_t* source,
		uint32_t *compareAddress, uint64_t *compareData,
		void(*CallBack)(void))
{
    uint32_t  destIndex;          /* destination address index*/
    uint32_t  counter;            /* loop counter*/
    uint32_t  suspendState;       /* suspend state*/
    uint32_t  returnCode;         /* return code*/
    uint64_t  destData;           /* dest data (double word type)*/
    uint64_t  sourceData;         /* source data (byte type)*/

    /* initialize return code*/
    returnCode = EE_OK;

    /* Clear comapreAddress and compareData arguments*/
    *compareAddress = 0;
    *compareData = 0;

    /* check high voltage operation and try to suspend it*/
    suspendState = FSL_FlashSuspend(c90flRegBase);

    /* init sourceData, default is for blank checking*/
    sourceData = CREATE_WORD64(ERASED_HIGH, ERASED_LOW);

    /* Verify data word by word*/
    for (destIndex = 0; (destIndex < DATA_VERIFY_NUMBER) && (*size > 0); destIndex += DWORD_SIZE)
    {
        /* callback service*/
        if ((destIndex % CALLBACK_VERIFY_BYTES_NUMBER) == 0)
        {
            if(NULL_CALLBACK != CallBack)
            {
               CallBack();
            }
        }

        /* get the data first*/
        destData = FSL_FlashRead64(*dest);

        /* check if it is blank check or data verfication*/
        if (!blankCheck)
        {
            /* data verification*/
            /* read source data*/
            sourceData = FSL_FlashRead64(*source);

            /* only updated source while performing data verify*/
            *source += DWORD_SIZE;
        }

        /* check if the size is not double word length*/
        if (*size < 8 )
        {
            /* fetch the correct data depending on the actual size,*/
            for (counter = 0; counter < *size; counter++)
            {
                if (*(uint8_t*)((uint32_t)(&destData) + counter) != *(uint8_t*)((uint32_t)(&sourceData) + counter))
                {
                    /* if not match, set error return code*/
                    returnCode = EE_ERROR_MISMATCH;
                    break;
                }
            }

            /* for end of the loop only*/
            *size = DWORD_SIZE;
        }
        else
        {
            /* compare the data against the Flash content*/
            if (destData != sourceData)
            {
                /* if not match, set error return code*/
                returnCode = EE_ERROR_MISMATCH;
            }
        }

        /* check return code*/
        if (returnCode == EE_ERROR_MISMATCH)
        {
            /* data mismatch*/
            *compareAddress = *dest;
            *compareData = destData;
            break;
        }

        /* update the dest*/
        *dest += DWORD_SIZE;

        /* update the size*/
        *size -= DWORD_SIZE;
    }

    /* check if it needs resume*/
    FSL_FlashResume(c90flRegBase, suspendState);

    return (returnCode);
}

uint32_t FSL_FlashSuspend (uint32_t c90flRegBase)
{
    uint32_t returnCode;          /* return code*/
    uint32_t MCRValue;            /* MCR register value*/

    /* initialize the return code*/
    returnCode = EE_OK;

    /* get the current MCR value*/
    MCRValue = READ32(c90flRegBase);

    /* no P/E sequence: PGM=0 and ERS=0*/

    /* possible stages for program sequece:*/
    /*    a. interlock write;               (PGM=1;EHV=0;  PSUS=0;DONE=1; ignore PEG)  OR (ERS=1;ESUS=1)*/
    /*    b. high voltage active;           (PGM=1;EHV=1;  PSUS=0;DONE=0; ignore PEG)  OR (ERS=1;ESUS=1)*/
    /*    c. entering suspend state;        (PGM=1;EHV=1;  PSUS=1;DONE=0; ignore PEG)  OR (ERS=1;ESUS=1)*/
    /*    d. in suspend state;              (PGM=1;EHV=1/0;PSUS=1;DONE=1; ignore PEG)  OR (ERS=1;ESUS=1)*/
    /*    e. resuming from suspend state;   (PGM=1;EHV=1;  PSUS=0;DONE=1; ignore PEG)  OR (ERS=1;ESUS=1)*/
    /*    f. high voltage stopped;          (PGM=1;EHV=1;  PSUS=0;DONE=1; valid  PEG)  OR (ERS=1;ESUS=1)*/
    /*    g. abort period.                  (PGM=1;EHV=0;  PSUS=0;DONE=0; ignore PEG)  OR (ERS=1;ESUS=1)*/

    /* possible stages for erase sequece:*/
    /*    a. interlock write;               (ERS=1;EHV=0;  ESUS=0;DONE=1; ignore PEG)  AND (PGM=0;PSUS=0)*/
    /*    b. high voltage active;           (ERS=1;EHV=1;  ESUS=0;DONE=0; ignore PEG)  AND (PGM=0;PSUS=0)*/
    /*    c. entering suspend state;        (ERS=1;EHV=1;  ESUS=1;DONE=0; ignore PEG)  AND (PGM=0;PSUS=0)*/
    /*    d. in suspend state;              (ERS=1;EHV=1/0;ESUS=1;DONE=1; ignore PEG)  AND (PGM=0;PSUS=0)*/
    /*    e. resuming from suspend state;   (ERS=1;EHV=1;  ESUS=0;DONE=1; ignore PEG)  AND (PGM=0;PSUS=0)*/
    /*    f. high voltage stopped;          (ERS=1;EHV=1;  ESUS=0;DONE=1; valid  PEG)  AND (PGM=0;PSUS=0)*/
    /*    g. abort period.                  (ERS=1;EHV=0;  ESUS=0;DONE=0; ignore PEG)  AND (PGM=0;PSUS=0)*/

    /* b. high voltage active*/
    /* e. resuming from suspend state*/
    /* f. high voltage stopped*/
    if ( (MCRValue & C90FL_MCR_EHV) &&
         ( ((MCRValue & C90FL_MCR_PGM) && !(MCRValue & C90FL_MCR_PSUS)) ||
           ((MCRValue & C90FL_MCR_ERS) && !(MCRValue & C90FL_MCR_ESUS)) ) )
    {
        if (MCRValue & C90FL_MCR_PGM)
        {
            /* program suspend*/
            C90FLMCR_EED_BIT_SET (c90flRegBase, C90FL_MCR_PSUS);   /* set MCR-PSUS bit*/
            returnCode = EE_INFO_PROGRAM_SUSPEND;
        }
        else
        {
            /* erase suspend*/
            C90FLMCR_EED_BIT_SET (c90flRegBase, C90FL_MCR_ESUS);   /* set MCR-ESUS bit*/
            returnCode = EE_INFO_ERASE_SUSPEND;
        }
    }

    /* Wait until MCR-DONE = 1*/
    while (!(READ32(c90flRegBase) & C90FL_MCR_DONE))
    {
    }

    /* Set MCR-EHV to 0*/
    C90FLMCR_EED_BIT_CLEAR (c90flRegBase, C90FL_MCR_EHV);

    return (returnCode);
}

uint32_t FSL_FlashResume (uint32_t c90flRegBase, uint32_t suspendStatus)
{
    uint32_t mask;            /* bit mask value*/
    uint32_t count;           /* loop counter*/

    /* initialize the loop counter*/
    count = FLASH_RESUME_WAIT;

    /* check the suspend state*/
    if (suspendStatus == EE_INFO_PROGRAM_SUSPEND)
    {
        /* need resume programming*/
        mask = C90FL_MCR_PSUS;
    }
    else if (suspendStatus == EE_INFO_ERASE_SUSPEND)
    {
        /* need resume erasing*/
        mask = C90FL_MCR_ESUS;
    }
    else
    {
        /* nothing to resume*/
        return (EE_OK);
    }

    /* Set MCR-EHV bit*/
    C90FLMCR_EED_BIT_SET (c90flRegBase, C90FL_MCR_EHV);

    /* clear the MCR-ESUS bit or MCR-PSUS bit*/
    C90FLMCR_EED_BIT_CLEAR (c90flRegBase, mask);

    /* wait the MCR-DONE bit goes low*/
    while ( (READ32(c90flRegBase) & C90FL_MCR_DONE) && (count--) > 0)
    {
    }

    return (EE_OK);
}

uint32_t FSL_FlashEraseAbort (uint32_t c90flRegBase)
{
    uint32_t returnCode = EE_OK;   /* return code*/

    if(READ32(c90flRegBase) & C90FL_MCR_ERS)
    {
       /* Set MCR-EHV to 0*/
       C90FLMCR_EED_BIT_CLEAR (c90flRegBase, C90FL_MCR_EHV);

       /* Wait until MCR-DONE = 1*/
       while (!(READ32(c90flRegBase) & C90FL_MCR_DONE))
       {
       }

       /* finish the erase operation*/
       CLEAR32(c90flRegBase, C90FL_MCR_ERS);
    }
    return (returnCode);
}

uint32_t FSL_FlashRead (uint32_t c90flRegBase, uint32_t* dest, uint32_t* size, uint32_t* buffer, void(*CallBack)(void))
{
    uint32_t      suspendState;   /* suspend state*/
    uint32_t      counter;        /* loop counter*/

    /* check high voltage operation*/
    suspendState = FSL_FlashSuspend(c90flRegBase);

    /* read data*/
    for (counter = 0; (*size > 0) && (counter < DATA_READ_NUMBER); counter++)
    {
        /* callback service*/
        if ((counter % CALLBACK_READ_BYTES_NUMBER) == 0)
        {
            if(NULL_CALLBACK != CallBack)
            {
               CallBack();
            }
        }

        WRITE8(*buffer, FSL_FlashRead8(*dest));
        (*dest) += 1;
        (*buffer) += 1;
        (*size) -= 1;
    }

    /* check if it needs resume*/
    FSL_FlashResume(c90flRegBase, suspendState);

    return (EE_OK);
}
//put Flash read functions in non-VLE sections
//#pragma section data_type ".text"

/*exception handler*/
void EER_exception_handler(void)
{
	if (eccErrorModule_Flag == EE_MODULE)
	{
		eccErrorStatus_Flag = true;
	}
	else
	{
	  /*do nothing*/
	}
	return;
}


/*put Flash read functions in non-VLE sections*/
/*#pragma section data_type ".text"*/
#endif
uint8_t FSL_FlashRead8(uint32_t address)
{
  return((uint8_t)(*(uint8_t*)(address)));
}

uint64_t FSL_FlashRead64(uint32_t address)
{
  return((uint64_t)(*(vuint64_t*)(address)));
}




