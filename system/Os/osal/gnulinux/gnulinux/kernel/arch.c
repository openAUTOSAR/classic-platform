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
#include "os_i.h"

void Os_ArchInit(void) {
}
void Os_ArchFirstCall( void ) {

}
uint32 Os_ArchGetScSize( void ) {
    return (unsigned int) 0;
}

void Os_ArchSetupContext( OsTaskVarType *pcb ) {

}
void Os_ArchSetSpAndCall(void *sp, void (*f)(void) ) {

}

void Os_ArchSwapContext(void *old,void *new) {

}
void Os_ArchSwapContextTo(void *old,void *new){

}
void *Os_ArchGetStackPtr( void ) {
    return NULL;
}

Std_ReturnType Os_ArchCheckStartStackMarker( void ) {
    return E_OK;
}
