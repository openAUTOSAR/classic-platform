
#include "McuExtensions.h"

imask_t McuE_EnterCriticalSection(void) {
	return 0;
}

void McuE_ExitCriticalSection(imask_t old_state) {
	(void)old_state;	// Added to remove compiler warnings
}
