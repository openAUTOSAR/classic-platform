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

#ifndef SD_INTERNAL_H_
#define SD_INTERNAL_H_

#include "SD.h"

//lint -w1
/*lint -esym(526,ReceiveSdMessage,FreeMessage,RandomDelay,TransmitSdMessage,DecodeType1Entry,DecodeType2Entry,
  Sd_DynConfig,FillMessage,DecodeMessage,BuildServiceEntry,BuildEventGroupsEntry,InitMessagePool,UpdateClientService,
  UpdateServerService,Handle_RxIndication,PutBackSdMessage,OptionsReceived, DecodeOptionIpv4Endpoint,Handle_PendingRespMessages) */

/* -----------------------------------------------------------------------*/

#define UDP_PROTO 0x11
#define TCP_PROTO 0x06

/* Types for dynamic data in the client and server service state machines */
typedef enum {
    SD_DOWN_PHASE,
    SD_INITIAL_WAIT_PHASE,
    SD_REPETITION_PHASE,
    SD_MAIN_PHASE
} Sd_PhaseType;

typedef struct {
    uint32 IPv4Address;
    uint8 Protocol;
    uint16 PortNumber;
    boolean valid;
}Ipv4Endpoint;

typedef struct {
    uint32 IPv4Address;
    uint8 Protocol;
    uint16 PortNumber;
    boolean valid;
} Ipv4Multicast;

/* Sd_DynConsumedEventGroupType */
typedef struct {
    Sd_ConsumedEventGroupSetStateType ConsumedEventGroupMode;
    Sd_ConsumedEventGroupCurrentStateType ConsumedEventGroupState;
    uint32 TTL_Timer_Value_ms;
    Ipv4Endpoint UdpEndpoint;
    Ipv4Endpoint TcpEndpoint;
    Ipv4Multicast MulticastAddress;
    boolean Acknowledged;
}Sd_DynConsumedEventGroupType;

/* Sd_DynClientServiceType */
typedef struct {
    const Sd_ClientServiceType *ClientServiceCfg; /* static config */
    Sd_PhaseType Phase;
    Sd_ClientServiceCurrentStateType CurrentState;
    Sd_ClientServiceSetStateType ClientServiceMode;
    boolean SocketConnectionOpened;
    uint32 TTL_Timer_Value_ms;
    boolean TTL_Timer_Running;
    sint32 FindDelay_Timer_Value_ms;
    sint32 FindRepDelay_Timer_Value_ms;
    Sd_DynConsumedEventGroupType *ConsumedEventGroups;
    boolean OfferActive;
    boolean FindDelayTimerOn;
    boolean FindRepDelayTimerOn;
    uint32 RepetitionFactor; /* Factor for interval between FindService calls in Rep Phase */
    uint32 FindRepetitions; /* Counter for Repetition Phase */
    boolean TcpSoConOpened;
    boolean UdpSoConOpened;
    Ipv4Endpoint UdpEndpoint;
    Ipv4Endpoint TcpEndpoint;
    uint16 UnicastSessionID;
} Sd_DynClientServiceType;

#define MAX_NO_OF_SUBSCRIBERS 15 /* TBD */
/* Sd_SubscriberType */
typedef struct {
    boolean TTL_Timer_On;
    uint32 TTL_Timer_Value_ms;
}Sd_SubscriberType;

/* Sd_DynEventHandlerType */
typedef struct {
    Sd_EventHandlerCurrentStateType EventHandlerState;
    uint16 NoOfSubscribers;
    Sd_SubscriberType FanOut[MAX_NO_OF_SUBSCRIBERS];
    Ipv4Endpoint UdpEndpoint;
    Ipv4Endpoint TcpEndpoint;
}Sd_DynEventHandlerType;

