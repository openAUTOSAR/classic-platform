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
 * - GetAlarmBase				E_OS_ID
 * - GetAlarm      				E_OS_ID, E_OS_NOFUNC
 * - SetRelAlarm                E_OS_ID, E_OS_STATE, E_OS_VALUE
 * - SetAbsAlarm                E_OS_ID, E_OS_STATE, E_OS_VALUE
 * - CancelAlarm                E_OS_ID, E_OS_NOFUNC
 *
 * Autosar
 * - IncrementCounter			E_OS_ID
 * - GetCounterValue            E_OS_ID
 * - GetElapsedCounterValue     E_OS_ID, E_OS_VALUE
 *
 * More tests here:
 *
 * Limitations:
 */

