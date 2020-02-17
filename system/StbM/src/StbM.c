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


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.2.2 */

/* @req SWS_StbM_00051 */ /* For types included from required files*/
/* @req SWS_StbM_00124 */ /* data types uint8, uint16 and sint32 used in the interfaces refer AUTOSAR data types*/
/* @req SWS_StbM_00180 */ /*  StbM shall maintain the Local Time Base autonomously either via StbMLocalTimeRef or via StbMEthGlobalTimeDomainRef */
/* @req SWS_StbM_00193 */ /*  For StbMOffsetTimeBase shall only be valid for StbMSynchronizedTimeBaseIdentifier 16 till 31, Added This check in .chk*/
/* @req SWS_StbM_00245  */ /* STBM shall support VARIANT-PRE-COMPILE, created only cfg.h and cfg.c in xpt */

/* RTE related tagging */
/* @req SWS_StbM_00131  */ /* Local RTE can access the internal behaviour of StbM */
/* @req SWS_StbM_00240  */ /* CS service interface - StbM_GlobalTime_Master*/
/* @req SWS_StbM_00244  */ /* Provider port - GlobalTime_Master*/
/* @req SWS_StbM_00247  */ /* CS service interface - StbM_GlobalTime_Slave*/
/* @req SWS_StbM_00248  */ /* Provider port - GlobalTime_Slave*/

#include <string.h>
#include "StbM.h"
#include "SchM_StbM.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_KERNEL) /* @req SWS_StbM_00065 */
#include "Os.h"
#endif
#if defined(USE_ETHTSYN) /* @req SWS_StbM_00246 */
#include "EthTSyn.h"
#endif
#if defined(USE_NVM)
#include "NvM.h"
#endif
#include "StbM_Internal.h"
#include "StbM_Cfg.h"
#include "Os.h"


#if (STBM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif


/*lint -emacro(904,STBM_DET_REPORTERROR)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/

#if (STBM_DEV_ERROR_DETECT == STD_ON)
#define STBM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(STBM_MODULE_ID, 0, _api, _error); \
        return __VA_ARGS__; \
    }
#else
#define STBM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

#define StbM_SET_BIT(byte,mask) ((byte) |= (mask))
#define StbM_CLR_BIT(byte,mask) ((byte) &= (~mask))
#define StbM_GET_BIT(byte,mask) (byte & (mask))

#define STBM_TIMEOUT_MASK            0x01u
#define STBM_TIMELEAP_MASK           0x02u
#define STBM_SYNC_TO_GATEWAY_MASK    0x04u
#define STBM_GLOBAL_TIME_BASE_MASK   0x08u

/* Configuration of StbM channel */
static const StbM_ConfigType* StbM_ConfigPtr = NULL;

static StbM_Internal_TimebaseType StbM_InternalTimeBases[STBM_SYNC_TIME_BASE_COUNT];

/* Internal run time data */
static StbM_Internal_RunTimeType StbM_Internal_RunTime = {
        .StbMInitStatus = STBM_STATUS_UNINIT, /* @req SWS_StbM_00100 */
        .timeBase       = StbM_InternalTimeBases,
#if (STBM_TRIGGERED_CUSTOMER_COUNT > 0)
        .StbmTriggeredPeriod = {0},
#endif
};


/* Internal function declarations  */
static Std_ReturnType StbM_Internal_ValidateTimeBaseId(const StbM_SynchronizedTimeBaseType timeBaseId, uint8 *timeBaseIndex);
static Std_ReturnType StbM_Internal_UpdateOSCounterTimeStamp(uint8 timeBaseIndex );
static uint64 Convert_To_NanoSec(StbM_TimeStampType timeStampValue);
#if (STBM_TRIGGERED_CUSTOMER_COUNT > 0)
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
static void stbmsyncScheduleTable(uint8 syncTriggerdCount);
#endif
#endif


/**
 *
 * @param version info
 */
/* This service returns the version information of this module. */
/* @req SWS_StbM_00066 */
#if ( STBM_VERSION_INFO_API == STD_ON )
void StbM_GetVersionInfo(Std_VersionInfoType* versioninfo){

    /* @req SWS_StbM_00094*/
    STBM_DET_REPORTERROR((NULL != versioninfo),STBM_SERVICE_ID_GET_VERSION_INFO,STBM_E_PARAM_POINTER);

    STD_GET_VERSION_INFO(versioninfo, STBM);
    return;
}
#endif /* STBM_VERSION_INFO_API */

/**
 *
 * @param ConfigPtr
 */
