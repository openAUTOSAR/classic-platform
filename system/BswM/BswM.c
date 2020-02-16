
#include "BswM.h"

#include "Det.h"
#include "ComM.h"
#include "Com.h"
#if defined(USE_NM)
#include "Nm.h"
#endif
#if defined(USE_LINSM)
#include "LinSM.h"
#endif
#ifdef EGSM_USE_HARDCODED_RTE_BSWM
#include "Rte_BswM_Internal.h"
#endif


/* TODO:
 * This is a hardcoded version of the module.
 * No configurability. All actions and conditions are hardcoded.
 */


const BswM_ConfigType BswM_Config;

/* Used for enabling/disabling PDU groups */
static Com_IpduGroupVector BswM_COMGoupVector;

#if ( BSWM_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_BSWM,0,_api,_err); \
          return E_NOT_OK; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_BSWM,0,_api,_err); \
          return; \
        }

#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif


typedef struct
{
    const BswM_ConfigType * ConfigPtr;
    boolean Initialized;
    BswM_ModeType Mode;
    BswM_ModeType RequestedMode;
    boolean pendingRequestedMode;
    CanSM_BswMCurrentStateType SMState;
    ComM_ModeType CommMode;
} BswM_InternalType;


static BswM_InternalType BswM_InternalState = {
    .Initialized = FALSE,
    .Mode = BSWM_MODE_STARTUP,
    .RequestedMode = BSWM_MODE_NONE,
    .SMState = CANSM_BSWM_NO_COMMUNICATION, /* We only have one CAN network  */
};


/* Helper function - calls RTE notification ports. */
#ifdef EGSM_USE_HARDCODED_RTE_BSWM
static void modeChangeNotification()
{

    // Shall be called regardless if mode change or not
    Rte_IWrite_BswM_MainFunction_modeStatusPort_bswmMode(BswM_InternalState.Mode);
}

/* Helper function - calls RTE receiver interface. */
static void pollModeChangeRequest()
{
    // Poll RTE read interface
    BswM_ModeType mode = Rte_IRead_BswM_MainFunction_modeRequestPort_bswmMode();

    // TODO: Add when we have a working BswM Service Component on the RTE.
    // Try to request mode change. It has no effect if mode is not changed.
    //BswM_RequestMode(BSWM_USER_1, mode);
}
#endif

/* Helper funcation - sets the correct PDU groups and NM for a certain state. */
static void enter_mode(BswM_ModeType mode) {
	switch (mode) {
		case BSWM_MODE_STARTUP:
			#if (NM_COM_CONTROL_ENABLED == STD_ON)
				(void)Nm_DisableCommunication(NmConf_NmChannelConfig_channel_PropulsionCan);
			#endif
			break;

		case BSWM_MODE_SILENT:
            #if (NM_COM_CONTROL_ENABLED == STD_ON)
				(void)Nm_DisableCommunication(NmConf_NmChannelConfig_channel_PropulsionCan);
			#endif
			Com_ClearIpduGroupVector(BswM_COMGoupVector);
			Com_SetIpduGroup(BswM_COMGoupVector, COM_PDU_GROUP_ID_COMIPDUGROUPRX, TRUE);
			Com_SetIpduGroup(BswM_COMGoupVector, COM_PDU_GROUP_ID_COMIPDUGROUPTX, FALSE);
			Com_IpduGroupControl(BswM_COMGoupVector, FALSE);
			break;

		case BSWM_MODE_NORMAL:
            #if (NM_COM_CONTROL_ENABLED == STD_ON)
				(void)Nm_EnableCommunication(NmConf_NmChannelConfig_channel_PropulsionCan);
			#endif
			Com_ClearIpduGroupVector(BswM_COMGoupVector);
			Com_SetIpduGroup(BswM_COMGoupVector, COM_PDU_GROUP_ID_COMIPDUGROUPRX, TRUE);
			Com_SetIpduGroup(BswM_COMGoupVector, COM_PDU_GROUP_ID_COMIPDUGROUPTX, TRUE);
			Com_IpduGroupControl(BswM_COMGoupVector, FALSE);
			break;

		case BSWM_MODE_FCC_FAIL:
            #if (NM_COM_CONTROL_ENABLED == STD_ON)
				(void)Nm_EnableCommunication(NmConf_NmChannelConfig_channel_PropulsionCan);
			#endif
			Com_ClearIpduGroupVector(BswM_COMGoupVector);
			Com_SetIpduGroup(BswM_COMGoupVector, COM_PDU_GROUP_ID_COMIPDUGROUPRX, TRUE);
			Com_SetIpduGroup(BswM_COMGoupVector, COM_PDU_GROUP_ID_COMIPDUGROUPTX, FALSE);
			Com_IpduGroupControl(BswM_COMGoupVector, FALSE);
			break;

		case BSWM_MODE_SHUTDOWN:
			(void)EcuM_SelectShutdownTarget(ECUM_STATE_OFF, 0);
			(void)EcuM_ReleaseRUN(ECUM_USER_HelloWorldUser);
			break;

		default:
			break;
	}
}

