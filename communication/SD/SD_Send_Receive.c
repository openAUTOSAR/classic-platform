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

#include "SD.h"
#include "SD_Internal.h"
#include "SoAd.h"
#include "cirq_buffer.h"

#define timerDec(timer) \
    if (timer > 0) { \
        timer = timer - 1; \
    } \

/*lint -e793 Ignore over 31 significant characters in SoAd.h */

extern const Sd_ConfigType *SdCfgPtr; /*lint -e526 */

typedef struct  {
    TcpIp_SockAddrType ipaddress;
    PduInfoType *msg;
    Sd_InstanceType *svcInstance;
    boolean multicast;
} MsgType;

typedef struct  {
    Sd_DynInstanceType *sd_instance;
    Sd_DynClientServiceType *client;
    Sd_DynServerServiceType *server;
    Sd_Entry_Type2_EventGroups *subscribe_entry;
    TcpIp_SockAddrType *address;
    Sd_EntryType entry_type;
    uint8 event_index;
    uint32 wait_delay_cntr;
} DelayedRespType;

static const TcpIp_SockAddrType wildcard = {
        (TcpIp_DomainType) TCPIP_AF_INET,
        TCPIP_PORT_ANY,
        {TCPIP_IPADDR_ANY, TCPIP_IPADDR_ANY, TCPIP_IPADDR_ANY, TCPIP_IPADDR_ANY }
};

/* -----------------Receive queue (client/server) --------------- */
#define NO_OF_TOTAL_QUEUES 3u /* Client and Server queues + delay response queue */
#define NO_OF_MSG_QUEUES 2u  /* Client and Server queues */
#define RECEIVE_MAX 50u  /* IMPROVEMENT:  Make this configurable */
#define PAYLOAD_MAX 1000 /* IMPROVEMENT:  Determine maximum length of an SD_Message, make this configurable */
#define POOL_MAX (RECEIVE_MAX * NO_OF_MSG_QUEUES)

static uint8 message [PAYLOAD_MAX];  /* TBD: Calculate Max length */
static uint8 options[500];    /* TBD: Calculate Max length */

/* Memory pool for MsgType */

static const TcpIp_SockAddrType Empty_TcpIp_SockAddrType = { 0,0,{0,0,0,0}};

static MsgType Message_Pool[POOL_MAX];
static PduInfoType PduInfo_Pool[POOL_MAX];
static uint8 Payload_Pool[POOL_MAX][PAYLOAD_MAX];
static DelayedRespType DelayedResp_Pool[RECEIVE_MAX];

static CirqBufferType cirqBuf[NO_OF_TOTAL_QUEUES];
static MsgType *fetched_message;



static void * rec_insert(uint8 queue)
{
    // Sanity check of queue parameter
    if (queue > (NO_OF_TOTAL_QUEUES-1)){
        return NULL;  // quit with an error
    }
    void *buffer;
    buffer = CirqBuff_PushLock(&cirqBuf[queue]);
    if(buffer != NULL){
        CirqBuff_PushRelease(&cirqBuf[queue]);
    }

    return buffer;
}


static void *  rec_fetch(uint8 queue)
{
    // Sanity check of queue parameter
    if (queue >  (NO_OF_TOTAL_QUEUES-1)){
        return NULL;  // quit with an error
    }
    return CirqBuff_Peek(&cirqBuf[queue],0);
}


void FreeSdMessage(uint8 queue)
{
    // Sanity check of queue parameter
    if (queue >  (NO_OF_TOTAL_QUEUES-1)){
        return;  // quit with an error
    }
    uint8 *buffer;
    buffer = CirqBuff_PopLock(&cirqBuf[queue]);
    if(buffer != NULL){
        CirqBuff_PopRelease(&cirqBuf[queue]);
    }
}

/* Use it for future to service all messages for one client/server and service combination */
static uint32 GetNumofQueueSdMessages(uint8 queue){
    return (uint32)CirqBuff_Size(&cirqBuf[queue]);
}

/* Service pending delayed transmit messages upon timeout
 * Reduce the timer value by 1 , called from SD main function
 */