/* Sd_DynServerServiceType */
typedef struct {
    const Sd_ServerServiceType *ServerServiceCfg; /* static config */
    Sd_PhaseType Phase;
    Sd_ServerServiceSetStateType ServerServiceMode;
    boolean SocketConnectionOpened;
    sint32 InitialOffer_Timer_Value_ms;
    sint32 OfferRepDelay_Timer_Value_ms;
    sint32 OfferCyclicDelay_Timer_Value_ms;
    boolean InitialOfferTimerOn;
    boolean OfferRepDelayTimerOn;
    boolean OfferCyclicDelayTimerOn;
    uint32 RepetitionFactor; /* Factor for interval between OfferService calls in Repetition Phase */
    uint32 OfferRepetitions; /* Counter for Repetition Phase */
    Sd_DynEventHandlerType *EventHandlers;
    boolean TcpSoConOpened;
    boolean UdpSoConOpened;
    uint16 UnicastSessionID;
} Sd_DynServerServiceType;

/* Sd_DynInstanceType */
typedef struct {
    const Sd_InstanceType* InstanceCfg; /* static config */
    Sd_DynClientServiceType* SdClientService;
    Sd_DynServerServiceType* SdServerService;
    boolean TxPduIpAddressAssigned;
    boolean SdInitCalled;
    SoAd_SoConIdType TxSoCon;
    SoAd_SoConIdType MulticastRxSoCon;
    SoAd_SoConIdType UnicastRxSoCon;
    uint16 MulticastSessionID;
}Sd_DynInstanceType;

typedef struct{
    Sd_DynInstanceType *Instance;
}Sd_DynConfigType;

typedef enum {
    SD_UNINITIALIZED,
    SD_INITIALIZED
} Sd_ModuleStateType;

/* -----------------------------------------------------------------------*/
/* Message format types */

/** @req SWS_SD_0033 */
#define CLIENT_ID 0x0000
/** @req SWS_SD_0141 */
#define PROTOCOL_VERSION 0x01
/** @req SWS_SD_0143 */
#define INTERFACE_VERSION 0x01
/** @req SWS_SD_0145 */
#define MESSAGE_TYPE 0x02
/** @req SWS_SD_0147 */
#define RETURN_CODE 0x00
/** @req SWS_SD_0160 */
#define ENTRY_TYPE_1_SIZE 16
/** @req SWS_SD_0183 */
#define ENTRY_TYPE_2_SIZE 16

#define TTL_TIMER_MAX 0xFFFFFFu

#define MAX_OPTIONS 15u

typedef struct{
    uint32 RequestID;
    uint8 ProtocolVersion;  /* = 0x01 */
    uint8 InterfaceVersion; /* = 0x01; */
    uint8 MessageType;      /* = 0x02; */
    uint8 ReturnCode;       /* = 0x00; */
    uint8 Flags; /* Only the MSB 8 bits is used, rest should be set to zero */
    uint32 Reserved;   /* 24 bits = 0 */
    uint32 LengthOfEntriesArray;  /* Length in bytes */
    uint8 *EntriesArray;
    uint32 LengthOfOptionsArray;  /* Length in bytes */
    uint8 *OptionsArray;
}Sd_Message;

typedef enum {
    SD_FIND_SERVICE,
    SD_OFFER_SERVICE,
    SD_STOP_OFFER_SERVICE,
    SD_SUBSCRIBE_EVENTGROUP,
    SD_STOP_SUBSCRIBE_EVENTGROUP,
    SD_SUBSCRIBE_EVENTGROUP_ACK,
    SD_SUBSCRIBE_EVENTGROUP_NACK
}Sd_EntryType;

#define FIND_SERVICE_TYPE 0x00
#define OFFER_SERVICE_TYPE 0x01
#define STOP_OFFER_SERVICE_TYPE 0x01
#define SUBSCRIBE_EVENTGROUP_TYPE 0x06
#define STOP_SUBSCRIBE_EVENTGROUP_TYPE 0x06
#define SUBSCRIBE_EVENTGROUP_ACK_TYPE 0x07
#define SUBSCRIBE_EVENTGROUP_NACK_TYPE 0x07


typedef struct {
    uint8 Type;
    uint8 IndexFirstOptionRun;
    uint8 IndexSecondOptionRun;
    uint8 NumberOfOption1;
    uint8 NumberOfOption2;
    uint16 ServiceID;
    uint16 InstanceID;
    uint8 MajorVersion;
    uint32 TTL; /* Only 24 bits used */
    uint32 MinorVersion;
}Sd_Entry_Type1_Services;

