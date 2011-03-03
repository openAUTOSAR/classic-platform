
/*************************************************************************
 *            (c) Copyright Motorola 2005, All Rights Reserved           *
 *************************************************************************
 *                                                                       *
 *   Motorola reserves the right to make changes without further notice  *
 *   to any product herein to improve reliability, function or design.   *
 *   Motorola does not assume any liability arising out of the           *
 *   application or use of any product, circuit, or software described   *
 *   herein; neither does it convey any license under its patent rights  *
 *   nor the rights of others.                                           *
 *                                                                       *
 *   Motorola products are not designed, intended, or authorized for     *
 *   use as components in systems intended for surgical implant into     *
 *   the body, or other applications intended to support life, or for    *
 *   any other application in which the failure of the Motorola product  *
 *   could create a situation where personal injury or death may occur.  *
 *                                                                       *
 *   Should Buyer purchase or use Motorola products for any such         *
 *   unintended or unauthorized application, Buyer shall indemnify and   *
 *   hold Motorola and its officers, employees, subsidiaries,            *
 *   affiliates, and distributors harmless against all claims costs,     *
 *   damages, and expenses, and reasonable attorney fees arising out     *
 *   of, directly or indirectly, any claim of personal injury or death   *
 *   associated with such unintended or unauthorized use, even if such   *
 *   claim alleges that Motorola was negligent regarding the design      *
 *   or manufacture of the part.                                         *
 *                                                                       *
 *   Motorola and the Motorola logo* are registered trademarks of        *
 *   Motorola Ltd.                                                       *
 *                                                                       *
 *************************************************************************/

/* This file is a a copy and modification of a freescale driver */

#include "ssd_types.h"
#include "ssd_h7f.h"
#include <string.h>
#include <assert.h>
#include "Fls_H7F.h"
#include "mpc55xx.h"


