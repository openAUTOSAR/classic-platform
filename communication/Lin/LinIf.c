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









#include "LinIf.h"
#include "LinIf_Types.h"
#include "LinIf_Cbk.h"
#include "LinIf_Cfg.h"
#include "Lin.h"
#include "LinSM_Cbk.h"
#if defined(USE_PDUR)
#include "PduR_LinIf.h"
#endif
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
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
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

typedef enum {
	LINIF_UNINIT,
	LINIF_INIT,
	LINIF_CHANNEL_UNINIT,
	LINIF_CHANNEL_OPERATIONAL,
	LINIF_CHANNEL_SLEEP_TRANS,
	LINIF_CHANNEL_SLEEP,
}LinIf_StatusType;

static LinIf_StatusType LinIfStatus = LINIF_UNINIT;
static LinIf_StatusType LinIfChannelStatus[LINIF_CONTROLLER_CNT];

static LinIf_ScheduleTableType *currentSchedule[LINIF_CONTROLLER_CNT];
static uint16 currentIndex[LINIF_CONTROLLER_CNT];
static uint16 currentDelayInTicks[LINIF_CONTROLLER_CNT];

static boolean newScheduleRequest[LINIF_CONTROLLER_CNT];
static LinIf_SchHandleType newSchedule[LINIF_CONTROLLER_CNT];

void LinIf_Init( const void* ConfigPtr )
{
	(void)ConfigPtr;
	uint8 i;

	// Initalize driver
	Lin_Init(0);

	// Call Lin_InitChannel
	for (i=0;i<LINIF_CONTROLLER_CNT;i++)
	{
		Lin_InitChannel(LinIfChannelCfg[i].LinIfChannelId, LinIfChannelCfg[i].LinIfChannelRef);
		LinIfChannelStatus[i] = LINIF_CHANNEL_OPERATIONAL;
		currentSchedule[i] = 0;
		currentIndex[i] = 0;
		currentDelayInTicks[i] = 0;
		newScheduleRequest[i] = FALSE;
	}
	LinIfStatus = LINIF_INIT;
}

void LinIf_DeInit()
{
	uint8 i;

	// Call Lin_InitChannel
	for (i=0;i<LINIF_CONTROLLER_CNT;i++)
	{
		Lin_DeInitChannel(LinIfChannelCfg[i].LinIfChannelId);
	}
	// Uninitalize driver
	Lin_DeInit();

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
	VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LINIF_CONTROLLER_CNT), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (LinIfChannelStatus[Channel] != LINIF_CHANNEL_SLEEP && LinIfChannelStatus[Channel] != LINIF_CHANNEL_SLEEP_TRANS), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_SCHEDULE_REQUEST_ERROR, E_NOT_OK);

	newScheduleRequest[Channel] = TRUE;
	newSchedule[Channel] = Schedule;
	return E_OK;
}


Std_ReturnType LinIf_GotoSleep(NetworkHandleType Channel)
{
	VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_GOTOSLEEP_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LINIF_CONTROLLER_CNT), LINIF_GOTOSLEEP_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);

	if (LinIfChannelStatus[Channel] == LINIF_CHANNEL_OPERATIONAL) {
		LinIfChannelStatus[Channel] = LINIF_CHANNEL_SLEEP_TRANS;
	}
	return E_OK;
}


