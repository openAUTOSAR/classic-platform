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


#ifndef CPU_H
#define CPU_H

#include "Std_Types.h"
typedef uint32 imask_t;

//#if defined(__ghs__)
//#include <ppc_ghs.h>
//#endif
//#if defined(__DCC__)
//#include <diab/ppcasm.h>
//#endif

#if defined (CFG_MPC5646B)
#define	SIMULATOR() (SIU.MIDR1.R==0uL)
#else
// Used if we are running a T32 instruction set simulator
#define SIMULATOR() (SIU.MIDR.R==0uL)
#endif
// 32-bit write to 32 bit register
#define  BIT32(x)
// 16 bit write to 16 bit register
#define  BIT16(x)

// Bits EREF and others define as 64 bit regs but in 32 bits regs.
// E.g. #define MSR_PR		BIT64TO32(49)
#define  BIT64TO32(x)       ((uint32)1u<<(uint32)(63u-(x)))



/*
 * SPR numbers, Used by set_spr and get_spr macros
 */
#define SPR_LR		8
#define SPR_IVPR	63
#define SPR_IVOR0	400
#define SPR_IVOR1	401
#define SPR_IVOR2	402
#define SPR_IVOR3	403
#define SPR_IVOR4	404
#define SPR_IVOR5	405
#define SPR_IVOR6	406
#define SPR_IVOR7	407
#define SPR_IVOR8	408
#define SPR_IVOR9	409
#define SPR_IVOR10	410
#define SPR_IVOR11	411
#define SPR_IVOR12	412
#define SPR_IVOR13	413
#define SPR_IVOR14	414
#define SPR_IVOR15	415

#define SPR_IVOR32	528
#define SPR_IVOR33	529
#define SPR_IVOR34	530
#if defined (CFG_E200Z7)
#define SPR_IVOR35  531
#endif

#define SPR_DEC		22
#define SPR_DECAR	54
#define SPR_USPRG0	256
#define SPR_TBU_R	269
#define SPR_TBU_W	285
#define SPR_TBL_R	268
#define SPR_TBL_W	284

#define SPR_TCR		340
#define SPR_TSR		336

#define SPR_HID0	1008

#define SPR_L1CSR0	1010
#define SPR_L1CSR1  1011
#define SPR_L1CFG0	515

#define L1CSR0_CINV	BIT64TO32(62)
#define L1CSR0_CE	BIT64TO32(63)


#define SPR_SRR0		26
#define SPR_SRR1		27

#define SPR_SPRG1_RW_S	273

#define MSR_PR		BIT64TO32(49U)
#define MSR_EE		BIT64TO32(48U)
#define MSR_SPE	    BIT64TO32(38U)
#define MSR_DS		BIT64TO32(58U)
#define MSR_IS		BIT64TO32(59U)
#define MSR_SPE     BIT64TO32(38U)

//#define ESR_PTR 	BIT64TO32(38)


/* Timer control regs
 */
#define TCR_DIE 	0x04000000UL
#define TCR_ARE	    0x00400000UL
#define TCR_FIE	    0x00800000UL

#define HID0_TBEN	0x4000UL


/*
 * String macros
 */

#define STR__(x)	#x
#define STRINGIFY__(x) STR__(x)


#define Irq_Save(flags)    flags = _Irq_Disable_save()
#define Irq_Restore(flags) _Irq_Disable_restore(flags)

/***********************************************************************
 * Common macro's
 */


// Misra 2004 2.1, 2012 4.3 : Inhibit lint error 586 for assembly
// language that is encapsulated and isolated in macros
#if defined(__CWCC__) || defined(__DCC__)
#define ASM_ARGS_MEMORY
#define ASM_ARGS_MEMORY_CC
#else
#define ASM_ARGS_MEMORY     :::"memory"
#define ASM_ARGS_MEMORY_CC  ::: "memory","cc"
#endif



