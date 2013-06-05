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


#ifndef CPU_H
#define CPU_H

#include "Std_Types.h"
typedef uint32 imask_t;

//#if defined(__DCC__)
//#include <diab/ppcasm.h>
//#endif

// Used if we are running a T32 instruction set simulator
#define SIMULATOR() (SIU.MIDR.R==0)

// 32-bit write to 32 bit register
#define  BIT32(x)
// 16 bit write to 16 bit register
#define  BIT16(x)

// Bits EREF and others define as 64 bit regs but in 32 bits regs.
// E.g. #define MSR_PR		BIT64TO32(49)
#define  BIT64TO32(x)		(1<<(63-(x)))



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


#define SPR_DEC		22
#define SPR_DECAR	54

#define SPR_TBU_R	269
#define SPR_TBU_W	285
#define SPR_TBL_R	268
#define SPR_TBL_W	284

#define SPR_TCR		340
#define SPR_TSR		336

#define SPR_HID0	1008

#define SPR_L1CSR0	1010
#define SPR_L1CFG0	515

#define L1CSR0_CINV	BIT64TO32(62)
#define L1CSR0_CE	BIT64TO32(63)


#define SPR_SRR0		26
#define SPR_SRR1		27

#define SPR_SPRG1_RW_S	273

#define MSR_PR		BIT64TO32(49)
#define MSR_EE		BIT64TO32(48)
#define MSR_SPE	BIT64TO32(38)
#define MSR_DS		BIT64TO32(58)
#define MSR_IS		BIT64TO32(59)
#define MSR_SPE     BIT64TO32(38)

//#define ESR_PTR 	BIT64TO32(38)


/* Timer control regs
 */
#define TCR_DIE 	0x04000000
#define TCR_ARE	0x00400000
#define TCR_FIE	0x00800000

#define HID0_TBEN	0x4000


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

#define isync()  		asm volatile(" isync");
#define sync()   		asm volatile(" sync");
#define msync() 		asm volatile(" isync");

#define Irq_Disable() 	asm volatile (" wrteei 0");
#define Irq_Enable() 	asm volatile (" wrteei 1");
#define tlbwe()			asm volatile (" tlbwe");


#define Irq_SuspendAll() 	Irq_Disable()
#define Irq_ResumeAll() 	Irq_Enable()

#define Irq_SuspendOs() 	Irq_Disable()
#define Irq_ResumeOs() 	Irq_Enable()


/*-----------------------------------------------------------------*/

/**
 * Sets a value to a specific SPR register
 */
#if defined(__DCC__)
asm void set_spr(uint32 spr_nr, uint32 val)
{
%reg val; con spr_nr
  mtspr spr_nr, val
}
#else
# define set_spr(spr_nr, val) \
	asm volatile (" mtspr " STRINGIFY__(spr_nr) ",%[_val]" : : [_val] "r" (val))
#endif
/**
 * Gets the value from a specific SPR register
 *
 * Note! Tried lots of other ways to do this but came up empty
 */

//https://community.freescale.com/thread/29234

#if defined(__DCC__)
asm uint32 get_spr(uint32 spr_nr)
{
% con spr_nr
  mfspr r3, spr_nr
}
#else
#define get_spr(spr_nr)	CC_EXTENSION \
({\
	uint32 __val;\
	asm volatile (" mfspr %0," STRINGIFY__(spr_nr) : "=r"(__val) : );\
	__val;\
})
#endif

/**
 * Get current value of the msr register
 * @return
 */
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
/*-----------------------------------------------------------------*/

/**
 * Set the current msr to msr
 * @param msr
 */
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
/*-----------------------------------------------------------------*/

/* Count the number of consecutive zero bits starting at ppc-bit 0 */
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

/**
 * Integer log2
 *
 * Examples:
 * - ilog2(0x0) = -1
 * - ilog2(0x1) = 0
 * - ilog2(0x2) = 1
 * - ilog2(0x8000_0000)=31
 *
 * @param val
 * @return
 */
static inline int ilog2( int val ) {
	return 31 - cntlzw(val);
}

/* Standard newlib ffs() is REALLY slow since ot loops over all over the place
 * TODO: Use _builin_ffs() instead.
 */

/*-----------------------------------------------------------------*/
static inline unsigned long _Irq_Disable_save(void)
{
   unsigned long result = get_msr();
   Irq_Disable();
   return result;
}

/*-----------------------------------------------------------------*/

static inline void _Irq_Disable_restore(unsigned long flags)
{
	set_msr(flags);
}

#if 0
#if defined(__DCC__)
#define _Irq_Disable_restore(flags)		(set_msr(flags))
#else
static inline void _Irq_Disable_restore(unsigned long flags)
{
	set_msr(flags);
   asm volatile ("mtmsr %0" : : "r" (flags) );
}
#endif
#endif
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


#if 0
#ifdef USE_T32_SIM
	#define mode_to_kernel() k_arch_sim_trap()
#else
	#define mode_to_kernel() asm volatile(" sc");
#endif
#endif

/*-----------------------------------------------------------------*/

static inline void to_user_mode( void ) {
	// Just set it back
	unsigned long msr;
	msr = get_msr();
	set_msr(msr & ~MSR_PR);
}

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
/* TODO: Fix this.. */
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

/* TODO: __VA_ARGS__ is ISO99 and not all compilers may have support for it*/
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
