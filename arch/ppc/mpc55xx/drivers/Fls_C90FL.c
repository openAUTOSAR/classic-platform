
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
#include "ssd_c90fl.h"
#include <string.h>
#include <assert.h>
#include "Fls_C90FL.h"
#include "mpc55xx.h"


#define CFLASH_BASE_ADDR   (UINT32)(0xC3F88000)
#define CFLASH1_BASE_ADDR  (UINT32)(0xC3FB0000)
#define DFLASH_BASE_ADDR   (UINT32)(0xC3F8C000)
#define C1_BASE_ADDR       (UINT32)(0xC3FB0000)
#define C2_BASE_ADDR       (UINT32)(0xC3FB4000)


UINT32 FlashInit ( PSSD_CONFIG pSSDConfig )
{
    register UINT32 returnCode;     /* return code */
    UINT32 MCRAddress;              /* address of C90FLMCR register */
    UINT32 MCRValue;                /* content of C90FLMCR register */
    UINT32 temp;                    /* temporary variable */

    MCRAddress = pSSDConfig->c90flRegBase + C90FL_MCR;
    MCRValue = C90FL_REG_READ (MCRAddress);

    if(CFLASH1_BASE_ADDR == pSSDConfig->c90flRegBase)
    {
    	MCRValue = 0x02100600;
    }
    returnCode = C90FL_OK;

    /* Check MCR-EER and MCR-RWE bits */
    returnCode = (MCRValue & (C90FL_MCR_EER|C90FL_MCR_RWE)) >> 14;
    if (returnCode)
    {
        /* Clear EER and RWE bits are set in MCR register */
        C90FL_REG_BIT_SET(MCRAddress,(C90FL_MCR_EER|C90FL_MCR_RWE));
    }

    pSSDConfig->lowBlockNum = 0;
    pSSDConfig->midBlockNum = 0;
    pSSDConfig->highBlockNum = 0;

    /* Number of blocks in low address space and fill into SSD_CONFIG structure */
    temp = (MCRValue & C90FL_MCR_LAS) >> 20;

    /**************************************************************************
    *  Number of blocks in low address space and fill into SSD_CONFIG structure
    *  LAS = 0: lowBlockNum = 0;        Sectorization = 0
    *  LAS = 1: lowBlockNum = 2;        Sectorization = 2x128K
    *  LAS = 2: lowBlockNum = 6;        Sectorization = 32K+2x16K+2x32K+128K
	   *  LAS = 3: lowBlockNum = Res;      Sectorization = n.a
    *  LAS = 4: lowBlockNum = Res;      Sectorization = n.a
	   *  LAS = 5: lowBlockNum = Res;      Sectorization = n.a
	   *  LAS = 6: lowBlockNum = 4;        Sectorization = 4x16K
	   *  LAS = 7: lowBlockNum = 8;        Sectorization = 2x16K+2x32K+2x16K+2x64K
	   **************************************************************************/
    switch(temp)
    {
           case 1:
                pSSDConfig->lowBlockNum = 2;
           break;
           case 2:
                pSSDConfig->lowBlockNum = 6;
           break;
           case 6:
                pSSDConfig->lowBlockNum = 4;
           break;
           case 7:
                pSSDConfig->lowBlockNum = 8;
           break;
    }

	   /* Find main array size and fill into SSD_CONFIG structure */
	   temp = (MCRValue & C90FL_MCR_SIZE) >> 24;

	   /**************************************************************************
	   *  Get Main array size and fill into SSD_CONFIG structure
	   *  Size = 0: Main Array Size = 128KB;
	   *  Size = 1: Main Array Size = 256KB;
	   *  Size = 2: Main Array Size = 512KB;
	   *  Size = 3: Main Array Size = 1MB;
	   *  Size = 4: Main Array Size = 1.5MB;
	   *  Size = 5: Main Array Size = 2MB;
	   *  Size = 6: Main Array Size = 64KB;
	   *  Size = 7: Main Array Size = Reserved;
	   **************************************************************************/
    switch(temp)
	   {
    /*  case 0:
	        temp = 0x20000;
	        break;*/
	     case 1:
	        temp = 0x40000;
	        break;
	     case 2:
	        temp = 0x80000;
	        break;
	     case 3:
	        temp = 0x100000;
	        break;
	     case 4:
	        temp = 0x180000;
	        break;
	  /* case 5:
	        temp = 0x200000;
	        break; */
	     case 6:
	        temp = 0x10000;
	        break;
    }

    /*update Main Array size in SSD config*/
    pSSDConfig->mainArraySize = temp;

    /**************************************************************************
    *  Number of blocks in middle address space and fill into SSD_CONFIG structure
    *
    *  C Flash
    *  MAS = 0: midBlockNum = 2;    Sectorization = 2x128K
	   *  MAS = 1: midBlockNum = Res;  Sectorization = n.a
    *
    *  D Flash
    *  MAS = 0: midBlockNum = 0;    Sectorization = 0
    *  MAS = 1: midBlockNum = Res;  Sectorization = n.a
    *
	   **************************************************************************/

    /* Determine the number of blocks in middle address space and fill into
       SSD_CONFIG structure.
    */
	   if ((UINT32)0x0 == (MCRValue & C90FL_MCR_MAS))
	   {
        if(CFLASH_BASE_ADDR == pSSDConfig->c90flRegBase)
        {
	          pSSDConfig->midBlockNum = 2;
        }

        if(DFLASH_BASE_ADDR == pSSDConfig->c90flRegBase)
        {
           pSSDConfig->midBlockNum = 0;
        }
	   }

    if ( temp > (UINT32)0x00080000 )
    {
        /* (mainArraySize - 512K) / 128K */
        pSSDConfig->highBlockNum = (temp - (UINT32)0x00080000) >> 17;
    }

#if 0
    /* For C1 and C2 flash array all blocks below to high address space !!!*/
    if((C1_BASE_ADDR == pSSDConfig->c90flRegBase) ||
       (C2_BASE_ADDR == pSSDConfig->c90flRegBase))
    {
        pSSDConfig->lowBlockNum = 0;
        pSSDConfig->midBlockNum = 0;
        pSSDConfig->highBlockNum = 4;
    }
#endif

    if (pSSDConfig->BDMEnable)
    {
        //asm ( "mr   r3,returnCode" );   /* save the return code to R3 */
        #ifdef VLE_ASM
        	asm ( "se_sc " );                /* generate system call interrupt */
        #else
        	asm ( "sc " );
        #endif
    }

    return returnCode;
}