#if defined(CFG_VLE) && !defined(__CWCC__)
#define isync() /*lint -save -e586 -restore */ asm volatile(" se_isync" ASM_ARGS_MEMORY)
#define msync() /*lint -save -e586 -restore */ asm volatile(" se_isync" ASM_ARGS_MEMORY)
#else
// CW does not like taking the right instruction, ie se_xxxx
#define isync() /*lint -save -e586 -restore */ asm volatile(" isync")
#define msync() /*lint -save -e586 -restore */ asm volatile(" isync")
#endif

#define sync()          /*lint -save -e586 -restore */ asm volatile(" sync" ASM_ARGS_MEMORY)

#define Irq_Disable()   /*lint -save -e586 -restore */ asm volatile (" wrteei 0" ASM_ARGS_MEMORY_CC)
#define Irq_Enable()    /*lint -save -e586 -restore */ asm volatile (" wrteei 1" ASM_ARGS_MEMORY_CC)
#define tlbwe()         /*lint -save -e586 -restore */ asm volatile (" tlbwe" ASM_ARGS_MEMORY )

#define Irq_SuspendAll() 	Irq_Disable()
#define Irq_ResumeAll() 	Irq_Enable()

#define Irq_SuspendOs() 	Irq_Disable()
#define Irq_ResumeOs() 	Irq_Enable()


/*-----------------------------------------------------------------*/

/**
 * memset that can be used for ecc mem init
 * @param msr
 */
extern void memset_uint64(unsigned char *to, const uint64 *val, unsigned long size);
extern void initECC_128bytesAligned(unsigned char *to, unsigned long size);

/*-----------------------------------------------------------------*/

/**
 * Sets a value to a specific SPR register
 */
/*lint -save -e10 */
#if defined(__DCC__)
asm void set_spr(uint32 spr_nr, uint32 val)
{
%reg val; con spr_nr
  mtspr spr_nr, val
}
#else
// Misra 2004 2.1, 2012 4.3 : Inhibit lint error 586 for assembly
// language that is encapsulated and isolated in macros
// Lint redefinition of set_spr is used to resolve false positive lint warnings caused by
// lint does not recognize assembly code accessing parameter val
# define set_spr(spr_nr, val) /*lint -save -e547 +dset_spr(a,b)=((void)b) -e586 -restore */ \
    asm volatile (" mtspr " STRINGIFY__(spr_nr) ",%[_val]" : : [_val] "r" (val))
#endif
/*lint -restore */
/**
 * Gets the value from a specific SPR register
 *
 * Note! Tried lots of other ways to do this but came up empty
 */

//https://community.freescale.com/thread/29234
/*lint -save -e10 */
#if defined(__DCC__) || defined(__ghs__)
uint32 get_spr(uint32 spr_nr); // MISRA 2004 8.1 and 2012 17.3   Resolved by this function declaration
asm uint32 get_spr(uint32 spr_nr)
{
% con spr_nr
  mfspr r3, spr_nr
}
#else
//lint --e{923,950,530,9008} LINT:FALSE_POSITIVE:Lint can't handle compound assembler macros
#define get_spr(spr_nr)	CC_EXTENSION \
({\
    uint32 __val;\
    asm volatile (" mfspr %0," STRINGIFY__(spr_nr) : "=r"(__val) : );\
    __val;\
})
#endif
/*lint -restore */

/*-----------------------------------------------------------------*/

/**
 * Sets a value to a specific DCR register
 */
/*lint -save -e10 */
#if defined(__DCC__)
asm void set_dcr(uint32 dcr_nr, uint32 val)
{
%reg val; con dcr_nr
  mtdcr dcr_nr, val
}
#else
# define set_dcr(dcr_nr, val) \
    asm volatile (" mtdcr " STRINGIFY__(dcr_nr) ",%[_val]" : : [_val] "r" (val))
#endif
/*lint -restore */

/**
 * Gets the value from a specific DCR register
 *
 */

