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

/* General requirements */
/* @req SWS_RamTst_00067 *//* Errors detected */
/* @req SWS_RamTst_00098 *//* Imported types */
/* @req SWS_RamTst_00167 *//* Variant pre-compile */
/* !req SWS_RamTst_00168 *//* Variant link-time */
/* @req SWS_RamTst_00206 *//* Foreground RAM test as an synchronous service. */

#include "RamTst.h"
#if ( RAMTST_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
#if !defined(USE_DET)
#error RamTst: Module Det required when dev error detect enabled
#endif
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif

typedef enum {
    RAMTST_UNINIT,
    RAMTST_INIT
}RamTstInitInitStatus;

typedef struct {
    RamTst_TestResultType RamTst_BlockResultBuffer[RAMTST_NUM_BLOCKS];
    RamTst_ExecutionStatusType ExecStatus;
#if (RAMTST_STOP_API == STD_ON)
    RamTst_ExecutionStatusType OrderedStatus;
#endif
    RamTst_AlgParamsIdType ActiveAlgParamID;
    RamTstInitInitStatus InitStatus;
    RamTst_TestResultType CurrentTestResult;
}RamTstInternalType;


static RamTstInternalType RamTst_Internal = {
        .ExecStatus = RAMTST_EXECUTION_UNINIT,/* @req SWS_RamTst_00006 */
#if (RAMTST_STOP_API == STD_ON)
        .OrderedStatus = RAMTST_EXECUTION_UNINIT,
#endif
        .ActiveAlgParamID = 0,/* @req SWS_RamTst_00013 */
        .InitStatus = RAMTST_UNINIT,
        .CurrentTestResult = RAMTST_RESULT_NOT_TESTED/* @req SWS_RamTst_00012 */
};