#define ERASE_STATE_START		0
#define ERASE_STATE_ERASING 	1


UINT32 Fls_C90FL_FlashErase ( PSSD_CONFIG pSSDConfig,
                    BOOL shadowFlag,
                    UINT32 lowEnabledBlocks,
                    UINT32 midEnabledBlocks,
                    UINT32 highEnabledBlocks,
                    Fls_EraseInfoType *eraseInfo)
{
    register UINT32 returnCode;     /* return code */
    UINT32 c90flRegBase;              /* base address of C90FL registers */
    UINT32 MCRAddress;              /* address of C90FLMCR register */
    UINT32 LMSAddress;              /* address of C90FLLMS register */
    UINT32 MCRValue;                /* content of C90FLMCR register */
    UINT32 interlockWriteAddress;   /* interlock write address */


    if( eraseInfo->state == ERASE_STATE_ERASING ) {
      UINT32 status;
      status = Fls_C90FL_EraseStatus(pSSDConfig);
      if( status == C90FL_OK )
      	eraseInfo->state = ERASE_STATE_START;

    	return status;
    }


    c90flRegBase = pSSDConfig->c90flRegBase;
    MCRAddress = c90flRegBase + C90FL_MCR;
    MCRValue = C90FL_REG_READ (MCRAddress);
    LMSAddress = c90flRegBase + C90FL_LMS;

    /* program and/or erase operation in progress */
    if (MCRValue & (C90FL_MCR_PGM | C90FL_MCR_ERS))
    {
        returnCode = C90FL_ERROR_BUSY;
        goto EXIT;
    }

    /* interlock write address: shadow row block key address */
    /* it will be modified to mainArrayBase in case of erasing main array */
    if((UINT32)0x0 != pSSDConfig->shadowRowBase)
    {
       interlockWriteAddress = pSSDConfig->shadowRowBase + 0xE0;
    }

    /* initialize returnCode */
    returnCode = C90FL_OK;

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
        C90FL_REG_WRITE (LMSAddress, lowEnabledBlocks);
        C90FL_REG_WRITE (c90flRegBase + C90FL_HBS, highEnabledBlocks);
    }
    else
    {
        if((UINT32)0x0 == pSSDConfig->shadowRowBase)
        {
            /* Shadow block erase has been requested but there is no shadow block */
            goto EXIT;
        }
    }

    /* set MCR-ERS to start erase operation */
    C90FLMCR_BIT_SET (MCRAddress, C90FL_MCR_ERS);

    /* interlock write */
    *( (UINT32 *)interlockWriteAddress ) = 0xFFFFFFFF;

    /* write a 1 to MCR-EHV */
    C90FLMCR_BIT_SET (MCRAddress, C90FL_MCR_EHV);

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