/* @req SWS_StbM_00052 */
void StbM_Init(const StbM_ConfigType* ConfigPtr){
    uint8 timeBaseIndex;
    /* @req SWS_StbM_00250 */ /* Configuration pointer ConfigPtr shall always have a NULL_PTR value */
    (void)*ConfigPtr; /* It is not used */
    StbM_ConfigPtr = &StbMConfigData;

    StbM_Internal_RunTime.StbMInitStatus = STBM_STATUS_UNINIT ;

    /* @req SWS_StbM_00170 */
    for(timeBaseIndex=0; timeBaseIndex < STBM_SYNC_TIME_BASE_COUNT;timeBaseIndex++){

        memset(&StbM_Internal_RunTime.timeBase[timeBaseIndex],0,sizeof(StbM_Internal_TimebaseType));

        /* @req SWS_StbM_00171 */
        if(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMStoreTimebaseNonVolatile == STORAGE_AT_SHUTDOWN){
#if defined(USE_NVM)
            /* NVM handling should be done */
            STBM_DET_REPORTERROR((StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMNvmUseBlockID != STBM_INVALID_NVM_HANDLE),STBM_SERVICE_ID_INIT,STBM_E_INIT_FAILED); /* @req SWS_StbM_00099 */

            (void)NvM_ReadBlock(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMNvmUseBlockID, &StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp);
#endif

        }

        if((StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMIsHardwareTimersupported==TRUE)
                || (StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMLocalTimeRef == STBM_INVALID_OS_COUNTER)){
            StbM_Internal_RunTime.timeBase[timeBaseIndex].synchronisedRawtime = STBM_INVALID_OS_COUNTER;
            StbM_Internal_RunTime.timeBase[timeBaseIndex].rawTimeDiff =STBM_INVALID_OS_COUNTER;
        }
    }
#if (STBM_TRIGGERED_CUSTOMER_COUNT > 0)
    uint8 trigCustCount;  /* added to avoid lint error when no trigger customers are configured*/
    for(trigCustCount=0;trigCustCount<STBM_TRIGGERED_CUSTOMER_COUNT;trigCustCount++)
    {
        StbM_Internal_RunTime.StbmTriggeredPeriod[trigCustCount]=0;
    }
#endif
    StbM_Internal_RunTime.StbMInitStatus = STBM_STATUS_INIT; /* @req SWS_StbM_00121 */

}

/**
 *
 * @param timeBaseId
 * @param timeStampPtr
 * @param userDataPtr
 * @return
 */
/* @req SWS_StbM_00195 */
Std_ReturnType StbM_GetCurrentTime( StbM_SynchronizedTimeBaseType timeBaseId, StbM_TimeStampType* timeStampPtr, StbM_UserDataType* userDataPtr ){
    Std_ReturnType status;
    uint8 timeBaseIndex;
    uint8 absoluteTimebaseIndex;

#if (defined(USE_ETHTSYN) && (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON))
    StbM_TimeStampType internalTimeStampValue;
    EthTSyn_SyncStateType syncState;
#endif
    status = E_NOT_OK;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_GET_CURRENT_TIME,STBM_E_NOT_INITIALIZED,status);
    status = StbM_Internal_ValidateTimeBaseId(timeBaseId,&timeBaseIndex);
    /* If time base Id not found */
    STBM_DET_REPORTERROR((status != E_NOT_OK),STBM_SERVICE_ID_GET_CURRENT_TIME,STBM_E_PARAM,status);    /* @req SWS_StbM_00196 */
    STBM_DET_REPORTERROR((NULL != timeStampPtr),STBM_SERVICE_ID_GET_CURRENT_TIME,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00197 */
    STBM_DET_REPORTERROR((NULL != userDataPtr),STBM_SERVICE_ID_GET_CURRENT_TIME,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00197 */

    if(timeBaseId <= VALID_TIME_DOMAIN_MAX){ /* @req SWS_StbM_00173 */

        /** Assumed if both local and eth time reference is selected Eth has more priority */
       if(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMEthGlobalTimeDomainRef!=STBM_INVALID_ETH_TIMEDOMAIN){

#if  (defined(USE_ETHTSYN) && (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON))
            status =  EthTSyn_GetCurrentTime( timeBaseId, &internalTimeStampValue, &syncState );

            if(status == E_OK){
                StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds = internalTimeStampValue.nanoseconds;
                StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds = internalTimeStampValue.seconds;
                StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi = internalTimeStampValue.secondsHi;

                switch(syncState){ /* @req SWS_StbM_00176 */
                    case ETHTSYN_SYNC:
                        StbM_SET_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_GLOBAL_TIME_BASE_MASK);
                        break;
                    case ETHTSYN_UNSYNC:/* @req SWS_StbM_00189 */
                        if (STBM_GLOBAL_TIME_BASE_MASK == (StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus & STBM_GLOBAL_TIME_BASE_MASK)){
                            StbM_SET_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMEOUT_MASK);
                        }
                        break;
                    case ETHTSYN_UNCERTAIN:
                        status = E_NOT_OK;
                        break;
                    case ETHTSYN_NEVERSYNC:
                        StbM_CLR_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_GLOBAL_TIME_BASE_MASK);
                        StbM_CLR_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMEOUT_MASK);
                        break;
                    default:
                        status = E_NOT_OK;
                        break;
                 }
                timeStampPtr->timeBaseStatus = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus;
                timeStampPtr->nanoseconds = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds;
                timeStampPtr->seconds = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds;
                timeStampPtr->secondsHi = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi ;
            }
