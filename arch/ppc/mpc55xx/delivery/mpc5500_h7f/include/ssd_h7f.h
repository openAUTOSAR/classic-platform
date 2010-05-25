
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
 *************************************************************************

 *************************************************************************
 *                                                                       *
 *               Standard Software H7F Driver for MPC55xx                *
 *                                                                       *
 * FILE NAME     :  ssd_h7f.h                                            *
 * DATE          :  June 23, 2005                                        *
 *                                                                       *
 * AUTHOR        :  Flash Team,                                          *
 *                  Global Software Group, China, Motorola Inc.          *
 * E-mail        :  flash@sc.mcel.mot.com                                *
 *                                                                       *
 *************************************************************************/

/******************************* CHANGES *********************************
 3.20   2005.06.23       Cloud Li        Initial Version
 *************************************************************************/

#ifndef _SSD_H7F_H_
#define _SSD_H7F_H_

/*************************************************************************/
/*     Offsets of H7F Control Registers and PFBIU Control Registers     */
/*************************************************************************/
#define H7F_MCR             0x0000       /* Module Configuration Register */
#define H7F_LML             0x0004       /* Low/Mid Address Space Block Locking Register */
#define H7F_HBL             0x0008       /* High Address Space Block Locking Register */
#define H7F_SLL             0x000C       /* Secondary Low/Mid Address Space Block Locking Register */
#define H7F_LMS             0x0010       /* Low/Mid Address Space Block Select Register */
#define H7F_HBS             0x0014       /* High Address Space Block Select Register */
#define H7F_ADR             0x0018       /* Address Register */
#define PFB_CR              0x001C       /* PFBIU Configuration Register */

/*************************************************************************/
/*              H7F Module Control Registers Field Definition           */
/*************************************************************************/
/* Module Configuration Register */
#define H7F_MCR_SFS         0x10000000   /* Special Flash Selector */
#define H7F_MCR_SIZE        0x0F000000   /* Array Space Size */
#define H7F_MCR_LAS         0x00700000   /* Low Address Space Size */
#define H7F_MCR_MAS         0x00010000   /* Mid Address Space Size */
#define H7F_MCR_EER         0x00008000   /* ECC Event Error */
#define H7F_MCR_RWE         0x00004000   /* Read While Write Event Error */
#define H7F_MCR_BBEPE       0x00002000   /* Boot Block External Program Erase Status */
#define H7F_MCR_EPE         0x00001000   /* External Program Erase Status */
#define H7F_MCR_PEAS        0x00000800   /* Program/Erase Access Space */
#define H7F_MCR_DONE        0x00000400   /* State Machine Status */
#define H7F_MCR_PEG         0x00000200   /* Program/Erase Good */
#define H7F_MCR_PRD         0x00000080   /* Pipelined Reads Disabled */
#define H7F_MCR_STOP        0x00000040   /* Stop Mode Enabled */
#define H7F_MCR_PGM         0x00000010   /* Program */
#define H7F_MCR_PSUS        0x00000008   /* Program Suspend */
#define H7F_MCR_ERS         0x00000004   /* Erase */
#define H7F_MCR_ESUS        0x00000002   /* Erase Suspend */
#define H7F_MCR_EHV         0x00000001   /* Enable High Voltage */


/* Low/Mid Address Space Block Locking Register */
#define H7F_LML_LME         0x80000000   /* Low and Mid Address Lock Enable */
#define H7F_LML_SLOCK       0x00100000   /* Shadow Lock */
#define H7F_LML_MLOCK       0x000F0000   /* Mid Address Block Lock */
#define H7F_LML_LLOCK       0x0000FFFF   /* Low Address Block Lock */

/* EEPROM High Address Space Block Locking Register */
#define H7F_HBL_HBE         0x80000000   /* High Address Lock Enable */
#define H7F_HBL_HBLOCK      0x0FFFFFFF   /* High Address Space Block Lock */

/* Secondary Low/Mid Address Space Block Locking Register */
#define H7F_SLL_SLE         0x80000000   /* Secondary Low and Mid Address Lock Enable */
#define H7F_SLL_SSLOCK      0x00100000   /* Secondary Shadow Lock */
#define H7F_SLL_SMLOCK      0x000F0000   /* Secondary Mid Address Block Lock */
#define H7F_SLL_SLLOCK      0x0000FFFF   /* Secondary Low Address Block Lock */