UINT32 Fls_C90FL_EraseStatus (  PSSD_CONFIG pSSDConfig  )
{

  UINT32 c90flRegBase;              /* base address of C90FL registers */
  UINT32 MCRAddress;              /* address of C90FLMCR register */
  UINT32 pfb_cr_val;				/* value of PFB_CR register */
  UINT32 returnCode;

  c90flRegBase = pSSDConfig->c90flRegBase;
  MCRAddress = c90flRegBase + C90FL_MCR;

  returnCode = C90FL_OK;

    /* wait until MCR-DONE set */
  if( !(C90FL_REG_READ (MCRAddress) & C90FL_MCR_DONE) )
  {
    return 0x1000;		// Ehh, busy
  }

    /* clear MCR-EHV bit */
  C90FLMCR_BIT_CLEAR (MCRAddress, C90FL_MCR_EHV);

    /* confirm MCR-PEG = 1 */
    if ( !(C90FL_REG_READ (MCRAddress) & C90FL_MCR_PEG) )
    {
        returnCode = C90FL_ERROR_EGOOD;
    }

    if(DFLASH_BASE_ADDR != pSSDConfig->c90flRegBase)   /* CFLASH */
    {
       /* save PFB_CR */
       pfb_cr_val = C90FL_REG_READ(CFLASH_BASE_ADDR + PFB_CR);

       /* Invalidate the PFBIU line read buffer */
       C90FL_REG_BIT_CLEAR (CFLASH_BASE_ADDR + PFB_CR, PFB_CR_BFEN);

       /* clear MCR-ERS bit */
       C90FLMCR_BIT_CLEAR (MCRAddress, C90FL_MCR_ERS);

       /* restore PFB_CR */
       C90FL_REG_WRITE(CFLASH_BASE_ADDR + PFB_CR, pfb_cr_val);
    }

    if(DFLASH_BASE_ADDR == pSSDConfig->c90flRegBase)  /* DFLASH */
    {
       /* save PFB_CR1 */
       pfb_cr_val = C90FL_REG_READ(CFLASH_BASE_ADDR + PFB_CR1);

       /* Invalidate the PFBIU holding register */
       C90FL_REG_BIT_CLEAR (CFLASH_BASE_ADDR + PFB_CR1, PFB_CR_BFEN);

       /* clear MCR-ERS bit */
       C90FLMCR_BIT_CLEAR (MCRAddress, C90FL_MCR_ERS);

       /* restore PFB_CR1 */
       C90FL_REG_WRITE(CFLASH_BASE_ADDR + PFB_CR1, pfb_cr_val);
    }
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

static UINT32 Fls_C90FL_ProgramPage( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo );
static UINT32 Fls_C90FL_ProgramStatus ( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo );
static UINT32 Fls_C90FL_ProgramInit( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo );

UINT32 Fls_C90FL_Program ( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo )
{
    UINT32 returnCode;     /* return code */

    switch( pInfo->state ) {
    case STATE_INIT:
      returnCode = Fls_C90FL_ProgramInit(pSSDConfig,pInfo);
      if( returnCode == C90FL_OK ) {
        returnCode = Fls_C90FL_ProgramPage(pSSDConfig,pInfo);
      }
      pInfo->state = STATE_PROGRAMMING;
      break;
    case STATE_PROGRAMMING:
      returnCode = Fls_C90FL_ProgramStatus(pSSDConfig,pInfo);
      break;
    default:
      returnCode = 0;
      assert(0);
      break;
    }

    return returnCode;
}

static UINT32 Fls_C90FL_ProgramInit( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo ) {


  UINT32 returnCode;
  //    UINT32 sourceIndex;             /* source address index */
      UINT32 MCRAddress;              /* address of C90FLMCR register */
      UINT32 MCRValue;                /* content of C90FLMCR register */

      UINT32 shadowRowEnd;            /* shadow row base + shadow size */
      UINT32 mainArrayEnd;            /* main array base + main array size */
  //    UINT32 temp;                    /* dest + size, or size / C90FL_DWORD_SIZE */
  //    UINT32 pageSize;                /* page size depending on flash type */

      UINT8 rangeType;                /* position of the program memory range */
                                      /* rangeType = 1    -    shadow row */
                                      /* rangeType = 2    -    main array */

  //	UINT32 pfb_cr_val;				/* value of PFB_CR register */
    returnCode = C90FL_OK;
    pInfo->pageSize = 16;                  /* default setting is 16 bytes */
    MCRAddress = pSSDConfig->c90flRegBase + C90FL_MCR;

    /* Check alignments */
    if ( (((pInfo->dest | pInfo->size) % C90FL_DWORD_SIZE) != 0) ||
         ((pInfo->source % C90FL_WORD_SIZE) != 0))
    {
        returnCode = C90FL_ERROR_ALIGNMENT;
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
        returnCode = C90FL_ERROR_RANGE;
        goto EXIT;
    }

    /* Anything to program? */
    if ( !pInfo->size )
        goto EXIT;

    MCRValue = C90FL_REG_READ (MCRAddress);

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

     /* Check if program and/or erase operation in progress */
    if (MCRValue & (C90FL_MCR_PGM | C90FL_MCR_ERS | C90FL_MCR_ESUS))
    {
        returnCode = C90FL_ERROR_BUSY;
        goto EXIT;
    }

    /* Check MCR-EER and MCR-RWE bit */
    if (MCRValue & (C90FL_MCR_EER | C90FL_MCR_RWE))
    {
        /* use shadow row to release bus error */
        pInfo->temp = *(VUINT32 *)(pSSDConfig->shadowRowBase + 0xE0);
    }

    /* Set MCR-PGM to start program operation */
    C90FLMCR_BIT_SET (MCRAddress, C90FL_MCR_PGM);

    /* number of double words */
    pInfo->temp = pInfo->size / C90FL_DWORD_SIZE;

    /* determine the page size */
    if (pSSDConfig->pageSize == C90FL_PAGE_SIZE_32)
    {
    	pInfo->pageSize = (UINT32)32;
    }
    else if(pSSDConfig->pageSize == C90FL_PAGE_SIZE_16)
    {
    	pInfo->pageSize = (UINT32)16;
    }
    else if(pSSDConfig->pageSize == C90FL_PAGE_SIZE_08)
    {
    	pInfo->pageSize = (UINT32)8;
    }

    pInfo->sourceIndex = 1;
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

static UINT32 Fls_C90FL_ProgramPage( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo ) {

  UINT32 returnCode = 0;

  UINT32 MCRAddress = pSSDConfig->c90flRegBase + C90FL_MCR;

    /* Program data page by page, with special attention to incomplete page */
  if( pInfo->sourceIndex <= pInfo->temp ) {
      /* Programming write */
      *(UINT64 *)pInfo->dest = *(UINT64 *)pInfo->source;

      /* Update pInfo->source index */
      pInfo->dest += C90FL_DWORD_SIZE;
      pInfo->source += C90FL_DWORD_SIZE;

      /* Is it time to do page programming?  */
      if ( ((pInfo->dest % pInfo->pageSize) == 0) || (pInfo->sourceIndex == pInfo->temp) )
      {
          /* Set MCR-EHV bit */
          C90FLMCR_BIT_SET (MCRAddress, C90FL_MCR_EHV);
      }
      returnCode = C90FL_BUSY;
  }

  return returnCode;
}


/**
 * Returns the status of the flash programming
 *
 */

UINT32 Fls_C90FL_ProgramStatus ( PSSD_CONFIG pSSDConfig, Fls_ProgInfoType *pInfo ) {

    UINT32 MCRAddress;              /* address of C90FLMCR register */
    UINT32 pfb_cr_val;
    UINT32 returnCode = C90FL_OK;

    MCRAddress = pSSDConfig->c90flRegBase + C90FL_MCR;

    /* Wait until MCR-DONE set */
    if ( !(C90FL_REG_READ (MCRAddress) & C90FL_MCR_DONE) )
    {
      return C90FL_BUSY;
    }

    /* Confirm MCR-PEG = 1 */
    if ( !(C90FL_REG_READ (MCRAddress) & C90FL_MCR_PEG) )
    {
        /* Clear MCR-EHV bit */
    	C90FLMCR_BIT_CLEAR (MCRAddress, C90FL_MCR_EHV);

        returnCode = C90FL_ERROR_PGOOD;
        goto EXIT_EHV;
    }

    /* Clear MCR-EHV bit */
    C90FLMCR_BIT_CLEAR (MCRAddress, C90FL_MCR_EHV);
    pInfo->sourceIndex++;

    returnCode = Fls_C90FL_ProgramPage( pSSDConfig,pInfo );
    if( (returnCode) == C90FL_BUSY ) {
      return C90FL_BUSY;
    }

EXIT_EHV:

  /* Clear MCR-PGM bit */
  C90FLMCR_BIT_CLEAR (MCRAddress, C90FL_MCR_PGM);

  if(DFLASH_BASE_ADDR != pSSDConfig->c90flRegBase)   /* CFLASH */
  {
     /* save PFB_CR */
     pfb_cr_val = C90FL_REG_READ(CFLASH_BASE_ADDR + PFB_CR);

     /* Invalidate the PFBIU line read buffer */
     C90FL_REG_BIT_CLEAR (CFLASH_BASE_ADDR + PFB_CR, PFB_CR_BFEN);

     /* restore PFB_CR */
     C90FL_REG_WRITE(CFLASH_BASE_ADDR + PFB_CR, pfb_cr_val);
  }

  if(DFLASH_BASE_ADDR == pSSDConfig->c90flRegBase)   /* DFLASH */
  {
     /* save PFB_CR1 */
     pfb_cr_val = C90FL_REG_READ(CFLASH_BASE_ADDR + PFB_CR1);

     /* Invalidate the PFBIU holding register */
     C90FL_REG_BIT_CLEAR (CFLASH_BASE_ADDR + PFB_CR1, PFB_CR_BFEN);

     /* restore PFB_CR1 */
     C90FL_REG_WRITE(CFLASH_BASE_ADDR + PFB_CR1, pfb_cr_val);
  }

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
void Fls_C90FL_SetLock ( PSSD_CONFIG pSSDConfig, Fls_EraseBlockType *blocks, UINT8 logic )
{
    vuint32_t *reg, *sreg;
    //struct FLASH_tag *flashHw = &CFLASH0;

    if( (blocks->lowEnabledBlocks != 0 ) ||
    		(blocks->midEnabledBlocks != 0 ) ||
    		(blocks->shadowBlocks != 0 ) )
    {
      if(CFLASH_BASE_ADDR == pSSDConfig->c90flRegBase)
      {
		  reg 	= &(CFLASH0.LML.R);
		  sreg	= &(CFLASH0.SLL.R);
      }
      else if(CFLASH1_BASE_ADDR == pSSDConfig->c90flRegBase)
      {
		  reg 	= &(CFLASH1.LML.R);
		  sreg	= &(CFLASH1.SLL.R);
      }
      else if(DFLASH_BASE_ADDR == pSSDConfig->c90flRegBase)
      {
		  reg 	= &(DFLASH.LML.R);
		  sreg	= &(DFLASH.SLL.R);
      }
      else
      {
    	  return;
      }

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
      	*reg |= ((blocks->midEnabledBlocks<<16)& C90FL_LML_MLOCK) +
								(blocks->lowEnabledBlocks & C90FL_LML_LLOCK) +
								(blocks->shadowBlocks & C90FL_LML_SLOCK);
      } else {
      	*reg &= ((~blocks->midEnabledBlocks<<16) & C90FL_LML_MLOCK) |
								((~blocks->lowEnabledBlocks) & C90FL_LML_LLOCK) |
								((~blocks->shadowBlocks) & C90FL_LML_SLOCK) ;
      }
      if( logic ) {
      	*sreg |= ((blocks->midEnabledBlocks<<16)& C90FL_LML_MLOCK) +
								(blocks->lowEnabledBlocks & C90FL_LML_LLOCK) +
								(blocks->shadowBlocks & C90FL_LML_SLOCK);
      } else {
      	*sreg &= ((~blocks->midEnabledBlocks<<16) & C90FL_LML_MLOCK) |
								((~blocks->lowEnabledBlocks) & C90FL_LML_LLOCK) |
								((~blocks->shadowBlocks) & C90FL_LML_SLOCK) ;
      }
    }
#if 0
    else {
      reg = &(flashHw->LML.R);
      // Set all
      *reg |= 0x8003003f;
    }
#endif

    if( (blocks->highEnabledBlocks != 0 )) {

      if(CFLASH_BASE_ADDR == pSSDConfig->c90flRegBase)
      {
		  reg 	= &(CFLASH0.HBL.R);
      }
      else if(CFLASH1_BASE_ADDR == pSSDConfig->c90flRegBase)
      {
		  reg 	= &(CFLASH1.HBL.R);
      }
      else if(DFLASH_BASE_ADDR == pSSDConfig->c90flRegBase)
      {
		  reg 	= &(DFLASH.HBL.R);
      }
      else
      {
    	  return;
      }

      if( !(*reg & 0x80000000 )) {
        // Unlock
        *reg = FLASH_HLR_PASSWORD;
      }
      // clear
      // *reg &= ~(C90FL_HBL_HBLOCK);
      // Set
      if( logic ) {
      	*reg |= ((blocks->highEnabledBlocks)& C90FL_HBL_HBLOCK);
      } else {
      	*reg &= ((~blocks->highEnabledBlocks)& C90FL_HBL_HBLOCK);
      }
    }
#if 0
    else {
      reg = &(flashHw->HBR.R);
      // Set all
      *reg |= 0x800000ff;
    }
#endif
}



