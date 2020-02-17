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
#include "machine/intrinsics.h"

typedef uint32 imask_t;

#define INTERRUPT_ENABLE_BIT 0x8000
#define CPU0_OPERATION_MODE_USER0 0U
#define CPU0_OPERATION_MODE_USER1 1U
#define CPU0_OPERATION_MODE_SUPERVISOR 2U

#define TRAP_0_MMU 0
#define TRAP_1_INTERNAL_PROTECTION 1
#define TRAP_1_INTERNAL_PROTECTION_PRIV 1
#define TRAP_1_INTERNAL_PROTECTION_MPR 2
#define TRAP_1_INTERNAL_PROTECTION_MPW 3
#define TRAP_1_INTERNAL_PROTECTION_MPX 4
#define TRAP_1_INTERNAL_PROTECTION_MPP 5
#define TRAP_1_INTERNAL_PROTECTION_MPN 6
#define TRAP_1_INTERNAL_PROTECTION_GRWP 7
#define TRAP_2_INST_ERROR 2
#define TRAP_3_CONTEXT_MGNT 3
#define TRAP_4_SYSBUS_PERIPHERAL 4
#define TRAP_5_ASSERTION 5
#define TRAP_6_SYSCALL 6
#define TRAP_7_NMI 7

#define REGISTER_SET_SUPERVISOR 0
#define REGISTER_SET_USER 1

#define CSA_TO_ADDRESS(_csa)    ( (((_csa) & 0x000f0000UL) << 12U) | (((_csa) & 0xffffL) << 6U ) )
#define ADDRESS_TO_CSA(_addr)   ( (((( _addr ))&0xf0000000UL ) >> 12U) | ((( _addr ) & 0x003FFFC0UL ) >> 6U ) )

#define STR__(x)	#x
#define STRINGIFY__(x) STR__(x)

#define Irq_Save(flags)		flags = _Irq_Disable_save()
#define Irq_Restore(flags)	_Irq_Disable_restore(flags)

#define Irq_Disable()		_disable();
#define Irq_Enable()		_enable();

#define Irq_SuspendAll() 	Irq_Disable()
#define Irq_ResumeAll() 	Irq_Enable()

#define Irq_SuspendOs() 	Irq_Disable()
#define Irq_ResumeOs() 		Irq_Enable()

#define ICR_IE_INT_ENABLE_BIT (1U<<15U)

#define set_reg(reg_nr, val) /*lint -save -e547 +dset_spr(a,b)=((void)b) -e586 -restore */ \
        __asm__ volatile (" mov " STRINGIFY__(reg_nr) ",%[_val]" : : [_val] "r" (val))

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
static inline uint8 ilog2( uint32 val ) {
    return 0;
}

static inline unsigned long _Irq_Disable_save(void)
{
    unsigned long result = _mfcr(0xFE2C);   /* CPU_ICR */
    Irq_Disable();
    return result;
}


static inline void _Irq_Disable_restore(unsigned long flags) {
    if( flags & INTERRUPT_ENABLE_BIT ) {
        Irq_Enable();
    } else {
        Irq_Disable();
    }
}

// lower context structure
typedef struct {
    uint32 pcxi;
    uint32 a11_ra;
    uint32 a2;
    uint32 a3;
    uint32 d0;
    uint32 d1;
    uint32 d2;
    uint32 d3;
    uint32 a4;
    uint32 a5;
    uint32 a6;
    uint32 a7;
    uint32 d4;
    uint32 d5;
    uint32 d6;
    uint32 d7;
} Os_ContextLowType;

// upper context structure
typedef struct {
    uint32 pcxi;
    uint32 psw;
    uint32 a10_sp;
    uint32 a11_ra;
    uint32 d8;
    uint32 d9;
    uint32 d10;
    uint32 d11;
    uint32 a12;
    uint32 a13;
    uint32 a14;
    uint32 a15;
    uint32 d12;
    uint32 d13;
    uint32 d14;
    uint32 d15;
} Os_ContextHighType;

#endif /* CPU_H */