/* Low/Mid Address Space Block Select Register */
#define H7F_LMS_MSEL        0x000F0000   /* Mid Address Space Block Select */
#define H7F_LMS_LSEL        0x0000FFFF   /* Low Address Space Block Select */

/* High Address Space Block Select Register */
#define H7F_HBS_HBSEL       0x0FFFFFFF   /* High Address Space Block Select */

/* Platform Flash BIU Configuration Register */
#define PFB_CR_APC          0x0000E000   /* Address Pipelining Control */
#define PFB_CR_WWSC         0x00001800   /* Write Wait State Control */
#define PFB_CR_RWSC         0x00000700   /* Read Wait State Control */
#define PFB_CR_BFEN         0x00000001   /* PFBIU Line Read Buffers Enable */

/* MCU ID Register */
#define SIU_MIDR            0xC3F90004

/* Macros for Accessing the Registers */
#define H7F_REG_BIT_SET(address, mask)        (*(VUINT32*)(address) |= (mask))
#define H7F_REG_BIT_CLEAR(address, mask)      (*(VUINT32*)(address) &= ~(mask))
#define H7F_REG_BIT_TEST(address, mask)       (*(VUINT32*)(address) & (mask))
#define H7F_REG_WRITE(address, value)         (*(VUINT32*)(address) = (value))
#define H7F_REG_READ(address)                 ((UINT32)(*(VUINT32*)(address)))

/* Set/Clear H7FMCR bits without affecting MCR-EER and MCR-RWE */
#define H7FMCR_BIT_SET(MCRAddress, mask)      \
    H7F_REG_WRITE(MCRAddress, ((mask | H7F_REG_READ(MCRAddress)) & (~(H7F_MCR_EER | H7F_MCR_RWE))))

#define H7FMCR_BIT_CLEAR(MCRAddress, mask)    \
    H7F_REG_WRITE(MCRAddress, (((~mask) & H7F_REG_READ(MCRAddress)) & (~(H7F_MCR_EER | H7F_MCR_RWE))))

/*************************************************************************/
/*                   Return Codes for SSD functions                      */
/*************************************************************************/

#define H7F_OK                      0x00000000
#define H7F_INFO_RWE                0x00000001   /* There is read-while-write error for previous reads */
#define H7F_INFO_EER                0x00000002   /* There is ECC error for previous reads */
#define H7F_INFO_EPE                0x00000004   /* The program/erase for all blocks including shadow row and excluding the boot block is disabled */
#define H7F_INFO_BBEPE              0x00000008   /* The program/erase for boot block is disabled */
#define H7F_ERROR_PARTID            0x00000010   /* The SSD cannot operate on this part */
#define H7F_ERROR_STOP              0x00000020   /* The flash is in STOP mode and no operation is allowed */
#define H7F_ERROR_ALIGNMENT         0x00000100   /* Alignment error */
#define H7F_ERROR_RANGE             0x00000200   /* Address range error */
#define H7F_ERROR_BUSY              0x00000300   /* New program/erase cannot be preformed while previous high voltage operation in progress */
#define H7F_ERROR_PGOOD             0x00000400   /* The program operation is unsuccessful */
#define H7F_ERROR_EGOOD             0x00000500   /* The erase operation is unsuccessful */
#define H7F_ERROR_NOT_BLANK         0x00000600   /* There is non-blank location in the checked flash memory */
#define H7F_ERROR_VERIFY            0x00000700   /* There is a mismatch between the source data and content in the checked flash memory */
#define H7F_ERROR_LOCK_INDICATOR    0x00000800   /* Invalid block lock indicator */
#define H7F_ERROR_RWE               0x00000900   /* Read while write error on the previous reads */
#define H7F_ERROR_PASSWORD          0x00000B00   /* The password provided cannot unlock the block lock register for register writes */


/*************************************************************************/
/*                   Other Macros for SSD functions                      */
/*************************************************************************/