#endif

       }else if(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMLocalTimeRef != STBM_INVALID_OS_COUNTER){
            /* @req SWS_StbM_00178 */
            status = StbM_Internal_UpdateOSCounterTimeStamp(timeBaseIndex);
            timeStampPtr->nanoseconds = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds;
            timeStampPtr->seconds = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds;
            timeStampPtr->secondsHi = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi ;
            timeStampPtr->timeBaseStatus = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus;
       }else{
           /* Do nothing */
        }
        userDataPtr->userByte0 = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte0;
        userDataPtr->userByte1 = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte1;
        userDataPtr->userByte2 = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte2;
        userDataPtr->userDataLength = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userDataLength;
    }else if ((timeBaseId >= OFFSET_TIME_DOMAIN_MIN)&&(timeBaseId <= OFFSET_TIME_DOMAIN_MAX)){
        if(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMOffsetTimeBaseID != STBM_INVALID_TIMEBASE){
            /* @req SWS_StbM_00177*/
            status = StbM_Internal_ValidateTimeBaseId(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMOffsetTimeBaseID,&absoluteTimebaseIndex);
            if(status == E_OK){

                timeStampPtr->nanoseconds = StbM_Internal_RunTime.timeBase[absoluteTimebaseIndex].StbMCurrentTimeStamp.nanoseconds+StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds;
                timeStampPtr->seconds = StbM_Internal_RunTime.timeBase[absoluteTimebaseIndex].StbMCurrentTimeStamp.seconds + StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds;
                timeStampPtr->secondsHi = StbM_Internal_RunTime.timeBase[absoluteTimebaseIndex].StbMCurrentTimeStamp.secondsHi + StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi;

                if ((StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds != 0)
                      || (StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds != 0)
                      || (StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi !=0)) {

                    timeStampPtr->timeBaseStatus = StbM_Internal_RunTime.timeBase[absoluteTimebaseIndex].StbMCurrentTimeStamp.timeBaseStatus;
                } else {
                    timeStampPtr->timeBaseStatus = 0;
                }
            }
        }
    }else{
        status = E_NOT_OK;
    }

  return status;

}

/**
 *
 * @param timeBaseId
 * @param timeStampPtr
 * @param userDataPtr
 * @return
 */
/* !req SWS_StbM_00200 */
#if (STBM_GET_CURRENT_TIME_EXT_AVIALBLE == STD_ON)
Std_ReturnType StbM_GetCurrentTimeExtended( StbM_SynchronizedTimeBaseType timeBaseId, StbM_TimeStampExtendedType* timeStampPtr, StbM_UserDataType* userDataPtr ){

    Std_ReturnType status;
    uint8 timeBaseIndex;

    status = E_NOT_OK;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_GET_CURRENT_TIME_EXTENDED,STBM_E_NOT_INITIALIZED,status);
    status = StbM_Internal_ValidateTimeBaseId(timeBaseId,&timeBaseIndex);
    /* If time base Id not found */
    STBM_DET_REPORTERROR((status != E_NOT_OK),STBM_SERVICE_ID_GET_CURRENT_TIME_EXTENDED,STBM_E_PARAM,E_NOT_OK);    /* !req SWS_StbM_00201 */
    STBM_DET_REPORTERROR((NULL != timeStampPtr),STBM_SERVICE_ID_GET_CURRENT_TIME_EXTENDED,STBM_E_PARAM_POINTER,E_NOT_OK); /* !req SWS_StbM_00202 */
    STBM_DET_REPORTERROR((NULL != userDataPtr),STBM_SERVICE_ID_GET_CURRENT_TIME_EXTENDED,STBM_E_PARAM_POINTER,E_NOT_OK); /* !req SWS_StbM_00202 */