UINT32 FlashInit ( PSSD_CONFIG pSSDConfig )
{
    register UINT32 returnCode;     /* return code */
    UINT32 MCRAddress;              /* address of H7FMCR register */
    UINT32 MCRValue;                /* content of H7FMCR register */
    UINT32 temp;                    /* temporary variable */

    MCRAddress = pSSDConfig->h7fRegBase + H7F_MCR;
    MCRValue = H7F_REG_READ (MCRAddress);

    /* If MCR-STOP = 1, return directly with error code */
    if (MCRValue & H7F_MCR_STOP)
    {
        returnCode = H7F_ERROR_STOP;
        goto EXIT;
    }
#if 0
    /* read and check the MASK */
    /* use returnCode temporarily */
    returnCode = H7F_REG_READ (SIU_MIDR);

    if ((returnCode >> 16) == 0x5554)
    {
        /* if MPC5554 part, check the mask number */
        if ( !((UINT16)returnCode > 3) &&
             !(((UINT16)returnCode == 3) &&
               ((H7F_REG_READ (pSSDConfig->shadowRowBase + 0xC8) == 0x4C4A4F4E) ||
                (H7F_REG_READ (pSSDConfig->shadowRowBase + 0xE8) == 0x4C4A4F4E))))
        {
            returnCode = H7F_ERROR_PARTID;
            goto EXIT;
        }
    }

    if (returnCode == 0x55340000)
    {
        /* cannot work for MPC5534 rev0 silicon */
        returnCode = H7F_ERROR_PARTID;
        goto EXIT;
    }
#endif
    /* Check MCR-EER and MCR-RWE bits */
    returnCode = (MCRValue & (H7F_MCR_EER|H7F_MCR_RWE)) >> 14;
    if (returnCode)
    {
        /* read the shadow row keyword to release the bus transfer error */
        /* this is a software workaround for some H7F revision. */
        temp = *(VUINT32 *)(pSSDConfig->shadowRowBase + 0xE0);
    }

    /* Check MCR-BBEPE and MCR-EPE bits */
    returnCode |= ((~MCRValue) & (H7F_MCR_BBEPE|H7F_MCR_EPE)) >> 10;

    pSSDConfig->midBlockNum = 0;
    pSSDConfig->highBlockNum = 0;

    /* Number of blocks in low address space and fill into SSD_CONFIG structure */
    temp = (MCRValue & H7F_MCR_LAS) >> 20;

    /* Number of blocks in low address space and fill into SSD_CONFIG structure
     *  LAS = 0: lowBlockNum = 2;
     *  LAS = 1: lowBlockNum = 4;
     *  LAS = 2: lowBlockNum = 8;
     *  LAS = 3: lowBlockNum = 16;
     *  LAS = 4: lowBlockNum = 10;
     *  LAS = 5: lowBlockNum = 12;
     *  LAS = 6: lowBlockNum = 6;
     *******************************************************************************
     *  Special configurations
     *  SFS = 1, SIZE = 0, LAS = 2, MAS = 0: lowBlockNum = 4 and Flash size = 128KB;
     *  SFS = 1, SIZE = 1, LAS = *, MAS = 1: midBlockNum = 4 and Flash size = 320KB;
     */

    if (temp < 4)
    {
        pSSDConfig->lowBlockNum = ((UINT32)0x00000002) << temp;
    }
    else if (4 == temp)
    {
        pSSDConfig->lowBlockNum = 10;
    }
    else if (5 == temp)
    {
        pSSDConfig->lowBlockNum = 12;
    }
    else if (6 == temp)
    {
        pSSDConfig->lowBlockNum = 6;
    }

   /* Check if SFS bit is set to 1 */
    if (MCRValue & H7F_MCR_SFS)
    {
        if (MCRValue & H7F_MCR_SIZE) /* SIZE = 1 */
        {
            pSSDConfig->mainArraySize = 0x00050000;   /* Flash size is 320KB */
            pSSDConfig->midBlockNum = 4; /* Mid address space: 4-16KB blocks */
        }
        else /* SIZE = 0 */
        {
            pSSDConfig->mainArraySize = 0x00020000;   /* Flash size is 128KB */
            /* Low address space: 2-16KB blocks and 2-48KB blocks */
            pSSDConfig->lowBlockNum = 4;
        }
    }
    else
    {
        /* Main array space size */
        temp = 0x00040000 * ( ((MCRValue & H7F_MCR_SIZE) >> 24) + 1);
        pSSDConfig->mainArraySize = temp;

        /* Determine the number of blocks in middle address space and fill into SSD_CONFIG structure */
        if ( temp > 0x00040000)
        {
            pSSDConfig->midBlockNum = 2 * (((MCRValue & H7F_MCR_MAS) >> 16) + 1);
        }

        /* Determine the number of blocks in high address space and fill into SSD_CONFIG structure */
        if ( temp > 0x00080000 )
        {
            /* (mainArraySize - 512K) / 128K */
            pSSDConfig->highBlockNum = (temp - 0x00080000) >> 17;
        }
    }

EXIT:
    if (pSSDConfig->BDMEnable)
    {
        //asm ( "mr   r3,returnCode" );   /* save the return code to R3 */
        asm ( "sc " );                  /* generate system call interrupt */
    }

    return returnCode;
}


#define ERASE_STATE_START		0
#define ERASE_STATE_ERASING 	1


