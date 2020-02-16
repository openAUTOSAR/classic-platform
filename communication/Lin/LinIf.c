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

#include "LinIf.h"
#include "LinIf_Types.h"
#include "LinIf_Cbk.h"
#include "Lin.h"
#include "LinSM_Cbk.h"
#include "PduR_LinIf.h"
#if (LINIF_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

/* Development error macros. */
#if ( LINIF_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_LINIF,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_LINIF,0,_api,_err); \
          return (_rv); \
        }

#define DET_REPORT_ERROR(_api,_err) Det_ReportError(MODULE_ID_LINIF, 0, _api, _err);

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#define DET_REPORT_ERROR(_api,_err)
#endif

static const LinIf_ConfigType *LinIf_ConfigPtr;

static LinIf_StatusType LinIfStatus = LINIF_UNINIT;
static LinIf_StatusType LinIfChannelStatus[LINIF_CONTROLLER_CNT];

static LinIf_ScheduleTableType *currentSchedule[LINIF_CONTROLLER_CNT];
static uint16 currentIndex[LINIF_CONTROLLER_CNT];
static uint16 currentDelayInTicks[LINIF_CONTROLLER_CNT];

static boolean newScheduleRequest[LINIF_CONTROLLER_CNT];
static boolean chSleepCmdAttmpt[LINIF_CONTROLLER_CNT]; //This is required for tx a sleep frame

static LinIf_SchHandleType newSchedule[LINIF_CONTROLLER_CNT];

void LinIf_Init( const LinIf_ConfigType* ConfigPtr )
{

    LinIf_ConfigPtr = ConfigPtr;

    /** @req LINIF486 */
    VALIDATE( (LinIf_ConfigPtr!=NULL), LINIF_INIT_SERVICE_ID, LINIF_E_PARAMETER_POINTER );
    /** @req LINIF562 */
    VALIDATE((LinIfStatus != LINIF_INIT), LINIF_INIT_SERVICE_ID, LINIF_E_ALREADY_INITIALIZED);

	uint8 i;
	for (i=0;i<LINIF_CONTROLLER_CNT;i++)
	{
	    /** @req LINIF507 */
		LinIfChannelStatus[i] = LinIf_ConfigPtr->LinIfChannel[i].LinIfStartupState;
		if (LINIF_CHANNEL_SLEEP == LinIfChannelStatus[i]) {
		    (void)Lin_GoToSleepInternal(i);//It is expected that Lin Driver goes to sleep
		}
		/** @req LINIF233 */
		currentSchedule[i] = (LinIf_ScheduleTableType *)LinIf_ConfigPtr->LinIfChannel[i].LinIfScheduleTable;
		currentIndex[i] = 0;
		currentDelayInTicks[i] = 0;
		newScheduleRequest[i] = FALSE;
		chSleepCmdAttmpt[i] = FALSE;
	}
	LinIfStatus = LINIF_INIT;
}

void LinIf_DeInit()
{
	LinIfStatus = LINIF_UNINIT;
}

Std_ReturnType LinIf_Transmit(PduIdType LinTxPduId,const PduInfoType* PduInfoPtr)
{
	(void)LinTxPduId;
	(void)PduInfoPtr;
	// Sporadic frames not supported in this release
	//printf("LinIf_Transmit received request. Id: %d, Data: %d\n", LinTxPduId, *(PduInfoPtr->SduDataPtr));
	return E_OK;
}


Std_ReturnType LinIf_ScheduleRequest(NetworkHandleType Channel,LinIf_SchHandleType Schedule)
{
    /** @req LINIF535 */
	VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
	/** @req LINIF563 */
	VALIDATE_W_RV( (Channel < LINIF_CONTROLLER_CNT), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);
    /** @req LINIF567 */
	VALIDATE_W_RV( (Schedule < LINIF_SCH_CNT), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_SCHEDULE_REQUEST_ERROR, E_NOT_OK);
	VALIDATE_W_RV( (LinIfChannelStatus[Channel] != LINIF_CHANNEL_SLEEP && LinIfChannelStatus[Channel] != LINIF_CHANNEL_SLEEP_TRANS), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_SCHEDULE_REQUEST_ERROR, E_NOT_OK);

	newScheduleRequest[Channel] = TRUE;
	/** @req LINIF389 */
	newSchedule[Channel] = Schedule;
	return E_OK;
}