return status;

}
#endif
/**
 *
 * @param timeStampRawPtr
 * @return
 */
/* @req SWS_StbM_00205 */
Std_ReturnType StbM_GetCurrentTimeRaw( StbM_TimeStampRawType* timeStampRawPtr ){
    Std_ReturnType status;
    StbM_TimeStampRawType timeStampRawdiff;
    uint8 timeBaseIndex;
    uint8 rawtimeindex;

    status = E_OK;
    rawtimeindex =0;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_GET_CURRENT_RAW,STBM_E_NOT_INITIALIZED,E_NOT_OK);
    STBM_DET_REPORTERROR((NULL != timeStampRawPtr),STBM_SERVICE_ID_GET_CURRENT_RAW,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00206 */

    //Consider first element as smallest
    timeStampRawdiff = StbM_Internal_RunTime.timeBase[0].rawTimeDiff;

    /* finding the smallest difference from all the time bases for the best raw time*/
    for (timeBaseIndex = 0; timeBaseIndex < STBM_SYNC_TIME_BASE_COUNT; timeBaseIndex++){
        if(StbM_Internal_RunTime.timeBase[timeBaseIndex].rawTimeDiff < timeStampRawdiff){
         timeStampRawdiff = StbM_Internal_RunTime.timeBase[timeBaseIndex].rawTimeDiff;
         rawtimeindex = timeBaseIndex;
        }
     }

    /* @req SWS_StbM_00174 */
    *timeStampRawPtr = StbM_Internal_RunTime.timeBase[rawtimeindex].synchronisedRawtime;

  return status;

}

/**
 *
 * @param givenTimeStamp
 * @param timeStampDiffPtr
 * @return
 */
/* @req SWS_StbM_00209 */
Std_ReturnType StbM_GetCurrentTimeDiff( StbM_TimeStampRawType givenTimeStamp, StbM_TimeStampRawType* timeStampDiffPtr ){

    Std_ReturnType status;
    StbM_TimeStampRawType timeStampRawdiff;
    uint8 timeBaseIndex;
    uint8 rawtimeindex;

    status = E_OK;
    rawtimeindex=0;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_GET_CURRENT_DIFF,STBM_E_NOT_INITIALIZED,E_NOT_OK);
    STBM_DET_REPORTERROR((NULL != timeStampDiffPtr),STBM_SERVICE_ID_GET_CURRENT_DIFF,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00210 */

    /* Consider first element as smallest */
    timeStampRawdiff = StbM_Internal_RunTime.timeBase[0].rawTimeDiff;

    /* finding the smallest difference from all the time bases for the best raw time*/
    for (timeBaseIndex = 0; timeBaseIndex < STBM_SYNC_TIME_BASE_COUNT; timeBaseIndex++) {
        if (StbM_Internal_RunTime.timeBase[timeBaseIndex].rawTimeDiff < timeStampRawdiff){
          timeStampRawdiff = StbM_Internal_RunTime.timeBase[timeBaseIndex].rawTimeDiff;
          rawtimeindex = timeBaseIndex;
        }
    }
    /* @req SWS_StbM_00175 */

    if(StbM_Internal_RunTime.timeBase[rawtimeindex].synchronisedRawtime < givenTimeStamp){
        /*lint -e{9048} to fix UINT32_MAX unsigned integer literal without a 'U' suffix */
        *timeStampDiffPtr = (UINT32_MAX - givenTimeStamp) + StbM_Internal_RunTime.timeBase[rawtimeindex].synchronisedRawtime + 1U;
    }else {
        *timeStampDiffPtr = StbM_Internal_RunTime.timeBase[rawtimeindex].synchronisedRawtime - givenTimeStamp ;
    }

    return status;
}

/**
 *
 * @param timeBaseId
 * @param timeStampPtr
 * @param userDataPtr
 * @return
 */
