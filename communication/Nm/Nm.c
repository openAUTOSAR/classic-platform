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


#if defined(USE_CANNM)
#include "CanNm.h"
#endif
#if defined(USE_FRNM)
#include "FrNm.h"
#endif
#if defined(USE_LINNM)
#include "LinNm.h"
#endif

#include "Nm.h" /** @req NM123 */

#if defined(USE_NM_EXTENSION)
#include "Nm_Extension.h"
#endif


/* Globally fulfilled requirements */
/** @req NM126 */ /* deterrors = uint8 */
/** @req NM125 */ /* event ids externally defined */
/** @req NM170 */ /* since mainfunctions does nothing this applies */
/** @req NM091 */ /* maybe implemented partly... */
/** @req NM095 */ /* do not provide more than specified */
/** @req NM006 */ /* convert generic to bus specific */
/** @req NM242 */ /* optionally dem */
/** @req NM243 */ /* optionally det */

extern const Nm_ConfigType Nm_Config;

/** Initializes the NM Interface. */
void Nm_Init( void )
{
#if defined(USE_NM_EXTENSION)
	Nm_Extension_Init();
#endif
}

/** This service returns the version information of this module */
void Nm_GetVersionInfo( Std_VersionInfoType* nmVerInfoPtr )
{
	(void)nmVerInfoPtr;
	/* TODO: Use macro instead */
}

/** This function calls the <BusNm>_PassiveStartUp function
  * (e.g. CanNm_PassiveStartUp function is called if channel is configured as CAN). */
Std_ReturnType Nm_PassiveStartUp( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_PassiveStartUp(NetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_PassiveStartUp(NetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_PassiveStartUp(NetworkHandle);
#endif
		default: return E_NOT_OK;
	}
}

/** This function calls the <BusNm>_NetworkRequest
  * (e.g. CanNm_NetworkRequest function is called if channel is configured as CAN). */
Std_ReturnType Nm_NetworkRequest( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_NetworkRequest(NetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_NetworkRequest(NetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_NetworkRequest(NetworkHandle);
#endif
		default: return E_NOT_OK;
	}
}

/** This function calls the <BusNm>_NetworkRelease bus specific function
  * (e.g. CanNm_NetworkRelease function is called if channel is configured as CAN). */
Std_ReturnType Nm_NetworkRelease( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_NetworkRelease(NetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_NetworkRelease(NetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_NetworkRelease(NetworkHandle);
#endif
		default: return E_NOT_OK;
	}
}

#if (NM_COM_CONTROL_ENABLED == STD_ON) /* @req NM134 */
/** This function calls the CanNm_NetworkRelease bus specific function, to disable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
Std_ReturnType Nm_DisableCommunication( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_DisableCommunication(NetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_DisableCommunication(NetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_DisableCommunication(NetworkHandle);
#endif
		default: return E_NOT_OK;
	}
}
#endif

#if (NM_COM_CONTROL_ENABLED == STD_ON) /* @req NM136 */
/** This function calls the CanNm_NetworkRequest bus specific function, to enable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
Std_ReturnType Nm_EnableCommunication( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_EnableCommunication(NetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_EnableCommunication(NetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_EnableCommunication(NetworkHandle);
#endif
		default: return E_NOT_OK;
	}
}
#endif

/** Set user data for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_SetUserData shall be called
  * (e.g. CanNm_SetUserData function is called if channel is configured as CAN). */
#if (NM_USER_DATA_ENABLED == STD_ON) /** @req NM241 */ /* to fulfill NM138 passivemodeenabled is missing */
Std_ReturnType Nm_SetUserData( const NetworkHandleType NetworkHandle, const uint8 * const nmUserDataPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_SetUserData(NetworkHandle, nmUserDataPtr);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_SetUserData(NetworkHandle, nmUserDataPtr);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_SetUserData(NetworkHandle, nmUserDataPtr);
#endif
		default: return E_NOT_OK;
	}
}
#endif

/** Get user data out of the last successfully received NM message.
  * For that purpose <BusNm>_GetUserData shall be called
  * (e.g. CanNm_GetUserData function is called if channel is configured as CAN). */
#if (NM_USER_DATA_ENABLED == STD_ON) /** @req NM140 */
Std_ReturnType Nm_GetUserData( const NetworkHandleType NetworkHandle, uint8 * const nmUserDataPtr ){
	/* For some reason the signature of this service differs from its busNm equivalents... */
	Std_ReturnType userDataRet = E_NOT_OK;
	Std_ReturnType nodeIdRet = E_NOT_OK;
	uint8 nmNodeIdPtr;
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];
	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
	case NM_BUSNM_CANNM:
		userDataRet = CanNm_GetUserData(NetworkHandle, nmUserDataPtr);
		nodeIdRet = CanNm_GetNodeIdentifier(NetworkHandle, &nmNodeIdPtr);
		break;
#endif
#if defined(USE_FRNM)
	case NM_BUSNM_FRNM:
		userDataRet = FrNm_GetUserData(NetworkHandle, nmUserDataPtr);
		nodeIdRet = FrNm_GetNodeIdentifier(NetworkHandle, &nmNodeIdPtr);
		break;
#endif
#if defined(USE_LINNM)
	case NM_BUSNM_LINNM:
		userDataRet = LinNm_GetUserData(NetworkHandle, nmUserDataPtr);
		nodeIdRet = LinNm_GetNodeIdentifier(NetworkHandle, &nmNodeIdPtr);
		break;
#endif
	default:
		break;
	}

	if( (E_OK != userDataRet) || ( E_OK != nodeIdRet )) {
		return E_NOT_OK;
	} else {
		return E_OK;
	}
}
#endif