UINT32 Fls_H7F_FlashErase ( PSSD_CONFIG pSSDConfig,
                    BOOL shadowFlag,
                    UINT32 lowEnabledBlocks,
                    UINT32 midEnabledBlocks,
                    UINT32 highEnabledBlocks,
                    Fls_EraseInfoType *eraseInfo)
{
    register UINT32 returnCode;     /* return code */
    UINT32 h7fRegBase;              /* base address of H7F registers */
    UINT32 MCRAddress;              /* address of H7FMCR register */
    UINT32 LMSAddress;              /* address of H7FLMS register */
    UINT32 MCRValue;                /* content of H7FMCR register */
    UINT32 interlockWriteAddress;   /* interlock write address */


    if( eraseInfo->state == ERASE_STATE_ERASING ) {
      UINT32 status;
      status = Fls_H7F_EraseStatus(pSSDConfig);
      if( status == H7F_OK )
      	eraseInfo->state = ERASE_STATE_START;

    	return status;
    }


    h7fRegBase = pSSDConfig->h7fRegBase;
    MCRAddress = h7fRegBase + H7F_MCR;
    MCRValue = H7F_REG_READ (MCRAddress);
    LMSAddress = h7fRegBase + H7F_LMS;

    /* program and/or erase operation in progress */
    if (MCRValue & (H7F_MCR_PGM | H7F_MCR_ERS))
    {
        returnCode = H7F_ERROR_BUSY;
        goto EXIT;
    }

    /* interlock write address: shadow row block key address */
    /* it will be modified to mainArrayBase in case of erasing main array */
    interlockWriteAddress = pSSDConfig->shadowRowBase + 0xE0;

    /* Check MCR-EER and MCR-RWE bit */
    if (MCRValue & (H7F_MCR_EER | H7F_MCR_RWE))
    {
        /* read shadow row block key address to clear bus transfer error */
        /* this is a software workaround for some H7F revision. */
        /* use returnCode temporarily */
        returnCode = *(VUINT32 *)interlockWriteAddress;
    }

    /* initialize returnCode */
    returnCode = H7F_OK;

    if (!shadowFlag)
    {
        /* erase the main array blocks */
        interlockWriteAddress = pSSDConfig->mainArrayBase;

        /* mask off reserved bits for low address space */
        lowEnabledBlocks &= 0xFFFFFFFF >> (32 - pSSDConfig->lowBlockNum);

        /* mask off reserved bits for mid address space */
        lowEnabledBlocks |= (midEnabledBlocks & (0xFFFFFFFF >> (32 - pSSDConfig->midBlockNum))) << 16;

        /* mask off reserved bits for high address space */
        highEnabledBlocks &= 0xFFFFFFFF >> (32 - pSSDConfig->highBlockNum);

        if ( !(lowEnabledBlocks | highEnabledBlocks) )
        {
            /* no blocks to be erased */
            goto EXIT;
        }

        /* set the block selection registers */
        H7F_REG_WRITE (LMSAddress, lowEnabledBlocks);
        H7F_REG_WRITE (h7fRegBase + H7F_HBS, highEnabledBlocks);
    }

    /* set MCR-ERS to start erase operation */
    H7FMCR_BIT_SET (MCRAddress, H7F_MCR_ERS);

    /* interlock write */
    *( (UINT32 *)interlockWriteAddress ) = 0xFFFFFFFF;

    /* write a 1 to MCR-EHV */
    H7FMCR_BIT_SET (MCRAddress, H7F_MCR_EHV);

    /* This is where the freescale driver ends */
    eraseInfo->state = ERASE_STATE_ERASING;

EXIT:
  if (pSSDConfig->BDMEnable)
  {
      //asm ( "mr   r3,returnCode" );   /* save the return code to R3 */
      asm ( "sc " );                  /* generate system call interrupt */
  }

    return returnCode;
}



UINT32 Fls_H7F_EraseStatus (  PSSD_CONFIG pSSDConfig  )
{

  UINT32 h7fRegBase;              /* base address of H7F registers */
  UINT32 MCRAddress;              /* address of H7FMCR register */
  UINT32 pfb_cr_val;				/* value of PFB_CR register */
  UINT32 returnCode;

  h7fRegBase = pSSDConfig->h7fRegBase;
  MCRAddress = h7fRegBase + H7F_MCR;

  returnCode = H7F_OK;

    /* wait until MCR-DONE set */
  if( !(H7F_REG_READ (MCRAddress) & H7F_MCR_DONE) )
  {
    return 0x1000;		// Ehh, busy
  }

    /* clear MCR-EHV bit */
    H7FMCR_BIT_CLEAR (MCRAddress, H7F_MCR_EHV);

    /* confirm MCR-PEG = 1 */
    if ( !(H7F_REG_READ (MCRAddress) & H7F_MCR_PEG) )
    {
        returnCode = H7F_ERROR_EGOOD;
    }

  /* save PFB_CR */
  pfb_cr_val = H7F_REG_READ(h7fRegBase + PFB_CR);

    /* invalidate the PFBIU line read buffer */
    H7F_REG_BIT_CLEAR (h7fRegBase + PFB_CR, PFB_CR_BFEN);

    /* clear MCR-ERS bit */
    H7FMCR_BIT_CLEAR (MCRAddress, H7F_MCR_ERS);

    /* invalidate the PFBIU line read buffer */
    H7F_REG_BIT_SET (h7fRegBase + PFB_CR, PFB_CR_BFEN);

  /* restore PFB_CR */
  H7F_REG_WRITE(h7fRegBase + PFB_CR, pfb_cr_val);

//EXIT:
    if (pSSDConfig->BDMEnable)
    {
        //asm ( "mr   r3,returnCode" );   /* save the return code to R3 */
        asm ( "sc " );                  /* generate system call interrupt */
    }

    return returnCode;
}


// First state.... init stuff
#define STATE_INIT			0
//
#define STATE_PROGRAMMING		 	1
//#define STATE_WAIT_DONE	2

#if 0
UINT32 FlashProgram ( PSSD_CONFIG pSSDConfig,
                      UINT32 dest,
                      UINT32 size,
                      UINT32 source,
                      void(*CallBack)(void)
                      )
