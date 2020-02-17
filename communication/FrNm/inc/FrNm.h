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


#ifndef FRNM_H_
#define FRNM_H_
/* @req  FRNM065 */
#include "ComStack_Types.h"
#ifdef USE_COMM
#include "ComM.h"
#endif
#include "NmStack_Types.h"
#include "Nm.h" /* @req  FRNM066 */
#include "FrNm_ConfigTypes.h"


#define FRNM_VENDOR_ID                     60u
#define FRNM_MODULE_ID                     32u

#define FRNM_AR_RELEASE_MAJOR_VERSION      4u
#define FRNM_AR_RELEASE_MINOR_VERSION      0u
#define FRNM_AR_RELEASE_REVISION_VERSION   3u

#define FRNM_AR_MAJOR_VERSION              FRNM_AR_RELEASE_MAJOR_VERSION
#define FRNM_AR_MINOR_VERSION              FRNM_AR_RELEASE_MINOR_VERSION
#define FRNM_AR_PATCH_VERSION              FRNM_AR_RELEASE_REVISION_VERSION

#define FRNM_SW_MAJOR_VERSION              2u
#define FRNM_SW_MINOR_VERSION              0u
#define FRNM_SW_PATCH_VERSION              0u


#include "FrNm_Cfg.h" /* @req FRNM369 */
#include "FrIf.h"

/** Event IDs */ /* @req FRNM021 */ /* @req FRNM293 */
#define FRNM_E_UINIT                    0x01u
#define FRNM_E_INVALID_CHANNEL          0x02u
#define FRNM_E_INVALID_POINTER          0x03u
#define FRNM_E_PDU_ID_INVALID           0x04u

/** Service IDs */
#define FRNM_SERVICE_ID_INIT                0x00u
#define FRNM_SERVICE_ID_PASSIVESTARTUP      0x01u
#define FRNM_SERVICE_ID_NETWORK_REQUEST     0x02u
#define FRNM_SERVICE_ID_NETWORK_RELEASE     0x03u
#define FRNM_SERVICE_ID_SETUSERDATA         0x06u
#define FRNM_SERVICE_ID_GETUSERDATA         0x07u
#define FRNM_SERVICE_ID_GETPDUDATA          0x08u
#define FRNM_SERVICE_ID_REPEATMSG_REQUEST   0x09u
#define FRNM_SERVICE_ID_GETNODE_ID          0x0Au
#define FRNM_SERVICE_ID_GETLOCAL_NODEID     0x0Bu
#define FRNM_SERVICE_ID_REQBUS_SYNC         0xC0u
#define FRNM_SERVICE_ID_CHKREMOTE_SLEEPIND  0x0Du
#define FRNM_SERVICE_ID_GETSTATE            0x0Eu
#define FRNM_SERVICE_ID_GETVERSIONINFO      0x0Fu
#define FRNM_SERVICE_ID_STARTUPERROR        0x10u
#define FRNM_SERVICE_ID_TRANSMIT            0x11u
#define FRNM_SERVICE_ID_ENABLECOMM          0x05u
#define FRNM_SERVICE_ID_DISABLECOMM         0x0Cu
#define FRNM_SERVICE_ID_SETSLEEP_READYBIT   0x12u
#define FRNM_SERVICE_ID_RX_INDICATION       0x42u
#define FRNM_SERVICE_ID_TRIGGER_TRANSMIT    0x41u
#define FRNM_SERVICE_ID_TX_CONFIRMATION     0x40u
#define FRNM_SERVICE_ID_MAIN_FUNCTION       0xF0u

#define FRNM_SOURCE_NODE_ID_POSITION         1u
#define FRNM_USRDATA_WITH_SOURCE_NODE_ID     2u
#define FRNM_USRDATA_WITHOUT_SOURCE_NODE_ID  1u
#define VOTE_INDEX                           0u
#define VOTE_VALUE                           0x80u
#define VOTE_PDU_LENGTH                      1u
#define CBV_INDEX                            0u

/* CBV repeat request bit */
#define FRNM_CBV_REPEAT_MESSAGE_REQUEST      0x01u
/* CBV NM Coordinator Sleep Ready Bit */
#define FRNM_CBV_NM_COORD_SLEEP_READY        0x08u
/* CBV Active wake up Bit */
#define FRNM_CBV_ACTIVE_WAKEUP               0x10u
/* CBV Cluster request information Bit */
#define FRNM_CBV_CLUSTER_REQ_INFO            0x40u

/* Max cycles supported in flexray */
#define FRNM_MAX_FLEXRAY_CYCLE_COUNT         64u

 /* Publish the configuration */