/* Used for Requesting a COMM mode */
static boolean request_COMM_mode(ComM_ModeType ComMode) {

	if (BswM_InternalState.pendingRequestedMode) {
		(void)ComM_RequestComMode(COMM_USER_HANDLE_ComMUser, ComMode);
		BswM_InternalState.pendingRequestedMode = FALSE;
		return FALSE;
	}
	else {
		/* TODO: Check if the comm mode is reported back, if so return TRUE*/
		return TRUE;
	}

}

/* This function is one tick in the BSWM state machine */
static void ModeArbitration(void) {

    switch (BswM_InternalState.Mode)
    {

		case BSWM_MODE_NONE:
			BswM_InternalState.Mode = BSWM_MODE_STARTUP;
			enter_mode(BSWM_MODE_STARTUP);
			break;

		case BSWM_MODE_STARTUP:

			if ((BswM_InternalState.RequestedMode == BSWM_MODE_NORMAL) ||
				(BswM_InternalState.RequestedMode == BSWM_MODE_SILENT)) {

				if (request_COMM_mode(COMM_FULL_COMMUNICATION) &&
					BswM_InternalState.SMState == CANSM_BSWM_FULL_COMMUNICATION) {
					enter_mode(BswM_InternalState.RequestedMode);
					BswM_InternalState.Mode = BswM_InternalState.RequestedMode;
				}
			}
			else if (BswM_InternalState.RequestedMode == BSWM_MODE_FCC_FAIL) {
				enter_mode(BswM_InternalState.RequestedMode);
				BswM_InternalState.Mode = BSWM_MODE_FCC_FAIL;
			}
			else {
				/* Invalid request */
			}
			break;

		case BSWM_MODE_NORMAL:
		case BSWM_MODE_SILENT:

			if (BswM_InternalState.RequestedMode == BSWM_MODE_SHUTDOWN) {
				if (request_COMM_mode(COMM_NO_COMMUNICATION) &&
					(BswM_InternalState.CommMode ==  COMM_NO_COMMUNICATION)) {
					BswM_InternalState.Mode = BSWM_MODE_SHUTDOWN;
					enter_mode(BswM_InternalState.RequestedMode);
				}
			}
			else if ((BswM_InternalState.Mode != BswM_InternalState.RequestedMode) &&
			         ((BswM_InternalState.RequestedMode == BSWM_MODE_NORMAL) ||
					 (BswM_InternalState.RequestedMode == BSWM_MODE_SILENT) ||
					 (BswM_InternalState.RequestedMode == BSWM_MODE_FCC_FAIL))) {
				enter_mode(BswM_InternalState.RequestedMode);
				BswM_InternalState.Mode = BswM_InternalState.RequestedMode;
			}
			else {
				/* Invalid request */
			}
			break;

		case BSWM_MODE_SHUTDOWN:
			/* Can't leave this state */
			break;

		case BSWM_MODE_FCC_FAIL:
			/* Mode: Frame configuration check fail
			 * The only transistion from this state is to shutdown
			 */
			if (BswM_InternalState.RequestedMode == BSWM_MODE_SHUTDOWN) {
				if (request_COMM_mode(COMM_NO_COMMUNICATION)) {
					BswM_InternalState.Mode = BSWM_MODE_SHUTDOWN;
					enter_mode(BswM_InternalState.RequestedMode);
				}
			}
			else {
				/* Invalid request */
			}
			break;

		default:
			/* Incorrect usage, do nothing */
			break;
		}
}