#endif


#define MY_BUSY 		0x1000

static UINT32 Fls_H7F_ProgramPage( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo );
static UINT32 Fls_H7F_ProgramStatus ( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo );
static UINT32 Fls_H7F_ProgramInit( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo );

UINT32 Fls_H7F_Program ( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo )
{
    UINT32 returnCode;     /* return code */

    switch( pInfo->state ) {
    case STATE_INIT:
      returnCode = Fls_H7F_ProgramInit(pSSDConfig,pInfo);
      if( returnCode == H7F_OK ) {
        returnCode = Fls_H7F_ProgramPage(pSSDConfig,pInfo);
      }
      pInfo->state = STATE_PROGRAMMING;
      break;
    case STATE_PROGRAMMING:
      returnCode = Fls_H7F_ProgramStatus(pSSDConfig,pInfo);
      break;
    default:
      returnCode = 0;
      assert(0);
      break;
    }

    return returnCode;
}

static UINT32 Fls_H7F_ProgramInit( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo ) {


  UINT32 returnCode;
  //    UINT32 sourceIndex;             /* source address index */
      UINT32 MCRAddress;              /* address of H7FMCR register */
      UINT32 MCRValue;                /* content of H7FMCR register */

      UINT32 shadowRowEnd;            /* shadow row base + shadow size */
      UINT32 mainArrayEnd;            /* main array base + main array size */
  //    UINT32 temp;                    /* dest + size, or size / H7F_DWORD_SIZE */
  //    UINT32 pageSize;                /* page size depending on flash type */

      UINT8 rangeType;                /* position of the program memory range */
                                      /* rangeType = 1    -    shadow row */
                                      /* rangeType = 2    -    main array */

  //	UINT32 pfb_cr_val;				/* value of PFB_CR register */
    returnCode = H7F_OK;
    pInfo->pageSize = 16;                  /* default setting is 16 bytes */
    MCRAddress = pSSDConfig->h7fRegBase + H7F_MCR;

    /* Check alignments */
    if ( (((pInfo->dest | pInfo->size) % H7F_DWORD_SIZE) != 0) ||
         ((pInfo->source % H7F_WORD_SIZE) != 0))
    {
        returnCode = H7F_ERROR_ALIGNMENT;
        goto EXIT;
    }

    /* The flash range should fall within either shadow row or main array */
    shadowRowEnd = pSSDConfig->shadowRowBase + pSSDConfig->shadowRowSize;
    mainArrayEnd = pSSDConfig->mainArrayBase + pSSDConfig->mainArraySize;
    pInfo->temp = pInfo->dest + pInfo->size;

    if ((pInfo->dest >= pSSDConfig->shadowRowBase) && (pInfo->dest < shadowRowEnd) &&
        (pInfo->size <= pSSDConfig->shadowRowSize) && (pInfo->temp <= shadowRowEnd))
    {
        /* fall in shadow row */
        rangeType = 1;
    }
    else if ((pInfo->dest >= pSSDConfig->mainArrayBase) && (pInfo->dest < mainArrayEnd) &&
             (pInfo->size <= pSSDConfig->mainArraySize) && (pInfo->temp <= mainArrayEnd))
    {
        /* fall in main array */
        rangeType = 2;
    }
    else
    {
        returnCode = H7F_ERROR_RANGE;
        goto EXIT;
    }

    /* Anything to program? */
    if ( !pInfo->size )
        goto EXIT;

    MCRValue = H7F_REG_READ (MCRAddress);

    /* Cases that program operation can start:
       1. no program and erase sequence:
          (PGM low and ERS low)
       2. erase suspend with EHV low on main array and try to program main array
          (PGM low, PEAS low, ERS high, ESUS high, EHV low, and rangeType = 2)

       Cases that program operation cannot start:
       1. program in progress (PGM high);
       2. program not in progress (PGM low):
          a. erase in progress but not in suspend state;
          b. erase in suspend state on main array but try to program shadow row;
          c. erase suspend on shadow row; */

    if ( !( (!(MCRValue & (H7F_MCR_PGM | H7F_MCR_ERS))) ||
            (!(MCRValue & (H7F_MCR_PGM | H7F_MCR_PEAS | H7F_MCR_EHV)) &&
              ((MCRValue & (H7F_MCR_ERS | H7F_MCR_ESUS)) == (H7F_MCR_ERS | H7F_MCR_ESUS)) &&
              (rangeType == 2)) ) )
    {
        returnCode = H7F_ERROR_BUSY;
        goto EXIT;
    }

    /* Check MCR-EER and MCR-RWE bit */
    if (MCRValue & (H7F_MCR_EER | H7F_MCR_RWE))
    {
        /* use shadow row to release bus error */
        pInfo->temp = *(VUINT32 *)(pSSDConfig->shadowRowBase + 0xE0);
    }

    /* Set MCR-PGM to start program operation */
    H7FMCR_BIT_SET (MCRAddress, H7F_MCR_PGM);

    /* number of double words */
    pInfo->temp = pInfo->size / H7F_DWORD_SIZE;

    /* determine the page size */
    if (pSSDConfig->pageSize == H7FA_PAGE_SIZE)
    {
        /* H7Fa page size is 32 bytes */
        pInfo->pageSize = 32;
    }
    else if(pSSDConfig->pageSize == H7FB_PAGE_SIZE)
    {
        /* H7Fb page size is 16 bytes */
        /* this is also the default setting */
        /*
        pInfo->pageSize = 16;
        */
    }

    pInfo->sourceIndex = 1;
#if 0
    if( FlashProgramPage_r(pSSDConfig,pInfo) == H7F_BUSY ) {
      return H7F_BUSY; /* busy */
    }

    returnCode = FlashProgramStatus_r ( pSSDConfig, pInfo );
#endif
EXIT:
  if (pSSDConfig->BDMEnable)
  {
    //asm ( "mr   r3,returnCode" );   /* save the return code to R3 */
    asm ( "sc " );                  /* generate system call interrupt */
  }


    return returnCode;
}