extern const FrNm_ConfigType FrNmConfigData;

/** Initialize the complete FrNm module, i.e. all channels which are activated */
/** @req FRNM236 */
void FrNm_Init(const FrNm_ConfigType * const frnmConfigPtr);

/* @req FRNM238 */
Std_ReturnType FrNm_NetworkRequest( const NetworkHandleType NetworkHandle );

/* @req FRNM239 */
Std_ReturnType FrNm_NetworkRelease( const NetworkHandleType NetworkHandle );

/* This service returns the version information of this module. */
/* @req FRNM074 *//*@req FRNM249*/
#if ( FRNM_VERSION_INFO_API == STD_ON ) /* @req FRNM274 */
#define FrNm_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,FRNM)  /*@req FRNM273*/ /*@req FRNM272*/
#endif /* FRNM_VERSION_INFO_API */


#if (FRNM_USER_DATA_ENABLED == STD_ON) /* @req FRNM263 *//* @req FRNM264 */
/* @req FRNM240 */
Std_ReturnType FrNm_SetUserData( const NetworkHandleType NetworkHandle, const uint8* const nmUserDataPtr );

/* @req FRNM241 */
Std_ReturnType FrNm_GetUserData(const NetworkHandleType NetworkHandle, uint8* const nmUserDataPtr );

#endif


#if(FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_ON)||(FRNM_SOURCE_NODE_INDENTIFIER_ENABLED==STD_ON)||(FRNM_USER_DATA_ENABLED == STD_ON)
/* @req FRNM242 */
Std_ReturnType FrNm_GetPduData( const NetworkHandleType NetworkHandle, uint8* const nmPduData );
#endif

#if (FRNM_NODE_DETECTION_ENABLED == STD_ON) /* @req FRNM228 */
/* @req FRNM243 */
Std_ReturnType FrNm_RepeatMessageRequest( const NetworkHandleType NetworkHandle );
#endif

#if(FRNM_SOURCE_NODE_INDENTIFIER_ENABLED == STD_ON)/* @req FRNM267 *//* @req FRNM268 */
/* @req FRNM244 */
Std_ReturnType FrNm_GetNodeIdentifier( const NetworkHandleType NetworkHandle, uint8* const nmNodeIdPtr);


/* @req FRNM245 */
Std_ReturnType FrNm_GetLocalNodeIdentifier( const NetworkHandleType NetworkHandle, uint8* const nmNodeIdPtr );
#endif

#if (FRNM_BUS_SYNCHRONIZATION_ENABLED == STD_ON) /* @req FRNM269 */
/* @req FRNM246 */
Std_ReturnType FrNm_RequestBusSynchronization( const NetworkHandleType NetworkHandle );
#endif


/* req FRNM247 */
Std_ReturnType FrNm_CheckRemoteSleepIndication( const NetworkHandleType NetworkHandle, boolean* const nmRemoteSleepIndPtr );

/* @req FRNM248 */
Std_ReturnType FrNm_GetState( const NetworkHandleType NetworkHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr );

/* @req FRNM393 */
void FrNm_StartupError( const NetworkHandleType NetworkHandle );

/* @req FRNM359 */
Std_ReturnType FrNm_Transmit( PduIdType FrNmTxPduId, const PduInfoType* PduInfoPtr );

/*  @req FRNM387*/
Std_ReturnType FrNm_EnableCommunication( const NetworkHandleType nmChannelHandle );

/*  @req FRNM390*/
Std_ReturnType FrNm_DisableCommunication( const NetworkHandleType nmChannelHandle );

/*  req FRNM237*/
Std_ReturnType FrNm_PassiveStartUp( const NetworkHandleType NetworkHandle );

/*  @req FRNM*/
Std_ReturnType FrNm_SetSleepReadyBit( const NetworkHandleType nmChannelHandle, const boolean nmSleepReadyBit );

/*  @req FRNM251*/
void FrNm_RxIndication( PduIdType RxPduId, PduInfoType* PduInfoPtr );

/*  @req FRNM252*/
Std_ReturnType FrNm_TriggerTransmit( PduIdType TxPduId, PduInfoType* PduInfoPtr );

/*  req FRNM460 in 4.2 spec*/
void FrNm_TxConfirmation( PduIdType TxPduId );
#ifdef HOST_TEST
void GetFrNmChannelRunTimeData( const NetworkHandleType NetworkHandle , boolean* repeatMessage, boolean* networkRequested);
#endif

#endif /* FRNM_H_ */