#define H7F_WORD_SIZE           4   /* size of a word in byte */
#define H7F_DWORD_SIZE          8   /* size of a double word in byte */

/* Indication for setting/getting block lock state */
#define LOCK_SHADOW_PRIMARY      0   /* Primary Block lock protection of shadow address space */
#define LOCK_SHADOW_SECONDARY    1   /* Secondary Block lock protection of shadow address space */
#define LOCK_LOW_PRIMARY         2   /* Primary block lock protection of low address space */
#define LOCK_LOW_SECONDARY       3   /* Secondary block lock protection of low address space */
#define LOCK_MID_PRIMARY         4   /* Primary block lock protection of mid address space */
#define LOCK_MID_SECONDARY       5   /* Secondary block lock protection of mid address space */
#define LOCK_HIGH                6   /* Block lock protection of high address space */

/* Macros for flash suspend operation */
#define NO_OPERATION             0   /* no program/erase operation */
#define PGM_WRITE                1   /* A program sequence in interlock write stage. */
#define ERS_WRITE                2   /* An erase sequence in interlock write stage. */
#define ERS_SUS_PGM_WRITE        3   /* A erase-suspend program sequence in interlock write stage. */
#define PGM_SUS                  4   /* The program operation is in suspend state */
#define ERS_SUS                  5   /* The erase operation on main array is in suspend state */
#define SHADOW_ERS_SUS           6   /* The erase operation on shadow row is in suspend state. */
#define ERS_SUS_PGM_SUS          7   /* The erase-suspended program operation is in suspend state */

/* Macros for flash resume operation */
#define RES_NOTHING              0   /* No suspended program/erase operation */
#define RES_PGM                  1   /* The program operation is resumed */
#define RES_ERS                  2   /* The erase operation is resumed */
#define RES_ERS_PGM              3   /* The erase-suspended program operation is resumed */


/*************************************************************************/
/*                   SSD Configuration Structure                         */
/*************************************************************************/
typedef enum _h7f_page_size
{
    H7FA_PAGE_SIZE,
    H7FB_PAGE_SIZE
} H7F_PAGE_SIZE;

typedef struct _ssd_config
{
    UINT32 h7fRegBase;           /* H7F control register base */
    UINT32 mainArrayBase;        /* base of main array */
    UINT32 mainArraySize;        /* size of main array */
    UINT32 shadowRowBase;        /* base of shadow row */
    UINT32 shadowRowSize;        /* size of shadow row */
    UINT32 lowBlockNum;          /* block number in low address space */
    UINT32 midBlockNum;          /* block number in middle address space */
    UINT32 highBlockNum;         /* block number in high address space */
    H7F_PAGE_SIZE pageSize;      /* page size */
    UINT32 BDMEnable;            /* debug mode selection */
} SSD_CONFIG, *PSSD_CONFIG;

/*************************************************************************/
/*                   NULL CallBack Function Pointer                      */
/*************************************************************************/
#define NULL_CALLBACK            ((void *)0xFFFFFFFF)

/*************************************************************************/
/*                      Prototypes of SSD Functions                      */
/*************************************************************************/

UINT32 FlashInit ( PSSD_CONFIG pSSDConfig );

UINT32 FlashErase ( PSSD_CONFIG pSSDConfig,
                BOOL shadowFlag,
                UINT32 lowEnabledBlocks,
                UINT32 midEnabledBlocks,
                UINT32 highEnabledBlocks,
                void (*CallBack)(void)
                );

UINT32 BlankCheck ( PSSD_CONFIG pSSDConfig,
                UINT32 dest,
                UINT32 size,
                UINT32 *pFailAddress,
                UINT64 *pFailData,
                void (*CallBack)(void)
                );

UINT32 FlashProgram ( PSSD_CONFIG pSSDConfig,
                UINT32 dest,
                UINT32 size,
                UINT32 source,
                void (*CallBack)(void)
                );

UINT32 ProgramVerify ( PSSD_CONFIG pSSDConfig,
                UINT32 dest,
                UINT32 size,
                UINT32 source,
                UINT32 *pFailAddress,
                UINT64 *pFailData,
                UINT64 *pFailSource,
                void (*CallBack)(void)
                );

