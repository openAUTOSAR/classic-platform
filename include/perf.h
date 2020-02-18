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

#ifndef PERF_H_
#define PERF_H_

/*
 * Configure:
 *   OS Editor
 *     OsOS->OsHooks->OsPreTaskHook = TRUE
 *     OsOS->OsHooks->OsPostTaskHook = TRUE
 *     OsOS->OsHooks->OsStartupHook = TRUE
 */


/**
 * Call cyclic to calculate load on the system
 */
void Perf_Trigger(void);

/**
 * Call at init
 */
void Perf_Init( void );

/**
 * Call whenever you want to install a name to an index.
 */
void Perf_InstallFunctionName(uint8 PerfFuncIdx, char *PerfNamePtr, uint8 PerfNameLen);

/**
 * Call before the function
 */
void Perf_PreFunctionHook(uint8 PerfFuncIdx);

/**
 * Call after the function
 */
void Perf_PostFunctionHook(uint8 PerfFuncIdx);

/**
 * Readout of CPU load
 */
uint8 Perf_ReadCpuLoad();

#if !defined CFG_PERF_FUNC
#define Perf_InstallFunctionName
#define Perf_PreFunctionHook
#define Perf_PostFunctionHook
#endif


#endif /* PERF_H_ */