Std_ReturnType LinIf_WakeUp(NetworkHandleType Channel)
{
	VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_WAKEUP_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LINIF_CONTROLLER_CNT), LINIF_WAKEUP_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);

	if (LinIfChannelStatus[Channel] == LINIF_CHANNEL_SLEEP) {
		Lin_WakeUp(LinIfChannelCfg[Channel].LinIfChannelId);
		currentIndex[Channel] = 0;
		currentDelayInTicks[Channel] = 0;
	}
  // LINIF432: The function LinIf_WakeUp shall do nothing and return E_OK when the
  // referenced channel is not in the sleep state.
	else{
		LinIfChannelStatus[Channel] = LINIF_CHANNEL_OPERATIONAL;
		LinSM_WakeUp_Confirmation(Channel, TRUE);
	}
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

	for (chIndex = 0; chIndex < LINIF_CONTROLLER_CNT; chIndex++)
	{
		// Check if there are any pending sleep transitions
		if (LinIfChannelStatus[chIndex] == LINIF_CHANNEL_SLEEP_TRANS) {
			if (Lin_GetStatus(LinIfChannelCfg[chIndex].LinIfChannelId, &Lin_SduPtr) != LIN_CH_SLEEP){
				Lin_GoToSleep(LinIfChannelCfg[chIndex].LinIfChannelId);
			}
			else
			{
				LinIfChannelStatus[chIndex] = LINIF_CHANNEL_SLEEP;
				LinSM_GotoSleep_Confirmation(chIndex, TRUE);
			}
			// Set NULL schedule at sleep
			currentIndex[chIndex] = 0;
			currentDelayInTicks[chIndex] = 0;
	    	currentSchedule[chIndex] = (LinIf_ScheduleTableType *)&LinIfScheduleTableCfg[0];
			continue;
		}

		// Check if there are any wakeup transitions
		if ((LinIfChannelStatus[chIndex] == LINIF_CHANNEL_SLEEP) &&
        (Lin_GetStatus(LinIfChannelCfg[chIndex].LinIfChannelId, &Lin_SduPtr) != LIN_CH_SLEEP)) {
			LinIfChannelStatus[chIndex] = LINIF_CHANNEL_OPERATIONAL;
			LinSM_WakeUp_Confirmation(chIndex, TRUE);
		}

		// Normal scheduling
		if(currentDelayInTicks[chIndex] > 0){
			// Not time for sending yet
			currentDelayInTicks[chIndex]--;
			continue;
		}


		if(LinIfChannelStatus[chIndex] == LINIF_CHANNEL_OPERATIONAL) {
			//Check if NULL schedule is present otherwise check status of last sent
			if(!(currentSchedule[chIndex] == 0 || currentSchedule[chIndex]->LinIfEntry == 0)){
				LinIfEntryType *ptrEntry = (LinIfEntryType *)&currentSchedule[chIndex]->LinIfEntry[currentIndex[chIndex]];
			    LinIf_FrameType *ptrFrame = (LinIf_FrameType *)&LinIfFrameCfg[ptrEntry->LinIfFrameRef];

				// Handle received and sent frames
				if(ptrFrame->LinIfPduDirection == LinIfRxPdu){
					if(Lin_GetStatus(LinIfChannelCfg[chIndex].LinIfChannelId, &Lin_SduPtr) == LIN_RX_OK){
						PduInfoType outgoingPdu;
						outgoingPdu.SduDataPtr = Lin_SduPtr;
						outgoingPdu.SduLength = ptrFrame->LinIfLength;
						PduR_LinIfRxIndication(ptrFrame->LinIfTxTargetPduId,&outgoingPdu);
					}else{// RX_ERROR or BUSY
#if defined(USE_DEM)
				        Dem_ReportErrorStatus(LINIF_E_RESPONSE, DEM_EVENT_STATUS_FAILED);
#endif
					}
				} else if(ptrFrame->LinIfPduDirection == LinIfTxPdu){
					Lin_StatusType status = Lin_GetStatus(LinIfChannelCfg[chIndex].LinIfChannelId, &Lin_SduPtr);
					if(status == LIN_TX_OK){
						PduR_LinIfTxConfirmation(ptrFrame->LinIfTxTargetPduId);
					}else{// TX_ERROR or BUSY
#if defined(USE_DEM)
				        Dem_ReportErrorStatus(LINIF_E_RESPONSE, DEM_EVENT_STATUS_FAILED);
#endif
					}
				}
				// Update index after getting status of last frame
				currentIndex[chIndex] = (currentIndex[chIndex] + 1) % currentSchedule[chIndex]->LinIfNofEntries;

			}

			//Set new schedule if ordered
		    if(newScheduleRequest[chIndex] == TRUE){
		    	currentSchedule[chIndex] = (LinIf_ScheduleTableType *)&LinIfScheduleTableCfg[newSchedule[chIndex]];
		    	currentIndex[chIndex] = 0;
		    	newScheduleRequest[chIndex]=FALSE;
		    	LinSM_ScheduleRequest_Confirmation(chIndex);
		    }

		    // Handle new transmissions
			if(!(currentSchedule[chIndex] == 0 || currentSchedule[chIndex]->LinIfEntry == 0)){
				Lin_PduType PduInfo;
				LinIfEntryType *ptrEntry = (LinIfEntryType *)&currentSchedule[chIndex]->LinIfEntry[currentIndex[chIndex]];
			    LinIf_FrameType *ptrFrame = (LinIf_FrameType *)&LinIfFrameCfg[ptrEntry->LinIfFrameRef];

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
					    PduR_LinIfTriggerTransmit(ptrFrame->LinIfTxTargetPduId, &outgoingPdu);
					    Lin_SendHeader(LinIfChannelCfg[chIndex].LinIfChannelId,  &PduInfo);
						Lin_SendResponse(LinIfChannelCfg[chIndex].LinIfChannelId,  &PduInfo);
					}
					else {
						//RX
					    Lin_SendHeader(LinIfChannelCfg[chIndex].LinIfChannelId,  &PduInfo);
					}
				}

				// Set new delay
				uint16 temp = (ptrEntry->LinIfDelay / LinIfGlobalConfig.LinIfTimeBase) ;
				currentDelayInTicks[chIndex] = (temp>0) ? (temp - 1):0; //LinIfDelay can be zero for the first frame on the table
			}
		}
	}
	LinSM_TimerTick();
}