/*lint -save -e10 */
#if defined(__DCC__) || defined(__ghs__)
asm uint32 get_dcr(uint32 dcr_nr)
{
% con dcr_nr
  mfdcr r3, dcr_nr
}
#else
#define get_dcr(dcr_nr) CC_EXTENSION \
({\
    uint32 __val;\
    asm volatile (" mfdcr %0," STRINGIFY__(dcr_nr) : "=r"(__val) : );\
    __val;\
})
#endif
/*lint -restore */
/**
 * Get current value of the msr register
 * @return
 */
/*lint -save -e10 */
#if defined(__DCC__)
asm volatile unsigned long get_msr()
{
  mfmsr r3
}
#else
static inline unsigned long get_msr( void ) {
    uint32 msr;
    asm volatile("mfmsr %[msr]":[msr] "=r" (msr ) );
    return msr;
}
#endif
/*lint -restore */
/*-----------------------------------------------------------------*/

/**
 * Set the current msr to msr
 * @param msr
 */
/*lint -save -e10 */
#if defined(__DCC__)
asm volatile void set_msr(unsigned long msr)
{
% reg msr
  mtmsr msr
  isync
}
#else
static inline void set_msr(unsigned long msr) {
  asm volatile ("mtmsr %0" : : "r" (msr) );

  // This is just necessary for some manipulations of MSR
  isync();
}
#endif
/*lint -restore */
/*-----------------------------------------------------------------*/

/* Count the number of consecutive zero bits starting at ppc-bit 0 */
/*lint -save -e10 */
#if defined(__DCC__)
asm volatile unsigned int cntlzw(unsigned int val)
{
% reg val
  cntlzw r3, val
}
#else

/**
 * Return number of leading zero's
 *
 * Examples:
 *  cntlzw(0x8000_0000) = 0
 *  cntlzw(0x0000_0000) = 32
 *  cntlzw(0x0100_0000) = 7
 *
 * @param val
 * @return
 */
static inline unsigned int cntlzw(unsigned int val)
{
   unsigned int result;
   asm volatile ("cntlzw  %[rv],%[val]" : [rv] "=r" (result) : [val] "r" (val) );
   return result;
}
#endif
/*lint -restore */

/**
 * Integer log2
 *
 * Examples:
 * - ilog2(0x0) = -1
 * - ilog2(0x1) = 0
 * - ilog2(0x2) = 1
 * - ilog2(0x3) = 1
 * - ilog2(0x4) = 2
 * - ilog2(0x8000_0000)=31
 *
 * @param val
 * @return
 */
static inline uint8 ilog2( uint32 val ) {
	return (uint8)(31U - cntlzw(val));
}

#if defined(__ghs__)
/* already in string.h */
#elif defined(__GNUC__)
/* We don't want to use newlib ffs(), use GCC instead */
#define ffs(_x)  __builtin_ffs(_x)
#else
static inline int ffs( uint32 val ) {
    return 32 - cntlzw(val & (-val));
}
#endif

/* Standard newlib ffs() is REALLY slow since ot loops over all over the place
 * IMPROVEMENT: Use _builin_ffs() instead.
 */

/*-----------------------------------------------------------------*/
static inline imask_t _Irq_Disable_save(void)
{
   unsigned long result = get_msr();
   Irq_Disable();
   return result;
}

/*-----------------------------------------------------------------*/
/*lint -save -e10 */
#if defined(__DCC__)
asm volatile void _Irq_Disable_restore(imask_t flags)
{
% reg flags
  wrtee flags
}
#else
static inline void _Irq_Disable_restore(imask_t flags) {
  asm volatile ("wrtee %0" : : "r" (flags) );
}
#endif
/*lint -restore */

/*-----------------------------------------------------------------*/
#if defined(__DCC__)
#else
#define get_lr(var) \
    do { \
        unsigned long lr; \
        asm volatile("mflr %[lr]":[lr] "=r" (lr ) ); \
        var = lr; \
    } while(0)
#endif

/*-----------------------------------------------------------------*/

static inline int in_user_mode( void ) {
    unsigned long msr;
    msr = get_msr();
    // In user mode?
    return (msr & MSR_PR);
}