/*lint -emacro(904,VALIDATE,VALIDATE_W_RV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/

/* Development error macros. */
#if ( RAMTST_DEV_ERROR_DETECT == STD_ON )
/* @req SWS_RamTst_00112 */
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
            (void)Det_ReportError(RAMTST_MODULE_ID,0,_api,_err); \
            return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(RAMTST_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

#if (RAMTST_MARCH_TEST_SELECTED == STD_ON)
static RamTst_TestResultType ramTstMarchX(uint32 *sa, uint32 *ea);
#endif

#ifdef HOST_TEST
static uint8 induceerror_flag;
static RamTst_NumberOfBlocksType induceErrorBlockId = 0;

void InduceErrorOnce(RamTst_NumberOfBlocksType BlockID);
void Getexecstatus(RamTst_ExecutionStatusType **ptr);
void getActiveAlgParamID(RamTst_AlgParamsIdType **ptr);
void getResultBuffer(RamTst_TestResultType **ptr);

void InduceErrorOnce(RamTst_NumberOfBlocksType BlockID) {
    induceErrorBlockId = BlockID;
}

void Getexecstatus(RamTst_ExecutionStatusType **ptr) {
    if (NULL == ptr) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    *ptr = &RamTst_Internal.ExecStatus;
}

#if (RAMTST_STOP_API == STD_ON)
void Getorderedstatus(RamTst_ExecutionStatusType **ptr);
void Getorderedstatus(RamTst_ExecutionStatusType **ptr) {
    if (NULL == ptr) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    *ptr = &RamTst_Internal.OrderedStatus;
}
#endif

void getActiveAlgParamID(RamTst_AlgParamsIdType **ptr) {
    if (NULL == ptr) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    *ptr = &RamTst_Internal.ActiveAlgParamID;
}

void getResultBuffer(RamTst_TestResultType **ptr) {
    if (NULL == ptr) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    *ptr = &RamTst_Internal.RamTst_BlockResultBuffer[0];
}
#endif

/* @req SWS_RamTst_00099 */
/* @req SWS_RamTst_00093 */
void RamTst_Init(void)
{
    /* @req SWS_RamTst_00089 */
    RamTst_Internal.InitStatus = RAMTST_INIT;

    /* @req SWS_RamTst_00096 */
    VALIDATE( ( RAMTST_EXECUTION_UNINIT == RamTst_Internal.ExecStatus ), RAMTST_INIT_SERVICE_ID, RAMTST_E_STATUS_FAILURE);

    /* @req SWS_RamTst_00007 */
    RamTst_Internal.ExecStatus = RAMTST_EXECUTION_STOPPED;

    RamTst_Internal.ActiveAlgParamID = RamTst_ConfigParams.RamTstDefaultAlgParamsId;

    for( RamTst_NumberOfBlocksType i = 0; i < RAMTST_NUM_BLOCKS; i++ ) {
        RamTst_Internal.RamTst_BlockResultBuffer[i] = RAMTST_RESULT_NOT_TESTED;
    }

    RamTst_Internal.CurrentTestResult = RAMTST_RESULT_NOT_TESTED;
}

/* @req SWS_RamTst_00146 */
void RamTst_DeInit( void )
{
    /* @req SWS_RamTst_00089 */
    VALIDATE( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_INIT_SERVICE_ID, RAMTST_E_UNINIT);

    /* @req SWS_RamTst_00147 */
    RamTst_Internal.ExecStatus = RAMTST_EXECUTION_UNINIT;

    RamTst_Internal.InitStatus = RAMTST_UNINIT;

    RamTst_Internal.CurrentTestResult = RAMTST_RESULT_NOT_TESTED;

}

#if (RAMTST_STOP_API == STD_ON)
/* @req SWS_RamTst_00100 */
void RamTst_Stop( void )
{
    /* @req SWS_RamTst_00089 */
    VALIDATE( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_INIT_SERVICE_ID, RAMTST_E_UNINIT);

    /* @req SWS_RamTst_00033 */
    VALIDATE( !((RamTst_Internal.ExecStatus == RAMTST_EXECUTION_UNINIT) || (RamTst_Internal.ExecStatus == RAMTST_EXECUTION_STOPPED)), RAMTST_STOP_SERVICE_ID, RAMTST_E_STATUS_FAILURE);

    /* @req SWS_RamTst_00014 */
    RamTst_Internal.OrderedStatus = RAMTST_EXECUTION_STOPPED;

}
#endif

#if(RAMTST_ALLOW_API == STD_ON)
/* @req SWS_RamTst_00149 */
void RamTst_Allow( void )
{
    /* @req SWS_RamTst_00089 */
    VALIDATE( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_INIT_SERVICE_ID, RAMTST_E_UNINIT);

    /* @req SWS_RamTst_00170 */
    VALIDATE( ( (RAMTST_EXECUTION_STOPPED) == RamTst_Internal.ExecStatus ), RAMTST_ALLOW_SERVICE_ID, RAMTST_E_STATUS_FAILURE);

    /* @req SWS_RamTst_00169 */
    RamTst_Internal.ExecStatus = RAMTST_EXECUTION_RUNNING;
}
#endif

#if(RAMTST_SUSPEND_API == STD_ON)
void RamTst_Suspend( void )
{
    /* Not supported */
}
#endif

#if(RAMTST_RESUME_API == STD_ON)
void RamTst_Resume( void )
{
    /* Not supported */
}
#endif

/* @req SWS_RamTst_00109 */
#if(RAMTST_GET_VERSION_INFO_API == STD_ON)
void RamTst_GetVersionInfo( Std_VersionInfoType* versioninfo )
{
    /* @req SWS_RamTst_00089 */
    VALIDATE( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_INIT_SERVICE_ID, RAMTST_E_UNINIT);

    /* @req SWS_RamTst_00222*/
    VALIDATE( (versioninfo != NULL), RAMTST_GETVERSIONINFO_SERVICE_ID, RAMTST_E_PARAM_POINTER );

    if(versioninfo != NULL) { /*lint !e774 Can be evaluated as false if used incorrectly by the user */
        versioninfo->vendorID = RAMTST_VENDOR_ID;
        versioninfo->moduleID = RAMTST_MODULE_ID;
        versioninfo->sw_major_version = RAMTST_SW_MAJOR_VERSION;
        versioninfo->sw_minor_version = RAMTST_SW_MINOR_VERSION;
        versioninfo->sw_patch_version = RAMTST_SW_PATCH_VERSION;
    }
}
#endif

#if(RAMTST_GET_EXECUTION_STATUS_API == STD_ON)
RamTst_ExecutionStatusType RamTst_GetExecutionStatus( void )
{
    /* Not supported */
}
#endif

#if(RAMTST_GET_TEST_RESULT_API == STD_ON)
RamTst_TestResultType RamTst_GetTestResult( void )
{
    /* @req SWS_RamTst_00089 */
    VALIDATE_W_RV( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_GETTESTRESULT_SERVICE_ID, RAMTST_E_UNINIT, RAMTST_RESULT_NOT_TESTED);
    /* @req SWS_RamTst_00024 */
    return RamTst_Internal.CurrentTestResult;
}
#endif

#if(RAMTST_GET_TEST_RESULT_PER_BLOCK_API == STD_ON)
RamTst_TestResultType RamTst_GetTestResultPerBlock( RamTst_NumberOfBlocksType BlockID )
{
    RamTst_TestResultType status;
    /* @req SWS_RamTst_00089 */
    VALIDATE_W_RV( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_GETRESULTPERBLOCK_SERVICE_ID, RAMTST_E_UNINIT, RAMTST_RESULT_NOT_TESTED);

    /* Find the block */
    const RamTst_BlockParamsType *blockParamPtr = NULL;
    const RamTst_AlgParamsType *algParams = NULL;
    for(RamTst_AlgParamsIdType i = 0; (i < RamTst_ConfigParams.RamTstNumberOfAlgParamSets) && (NULL == blockParamPtr); i++) {
        algParams = &RamTst_ConfigParams.RamTstAlgParams[i];
        /* Run for all blocks */
        for(RamTst_NumberOfBlocksType block = 0; (block < algParams->RamTstNumberOfBlocks) && (NULL == blockParamPtr); block++) {
            if(algParams->RamTstBlockParams[block].RamTstBlockId == BlockID) {
                blockParamPtr = &algParams->RamTstBlockParams[block];
            }
        }
    }

    if(NULL != blockParamPtr) {
        /* @req SWS_RamTst_00038 */
        status = RamTst_Internal.RamTst_BlockResultBuffer[blockParamPtr->RamTstBlockIndex];
    } else {
        /* Invalid block id */
#if ( RAMTST_DEV_ERROR_DETECT == STD_ON )
        /* @req SWS_RamTst_00039 */
        (void)Det_ReportError(RAMTST_MODULE_ID, 0, RAMTST_GETRESULTPERBLOCK_SERVICE_ID, RAMTST_E_OUT_OF_RANGE);
#endif
        status = RAMTST_RESULT_UNDEFINED;
    }
    return status;
}
#endif

#if(RAMTST_GET_ALG_PARAMS_API == STD_ON)
RamTst_AlgParamsIdType RamTst_GetAlgParams( void )
{
    /* @req SWS_RamTst_00089 */
    VALIDATE_W_RV( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_GETALGPARAMS_SERVICE_ID, RAMTST_E_UNINIT, 0);
    /* @req SWS_RamTst_00194 */
    return RamTst_Internal.ActiveAlgParamID;

}
#endif

#if(RAMTST_GET_TEST_ALGORITHM_API == STD_ON)
RamTst_AlgorithmType RamTst_GetTestAlgorithm( void )
{
    /* @req SWS_RamTst_00089 */
    VALIDATE_W_RV( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_GETTESTALGORITHM_SERVICE_ID, RAMTST_E_UNINIT, RAMTST_ALGORITHM_UNDEFINED);

    RamTst_AlgorithmType algorithm = RAMTST_ALGORITHM_UNDEFINED;/* @req SWS_RamTst_00188 */
    for(RamTst_AlgParamsIdType i = 0; (i < RamTst_ConfigParams.RamTstNumberOfAlgParamSets) && (RAMTST_ALGORITHM_UNDEFINED == algorithm); i++) {
        if( RamTst_ConfigParams.RamTstAlgParams[i].RamTstAlgParamsId == RamTst_Internal.ActiveAlgParamID ) {
            algorithm = RamTst_ConfigParams.RamTstAlgParams[i].RamTstAlgorithm;
        }
    }
    /* @req SWS_RamTst_00021 */
    return algorithm;
}
#endif

#if(RAMTST_GET_NUMBER_OF_TESTED_CELLS_API == STD_ON)
RamTst_NumberOfTestedCellsType RamTst_GetNumberOfTestedCells( void )
{
    /* Not supported */
}
#endif

#if(RAMTST_SELECT_ALG_PARAMS_API == STD_ON)
void RamTst_SelectAlgParams( RamTst_AlgParamsIdType NewAlgParamsId )
{
    /* @req SWS_RamTst_00089 */
    VALIDATE( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_SELECTALGPARAMS_SERVICE_ID, RAMTST_E_UNINIT);

    /* @req SWS_RamTst_00097 */
    VALIDATE( ( RamTst_Internal.ExecStatus == RAMTST_EXECUTION_STOPPED ), RAMTST_SELECTALGPARAMS_SERVICE_ID, RAMTST_E_STATUS_FAILURE);

    /* Check parameter NewAlgParamsId */
    const RamTst_AlgParamsType *algParams = NULL;
    for(RamTst_AlgParamsIdType i = 0; (i < RamTst_ConfigParams.RamTstNumberOfAlgParamSets) && (NULL == algParams); i++) {
        if( RamTst_ConfigParams.RamTstAlgParams[i].RamTstAlgParamsId == NewAlgParamsId ) {
            algParams = &RamTst_ConfigParams.RamTstAlgParams[i];
            /* @req SWS_RamTst_00094 */
            for(RamTst_NumberOfBlocksType block = 0; block < algParams->RamTstNumberOfBlocks; block++) {
                RamTst_Internal.RamTst_BlockResultBuffer[algParams->RamTstBlockParams[block].RamTstBlockIndex] = RAMTST_RESULT_NOT_TESTED;
            }
            /* @req SWS_RamTst_00085 */
            RamTst_Internal.ActiveAlgParamID = NewAlgParamsId;/* @req SWS_RamTst_00083 */
            RamTst_Internal.ExecStatus = RAMTST_EXECUTION_STOPPED;
            RamTst_Internal.CurrentTestResult = RAMTST_RESULT_NOT_TESTED;
        }
    }
    /* @req SWS_RamTst_00084 */
    VALIDATE( ( NULL != algParams ), RAMTST_SELECTALGPARAMS_SERVICE_ID, RAMTST_E_OUT_OF_RANGE);
}
#endif

#if(RAMTST_CHANGE_NUMBER_OF_TESTED_CELLS_API == STD_ON)
void RamTst_ChangeNumberOfTestedCells( RamTst_NumberOfTestedCellsType NewNumberOfTestedCells )
{
    /* Not supported */
}
#endif

#if (RAMTST_MARCH_TEST_SELECTED == STD_ON)
/* Algorithm March X
Step1: write 0 with up addressing order;
Step2: read 0 and write 1 with up addressing order;
Step3: read 1 and write 0 with down addressing order;
Step4: read0 with down addressing order.
*/

static RamTst_TestResultType ramTstMarchX(uint32 *sa, uint32 *ea)
/*lint --e{946} Pointing at hardware RAM, which can be considered as a single object*/
{

    RamTst_TestResultType status;
    boolean ret;
    uint32 *ptr_start;
    uint32 *ptr_end;
    status = RAMTST_RESULT_OK;
    ptr_end = ea;
    ptr_start = sa;
    ret = TRUE;

    /* Step1:write 0 with up addressing order */
    for( ;ptr_start < ptr_end;ptr_start++){
        *ptr_start = 0;
    }
#ifdef HOST_TEST
    //inducing error
    if(induceerror_flag == 1){
        ptr_start = sa;
        for( ;ptr_start < ptr_end;ptr_start++){
            *ptr_start = 1;
        }
    }
#endif

    /* Step2: read 0 and write 1 with up addressing order */
    ptr_start = sa;
    for( ;ptr_start < ptr_end;ptr_start++){
        if(0 != *ptr_start){
            status = RAMTST_RESULT_NOT_OK;
            ret = FALSE;
            break;
        }
        *ptr_start = 1;
    }
    if (ret == TRUE) {
        /* Step3: read 1 and write 0 with down addressing order */
        ptr_start = sa;
        ptr_end--;
        for( ; ptr_end >= ptr_start;ptr_end--){
            if(1 != *ptr_end){
                status = RAMTST_RESULT_NOT_OK;
                ret = FALSE;
                break;
            }
            *ptr_end = 0;
        }
        if (ret == TRUE) {
            /* Step4: read0 with down addressing order. */
            ptr_end = ea;
            ptr_end--;
            for( ; ptr_end >= ptr_start;ptr_end--){
                if(0 != *ptr_end){
                    status = RAMTST_RESULT_NOT_OK;
                    break;
                }
            }
        }
    }
    return status;
}
#endif

#if (RAMTST_GALPAT_TEST_SELECTED == STD_ON)
RamTst_TestResultType RamTst_Galpat(uint32 start_addr, uint32 end_addr)
{
    /* Not supported */
}
#endif

#if (RAMTST_ABRAHAM_TEST_SELECTED == STD_ON)
RamTst_TestResultType RamTst_Abraham(uint32 start_addr, uint32 end_addr)
{
    /* Not supported */
}
#endif

#if (RAMTST_TRANSP_GALPAT_TEST_SELECTED == STD_ON)
RamTst_TestResultType RamTst_Transp_Galpat(uint32 start_addr, uint32 end_addr)
{
    /* Not supported */
}
#endif

#if (RAMTST_WALK_PATH_TEST_SELECTED == STD_ON)
RamTst_TestResultType RamTst_Walk_Path(uint32 start_addr, uint32 end_addr)
{
    /* Not supported */
}
#endif

#if (RAMTST_CHECKERBOARD_TEST_SELECTED == STD_ON)
/* Algorithm Checkerboard
 * Step1: Write 0 and Write 1 to alternate cells to form checkboard pattern
 * Step2: Read all cells
 * Step3: Write 1 and Write 0 to alternate cells to form checkboard pattern
 * Step4: Read all cells
 * */
RamTst_TestResultType RamTst_CheckerBoard(uint32 start_addr, uint32 end_addr)
{
    /* Not supported */
}
#endif

#if(RAMTST_RUN_PARTIAL_TEST_API == STD_ON)
void RamTst_RunPartialTest(RamTst_NumberOfBlocksType BlockId )
{
    /* Not supported */
}
#endif

#if(RAMTST_RUN_FULL_TEST_API == STD_ON)
/* @req SWS_RamTst_00195 */
void RamTst_RunFullTest( void )
{
    /* @req SWS_RamTst_00202 */
    /* !req SWS_RamTst_00207 *//* While test running block is NOT_TESTED */
    /* @req SWS_RamTst_00089 */
    VALIDATE( ( RamTst_Internal.InitStatus != RAMTST_UNINIT ), RAMTST_INIT_SERVICE_ID, RAMTST_E_UNINIT);
    /* @req SWS_RamTst_00210 */
    VALIDATE( ( (RAMTST_EXECUTION_STOPPED) == RamTst_Internal.ExecStatus ), RAMTST_RUNFULLTEST_SERVICE_ID, RAMTST_E_STATUS_FAILURE);
    const RamTst_AlgParamsType *algParams = NULL;

    /* Find config for the active algorithm */
    for(RamTst_AlgParamsIdType i = 0; (i < RamTst_ConfigParams.RamTstNumberOfAlgParamSets) && (NULL == algParams); i++) {
        if( RamTst_ConfigParams.RamTstAlgParams[i].RamTstAlgParamsId == RamTst_Internal.ActiveAlgParamID ) {
            algParams = &RamTst_ConfigParams.RamTstAlgParams[i];
        }
    }
    if( NULL != algParams ) {
        /* @req SWS_RamTst_00211 */
        RamTst_Internal.ExecStatus = RAMTST_EXECUTION_RUNNING;

        RamTst_Internal.CurrentTestResult = RAMTST_RESULT_UNDEFINED;

        /* @req SWS_RamTst_00196 */

        /* Run for all blocks */
        for(RamTst_NumberOfBlocksType i = 0; i<algParams->RamTstNumberOfBlocks;i++)
        {
            RamTst_Internal.RamTst_BlockResultBuffer[algParams->RamTstBlockParams[i].RamTstBlockIndex] = RAMTST_RESULT_NOT_TESTED;
        }
        for(RamTst_NumberOfBlocksType i = 0; i < algParams->RamTstNumberOfBlocks; i++)
        {
            RamTst_TestResultType res;
#ifdef HOST_TEST
            if( algParams->RamTstBlockParams[i].RamTstBlockId == induceErrorBlockId ) {
                induceerror_flag = 1;
            }
#endif
            switch(algParams->RamTstAlgorithm) {
#if (RAMTST_MARCH_TEST_SELECTED == STD_ON)
                case RAMTST_MARCH_TEST:
                    res = ramTstMarchX(algParams->RamTstBlockParams[i].RamTstStartAddress.dataPtr, algParams->RamTstBlockParams[i].RamTstEndAddress.dataPtr);
                    break;
#endif
                default:// not implemented yet
                    res = RAMTST_RESULT_NOT_OK;
                    break;
            }
            /* @req SWS_RamTst_00212 */
            /* @req SWS_RamTst_00213 */
            if(res != RAMTST_RESULT_OK){
#if defined(USE_DEM)
                if(DEM_EVENT_ID_NULL != RamTst_ConfigParams.RAMTST_E_RAM_FAILURE) {
                    /* @req SWS_RamTst_00071 */
                    /* @req SWS_RamTst_00111 */
                    (void)Dem_ReportErrorStatus(RamTst_ConfigParams.RAMTST_E_RAM_FAILURE, DEM_EVENT_STATUS_FAILED);
                }
#endif
                RamTst_Internal.CurrentTestResult = RAMTST_RESULT_NOT_OK;
            }
            RamTst_Internal.RamTst_BlockResultBuffer[algParams->RamTstBlockParams[i].RamTstBlockIndex] = res;
#ifdef HOST_TEST
            induceerror_flag = 0;
#endif
        }
        if( RAMTST_RESULT_UNDEFINED == RamTst_Internal.CurrentTestResult ) {
            RamTst_Internal.CurrentTestResult = RAMTST_RESULT_OK;
        }
            /* @req SWS_RamTst_00211 */
        RamTst_Internal.ExecStatus = RAMTST_EXECUTION_STOPPED;
    }
#ifdef HOST_TEST
    induceErrorBlockId = 0;
#endif
}
#endif