/* @req SWS_StbM_00213 */
Std_ReturnType StbM_SetGlobalTime( StbM_SynchronizedTimeBaseType timeBaseId, const StbM_TimeStampType* timeStampPtr, const StbM_UserDataType* userDataPtr ){

    Std_ReturnType status;
    uint8 timeBaseIndex;
    status = E_NOT_OK;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_SET_GLOBAL_TIME,STBM_E_NOT_INITIALIZED,status);
    status = StbM_Internal_ValidateTimeBaseId(timeBaseId,&timeBaseIndex);
    /* If time base Id not found */
    STBM_DET_REPORTERROR((status != E_NOT_OK),STBM_SERVICE_ID_SET_GLOBAL_TIME,STBM_E_PARAM,E_NOT_OK);    /* @req SWS_StbM_00214 */
    STBM_DET_REPORTERROR((NULL != timeStampPtr),STBM_SERVICE_ID_SET_GLOBAL_TIME,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00215 */

    if(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMIsSystemWideGlobalTimeMaster == TRUE){
        /* @req SWS_StbM_00181 */
        StbM_SET_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_GLOBAL_TIME_BASE_MASK);
        StbM_CLR_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_SYNC_TO_GATEWAY_MASK);
        StbM_CLR_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMELEAP_MASK);
        StbM_CLR_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMEOUT_MASK);

        StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds = timeStampPtr->nanoseconds;
        StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds = timeStampPtr->seconds;
        StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi= timeStampPtr->secondsHi;



        if(userDataPtr != NULL){
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte0= userDataPtr->userByte0;
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte1 = userDataPtr->userByte1;
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte2 = userDataPtr->userByte2;
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userDataLength = userDataPtr->userDataLength;
        }

#if  (defined(USE_ETHTSYN) && (ETHTSYN_HARDWARE_TIMESTAMP_SUPPORT == STD_ON))
        /* This is as per sequence diagram in EthTSyn module 9.5 section */
        status = EthTSyn_SetGlobalTime(timeBaseId,&StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp);
#endif


    }

return status;

}

/**
 *
 * @param timeBaseId
 * @param userDataPtr
 * @return
 */
/* @req SWS_StbM_00218 */
Std_ReturnType StbM_SetUserData( StbM_SynchronizedTimeBaseType timeBaseId, const StbM_UserDataType* userDataPtr){

    Std_ReturnType status;
    uint8 timeBaseIndex;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_SET_USER_DATA,STBM_E_NOT_INITIALIZED,E_NOT_OK);
    status = StbM_Internal_ValidateTimeBaseId(timeBaseId,&timeBaseIndex);
    /* If time base Id not found */
    STBM_DET_REPORTERROR((status != E_NOT_OK),STBM_SERVICE_ID_SET_USER_DATA,STBM_E_PARAM,E_NOT_OK);    /* @req SWS_StbM_00219 */
    STBM_DET_REPORTERROR((NULL != userDataPtr),STBM_SERVICE_ID_SET_USER_DATA,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00220 */

    StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte0= userDataPtr->userByte0;
    StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte1 = userDataPtr->userByte1;
    StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte2 = userDataPtr->userByte2;
    StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userDataLength = userDataPtr->userDataLength;

    return status;

}


/**
 *
 * @param timeBaseId
 * @param timeStampPtr
 * @return
 */