void Handle_PendingRespMessages(void)
{
    uint32 sdBufferedMsgs =  GetNumofQueueSdMessages(DELAYRESP_QUEUE);
    for (uint32 i = 0; i < sdBufferedMsgs; i++){
        /* Fetch next item in queue */
        DelayedRespType *respMsg = (DelayedRespType*)rec_fetch(DELAYRESP_QUEUE);
        if(respMsg != NULL){
            timerDec(respMsg->wait_delay_cntr);
            if(respMsg->wait_delay_cntr == 0u){
                FreeSdMessage(DELAYRESP_QUEUE);
                TransmitSdMessage(respMsg->sd_instance, \
                                   respMsg->client,respMsg->server,respMsg->subscribe_entry, \
                                   respMsg->event_index,respMsg->entry_type,respMsg->address, FALSE); /* last parameter should be FALSE */
            }
        }
    }
}


void InitMessagePool()
{
    for (uint32 i=0; i<POOL_MAX;i++){
        for (uint32 j=0; j<PAYLOAD_MAX; j++){
            Payload_Pool[i][j] = 0;
        }
        PduInfo_Pool[i].SduLength = 0;
        PduInfo_Pool[i].SduDataPtr = Payload_Pool[i];

        Message_Pool[i].ipaddress = Empty_TcpIp_SockAddrType;
        Message_Pool[i].msg = &PduInfo_Pool[i];
    }

    /* Initialize circular buffers */
    CirqBuff_Init(&cirqBuf[CLIENT_QUEUE],    &Message_Pool[0],            RECEIVE_MAX, sizeof(MsgType));
    CirqBuff_Init(&cirqBuf[SERVER_QUEUE],    &Message_Pool[RECEIVE_MAX],  RECEIVE_MAX, sizeof(MsgType));
    CirqBuff_Init(&cirqBuf[DELAYRESP_QUEUE], &DelayedResp_Pool[0],        RECEIVE_MAX, sizeof(DelayedRespType));
}

void Handle_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr) {

#define ENTRY_TYPE_INDEX 16u

    Sd_InstanceType *svcInstance = NULL;
    boolean is_multicast = FALSE;

    /** @req 4.2.2/SWS_SD_00482 */
    /* Determine service instance. */
    for (uint16 i = 0; i <= SD_NUMBER_OF_INSTANCES; i++) {
        if (SdCfgPtr != NULL) {
            if (SdCfgPtr->Instance[i].MulticastRxPduId == RxPduId) {
                is_multicast = TRUE;
                svcInstance = (Sd_InstanceType *) &SdCfgPtr->Instance[i];
                break;
            } else if (SdCfgPtr->Instance[i].UnicastRxPduId == RxPduId) {
                is_multicast = FALSE;
                svcInstance = (Sd_InstanceType *) &SdCfgPtr->Instance[i];
                break;
            }
        }
    }

    if (svcInstance == NULL) {
        /* No matching service - IMPROVEMENT: Add error handling */
        return;
    }

    /* Peek the type of message */
    /* IMPROVEMENT: Currently only supports one entry per message */
    uint8 entry_type = PduInfoPtr->SduDataPtr[ENTRY_TYPE_INDEX];
    uint8 queue = CLIENT_QUEUE;
    if ((entry_type == FIND_SERVICE_TYPE)
            || (entry_type == SUBSCRIBE_EVENTGROUP_TYPE)) {
        queue = SERVER_QUEUE;
    } else if (entry_type == OFFER_SERVICE_TYPE)
/*			|| (entry_type == SUBSCRIBE_EVENTGROUP_ACK_TYPE)*/
    {
        queue = CLIENT_QUEUE;
    } else if  /*(entry_type == OFFER_SERVICE_TYPE)
            || */ (entry_type == SUBSCRIBE_EVENTGROUP_ACK_TYPE) {
        queue = CLIENT_QUEUE;
    }

    /* save Message and address */
    SchM_Enter_SD_EA_0();
    MsgType *received_message = (MsgType *)rec_insert(queue);
    if (received_message == NULL) {
        /* Message Pool empty or Error -  IMPROVEMENT: Add error handling */
        SchM_Exit_SD_EA_0();
        return;
    }

    received_message->ipaddress = wildcard; // Set at higher level.
    received_message->msg->SduLength = PduInfoPtr->SduLength;
    received_message->multicast = is_multicast;
    received_message->svcInstance = svcInstance;
    memcpy(received_message->msg->SduDataPtr, PduInfoPtr->SduDataPtr,
            PduInfoPtr->SduLength);
    SchM_Exit_SD_EA_0();
    /** @req 4.2.2/SWS_SD_00708 */
    /* IMPROVEMENT: Check consistency */
}


