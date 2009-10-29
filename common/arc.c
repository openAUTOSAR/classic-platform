/*
 * arc.c
 *
 *  Created on: 28 sep 2009
 *      Author: mahi
 */

#include "Os.h"
#include "arc.h"

#define ENTRY(_x)  [_x] = OS_STRSTR__(_x)

const char *Arc_StatusToString(StatusType err) {

	static char *errToStr[] = {

		ENTRY(E_OK),

		/* STD OSEK */
		ENTRY(E_OS_ACCESS),
		ENTRY(E_OS_CALLEVEL),
		ENTRY(E_OS_ID),
		ENTRY(E_OS_LIMIT),
		ENTRY(E_OS_NOFUNC),
		ENTRY(E_OS_RESOURCE),
		ENTRY(E_OS_STATE),

		ENTRY(E_OS_VALUE),

		/* AUTOSAR, see 7.10 */
		ENTRY(E_OS_SERVICEID),
		ENTRY(E_OS_RATE),
		ENTRY(E_OS_ILLEGAL_ADDRESS ),
		ENTRY(E_OS_MISSINGEND ),
		ENTRY(E_OS_DISABLEDINT ),
		ENTRY(E_OS_STACKFAULT ),
		ENTRY(E_OS_PROTECTION_MEMORY ),
		ENTRY(E_OS_PROTECTION_TIME ),
		ENTRY(E_OS_PROTECTION_LOCKED ),
		ENTRY(E_OS_PROTECTION_EXCEPTION ),
		ENTRY(E_OS_PROTECTION_RATE),

		/* COM.. TODO: move ?? */
		ENTRY(E_COM_ID),

		/* Implementation specific */
		ENTRY(E_OS_SYS_APA),

		ENTRY(E_NOT_OK),
	};

	return errToStr[err];

}
