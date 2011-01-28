
#include "McuExtensions.h"

imask_t McuE_EnterCriticalSection(void) {
	return 0;
}

void McuE_ExitCriticalSection(imask_t old_state) {
	  (void)old_state; // Nothing to be done. This is just to avoid PC-Lint warning.
}
