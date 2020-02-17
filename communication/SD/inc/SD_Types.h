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

/** @file SD_Types.h
 * Definitions of configuration types and parameters for the SD module.
 */

#ifndef SD_TYPES_H
#define SD_TYPES_H

#include "SoAd_Types.h"

#if defined(USE_DEM)
#include "Dem.h"
#endif

typedef struct {
    uint32 InitialOfferDelayMax_ms;
    uint32 InitialOfferDelayMin_ms;
    uint32 InitialOfferRepetitionBaseDelay_ms;
    uint8   InitialOfferRepetitionsMax;
    uint32 OfferCyclicDelay_ms;
    uint32 RequestResponseMaxDelay_ms;
    uint32 RequestResponseMinDelay_ms;
    uint32 TTL;
} Sd_ServerTimerType;

typedef struct {
    const uint32 InitialFindDelayMax_ms;
    const uint32 InitialFindDelayMin_ms;
    const uint32 InitialFindRepetitionBaseDelay_ms;
    const uint8   InitialFindRepetitionsMax;
    const uint32 RequestResponseMaxDelay_ms;
    const uint32 RequestResponseMinDelay_ms;
    const uint32 TTL;
} Sd_ClientTimerType;

typedef struct {
    char* Key;
    char* Value;
} Sd_CapabilityRecordType;

typedef uint16 Sd_SoAdSocketConnectionIndex;

typedef struct {
    uint16 EventActivationRef;
    const Sd_SoAdSocketConnectionIndex MulticastEventSoConRef;
} Sd_EventHandlerMulticastType;

typedef struct {
    uint16 EventActivationRef;
    const SoAd_RoutingGroupType* EventTriggeringRef;
} Sd_EventHandlerTcpUdpType;

typedef struct {
    uint16 EventGroupId;
    uint16 HandleId;
    uint16 MulticastThreshold;
    const Sd_ServerTimerType* TimerRef;
    const Sd_EventHandlerMulticastType* Multicast;
    const Sd_EventHandlerTcpUdpType* Tcp;
    const Sd_EventHandlerTcpUdpType* Udp;
    const Sd_CapabilityRecordType* CapabilityRecord;
    const uint32 SdNoOfCapabiltyRecord;
} Sd_EventHandlerType;

typedef uint16 Sd_SoAdSocketConnectionGroupIndex;

#define ACTIVATION_REF_NOT_SET 0xFFFF

typedef struct {
    boolean AutoRequire;
    uint16 HandleId;
    uint16 Id;
    uint16 MulticastActivationRef;
    const Sd_SoAdSocketConnectionGroupIndex* MulticastGroupRef;
    const uint32 NoOfMulticastGroups;
    uint16 TcpActivationRef;
    const Sd_ClientTimerType* TimerRef;
    uint16 UdpActivationRef;
    const Sd_CapabilityRecordType* CapabilityRecord;
    const uint32 SdNoOfCapabiltyRecord;
} Sd_ConsumedEventGroupType;

typedef struct {
    uint32 ClientServiceActivationRef;
} Sd_ConsumedMethodsType;

typedef struct {
    uint32 ServerServiceActivationRef;
} Sd_ProvidedMethodsType;

#if defined(USE_DEM)
typedef struct {
    Dem_EventParameterType* SD_E_MALFORMED_MSG;
    Dem_EventParameterType* SD_E_OUT_OF_RES;
    Dem_EventParameterType* SD_E_SUBSCR_NACK_RECV;
} Sd_InstanceDemEventParameterRefsType;
#endif

#define SOCKET_CONNECTION_GROUP_NOT_SET 0xFFFF

typedef struct {
    const boolean AutoAvailable;
    const uint16 	HandleId;
    const uint16	Id;
    const uint16  InstanceId;
    const uint8	MajorVersion;
    const uint32  MinorVersion;
    const uint16 TcpSocketConnectionGroupId;
    const uint16* TcpSocketConnectionGroupSocketConnectionIdsPtr;
    const Sd_ServerTimerType* TimerRef;
    const uint16 UdpSocketConnectionGroupId;
    const uint16* UdpSocketConnectionGroupSocketConnectionIdsPtr;
    const Sd_CapabilityRecordType* CapabilityRecord;
    const uint32 SdNoOfCapabiltyRecord;
    const Sd_EventHandlerType* EventHandler;
    const uint32 NoOfEventHandlers;
    const Sd_ProvidedMethodsType ProvidedMethods;
} Sd_ServerServiceType;