/* @req BswM0002 */
void BswM_Init(const BswM_ConfigType * ConfigPtr) {
    /* @req BswM0088 */
    VALIDATE_NO_RV(ConfigPtr != NULL, 0x0, BSWM_E_NULL_POINTER);

    BswM_InternalState.ConfigPtr = ConfigPtr;
    BswM_InternalState.Mode = BSWM_MODE_NONE;
    BswM_InternalState.RequestedMode = BSWM_MODE_NONE;
    BswM_InternalState.Initialized = TRUE;

}

/* @req BswM0119 */
void BswM_Deinit(void) {
    BswM_InternalState.Initialized = FALSE;
}

/* @req BswM0046*/
void BswM_RequestMode(BswM_UserType requesting_user, BswM_ModeType requested_mode) {

	(void)requesting_user;

	/* Check if already got a requested mode*/
	if (requested_mode != BswM_InternalState.Mode) {
		BswM_InternalState.RequestedMode = requested_mode;
		BswM_InternalState.pendingRequestedMode = TRUE;
	}
}



/* @req BswM0053 */
void BswM_MainFunction( void ) {
#ifdef EGSM_USE_HARDCODED_RTE_BSWM
    pollModeChangeRequest();
#endif
	ModeArbitration();
#ifdef EGSM_USE_HARDCODED_RTE_BSWM
	modeChangeNotification();
#endif
}


/* @req BswM0056*/
void BswM_EcuM_CurrentState(EcuM_StateType CurrentState ) {
	(void)CurrentState;

	/* TODO: The current state should be used as a guard in the state machine */
}

/* @req BswM0031*/
void BswM_EcuM_CurrentWakeup(EcuM_WakeupSourceType source, EcuM_WakeupStatusType state) {
	(void)source;
	(void)state;
	/* No action needed since we don't care about the wakeup sourece */
}

/* @req BswM0047 */
void BswM_ComM_CurrentMode(NetworkHandleType Network, ComM_ModeType RequestedMode) {
	(void)(Network);
	BswM_InternalState.CommMode = RequestedMode;
}

/* @req BswM0104 */
void BswM_NvM_CurrentBlockMode(NvM_BlockIdType Block, NvM_RequestResultType CurrentBlockMode) {
	(void)Block;
	(void)CurrentBlockMode;
}

/* @req BswM0152 */
void BswM_NvM_CurrentJobMode(uint8 ServiceId, NvM_RequestResultType CurrentJobMode) {
	(void)(ServiceId);
	(void)CurrentJobMode;
}
/* @req BswM0157 */
void BswM_WdgM_RequestPartitionReset(ApplicationType Application) {
	/* This is not used */
	(void)Application;
}
/* @req BswM0049 */
void BswM_CanSM_CurrentState(NetworkHandleType Network, CanSM_BswMCurrentStateType CurrentState)
{
    (void)Network;
    BswM_InternalState.SMState = CurrentState;
}
#if defined(USE_LINSM)

/* @req BswM0058 */
void BswM_LinSM_CurrentSchedule(NetworkHandleType Network, LinIf_SchHandleType CurrentSchedule) {
    (void)Network;
    (void)CurrentSchedule;
    /* This is not used */
}

/* @req BswM0052*/
void BswM_LinSM_CurrentState(NetworkHandleType Network, LinSM_ModeType CurrentState){
    (void)Network;
    (void)CurrentState;
}
#endif