/* @req SWS_StbM_00223 */
Std_ReturnType StbM_SetOffset( StbM_SynchronizedTimeBaseType timeBaseId, const StbM_TimeStampType* timeStampPtr ){

    Std_ReturnType status;
    uint8 offsettimeBaseIndex;
    status = E_NOT_OK;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 *//* @req SWS_StbM_00190 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_SET_OFFSET,STBM_E_NOT_INITIALIZED,status);
    status = StbM_Internal_ValidateTimeBaseId(timeBaseId,&offsettimeBaseIndex);
    /* If time base Id not found */
    STBM_DET_REPORTERROR((status != E_NOT_OK),STBM_SERVICE_ID_SET_OFFSET,STBM_E_PARAM,E_NOT_OK);    /* @req SWS_StbM_00224 */
    STBM_DET_REPORTERROR((NULL != timeStampPtr),STBM_SERVICE_ID_SET_OFFSET,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00225 */
    /* @req SWS_StbM_00191 */
    if ((timeBaseId >= OFFSET_TIME_DOMAIN_MIN)&&(timeBaseId <= OFFSET_TIME_DOMAIN_MAX)){
            /* @req SWS_StbM_00177 */
            StbM_Internal_RunTime.timeBase[offsettimeBaseIndex].StbMCurrentTimeStamp.nanoseconds =timeStampPtr->nanoseconds;
            StbM_Internal_RunTime.timeBase[offsettimeBaseIndex].StbMCurrentTimeStamp.seconds =timeStampPtr->seconds;
            StbM_Internal_RunTime.timeBase[offsettimeBaseIndex].StbMCurrentTimeStamp.secondsHi =timeStampPtr->secondsHi;

    } else {
        status = E_NOT_OK;
    }


    return status;
}


/**
 *
 * @param timeBaseId
 * @param timeStampPtr
 * @return
 */
/* @req SWS_StbM_00228 */
Std_ReturnType StbM_GetOffset( StbM_SynchronizedTimeBaseType timeBaseId, StbM_TimeStampType* timeStampPtr ){

    Std_ReturnType status;
    uint8 timeBaseIndex;
    status = E_NOT_OK;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_GET_OFFSET,STBM_E_NOT_INITIALIZED,status);
    status = StbM_Internal_ValidateTimeBaseId(timeBaseId,&timeBaseIndex);
    /* If time base Id not found */
    STBM_DET_REPORTERROR((status != E_NOT_OK),STBM_SERVICE_ID_GET_OFFSET,STBM_E_PARAM,E_NOT_OK);    /* @req SWS_StbM_00229 */
    STBM_DET_REPORTERROR((NULL != timeStampPtr),STBM_SERVICE_ID_GET_OFFSET,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00230 */
    /* @req SWS_StbM_00191 *//* @req SWS_StbM_00192 */
    if ((timeBaseId >= OFFSET_TIME_DOMAIN_MIN)&&(timeBaseId <= OFFSET_TIME_DOMAIN_MAX)){

        timeStampPtr->nanoseconds = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds;
        timeStampPtr->seconds     = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds ;
        timeStampPtr->secondsHi     = StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi;

    }else {
        status = E_NOT_OK;
    }


    return status;
}

/**
 *
 * @param timeBaseId
 * @param timeStampPtr
 * @param userDataPtr
 * @param syncToTimeBase
 * @return
 */
/* @req SWS_StbM_00233 */
Std_ReturnType StbM_BusSetGlobalTime( StbM_SynchronizedTimeBaseType timeBaseId, const StbM_TimeStampType* timeStampPtr, const StbM_UserDataType* userDataPtr, boolean syncToTimeBase ){

    /* @req SWS_StbM_00179*/ /** This is generic on updating time base status for each time domain */
    Std_ReturnType status;
    uint8 timeBaseIndex;
    uint64 timeDiffToCmpThreshold;
    uint64 stbmTimestampinNs;
    uint64 recvdTimeStampinNs;
    timeDiffToCmpThreshold =0;
    status = E_NOT_OK;
    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_BUS_SET_GLOBAL_TIME,STBM_E_NOT_INITIALIZED,status);
    status = StbM_Internal_ValidateTimeBaseId(timeBaseId,&timeBaseIndex);
    /* If time base Id not found */
    STBM_DET_REPORTERROR((status != E_NOT_OK),STBM_SERVICE_ID_BUS_SET_GLOBAL_TIME,STBM_E_PARAM,E_NOT_OK);    /* @req SWS_StbM_00234 */
    STBM_DET_REPORTERROR((NULL != timeStampPtr),STBM_SERVICE_ID_BUS_SET_GLOBAL_TIME,STBM_E_PARAM_POINTER,E_NOT_OK); /* @req SWS_StbM_00235 */


    /** from SRS_StbM_20014 considered only for time slave time value will be modified */
     if(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMIsSystemWideGlobalTimeMaster ==FALSE){

            recvdTimeStampinNs = Convert_To_NanoSec(*timeStampPtr);
            stbmTimestampinNs = Convert_To_NanoSec(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp);
            if((StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMIsHardwareTimersupported==FALSE)&&(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMLocalTimeRef!=STBM_INVALID_OS_COUNTER)){

              if(stbmTimestampinNs >recvdTimeStampinNs){
                  timeDiffToCmpThreshold =stbmTimestampinNs - recvdTimeStampinNs;
                  StbM_Internal_RunTime.timeBase[timeBaseIndex].synchronisedRawtime -=(uint32)timeDiffToCmpThreshold;
              }else{
                  timeDiffToCmpThreshold = (recvdTimeStampinNs -stbmTimestampinNs);
                  StbM_Internal_RunTime.timeBase[timeBaseIndex].synchronisedRawtime +=(uint32)timeDiffToCmpThreshold;
              }
              StbM_Internal_RunTime.timeBase[timeBaseIndex].rawTimeDiff =(uint32)timeDiffToCmpThreshold;
            }

            /* @req SWS_StbM_00186 */ /* @req SWS_StbM_00182 */
            if(StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMSyncLossThreshold !=0){

                if(timeDiffToCmpThreshold > StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMSyncLossThreshold){

                    StbM_SET_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMELEAP_MASK);
                }else{
                    StbM_CLR_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMELEAP_MASK);
                }
            }

            /* @req SWS_StbM_00183 *//* @req SWS_StbM_00187 */
            StbM_Internal_RunTime.timeBase[timeBaseIndex].syncLosstimer = StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMSyncLossTimeout;
            StbM_CLR_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMEOUT_MASK);

            if(userDataPtr != NULL){
                StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte0 = userDataPtr->userByte0;
                StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte1 = userDataPtr->userByte1;
                StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userByte2 = userDataPtr->userByte2;
                StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentUserData.userDataLength = userDataPtr->userDataLength;
            }

            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds = timeStampPtr->nanoseconds;
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds     = timeStampPtr->seconds;
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi   = timeStampPtr->secondsHi;

            /* @req SWS_StbM_00185*/ /* @req SWS_StbM_00189 */
            StbM_SET_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_GLOBAL_TIME_BASE_MASK);

            /* @req SWS_StbM_00184 */ /* @req SWS_StbM_00188 */
            if(syncToTimeBase == FALSE){
                StbM_CLR_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_SYNC_TO_GATEWAY_MASK);
            }else{
                StbM_SET_BIT(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_SYNC_TO_GATEWAY_MASK);
            }
       }
    return status;

}