/** Get the whole PDU data out of the most recently received NM message.
  * For that purpose CanNm_GetPduData shall be called. */
#if ((NM_NODE_ID_ENABLED == STD_ON) || (NM_NODE_DETECTION_ENABLED == STD_ON) || (NM_USER_DATA_ENABLED == STD_ON)) /** @req NM142 */
Std_ReturnType Nm_GetPduData( const NetworkHandleType NetworkHandle, uint8 * const nmPduData ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_GetPduData(NetworkHandle, nmPduData);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_GetPduData(NetworkHandle, nmPduData);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_GetPduData(NetworkHandle, nmPduData);
#endif
		default: return E_NOT_OK;
	}
}
#endif

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_RepeatMessageRequest shall be called
  * (e.g. CanNm_RepeatMessageRequest function is called if channel is configured as CAN) */
#if (NM_NODE_DETECTION_ENABLED == STD_ON) /** @req NM144 */
Std_ReturnType Nm_RepeatMessageRequest( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_RepeatMessageRequest(NetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_RepeatMessageRequest(NetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_RepeatMessageRequest(NetworkHandle);
#endif
		default: return E_NOT_OK;
	}
}
#endif

/** Get node identifier out of the last successfully received NM-message.
  * The function <BusNm>_GetNodeIdentifier shall be called. */
#if (NM_NODE_ID_ENABLED == STD_ON) /** @req NM146 */ /** @req NM148 */
Std_ReturnType Nm_GetNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_GetNodeIdentifier(NetworkHandle, nmNodeIdPtr);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_GetNodeIdentifier(NetworkHandle, nmNodeIdPtr);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_GetNodeIdentifier(NetworkHandle, nmNodeIdPtr);
#endif
		default: return E_NOT_OK;
	}
}

/** Get node identifier configured for the local node.
  * For that purpose <BusNm>_GetLocalNodeIdentifier shall be called
  * (e.g. CanNm_GetLocalNodeIdentifier function is called if channel is configured as CAN). */
Std_ReturnType Nm_GetLocalNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];
 
	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_GetLocalNodeIdentifier(NetworkHandle, nmNodeIdPtr);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_GetLocalNodeIdentifier(NetworkHandle, nmNodeIdPtr);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_GetLocalNodeIdentifier(NetworkHandle, nmNodeIdPtr);
#endif
		default: return E_NOT_OK;
	}
}
#endif

/** Check if remote sleep indication takes place or not. This in turn calls the
  * <BusNm>_CheckRemoteSleepIndication for the bus specific NM layer
  * (e.g. CanNm_CheckRemoteSleepIndication function is called if channel is configured as CAN). */
Std_ReturnType Nm_CheckRemoteSleepIndication( const NetworkHandleType NetworkHandle, boolean * const nmRemoteSleepIndPtr ){
	(void)NetworkHandle;
	(void)nmRemoteSleepIndPtr;
	return E_NOT_OK;
}

/** Returns the state of the network management. This function in turn calls the
  * <BusNm>_GetState function (e.g. CanNm_GetState function is called if channel is configured as CAN). */
Std_ReturnType Nm_GetState( const NetworkHandleType NetworkHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr ){
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_GetState(NetworkHandle, nmStatePtr, nmModePtr);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_GetState(NetworkHandle, nmStatePtr, nmModePtr);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_GetState(NetworkHandle, nmStatePtr, nmModePtr);
#endif
		default: return E_NOT_OK;
	}
}

/** Notification that a NM-message has been received in the Bus-Sleep Mode, what
  * indicates that some nodes in the network have already entered the Network Mode.
  * The callback function shall start the network management state machine. */