/**
 * Programs a flash page. Assumes that FlashProgram_r() is called before.
 * Subsequent calls are made to this function until the programming operation is done
 *
 * @param pSSDConfig Flash configuration
 * @param pInfo Structure used by the page programmer. It's initially filled in by FlashProgram_r()
 *
 * @returns Status of the flash programming. See ssd_h7f. for more information. 0x1000 added
 * as return value when the device is busy

 */

static UINT32 Fls_H7F_ProgramPage( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo ) {

  UINT32 returnCode = 0;

  UINT32 MCRAddress = pSSDConfig->h7fRegBase + H7F_MCR;

    /* Program data page by page, with special attention to incomplete page */
  if( pInfo->sourceIndex <= pInfo->temp ) {
      /* Programming write */
      *(UINT64 *)pInfo->dest = *(UINT64 *)pInfo->source;

      /* Update pInfo->source index */
      pInfo->dest += H7F_DWORD_SIZE;
      pInfo->source += H7F_DWORD_SIZE;

      /* Is it time to do page programming?  */
      if ( ((pInfo->dest % pInfo->pageSize) == 0) || (pInfo->sourceIndex == pInfo->temp) )
      {
          /* Set MCR-EHV bit */
          H7FMCR_BIT_SET (MCRAddress, H7F_MCR_EHV);
      }
      returnCode = H7F_BUSY;
  }

  return returnCode;
}


/**
 * Returns the status of the flash programming
 *
 */

UINT32 Fls_H7F_ProgramStatus ( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo ) {

    UINT32 MCRAddress;              /* address of H7FMCR register */
    UINT32 pfb_cr_val;
    UINT32 returnCode = H7F_OK;

    MCRAddress = pSSDConfig->h7fRegBase + H7F_MCR;

    /* Wait until MCR-DONE set */
    if ( !(H7F_REG_READ (MCRAddress) & H7F_MCR_DONE) )
    {
      return H7F_BUSY;
    }

    /* Confirm MCR-PEG = 1 */
    if ( !(H7F_REG_READ (MCRAddress) & H7F_MCR_PEG) )
    {
        /* Clear MCR-EHV bit */
        H7FMCR_BIT_CLEAR (MCRAddress, H7F_MCR_EHV);

        returnCode = H7F_ERROR_PGOOD;
        goto EXIT_EHV;
    }

    /* Clear MCR-EHV bit */
    H7FMCR_BIT_CLEAR (MCRAddress, H7F_MCR_EHV);
    pInfo->sourceIndex++;

    returnCode = Fls_H7F_ProgramPage( pSSDConfig,pInfo );
    if( (returnCode) == H7F_BUSY ) {
      return H7F_BUSY;
    }

EXIT_EHV:

  /* Clear MCR-PGM bit */
  H7FMCR_BIT_CLEAR (MCRAddress, H7F_MCR_PGM);

  /* save PFB_CR */
  pfb_cr_val = H7F_REG_READ(pSSDConfig->h7fRegBase + PFB_CR);

  /* Invalidate the PFBIU line read buffer */
  H7F_REG_BIT_CLEAR (pSSDConfig->h7fRegBase + PFB_CR, PFB_CR_BFEN);

  /* Invalidate the PFBIU line read buffer */
  H7F_REG_BIT_SET (pSSDConfig->h7fRegBase + PFB_CR, PFB_CR_BFEN);

  /* restore PFB_CR */
  H7F_REG_WRITE(pSSDConfig->h7fRegBase + PFB_CR, pfb_cr_val);

  // Clear our struct....
  memset( pInfo,0x0,sizeof(Fls_ProgInfoType) );

  return returnCode;
}

