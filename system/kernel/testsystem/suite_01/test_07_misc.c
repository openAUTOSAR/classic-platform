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

/*
 * Tests:
 *
 * OSEK
 * - DisableAllInterrupts
 * - EnableAllInterrupts
 * - SuspendAllInterrupts
 * - ResumeAllInterrupts
 * - SuspendOSInterrupts
 * - ResumeOSInterrupts
 *
 * - GetActiveApplicationMode
 * - StartOS
 * - ShutdownOS
 *
 *
 * Autosar
 *
 * Class 3 and 4
 * - GetApplicationID
 * - GetISRID					IVALID_ISR
 * - CallTrustedFunction  		...
 * - CheckISRMemoryAccess		...
 * - CheckTaskMemoryAccess
 * - CheckObjectMemoryAccess
 * - CheckObjectOwnership
 * - TerminateApplication
 *
 * Autosar additional tests:
 *
 *   Class 2,3,4
 * - ProtectionHook
 *
 *   Class 2,4
 * - Timing Protection
 * - Global Time
 *
 *   Class 3,4
 * - Memory Protection
 * - Os-Applications
 * - Service Protection
 * - Call trusted function
 *
 * Limitations:
 */

void intTest( void ) {

	/* Just call them to its works */
	DisableAllInterrupts();
	EnableAllInterrupts();

	SuspendAllInterrupts();
	ResumeAllInterrupts();

	SuspendOSInterrupts();
	ResumeOSInterrupts();

	/** @req OS092 */
	/* We should be able to call them and they should be ignored */
	EnableAllInterrupts();
	EnableAllInterrupts();

	ResumeAllInterrupts();
	ResumeAllInterrupts();

	ResumeOSInterrupts();
	ResumeOSInterrupts();

	/* No nesting for DisableAllInterrupts() and EnableAllInterrupts */
	DisableAllInterrupts();
	/* No calls allowed here */
	EnableAllInterrupts();

	/* Nesting allowed for these calls */
	SuspendAllInterrupts();
	SuspendAllInterrupts();
	SuspendOSInterrupts();
	ResumeOSInterrupts();
	ResumeAllInterrupts();
	ResumeAllInterrupts();

	/* MORE MORE */

}