UINT32 CheckSum ( PSSD_CONFIG pSSDConfig,
                UINT32 dest,
                UINT32 size,
                UINT32 *pSum,
                void (*CallBack)(void)
                );

UINT32 FlashSuspend ( PSSD_CONFIG pSSDConfig,
                UINT8 *suspendState,
                BOOL *suspendFlag
                );

UINT32 FlashResume ( PSSD_CONFIG pSSDConfig,
                UINT8 *resumeState
                );

UINT32 GetLock ( PSSD_CONFIG pSSDConfig,
                UINT8 blkLockIndicator,
                BOOL *blkLockEnabled,
                UINT32 *blkLockState
                );

UINT32 SetLock ( PSSD_CONFIG pSSDConfig,
                UINT8 blkLockIndicator,
                UINT32 blkLockState,
                UINT32 password
                );

UINT32 RWECheck ( PSSD_CONFIG pSSDConfig );

UINT32 GetWaitState ( PSSD_CONFIG pSSDConfig,
                UINT32 *pAPCValue,
                UINT32 *pWWSCValue,
                UINT32 *pRWSCValue
                );

UINT32 SetWaitState ( PSSD_CONFIG pSSDConfig,
                UINT32 APCValue,
                UINT32 WWSCValue,
                UINT32 RWSCValue
                );

/*************************************************************************/
/*                      SSD Function Pointer Types                       */
/*************************************************************************/

typedef UINT32 (*pFLASHINIT) ( PSSD_CONFIG pSSDConfig );

typedef UINT32 (*pFLASHERASE) (
                PSSD_CONFIG pSSDConfig,
                BOOL shadowFlag,
                UINT32 lowEnabledBlocks,
                UINT32 midEnabledBlocks,
                UINT32 highEnabledBlocks,
                void (*CallBack)(void)
               );

typedef UINT32 (*pBLANKCHECK) (
                PSSD_CONFIG pSSDConfig,
                UINT32 dest,
                UINT32 size,
                UINT32 *pFailAddress,
                UINT64 *pFailData,
                void (*CallBack)(void)
               );

typedef UINT32 (*pFLASHPROGRAM) (
                PSSD_CONFIG pSSDConfig,
                UINT32 dest,
                UINT32 size,
                UINT32 source,
                void (*CallBack)(void)
               );

typedef UINT32 (*pPROGRAMVERIFY) (
                PSSD_CONFIG pSSDConfig,
                UINT32 dest,
                UINT32 size,
                UINT32 source,
                UINT32 *pFailAddress,
                UINT64 *pFailData,
                UINT64 *pFailSource,
                void (*CallBack)(void)
               );

typedef UINT32 (*pCHECKSUM) (
                PSSD_CONFIG pSSDConfig,
                UINT32 dest,
                UINT32 size,
                UINT32 *pSum,
                void (*CallBack)(void)
               );

typedef UINT32 (*pFLASHSUSPEND) (
                PSSD_CONFIG pSSDConfig,
                UINT8 *suspendState,
                BOOL *suspendFlag
               );

typedef UINT32 (*pFLASHRESUME) (
                PSSD_CONFIG pSSDConfig,
                UINT8 *resumeState
               );

typedef UINT32 (*pGETLOCK) (
                PSSD_CONFIG pSSDConfig,
                UINT8 blkLockIndicator,
                BOOL *blkLockEnabled,
                UINT32 *blkLockState
               );

typedef UINT32 (*pSETLOCK) (
                PSSD_CONFIG pSSDConfig,
                UINT8 blkLockIndicator,
                UINT32 blkLockState,
                UINT32 password
               );

typedef UINT32 (*pRWECHECK) ( PSSD_CONFIG pSSDConfig );

typedef UINT32 (*pGETWAITSTATE) (
                PSSD_CONFIG pSSDConfig,
                UINT32 *pAPCValue,
                UINT32 *pWWSCValue,
                UINT32 *pRWSCValue
               );

typedef UINT32 (*pSETWAITSTATE) (
                PSSD_CONFIG pSSDConfig,
                UINT32 APCValue,
                UINT32 WWSCValue,
                UINT32 RWSCValue
               );

#endif