#define FLASH_LMLR_PASSWORD             0xA1A11111  /* Low/Mid address lock enabled password */
#define FLASH_HLR_PASSWORD              0xB2B22222  /* High address lock enabled password */
#define FLASH_SLMLR_PASSWORD            0xC3C33333  /* Secondary low and middle address lock enabled password */

/* TODO: use PSSD_CONFIG ( regbase, etc instead of hardcoded FLASH ) */


/**
 * Function that handles the locks bits the flash. Handled bits
 * are LLOCK, MLOCK, SLOCK, HLOCK
 * Secondary locks are NOT supported.
 *
 * @param blocks - Blocks to set lock or unlock.
 * @param logic - A '1' interpretes 1 in blocks as lock, A '0' as unlock
 */
void Fls_H7F_SetLock ( Fls_EraseBlockType *blocks, UINT8 logic )
{
    vuint32_t *reg, *sreg;
    struct FLASH_tag *flashHw = &FLASH;

    if( (blocks->lowEnabledBlocks != 0 ) ||
    		(blocks->midEnabledBlocks != 0 ) ||
    		(blocks->shadowBlocks != 0 ) )
    {
      reg 	= &(flashHw->LMLR.R);
      sreg	= &(flashHw->SLMLR.R);

      // Check if sector is locked
      if( !(*reg & 0x80000000 )) {
              // Unlock the sector with password
              *reg = FLASH_LMLR_PASSWORD;
            }
      if( !(*sreg & 0x80000000 )) {
              // Unlock the sector with password
              *sreg = FLASH_SLMLR_PASSWORD;
            }

      // set/clear them
      if( logic ) {
      	*reg |= ((blocks->midEnabledBlocks<<16)& H7F_LML_MLOCK) +
								(blocks->lowEnabledBlocks & H7F_LML_LLOCK) +
								(blocks->shadowBlocks & H7F_LML_SLOCK);
      } else {
      	*reg &= ((~blocks->midEnabledBlocks<<16) & H7F_LML_MLOCK) |
								((~blocks->lowEnabledBlocks) & H7F_LML_LLOCK) |
								((~blocks->shadowBlocks) & H7F_LML_SLOCK) ;
      }
      if( logic ) {
      	*sreg |= ((blocks->midEnabledBlocks<<16)& H7F_LML_MLOCK) +
								(blocks->lowEnabledBlocks & H7F_LML_LLOCK) +
								(blocks->shadowBlocks & H7F_LML_SLOCK);
      } else {
      	*sreg &= ((~blocks->midEnabledBlocks<<16) & H7F_LML_MLOCK) |
								((~blocks->lowEnabledBlocks) & H7F_LML_LLOCK) |
								((~blocks->shadowBlocks) & H7F_LML_SLOCK) ;
      }
    }
#if 0
    else {
      reg = &(flashHw->LMLR.R);
      // Set all
      *reg |= 0x8003003f;
    }
#endif

    if( (blocks->highEnabledBlocks != 0 )) {
      reg = &(flashHw->HLR.R);

      if( !(*reg & 0x80000000 )) {
        // Unlock
        *reg = FLASH_HLR_PASSWORD;
      }
      // clear
      // *reg &= ~(H7F_HBL_HBLOCK);
      // Set
      if( logic ) {
      	*reg |= ((blocks->highEnabledBlocks)& H7F_HBL_HBLOCK);
      } else {
      	*reg &= ((~blocks->highEnabledBlocks)& H7F_HBL_HBLOCK);
      }
    }
#if 0
    else {
      reg = &(flashHw->HLR.R);
      // Set all
      *reg |= 0x800000ff;
    }
#endif
}