/** @req NM012.partially.1 */
void Nm_NetworkStartIndication( const NetworkHandleType NetworkHandle ){
#if defined(USE_NM_EXTENSION)
	Nm_Extension_NetworkStartIndication(NetworkHandle);
#else
	/** @req NM155 */
	ComM_Nm_NetworkStartIndication(NetworkHandle);
#endif
}

/** Notification that the network management has entered Network Mode. The
  * callback function shall enable transmission of application messages. */
/** @req NM012.partially.2 */
void Nm_NetworkMode( const NetworkHandleType NetworkHandle ){
#if defined(USE_NM_EXTENSION)
    Nm_Extension_NetworkMode(NetworkHandle);
#else
	/** @req NM158 */
	ComM_Nm_NetworkMode(NetworkHandle);
#endif
}

/** Notification that the network management has entered Prepare Bus-Sleep Mode.
  * The callback function shall disable transmission of application messages. */
/** @req NM012.partially.3 */
void Nm_PrepareBusSleepMode( const NetworkHandleType NetworkHandle ){
#if defined(USE_NM_EXTENSION)
    Nm_Extension_PrepareBusSleepMode(NetworkHandle);
#else
	/** @req NM161 */
	ComM_Nm_PrepareBusSleepMode(NetworkHandle);
#endif
}

/** Notification that the network management has entered Bus-Sleep Mode. */
/** @req NM012.partially.4 */
void Nm_BusSleepMode( const NetworkHandleType NetworkHandle ){
#if defined(USE_NM_EXTENSION)
    Nm_Extension_BusSleepMode(NetworkHandle);
#else
	/** @req NM163 */
	ComM_Nm_BusSleepMode(NetworkHandle);
#endif
}

/* NEW */
/** Notification that the network management has detected that all other nodes on the network are ready to enter Bus-Sleep Mode */
/** @req NM192 */
void Nm_RemoteSleepIndication( const NetworkHandleType nmNetworkHandle )
{
	(void)nmNetworkHandle;
}

/* NEW */
/**Notification that the network management has detected that not all other nodes on the network are longer ready to enter Bus-Sleep Mode */
/** @req NM193 */
void Nm_RemoteSleepCancellation( const NetworkHandleType nmNetworkHandle )
{
	(void)nmNetworkHandle;
}

/* NEW */
/** Notification to the NM Coordinator functionality that this is a suitable point in time to initiate the coordination algorithm on */
/** @req NM194 */
void Nm_SynchronizationPoint( const NetworkHandleType nmNetworkHandle )
{
	(void)nmNetworkHandle;
}

/** Notification that a NM message has been received. */
void Nm_PduRxIndication( const NetworkHandleType NetworkHandle )
{
	(void)NetworkHandle;
}

#if (NM_STATE_CHANGE_IND_ENABLED == STD_ON)
/** Notification that the CAN Generic NM state has changed. */
void Nm_StateChangeNotification(const NetworkHandleType nmNetworkHandle, const Nm_StateType nmPreviousState, const Nm_StateType nmCurrentState )
{
	const Nm_ChannelType* ChannelConf = &Nm_Config.Channels[nmNetworkHandle];

	(void)(nmPreviousState);

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
#if defined(USE_NM_EXTENSION)
				Nm_Extension_WriteState(nmNetworkHandle, nmCurrentState);
#endif
		break;
#endif
		default:
			break;
	}
}
#endif

/* NEW */
/** Service to indicate that an NM message with set Repeat Message Request Bit has been received */
/** @req NM230 */
void Nm_RepeatMessageIndication( const NetworkHandleType nmNetworkHandle )
{
	(void)nmNetworkHandle;
}

/* NEW */
/** Service to indicate that an attempt to send an NM message failed */
/** @req NM234 */
void Nm_TxTimeoutException( const NetworkHandleType nmNetworkHandle )
{
	(void)nmNetworkHandle;
}

/* NEW */
/** This function is called by a <Bus>Nm to indicate reception of a CWU request */
/** @req NM250 */
void Nm_CarWakeUpIndication( const NetworkHandleType nmNetworkHandle )
{
	(void)nmNetworkHandle;
}

/* NEW */
/** Seta an indication, when the NM coordinator Sleep Ready bit in the Control Bit Vector is set */
/** @req NM254 */
void Nm_CoordReadyToSleepIndication( const NetworkHandleType nmNetworkHandle )
{
	(void)nmNetworkHandle;
}

/** This function implements the processes of the NM Interface, which need a fix
  * cyclic scheduling. This function is supplied for the NM coordinator functionality
  * (Nm020). However, specific implementation may not need it (Nm093) */
/** @req 121 */
void Nm_MainFunction()
{}

