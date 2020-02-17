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

#ifndef RAMTST_H_
#define RAMTST_H_

#include "Std_Types.h"

#define RAMTST_VENDOR_ID			    60u
#define RAMTST_MODULE_ID			    93u /* TBD cannot find any in spec */
#define RAMTST_AR_MAJOR_VERSION  		4u
#define RAMTST_AR_MINOR_VERSION 		1u
#define RAMTST_AR_PATCH_VERSION			2u

#define RAMTST_SW_MAJOR_VERSION			2u
#define RAMTST_SW_MINOR_VERSION 		2u
#define RAMTST_SW_PATCH_VERSION			0u

/* Service ids */
#define RAMTST_INIT_SERVICE_ID                      0x00u
#define RAMTST_DEINIT_SERVICE_ID                    0x0cu
#define RAMTST_STOP_SERVICE_ID                      0x02u
#define RAMTST_ALLOW_SERVICE_ID                     0x03u
#define RAMTST_GETVERSIONINFO_SERVICE_ID            0x0au
#define RAMTST_SUSPEND_SERVICE_ID                   0x0du
#define RAMTST_RESUME_SERVICE_ID                    0x0eu
#define RAMTST_GETEXECUTIONSTATUS_SERVICE_ID        0x04u
#define RAMTST_GETTESTRESULT_SERVICE_ID             0x05u
#define RAMTST_GETRESULTPERBLOCK_SERVICE_ID         0x06u
#define RAMTST_GETALGPARAMS_SERVICE_ID              0x12u
#define RAMTST_GETTESTALGORITHM_SERVICE_ID          0x07u
#define RAMTST_GETNUMBEROFTESTEDCELLS_SERVICE_ID    0x09u
#define RAMTST_SELECTALGPARAMS_SERVICE_ID           0x0bu
#define RAMTST_CHANGENUMBEROFTESTEDCELLS_SERVICE_ID 0x08u
#define RAMTST_RUNPARTIALTEST_SERVICE_ID            0x11u
#define RAMTST_MAINFUNCTION_SERVICE_ID              0x01u
#define RAMTST_RUNFULLTEST_SERVICE_ID               0x10u

/* Development errors */
#define RAMTST_E_STATUS_FAILURE 0x1u
#define RAMTST_E_OUT_OF_RANGE   0x2u
#define RAMTST_E_UNINIT         0x3u
#define RAMTST_E_PARAM_POINTER  0x4u

#include "RamTst_Types.h"

/*@req SWS_RamTst_00003 */
#include "RamTst_Cfg.h"

/* @req SWS_RamTst_00099 */
void RamTst_Init(void);

/* @req SWS_RamTst_00146 */
void RamTst_DeInit( void );

#if (RAMTST_STOP_API == STD_ON)
/* @req SWS_RamTst_00100 */
void RamTst_Stop( void );
#endif

#if(RAMTST_ALLOW_API == STD_ON)
/* @req SWS_RamTst_00149 */
void RamTst_Allow( void );
#endif

#if(RAMTST_SUSPEND_API == STD_ON)
void RamTst_Suspend( void );
#endif

#if(RAMTST_RESUME_API == STD_ON)
void RamTst_Resume( void );
#endif

#if(RAMTST_GET_EXECUTION_STATUS_API == STD_ON)
RamTst_ExecutionStatusType RamTst_GetExecutionStatus( void );
#endif

#if(RAMTST_GET_TEST_RESULT_API == STD_ON)
/* @req SWS_RamTst_00103 */
RamTst_TestResultType RamTst_GetTestResult( void );
#endif

#if(RAMTST_GET_TEST_RESULT_PER_BLOCK_API == STD_ON)
/* @req SWS_RamTst_00104 */
RamTst_TestResultType RamTst_GetTestResultPerBlock( RamTst_NumberOfBlocksType BlockID );
#endif

#if(RAMTST_GET_ALG_PARAMS_API == STD_ON)
/* @req SWS_RamTst_00193 */
RamTst_AlgParamsIdType RamTst_GetAlgParams( void );
#endif

#if(RAMTST_GET_TEST_ALGORITHM_API == STD_ON)
/* @req SWS_RamTst_00106 */
RamTst_AlgorithmType RamTst_GetTestAlgorithm( void );
#endif

#if(RAMTST_GET_NUMBER_OF_TESTED_CELLS_API == STD_ON)
RamTst_NumberOfTestedCellsType RamTst_GetNumberOfTestedCells( void );
#endif

#if(RAMTST_SELECT_ALG_PARAMS_API == STD_ON)
/* @req SWS_RamTst_00105 */
void RamTst_SelectAlgParams( RamTst_AlgParamsIdType NewAlgParamsId );
#endif

#if(RAMTST_CHANGE_NUMBER_OF_TESTED_CELLS_API == STD_ON)
void RamTst_ChangeNumberOfTestedCells( RamTst_NumberOfTestedCellsType NewNumberOfTestedCells );
#endif

#if(RAMTST_RUN_PARTIAL_TEST_API == STD_ON)
void RamTst_RunPartialTest(RamTst_NumberOfBlocksType BlockId );
#endif

#if(RAMTST_RUN_FULL_TEST_API == STD_ON)
void RamTst_RunFullTest( void );
#endif


void RamTst_MainFunction( void );

#if (RAMTST_GET_VERSION_INFO_API == STD_ON)
/* @req SWS_RamTst_00109 */
void RamTst_GetVersionInfo( Std_VersionInfoType *versionInfo );
#endif

#endif /* RAMTST_H_ */