Std_ReturnType LinIf_GotoSleep(NetworkHandleType Channel)
{
    /** @req LINIF535 */
	VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_GOTOSLEEP_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
	/** @req LINIF564 */
	VALIDATE_W_RV( (Channel < LINIF_CONTROLLER_CNT), LINIF_GOTOSLEEP_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);

	if (LinIfChannelStatus[Channel] == LINIF_CHANNEL_OPERATIONAL) {
	    /** @req LINIF488 */
		LinIfChannelStatus[Channel] = LINIF_CHANNEL_SLEEP_TRANS;
		chSleepCmdAttmpt[Channel] = FALSE;
	}
	/** LINIF597 cannot be implemented since no Lin_GoToSleepInternal */
	/** @req LINIF113 */
	return E_OK;
}


Std_ReturnType LinIf_WakeUp(NetworkHandleType Channel)
{
    /** @req LINIF535 */
	VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_WAKEUP_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
	/** @req LINIF565 */
	VALIDATE_W_RV( (Channel < LINIF_CONTROLLER_CNT), LINIF_WAKEUP_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);

	switch(LinIfChannelStatus[Channel]) {
	    case LINIF_CHANNEL_SLEEP:
	        /** @req LINIF296 */
	        (void)Lin_WakeUp(LinIf_ConfigPtr->LinIfChannel[Channel].LinIfChannelId); //Driver always returns E_OK
	        currentIndex[Channel] = 0;
	        currentDelayInTicks[Channel] = 0;
	        break;

	    case LINIF_CHANNEL_OPERATIONAL:
	        /** @req LINIF670 */
	        LinSM_WakeUp_Confirmation(Channel, TRUE);
	        break;

	    case LINIF_CHANNEL_SLEEP_TRANS:
	        /** !req LINIF459 LINIF460 */
	        break;
	    default:
	        break;
	}

	// LINIF432: The function LinIf_WakeUp shall do nothing and return E_OK when the
    // referenced channel is not in the sleep state.
	/** @req LINIF432 */
	return E_OK;

}

