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
#include "Os.h"

uint16 EA_count;

/*lint -w1*/
void ShutdownOS( StatusType error )  {
}

#if defined(CFG_TC2XX)
uint32 Irq_VectorTable[1];

void Os_Isr( uint32 x) {

}

void Trap_MMU( void ) {

}
void Trap_InternalProtection( void ) {

}

void Trap_InstructionErrors( void ) {

}
void Trap_ContextManagement( void ) {

}
void Trap_SystemBusAndPeripheralErrors( void ) {

}
void Trap_AssertionTraps( void ) {

}

void Trap_SystemCall( void ) {

}

void Trap_NMI( void ) {

}

#endif

#if defined(CFG_ARM)
#define _STAY()     while(1) {}

void Undefined_Instruction_Handler( void ) {
    _STAY();
}

void Dummy_Irq( void ) {
    _STAY();
}

void Prefetch_Exc_Handler( void ) {
    _STAY();
}

void Data_Exc_Handler( void ) {
    _STAY();
}

void Irq_Handler( void ) {
    _STAY();
}

void Fiq_Handler( void ) {
    _STAY();
}

void SupervisorCall_Handler( void ) {
    _STAY();
}


#endif