/**
 *
 */
/* @req SWS_StbM_00057 */
void StbM_MainFunction(void){

    uint8 timebaseindex;

    /* @req SWS_StbM_00198 */ /* @req SWS_StbM_00199 */
    STBM_DET_REPORTERROR((STBM_STATUS_INIT == StbM_Internal_RunTime.StbMInitStatus),STBM_SERVICE_ID_MAIN_FUNCTION,STBM_E_NOT_INITIALIZED);

    for(timebaseindex=0;timebaseindex<STBM_SYNC_TIME_BASE_COUNT;timebaseindex++){

        if((StbM_ConfigPtr->StbMSyncTimeBase[timebaseindex].StbMIsHardwareTimersupported==FALSE)&&(StbM_ConfigPtr->StbMSyncTimeBase[timebaseindex].StbMLocalTimeRef!=STBM_INVALID_OS_COUNTER)){

            (void)StbM_Internal_UpdateOSCounterTimeStamp(timebaseindex);

            /* @req SWS_StbM_00187 */
            /* Check if this time base is on a slave port */
            if (FALSE == StbM_ConfigPtr->StbMSyncTimeBase[timebaseindex].StbMIsSystemWideGlobalTimeMaster) {
                if(StbM_Internal_RunTime.timeBase[timebaseindex].syncLosstimer >= STBM_MAIN_FUNC_PERIOD){
                    StbM_Internal_RunTime.timeBase[timebaseindex].syncLosstimer -=STBM_MAIN_FUNC_PERIOD;
                }else{
                    StbM_Internal_RunTime.timeBase[timebaseindex].syncLosstimer =0;
                    StbM_SET_BIT(StbM_Internal_RunTime.timeBase[timebaseindex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMEOUT_MASK);
                }
            } /* Sync loss timer is not run for time base with hardware support */
        }
    }
#if (STBM_TRIGGERED_CUSTOMER_COUNT > 0)
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
    uint8 syncTriggerdCount;

    /* @req SWS_StbM_00084 */
    for(syncTriggerdCount = 0; syncTriggerdCount<STBM_TRIGGERED_CUSTOMER_COUNT;syncTriggerdCount++)
    {
        StbM_Internal_RunTime.StbmTriggeredPeriod[syncTriggerdCount]++;
        if(StbM_ConfigPtr->StbMTriggeredCustomer[syncTriggerdCount].StbMTriggeredCustomerPeriod == StbM_Internal_RunTime.StbmTriggeredPeriod[syncTriggerdCount])
        {
            stbmsyncScheduleTable(syncTriggerdCount);
        }
    }

#endif
#endif

}





/**
 * @brief - To validated the correct time base ID received.
 * @param timeBaseId
 * @param SynctimeBaseIndex
 * @return
 */
static Std_ReturnType StbM_Internal_ValidateTimeBaseId(const StbM_SynchronizedTimeBaseType timeBaseId, uint8 *syncTimeBaseIndex){

 uint8 loopIndex;
 Std_ReturnType retValue;
 retValue = E_NOT_OK;

 for (loopIndex = 0; loopIndex < STBM_SYNC_TIME_BASE_COUNT; loopIndex++) {
    if(StbM_ConfigPtr->StbMSyncTimeBase[loopIndex].StbMSyncTimeBaseId == timeBaseId)
    {
        *syncTimeBaseIndex = loopIndex;
        retValue = E_OK;
        break;
    }
 }
return retValue;
}

#if (STBM_TRIGGERED_CUSTOMER_COUNT > 0)
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
/**
 *
 * @param syncTriggerdCount
 */
static void stbmsyncScheduleTable(uint8 syncTriggerdCount){
    /* @req SWS_StbM_00020 */

    TickType stbmNanoSecToTick;
    Std_ReturnType retValue;
    uint8 internalTimebaseIndex;

    retValue = StbM_Internal_ValidateTimeBaseId(StbM_ConfigPtr->StbMTriggeredCustomer[syncTriggerdCount].StbMSynchronizedTimeBaseRef->StbMSyncTimeBaseId,&internalTimebaseIndex);

    if(E_OK == retValue){
        /* @req SWS_StbM_00077 */
        if((STBM_GLOBAL_TIME_BASE_MASK == StbM_GET_BIT(StbM_Internal_RunTime.timeBase[internalTimebaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_GLOBAL_TIME_BASE_MASK))
                && (FALSE == StbM_GET_BIT(StbM_Internal_RunTime.timeBase[internalTimebaseIndex].StbMCurrentTimeStamp.timeBaseStatus, STBM_TIMEOUT_MASK))){
            stbmNanoSecToTick = (TickType)(StbM_Internal_RunTime.timeBase[internalTimebaseIndex].synchronisedRawtime/OSTICKDURATION);
            /* req SWS_StbM_00092 */  /* states check is done in API stbmsyncScheduleTable */
            /* @req SWS_StbM_00107 */
            (void)SyncScheduleTable(StbM_ConfigPtr->StbMTriggeredCustomer[syncTriggerdCount].StbMOSScheduleTableRef, stbmNanoSecToTick);
        }
        /* Reset the triggerring Counter */
        StbM_Internal_RunTime.StbmTriggeredPeriod[syncTriggerdCount] = 0u;
    }
}
#endif
#endif

/**
 *
 * @param timeStampNSValue - ns value of the time stamp
 * @return
 */
static Std_ReturnType StbM_Internal_UpdateOSCounterTimeStamp(uint8 timeBaseIndex ){

    Std_ReturnType status;
    TickType osElapsedCounterValue;
    uint32 osGetCounterTicksinNs;
    uint64 secondsValue;
    CounterType osCounterId;

    osCounterId = (CounterType) StbM_ConfigPtr->StbMSyncTimeBase[timeBaseIndex].StbMLocalTimeRef;

    status = GetElapsedValue(osCounterId,&StbM_Internal_RunTime.timeBase[timeBaseIndex].stbmInternalOsCounter,&osElapsedCounterValue);
    if(status == E_OK){
        osGetCounterTicksinNs = OS_TICKS2NS_OS_TICK(osElapsedCounterValue);
        StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds += osGetCounterTicksinNs;
        StbM_Internal_RunTime.timeBase[timeBaseIndex].synchronisedRawtime += osGetCounterTicksinNs;
        if(StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds >(STBM_NANOSEC_MAX_VALUE-1) ){
            secondsValue = ((((uint64)StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi << 32u)) | StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds);
            secondsValue++;
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.secondsHi = (uint16)(secondsValue>>32);
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.seconds = (uint32)secondsValue;
            StbM_Internal_RunTime.timeBase[timeBaseIndex].StbMCurrentTimeStamp.nanoseconds-= STBM_NANOSEC_MAX_VALUE;
        }

    }

    return status;
}

/**
 * @brief - To convert StbM_TimeStampType to 48 bit NanoSec value
 * @param timeStampValue
 * @return
 */
static uint64 Convert_To_NanoSec(StbM_TimeStampType timeStampValue)
{
    uint64 convrtedNSValue;
    uint64 secondsValue;

    secondsValue = ((((uint64)timeStampValue.secondsHi << 32u)) | timeStampValue.seconds);
    convrtedNSValue = (1000000000 * secondsValue) + timeStampValue.nanoseconds;
    return (convrtedNSValue);
}



#ifdef HOST_TEST
StbM_Internal_RunTimeType* readinternal_StbMstatus(void );
StbM_Internal_RunTimeType* readinternal_StbMstatus(void)
{
    return &StbM_Internal_RunTime;
}
#endif