/* Fetch the next received SD message from the queue
 * If the queue is empty FALSE is returned.
 * otherwise 0 is returned, and the Sd_Message is found in the msg parameter.
 */
boolean ReceiveSdMessage(Sd_Message *msg,  TcpIp_SockAddrType *ipaddress, uint8 queue, Sd_InstanceType **server_svc, boolean *is_multicast)
{
    /* Fetch next item in queue */
    fetched_message = rec_fetch(queue);

    /* Error */
    if (fetched_message == NULL) {
        return FALSE;
    }
    else {
        *ipaddress = fetched_message->ipaddress;
        *server_svc = fetched_message->svcInstance;
        *is_multicast = fetched_message->multicast;
        DecodeMessage(msg, fetched_message->msg->SduDataPtr, fetched_message->msg->SduLength);
    }
    return TRUE;
}


/* TransmitSdMessage assembles and transmits one SD message of any type, both client and server messages
 * Parameters:
 * instance - ref to the current instance
 * client - ref to the current client service instance dynamic data structure
 * server - ref to the current server service instance dynamic data structure
 * subscribe_entry - Used for SD_SUBSCRIBE_EVENTGROUP_ACK/NACK messages. ref to subscribe entry to reply to.
 * event_index - Used for SD_SUBSCRIBE_EVENTGROUP/STOP_SUBSCRIBE and SD_SUBSCRIBE_EVENTGROUP_ACK/NACK messages.
 *               Contains the index of the ConsumedEventGroup that should be subscribed (SUBSCRIBE) or
 *               the index of the EvvneHandler thit is subscribed (SUBSCRIBE_ACK)
 * ipaddress - unicast address to set if applicable
 * is_rxmulticast - parameter to identify if the reply message has to be delayed in case message
 *                  is received from multicast address
 */