typedef struct {
    const boolean AutoRequire;
    const uint16 	HandleId;
    const uint16	Id;
    const uint16  InstanceId;
    const uint8	MajorVersion;
    const uint32  MinorVersion;
    const uint16 TcpSocketConnectionGroupId;
    const uint16* TcpSocketConnectionGroupSocketConnectionIdsPtr;
    const Sd_ClientTimerType* TimerRef;
    const uint16 UdpSocketConnectionGroupId;
    const uint16* UdpSocketConnectionGroupSocketConnectionIdsPtr;
    const Sd_CapabilityRecordType* CapabilityRecord;
    const uint32 SdNoOfCapabiltyRecord;
    const Sd_ConsumedEventGroupType* ConsumedEventGroup;
    const uint32 NoOfConsumedEventGroups;
    const Sd_ConsumedMethodsType ConsumedMethods;
} Sd_ClientServiceType;



/* Sd_InstanceType */
typedef struct {
    const char* HostName;
    const uint32 SdNoOfClientServices;
    const Sd_ClientServiceType* SdClientService;
    const Sd_ClientTimerType* SdClientTimer;
#if defined(USE_DEM)
    const Sd_InstanceDemEventParameterRefsType* DemEventParameterRefs;
#endif
    const PduIdType MulticastRxPduId;
    const Sd_SoAdSocketConnectionIndex MulticastRxPduSoConRef;
    const PduIdType TxPduId;
    const PduIdType UnicastRxPduId;
    const Sd_SoAdSocketConnectionIndex UnicastRxPduSoConRef;
    const uint32 SdNoOfServerServices;
    const Sd_ServerServiceType* SdServerService;
    const Sd_ServerTimerType*  SdServerTimer;
}Sd_InstanceType;


/** @req SWS_SD_00690 */
/* Sd_Config */
typedef struct{
    const Sd_InstanceType* Instance;
}Sd_ConfigType;

/** @req SWS_SD_00118 */
/* Sd_ServerServiceSetStateType */
typedef enum
{
    SD_SERVER_SERVICE_DOWN,
    SD_SERVER_SERVICE_AVAILABLE
} Sd_ServerServiceSetStateType;

/** @req SWS_SD_00405 */
/* Sd_ClientServiceSetStateType */
typedef enum
{
    SD_CLIENT_SERVICE_RELEASED,
    SD_CLIENT_SERVICE_REQUESTED
} Sd_ClientServiceSetStateType;

/** @req SWS_SD_00550 */
/* Sd_ConsumedEventGroupSetStateType */
typedef enum
{
    SD_CONSUMED_EVENTGROUP_RELEASED,
    SD_CONSUMED_EVENTGROUP_REQUESTED
} Sd_ConsumedEventGroupSetStateType;

/** @req SWS_SD_00007 */
/** @req SWS_SD_00551 */
/* Sd_ClientServiceCurrentStateType */
typedef enum
{
    SD_CLIENT_SERVICE_DOWN,
    SD_CLIENT_SERVICE_AVAILABLE
} Sd_ClientServiceCurrentStateType;

/** @req SWS_SD_00552 */
/* Sd_ConsumedEventGroupCurrentStateType */
typedef enum
{
    SD_CONSUMED_EVENTGROUP_DOWN,
    SD_CONSUMED_EVENTGROUP_AVAILABLE
} Sd_ConsumedEventGroupCurrentStateType;

/** @req SWS_SD_00553 */
/* Sd_EventHandlerCurrentStateType */
typedef enum
{
    SD_EVENT_HANDLER_RELEASED,
    SD_EVENT_HANDLER_REQUESTED
} Sd_EventHandlerCurrentStateType;


#endif
