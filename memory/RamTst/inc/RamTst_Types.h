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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=GENERIC */

#ifndef RAMTST_TYPES_H_
#define RAMTST_TYPES_H_

#if defined(USE_DEM)
#include "Dem.h" /* For Dem_EventIdType */
#endif

typedef union {
    uint32 address;
    uint32* dataPtr;
}RamTst_AddrType;

/* @req SWS_RamTst_00189 */
typedef enum
{
    RAMTST_EXECUTION_UNINIT = 0,
    RAMTST_EXECUTION_STOPPED,
    RAMTST_EXECUTION_RUNNING,
    RAMTST_EXECUTION_SUSPENDED,
}RamTst_ExecutionStatusType;

/* @req SWS_RamTst_00190 */
typedef enum
{
    RAMTST_RESULT_NOT_TESTED = 0,
    RAMTST_RESULT_OK,
    RAMTST_RESULT_NOT_OK,
    RAMTST_RESULT_UNDEFINED,
}RamTst_TestResultType;

/* @req SWS_RamTst_00191 */
typedef uint8 RamTst_AlgParamsIdType;

/* @req SWS_RamTst_00173 */
typedef uint32 RamTst_NumberOfTestedCellsType;

/* @req SWS_RamTst_00174 */
typedef uint16 RamTst_NumberOfBlocksType;

/* @req SWS_RamTst_00227 */
/* !req SWS_RamTst_00058 */
typedef enum
{
    RAMTST_ALGORITHM_UNDEFINED = 0,
    RAMTST_CHECKERBOARD_TEST,
    RAMTST_MARCH_TEST,
    RAMTST_WALK_PATH_TEST,
    RAMTST_GALPAT_TEST,
    RAMTST_TRANSP_GALPAT_TEST,
    RAMTST_ABRAHAM_TEST,
}RamTst_AlgorithmType;

typedef enum
{
    RAMTEST_DESTRUCTIVE = 0,
    RAMTEST_NON_DESTRUCTIVE,
}RamTst_TestPolicyType;

typedef void (*RamTst_TestCompletedNotificationType)();
typedef void (*RamTst_TestErrorNotificationType)();

typedef struct
{
    RamTst_AddrType RamTstEndAddress;
    RamTst_AddrType RamTstStartAddress;
    RamTst_NumberOfBlocksType RamTstBlockId;
    RamTst_NumberOfBlocksType RamTstBlockIndex;
}RamTst_BlockParamsType;

typedef struct
{
    RamTst_AlgParamsIdType RamTstAlgParamsId;
    RamTst_AlgorithmType RamTstAlgorithm;
    RamTst_NumberOfBlocksType RamTstNumberOfBlocks;
    const RamTst_BlockParamsType * const RamTstBlockParams;
}RamTst_AlgParamsType;

typedef struct
{
    RamTst_AlgParamsIdType RamTstDefaultAlgParamsId;
    RamTst_AlgParamsIdType RamTstNumberOfAlgParamSets;
#if defined(USE_DEM)
    Dem_EventIdType RAMTST_E_RAM_FAILURE;
#endif
    const RamTst_AlgParamsType * const RamTstAlgParams;
}RamTst_ConfigParamsType;


#endif /* RAMTST_TYPES_H_ */