void TransmitSdMessage(Sd_DynInstanceType *instance,
                       Sd_DynClientServiceType *client,
                       Sd_DynServerServiceType *server,
                       Sd_Entry_Type2_EventGroups *subscribe_entry,
                       uint8 event_index,
                       Sd_EntryType entry_type,
                       TcpIp_SockAddrType *ipaddress,
                       boolean is_rxmulticast)
{
    uint32 delay_ms = 0;
    uint32 wait_delay_cntr = 0;
    if(is_rxmulticast == TRUE){
        /** @req 4.2.2/SWS_SD_00491 */
        if(client != NULL){
            delay_ms = RandomDelay(client->ClientServiceCfg->TimerRef->RequestResponseMinDelay_ms,client->ClientServiceCfg->TimerRef->RequestResponseMaxDelay_ms);
        }else{ /* server */
            delay_ms = RandomDelay(server->ServerServiceCfg->TimerRef->RequestResponseMinDelay_ms,server->ServerServiceCfg->TimerRef->RequestResponseMaxDelay_ms);
        }
        wait_delay_cntr = (delay_ms / SD_MAIN_FUNCTION_CYCLE_TIME_MS);
        /** @req 4.2.2/SWS_SD_00494 */
        if(wait_delay_cntr != 0){
            SchM_Enter_SD_EA_0();
            DelayedRespType *respMsg = (DelayedRespType*) rec_insert(DELAYRESP_QUEUE);
            if (respMsg == NULL) {
                /* IMPROVEMENT: DET ERROR - ticket pending*/
                SchM_Exit_SD_EA_0();
                return;
            }
            respMsg->sd_instance = instance;
            respMsg->client = client;
            respMsg->server = server;
            respMsg->entry_type = entry_type;
            respMsg->event_index = event_index;
            respMsg->subscribe_entry = subscribe_entry;
            respMsg->address = ipaddress;
            respMsg->wait_delay_cntr = wait_delay_cntr;
            SchM_Exit_SD_EA_0();
        }
    }
    if(wait_delay_cntr == 0u){

        /** @req 4.2.2/SWS_SD_00480 */

        /* IMPROVEMENT: We need to check that parameters are OK, to avoid null pointer reference bugs.
         * i.e. server parameters is not null and is used only for server entries etc.
         * or maybe divide it for client and server messages. */

        Sd_Message sd_msg;
        uint8 entry_type1_array[ENTRY_TYPE_1_SIZE];
        uint8 entry_type2_array[ENTRY_TYPE_2_SIZE];
        boolean send_by_multicast = FALSE;

        PduIdType pdu;
        PduInfoType pduinfo;
        uint32  messagelength;
        uint32 optionslength = 0;
        uint8 no_of_options = 0;

        /* IMPROVEMENT: Handling of flags according to chapter 7.3.6. Reboot_Flag and Unicast_Flag is const until then. */
        const uint8 Reboot_Flag = 1u; /* Default value after reboot */
        const uint8 Unicast_Flag = 1u; /* Supports Unicast messages */
        const uint8 Flags = 0u;
        const uint8 NoOfEntries = 1; /* IMPROVEMENT:Change this later.*/

        sd_msg.RequestID = (uint32) ((CLIENT_ID << 16)); /* SessionID is filled in later */ /*lint !e835 Want to use CLIENT_ID even though zero as left argument */
        sd_msg.ProtocolVersion = (uint8) PROTOCOL_VERSION;
        sd_msg.InterfaceVersion = (uint8) INTERFACE_VERSION;
        sd_msg.MessageType = (uint8) MESSAGE_TYPE;
        sd_msg.ReturnCode = (uint8) RETURN_CODE;
        sd_msg.Flags =  (uint8) (Flags | (Reboot_Flag << 7u) |  (Unicast_Flag << 6u));
        sd_msg.Reserved = 0u;

        /* Create the options array */
        sd_msg.LengthOfOptionsArray = 0;
        sd_msg.OptionsArray = NULL;
        BuildOptionsArray(entry_type, client, server, event_index, options, &optionslength, &no_of_options,instance->InstanceCfg->HostName);
        if (optionslength > 0) {
            sd_msg.OptionsArray = &options[0];
            sd_msg.LengthOfOptionsArray = optionslength;
        } else {
            sd_msg.OptionsArray = NULL;
            sd_msg.LengthOfOptionsArray = 0;
        }

        /* Create entries array */
        /* NB: Currently only one entry per SD_Message is used.
         * IMPROVEMENT: Handle packing of more than one entry in each message. */
        switch (entry_type) {
        case SD_OFFER_SERVICE: /** @req 4.2.2/SWS_SD_00478 */
        case SD_FIND_SERVICE:
        case SD_STOP_OFFER_SERVICE:
            sd_msg.LengthOfEntriesArray = (uint32) ENTRY_TYPE_1_SIZE  * NoOfEntries;
            BuildServiceEntry(entry_type, client, server, entry_type1_array, no_of_options);
            sd_msg.EntriesArray = entry_type1_array;
            break;
        case SD_SUBSCRIBE_EVENTGROUP:
        case SD_STOP_SUBSCRIBE_EVENTGROUP:
        case SD_SUBSCRIBE_EVENTGROUP_ACK:
        case SD_SUBSCRIBE_EVENTGROUP_NACK:
            sd_msg.LengthOfEntriesArray = (uint32) ENTRY_TYPE_2_SIZE  * NoOfEntries;
            BuildEventGroupsEntry(entry_type, client, subscribe_entry, event_index, entry_type2_array, no_of_options);
            sd_msg.EntriesArray = entry_type2_array;
            break;
        default:
            /* Error */
            break;
        }


        if (client != NULL) {
            /* set remote address for client call */
            switch (entry_type) {
            case SD_FIND_SERVICE:
                send_by_multicast = TRUE;
                break;
            case SD_SUBSCRIBE_EVENTGROUP:
                /** @req 4.2.2/SWS_SD_00702 */
                if (client->ClientServiceCfg->ConsumedEventGroup[event_index].TcpActivationRef != ACTIVATION_REF_NOT_SET) {
                    (void)SoAd_EnableSpecificRouting (client->ClientServiceCfg->ConsumedEventGroup[event_index].TcpActivationRef,
                        client->ClientServiceCfg->TcpSocketConnectionGroupId);
                }
                /** @req 4.2.2/SWS_SD_00703 */
                else if (client->ClientServiceCfg->ConsumedEventGroup[event_index].UdpActivationRef != ACTIVATION_REF_NOT_SET) {
                    (void)SoAd_EnableSpecificRouting (client->ClientServiceCfg->ConsumedEventGroup[event_index].UdpActivationRef,
                        client->ClientServiceCfg->UdpSocketConnectionGroupId);
                }
                send_by_multicast = FALSE;
                if ((client->UdpEndpoint.valid != FALSE) || (client->TcpEndpoint.valid != FALSE)) {
                    /* Use Unicast. Set remote address to address retrieved in incoming option parameter, but use port number from SD multicast socket.
                     * ipaddress was set to the correct value when receiving OFFER_SERVICE. */
                    SoAd_SoConIdType tx_socket = instance->TxSoCon;

                    SoAd_SoConIdType rx_socket = instance->MulticastRxSoCon;
                    TcpIp_SockAddrType sd_remoteaddress;
                    sd_remoteaddress.domain = TCPIP_AF_INET;
                    (void)SoAd_GetRemoteAddr(rx_socket, &sd_remoteaddress);
                    ipaddress->port = sd_remoteaddress.port;

                    (void)SoAd_SetRemoteAddr(tx_socket, ipaddress);

                    /* Assign sessionID for this SD messsage */
                    sd_msg.RequestID |= client->UnicastSessionID;
                    /** @req 4.2.2/SWS_SD_00035 */
                    /** @req 4.2.2/SWS_SD_00036 */
                    if (client->UnicastSessionID == 0xFFFF){
                        client->UnicastSessionID = 1u;
                    }
                    else {
                        client->UnicastSessionID++;
                    }
                }
                break;
            case SD_STOP_SUBSCRIBE_EVENTGROUP:
                /** @req 4.2.2/SWS_SD_00701 */
                /** @req 4.2.2/SWS_SD_00694 */
                send_by_multicast = FALSE;
                if ((client->UdpEndpoint.valid != FALSE) || (client->TcpEndpoint.valid != FALSE)) {
                    /* Use Unicast. Set remote address to address retreived in previous option to OFFER_SERVICE,
                     * but use port number from SD multicast socket. */
                    SoAd_SoConIdType tx_socket = instance->TxSoCon;

                    if (client->TcpEndpoint.valid != FALSE) {
                        ipaddress->addr[0] = ((client->TcpEndpoint.IPv4Address & 0xFF000000) >> 24);
                        ipaddress->addr[1] = ((client->TcpEndpoint.IPv4Address & 0x00FF0000) >> 16);
                        ipaddress->addr[2] = ((client->TcpEndpoint.IPv4Address & 0x0000FF00) >> 8);
                        ipaddress->addr[3] = (client->TcpEndpoint.IPv4Address & 0x000000FF);
                        ipaddress->domain = TCPIP_AF_INET;
                    } else if (client->UdpEndpoint.valid != FALSE) {
                        ipaddress->addr[0] = ((client->UdpEndpoint.IPv4Address & 0xFF000000) >> 24);
                        ipaddress->addr[1] = ((client->UdpEndpoint.IPv4Address & 0x00FF0000) >> 16);
                        ipaddress->addr[2] = ((client->UdpEndpoint.IPv4Address & 0x0000FF00) >> 8);
                        ipaddress->addr[3] = (client->UdpEndpoint.IPv4Address & 0x000000FF);
                        ipaddress->domain = TCPIP_AF_INET;
                    }

                    /* Fetch port number from SD RX_Multicast socket. */
                    SoAd_SoConIdType rx_socket = instance->MulticastRxSoCon;
                    TcpIp_SockAddrType sd_remoteaddress;
                    sd_remoteaddress.domain = TCPIP_AF_INET;
                    (void)SoAd_GetRemoteAddr(rx_socket, &sd_remoteaddress);
                    ipaddress->port = sd_remoteaddress.port;

                    (void)SoAd_SetRemoteAddr(tx_socket, ipaddress);

                    /* Assign sessionID for this SD messsage */
                    sd_msg.RequestID |= client->UnicastSessionID;
                    /** @req 4.2.2/SWS_SD_00035 */
                    /** @req 4.2.2/SWS_SD_00036 */
                    if (client->UnicastSessionID == 0xFFFF){
                        client->UnicastSessionID = 1u;
                    }
                    else {
                        client->UnicastSessionID++;
                    }
                }
                break;
            default:
                /* Should not occur */
                break;
            }

        }

        if (server != NULL) {
            /* set remote address for server call */
            switch (entry_type) {
            case SD_OFFER_SERVICE:
            case SD_STOP_OFFER_SERVICE:
                send_by_multicast = TRUE;
                break;
            case SD_SUBSCRIBE_EVENTGROUP_ACK:
            case SD_SUBSCRIBE_EVENTGROUP_NACK:
                send_by_multicast = FALSE;
                /* Set remote address to address retrieved in incoming option parameter.
                 * ipaddress was not set in this case.
                 * If no option parameter, use ipaddress as inparameter */
                if (server->EventHandlers[event_index].UdpEndpoint.valid != FALSE) {
                       ipaddress->addr[0] = ((server->EventHandlers[event_index].UdpEndpoint.IPv4Address & 0xFF000000) >> 24);
                       ipaddress->addr[1] = ((server->EventHandlers[event_index].UdpEndpoint.IPv4Address & 0x00FF0000) >> 16);
                       ipaddress->addr[2] = ((server->EventHandlers[event_index].UdpEndpoint.IPv4Address & 0x0000FF00) >> 8);
                       ipaddress->addr[3] = (server->EventHandlers[event_index].UdpEndpoint.IPv4Address & 0x000000FF);
                       ipaddress->domain = TCPIP_AF_INET;
                       ipaddress->port = server->EventHandlers[event_index].UdpEndpoint.PortNumber;
                } else if (server->EventHandlers[event_index].TcpEndpoint.valid != FALSE) {
                       ipaddress->addr[0] = ((server->EventHandlers[event_index].TcpEndpoint.IPv4Address & 0xFF000000) >> 24);
                       ipaddress->addr[1] = ((server->EventHandlers[event_index].TcpEndpoint.IPv4Address & 0x00FF0000) >> 16);
                       ipaddress->addr[2] = ((server->EventHandlers[event_index].TcpEndpoint.IPv4Address & 0x0000FF00) >> 8);
                       ipaddress->addr[3] = (server->EventHandlers[event_index].TcpEndpoint.IPv4Address & 0x000000FF);
                       ipaddress->domain = TCPIP_AF_INET;
                       ipaddress->port = server->EventHandlers[event_index].TcpEndpoint.PortNumber;
                }

                /* Use Unicast. Set the remote address before sending */
                if (ipaddress != NULL) {
                    SoAd_SoConIdType tx_socket = instance->TxSoCon;
                    (void)SoAd_SetRemoteAddr(tx_socket, ipaddress);

                    /* Assign sessionID for this SD messsage */
                    sd_msg.RequestID |= server->UnicastSessionID;
                    /** @req 4.2.2/SWS_SD_00035 */
                    /** @req 4.2.2/SWS_SD_00036 */
                    if (server->UnicastSessionID == 0xFFFF){
                        server->UnicastSessionID = 1u;
                    }
                    else {
                        server->UnicastSessionID++;
                    }
                }
                break;
            default:
                /* Should not occur */
                break;
            }

        }

        if (send_by_multicast) {
            /* Use Multicast. Retreive multicast address from Multicast RxPdu. */
            SoAd_SoConIdType multicast_rx_socket = instance->MulticastRxSoCon;
            SoAd_SoConIdType tx_socket = instance->TxSoCon;
            TcpIp_SockAddrType destination;
            uint8 netmask;
            TcpIp_SockAddrType default_router;

            destination.domain = TCPIP_AF_INET;
            default_router.domain = TCPIP_AF_INET;
            (void)SoAd_GetLocalAddr(multicast_rx_socket,  &destination, &netmask, &default_router);
            /* Set the remote multicast address before sending */
            (void)SoAd_SetRemoteAddr(tx_socket, &destination);

            /* Assign sessionID for this SD messsage */
            sd_msg.RequestID |= instance->MulticastSessionID;
            /** @req 4.2.2/SWS_SD_00035 */
            /** @req 4.2.2/SWS_SD_00036 */
            if (instance->MulticastSessionID == 0xFFFF){
                instance->MulticastSessionID = 1u;
            }
            else {
                instance->MulticastSessionID++;
            }
        }


        /* Combine entries and options to sd message */
        memset(message,0,PAYLOAD_MAX);
        FillMessage(sd_msg, message, &messagelength);
        pduinfo.SduDataPtr = message;
        pduinfo.SduLength = (uint16) messagelength;

        pdu = instance->InstanceCfg->TxPduId;

        /** @req 4.2.2/SWS_SD_00039 */
        /** @req 4.2.2/SWS_SD_00709 */
        (void)SoAd_IfTransmit(pdu,&pduinfo);

        /** @req 4.2.2/SWS_SD_00705 */
        (void)SoAd_SetRemoteAddr(instance->TxSoCon, &wildcard);
    }
}