typedef struct {
    uint8 Type;
    uint8 IndexFirstOptionRun;
    uint8 IndexSecondOptionRun;
    uint8 NumberOfOption1; /* Only 4 bits used */
    uint8 NumberOfOption2; /* Only 4 bits used */
    uint16 ServiceID;
    uint16 InstanceID;
    uint8 MajorVersion;
    uint32 TTL; /* Only 24 bits used */
    uint8 Counter;
    uint16 EventgroupID;
}Sd_Entry_Type2_EventGroups;

/* -------------------------Sd_Client/ServerServices--------------------*/

void UpdateClientService(const Sd_ConfigType *cfgPtr, uint32 instanceno, uint32 clientno);

void UpdateServerService(const Sd_ConfigType *cfgPtr, uint32 instanceno, uint32 serverno);

/* Currently implemented in ClientService module. IMPROVEMENT: Where is the best place?*/
uint32 RandomDelay(uint32 min, uint32 max); //lint !e526

/* -------------------------Sd_Send_Receiver-----------------------------------*/

/* Index for the different queues*/
#define CLIENT_QUEUE 0u
#define SERVER_QUEUE 1u
#define DELAYRESP_QUEUE 2u

void TransmitSdMessage(Sd_DynInstanceType *instance, Sd_DynClientServiceType *client, Sd_DynServerServiceType *server, Sd_Entry_Type2_EventGroups *subscribe_entry, uint8 event_group_index, Sd_EntryType entry_type, TcpIp_SockAddrType *ipaddress, boolean is_rxmulticast); //lint !e526

boolean ReceiveSdMessage(Sd_Message *msg,  TcpIp_SockAddrType *ipaddress, uint8 queue, Sd_InstanceType **server_svc, boolean *is_multicast);

void InitMessagePool(void);

void Handle_RxIndication( PduIdType RxPduId, const PduInfoType* PduInfoPtr);

void FreeSdMessage(uint8 queue);

void Handle_PendingRespMessages(void);

/* -------------------------Sd_Entries-----------------------------------*/

void BuildServiceEntry(Sd_EntryType entry_type, const Sd_DynClientServiceType *client, const Sd_DynServerServiceType *server, uint8 *entry_array, uint8 no_of_options);

void BuildEventGroupsEntry(Sd_EntryType entry_type, const Sd_DynClientServiceType *client, const Sd_Entry_Type2_EventGroups *subscribe_entry, uint8 event_group_index, uint8 *entry_array, uint8 no_of_options);

void OptionsReceived (uint8 *options_array, uint32 length, Sd_Entry_Type1_Services *entry1, Sd_Entry_Type2_EventGroups *entry2, uint8 **options1, uint8 **options2); /*lint !e526 */

void BuildOptionsArray(Sd_EntryType entry_type,  const Sd_DynClientServiceType *client, const Sd_DynServerServiceType *server, uint8 event_index, uint8 *options_array, uint32 *options_length, uint8 *no_of_options,const char* hostName);

/* -------------------------Sd_Messages-----------------------------------*/

void FillMessage(Sd_Message msg, uint8* message, uint32 *length);

void FillType1Entry(Sd_Entry_Type1_Services entry, uint8 *entry_array);

void FillType2Entry(Sd_Entry_Type2_EventGroups entry, uint8* entry_array);

void DecodeType1Entry(uint8 *entries_array, Sd_Entry_Type1_Services *entry); //lint !e526

void DecodeType2Entry(uint8 *entries_array, Sd_Entry_Type2_EventGroups *entry); //lint !e526

void DecodeMessage(Sd_Message *msg, uint8* message, uint32 length);

void DecodeOptionIpv4Endpoint (uint8 *options[], Ipv4Endpoint endpoint[], uint8 *no_of_endpoints);

void DecodeOptionIpv4Multicast (uint8 *options[], Ipv4Multicast multicast[], uint8 *no_of_multicasts);

void DecodeOptionConfiguration (uint8 *options[], Sd_CapabilityRecordType capabilty_record[], uint8 *no_of_capabilty_records );

uint32 htonl(uint32 lValue1);

uint16 htonl16(uint16 lValue1);

#endif /* SD_INTERNAL_H_ */
