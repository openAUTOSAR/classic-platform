#include "Os.h"
#include "internal.h"
#include "sys.h"
void Os_ArchInit(void) {
}
void Os_ArchFirstCall( void ) {

}
int Os_ArchGetScSize( void ) {
}
void Os_ArchSetTaskEntry(OsTaskVarType *pcbPtr ) {
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

}