static inline void Os_EnterUserMode( void ) {
    unsigned long msr;
    msr = get_msr();
    set_msr(msr | MSR_PR);
}


#if 0
#ifdef USE_T32_SIM
    #define mode_to_kernel() k_arch_sim_trap()
#else
    #define mode_to_kernel() asm volatile(" sc");
#endif
#endif

/*-----------------------------------------------------------------*/

#if 0
#define SC_CALL(name,...) \
    ({ \
        int rv; \
        unsigned int msr = get_msr(); \
        if( msr & MSR_PR ) { \
/*			asm volatile(" sc"); */\
            rv = _ ## name( __VA_ARGS__ ); \
/*			set_msr(msr); */ \
        } else { \
            rv = _ ## name( __VA_ARGS__ ); \
        } \
        rv; \
    })
#endif

/*-----------------------------------------------------------------*/



// StatusType CallService(	TrustedFunctionIndexType ix,
//							TrustedFunctionParameterRefType params )

typedef void ( * service_func_t)( uint16 , void * );
extern service_func_t oil_trusted_func_list[];

/* Macros for SC_CALL macro */
#define REG_DEF_1 register unsigned int r3 asm ("r3");
#define REG_DEF_2 REG_DEF_1 register unsigned int r4 asm ("r4");

/* Build argument list */
#define REG_IN_1 "r" (r3)
#define REG_IN_2 REG_IN_1 ,"r" (r4)


#if defined(USE_MM_USER_MODE)
/* IMPROVEMENT: Improve it */
#define CallService(index,param) \
    ({ \
        register uint32 r3 asm ("r3"); \
        register void * r4 asm ("r4"); \
        r3 = index; \
        r4 = param; \
        asm volatile( \
            " sc\r\t" \
            :  \
            : "r" (r3),"r" (r4) ); \
    })
#else
#define CallService(index,param)
#endif


#if 0

/* Macros for SC_CALL macro */
#define REG_DEF_1 register unsigned int r3 asm ("r3");
#define REG_DEF_2 REG_DEF_1 register unsigned int r4 asm ("r4");

/* Split the __VA_ARGS__ */
#define SPLIT_ARGS_1(_arg1) r3 = _arg1;
#define SPLIT_ARGS_2(_arg1,_arg2) r3 = _arg1;r4 = _arg2;

/* Build argument list */
#define REG_IN_1 "r" (r3)
#define REG_IN_2 REG_IN_1 ,"r" (r4)

/* System call
 *
 * name - just for show
 * index - The index to the function name above..
 * arg_cnt - How many argument the function takes
 * ... - var args list..
 * */

/*unsigned int t = index; \ */

/* NOTE: __VA_ARGS__ is ISO99 and not all compilers may have support for it*/
#if __STDC_VERSION__ < 199901L
#error Sorry, using macros iso99 VA_ARGS...implement in some other way
#endif


#define SC_CALL(name,index,arg_cnt,...) \
    ({ \
        void * t = _ ## name; \
        REG_DEF_ ## arg_cnt \
        SPLIT_ARGS_ ## arg_cnt(__VA_ARGS__) \
        asm volatile( \
            " mr 0,%[t];\r\t" \
            " sc\r\t" \
            :  \
            : [t] "r" (t), REG_IN_ ## arg_cnt ); \
    })
#endif

/*
 * 		asm volatile( " sc\r\t" ); \
 */

/*-----------------------------------------------------------------*/

#if 0
static inline unsigned int mode_to_kernel( void ) {
    unsigned long msr;
    msr = get_msr();
    // In user mode?
    if( msr & MSR_PR ) {
        // switch to supervisor mode...
#ifdef USE_T32_SIM
        k_arch_sim_trap();
#else
        asm volatile(" tw");
#endif
    }
    return msr;
}
#endif

void Cache_Invalidate( void );
void Cache_EnableU( void );
void Cache_EnableD( void );
void Cache_EnableI( void );


#endif /* CPU_H */
