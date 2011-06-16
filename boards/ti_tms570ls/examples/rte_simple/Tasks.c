/*
 * Tasks.c
 *
 *  Created on: 16 jun 2011
 *      Author: maek
 */

#include "Os.h"
#include "Mcu.h"
#include "arc.h"

//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
#include "debug.h"

// How many errors to keep in error log.
#define ERROR_LOG_SIZE 20


void StartupTask( void ) {
	LDEBUG_FPUTS("StartupTask start\n");

	// Call second phase of startup sequence.
	EcuM_StartupTwo();

	// Startup CanIf due to ComM is missing in this example
	CanIf_InitController(CANIF_Channel_1, CANIF_Channel_1_CONFIG_0);
	CanIf_SetControllerMode(CANIF_Channel_1, CANIF_CS_STARTED);

	// Make sure that the right PDU-groups are ready for communication.
	Com_IpduGroupStart(ComPduGroup, 0);

	// End of startup_task().
	TerminateTask();

}