void LinIf_MainFunction()
{
	uint8 chIndex;
	uint8 buf[8];
	uint8 *Lin_SduPtr;

	if (LinIfStatus == LINIF_UNINIT) {
		return;
	}
	/** @req LINIF473 */
	for (chIndex = 0; chIndex < LINIF_CONTROLLER_CNT; chIndex++)
	{
		// Check if there are any pending sleep transitions
		if (LinIfChannelStatus[chIndex] == LINIF_CHANNEL_SLEEP_TRANS) {
			if (Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfChannelId, &Lin_SduPtr) != LIN_CH_SLEEP) {
			    if (!chSleepCmdAttmpt[chIndex]) {
			        /** @req LINIF453 */
			        (void)Lin_GoToSleep(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfChannelId); //Driver always returns E_OK
			        chSleepCmdAttmpt[chIndex] = TRUE;
			    }
			    else {
			        /** @req LINIF454  */
			        // The delay of sleep mode frame is assumed to be time_base arbitarily since ASR is not specific
			        LinIfChannelStatus[chIndex] = LINIF_CHANNEL_OPERATIONAL;
			        chSleepCmdAttmpt[chIndex] = FALSE;
			        /** @req LINIF558 */
			        LinSM_GotoSleep_Confirmation(chIndex, FALSE);
			    }
			}
			else
			{
			    /** @req LINIF455 */
				LinIfChannelStatus[chIndex] = LINIF_CHANNEL_SLEEP;
				/** @req LINIF557 */
				LinSM_GotoSleep_Confirmation(chIndex, TRUE);
				// Set NULL schedule at sleep
				/** @req LINIF444 LINIF293*/
				currentIndex[chIndex] = 0;
				currentDelayInTicks[chIndex] = 0;
				currentSchedule[chIndex] = (LinIf_ScheduleTableType *)LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfScheduleTable;
			}
			continue;
		}

		// Check if there are any wakeup transitions
		if ((LinIfChannelStatus[chIndex] == LINIF_CHANNEL_SLEEP) &&
		        (Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfChannelId, &Lin_SduPtr) != LIN_CH_SLEEP)) {
		    /** @req LINIF478 */
			LinIfChannelStatus[chIndex] = LINIF_CHANNEL_OPERATIONAL;
			/** @req LINIF496 */
			LinSM_WakeUp_Confirmation(chIndex, TRUE);
		}

		// Normal scheduling
		if(currentDelayInTicks[chIndex] > 0){
			// Not time for sending yet
			currentDelayInTicks[chIndex]--;
			continue;
		}

		/** @req LINIF053 */
		if(LinIfChannelStatus[chIndex] == LINIF_CHANNEL_OPERATIONAL) {
			//Check if NULL schedule is present otherwise check status of last sent
			if(!(currentSchedule[chIndex] == 0 || currentSchedule[chIndex]->LinIfEntry == 0)){
				LinIfEntryType *ptrEntry = (LinIfEntryType *)&currentSchedule[chIndex]->LinIfEntry[currentIndex[chIndex]];
			    LinIf_FrameType *ptrFrame = (LinIf_FrameType *)&LinIf_ConfigPtr->LinIfFrameConfig[ptrEntry->LinIfFrameRef];
			    // Handle received and sent frames
				if(ptrFrame->LinIfPduDirection == LinIfRxPdu){
				    Lin_StatusType linSts = Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfChannelId, &Lin_SduPtr);
  					if(linSts == LIN_RX_OK){
						PduInfoType outgoingPdu;
						outgoingPdu.SduDataPtr = Lin_SduPtr;
						outgoingPdu.SduLength = ptrFrame->LinIfLength;
						/** @req LINIF289 */
						PduR_LinIfRxIndication(ptrFrame->LinIfTxTargetPduId,&outgoingPdu);
					}else {// RX_ERROR or BUSY
					    /** @req LINIF254 LINIF466 */
					    DET_REPORT_ERROR(LINIF_MAINFUNCTION_SERVICE_ID,LINIF_E_RESPONSE);
					}
				} else if(ptrFrame->LinIfPduDirection == LinIfTxPdu){
					Lin_StatusType status = Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfChannelId, &Lin_SduPtr);
					if(status == LIN_TX_OK){
					    /** @req LINIF289 */
						PduR_LinIfTxConfirmation(ptrFrame->LinIfTxTargetPduId);
					}else{// TX_ERROR or BUSY
					    /** @req LINIF036 LINIF465 */
					    DET_REPORT_ERROR(LINIF_MAINFUNCTION_SERVICE_ID,LINIF_E_RESPONSE);
					}
				}
				// Update index after getting status of last frame
				currentIndex[chIndex] = (currentIndex[chIndex] + 1) % currentSchedule[chIndex]->LinIfNofEntries;

			}

			//Set new schedule if ordered
		    if(newScheduleRequest[chIndex] == TRUE){
		    	currentSchedule[chIndex] = (LinIf_ScheduleTableType *)&LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfScheduleTable[newSchedule[chIndex]];
		    	currentIndex[chIndex] = 0;
		    	newScheduleRequest[chIndex]=FALSE;
		    	/** @req LINIF495 */
		    	LinSM_ScheduleRequest_Confirmation(chIndex);
		    }

		    // Handle new transmissions
		    if(!(currentSchedule[chIndex] == 0 || currentSchedule[chIndex]->LinIfEntry == 0)){
				Lin_PduType PduInfo;
				LinIfEntryType *ptrEntry = (LinIfEntryType *)&currentSchedule[chIndex]->LinIfEntry[currentIndex[chIndex]];
			    LinIf_FrameType *ptrFrame = (LinIf_FrameType *)&LinIf_ConfigPtr->LinIfFrameConfig[ptrEntry->LinIfFrameRef];
			    /** @req LINIF286 LINIF287 */
				// Only UNCONDITIONAL frames is supported in first version
				if (ptrFrame->LinIfFrameType == UNCONDITIONAL){
					// SendHeader
					if(ptrFrame->LinIfChecksumType==ENHANCED){
						PduInfo.Cs = LIN_ENHANCED_CS;
					}else{
						PduInfo.Cs = LIN_CLASSIC_CS;
					}
					PduInfo.Pid = ptrFrame->LinIfPid;
					PduInfo.SduPtr = buf; // Data will be added in PduR_LinIfTriggerTransmit
					PduInfo.DI = ptrFrame->LinIfLength;
					if(ptrFrame->LinIfPduDirection == LinIfTxPdu){
						PduInfo.Drc = LIN_MASTER_RESPONSE;
					}else{
						PduInfo.Drc = LIN_SLAVE_RESPONSE;
					}


					// Maybe send response also
					if(ptrFrame->LinIfPduDirection == LinIfTxPdu){
						PduInfoType outgoingPdu;
						outgoingPdu.SduDataPtr = PduInfo.SduPtr;
						outgoingPdu.SduLength = PduInfo.DI;
						//TX
						/** @req LINIF289 */
					    (void)PduR_LinIfTriggerTransmit(ptrFrame->LinIfTxTargetPduId, &outgoingPdu);
					    (void)Lin_SendFrame(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfChannelId,  &PduInfo);//Driver always returns E_OK
					}
					else {
						//RX
					    (void)Lin_SendFrame(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfChannelId,  &PduInfo);//Driver always returns E_OK
					}
				}

				// Set new delay
				uint16 temp = ptrEntry->LinIfDelay / LinIf_ConfigPtr->LinIfTimeBase ;
				currentDelayInTicks[chIndex] =  (temp > 0)? temp - 1: 0; //LinIfDelay can be zero for the first frame on the table
			}
		}
	}
}








