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

#include "SD_Internal.h"

#define SD_CONFIGURATION_OPTION_TYPE       0x01u
#define SD_CONFIGURATION_RESERVED_BYTE     0x00u
#define SD_HOSTNAME_KEY_LEN                0x08u
#define SD_CONFIG_OPTION_HEADER_LEN        0x04u
#define SD_CONFIGURATION_TAIL_BYTE         0x00u

void BuildServiceEntry(Sd_EntryType entry_type, const Sd_DynClientServiceType *client, const Sd_DynServerServiceType *server, uint8 *entry_array, uint8 no_of_options){
    Sd_Entry_Type1_Services entry;

    /* Fill in entry and msg with data from client or server configuration */
    /** @req SWS_SD_0162 */

    /** @req SWS_SD_0180 */
    switch (entry_type)
    {
    case SD_FIND_SERVICE:
        /** @req SWS_SD_0296 */
        /** @req SWS_SD_0240 */
        entry.Type = 0x00;
        /** @req SWS_SD_0504 */
        /** @req SWS_SD_0298 */
        /** @req SWS_SD_0506. Validation check guarantees not 0 value. */
        entry.TTL = client->ClientServiceCfg->TimerRef->TTL;
        /** @req SWS_SD_0444 */
        /** @req SWS_SD_0173 **/
        entry.ServiceID = client->ClientServiceCfg->Id;
        /** @req SWS_SD_0501 */
        /** @req SWS_SD_0175 **/
        /** @req SWS_SD_0295 **/
        entry.InstanceID = client->ClientServiceCfg->InstanceId;
        /** @req SWS_SD_0502 */
        /** @req SWS_SD_0178 **/
        entry.MajorVersion = client->ClientServiceCfg->MajorVersion;
        /** @req SWS_SD_0503 */
        /** @req SWS_SD_0182 **/
        entry.MinorVersion = client->ClientServiceCfg->MinorVersion;
        break;
    case SD_OFFER_SERVICE:
        /** @req SWS_SD_0297 */
        /** @req SWS_SD_0254 */
        entry.Type = 0x01;
        /** @req SWS_SD_0513 */
        /** @req SWS_SD_0299 */
        /** @req SWS_SD_0515. Validation check guarantees not 0 value. */
        entry.TTL = server->ServerServiceCfg->TimerRef->TTL;
        /** @req SWS_SD_0509 */
        /** @req SWS_SD_0173 **/
        entry.ServiceID = server->ServerServiceCfg->Id;
        /** @req SWS_SD_0510 */
        /** @req SWS_SD_0175 **/
        /** @req SWS_SD_0295 **/
        entry.InstanceID = server->ServerServiceCfg->InstanceId;
        /** @req SWS_SD_0511 */
        /** @req SWS_SD_0178 **/
        entry.MajorVersion = server->ServerServiceCfg->MajorVersion;
        /** @req SWS_SD_0512 */
        /** @req SWS_SD_0182 **/
        entry.MinorVersion = server->ServerServiceCfg->MinorVersion;
        break;
    case SD_STOP_OFFER_SERVICE:
        /** @req SWS_SD_0297 */
        /** @req SWS_SD_0423 */
        entry.Type = 0x01;
        /** @req SWS_SD_0425 */
        /** @req SWS_SD_0253 */
        entry.TTL = 0;
        /** @req SWS_SD_0424 */
        /** @req SWS_SD_0173 **/
        entry.ServiceID = server->ServerServiceCfg->Id;
        /** @req SWS_SD_0175 **/
        /** @req SWS_SD_0295 **/
        entry.InstanceID = server->ServerServiceCfg->InstanceId;
        /** @req SWS_SD_0178 **/
        entry.MajorVersion = server->ServerServiceCfg->MajorVersion;
        /** @req SWS_SD_0182 **/
        entry.MinorVersion = server->ServerServiceCfg->MinorVersion;
        break;
    default:
        /* Error. IMPROVEMENT: Error handling? */
        entry.Type = 0x00;
        entry.TTL = 0;
        entry.ServiceID = 0;
        entry.InstanceID = 0;
        entry.MajorVersion = 0;
        entry.MinorVersion = 0;
        break;
    }

    /** @req SWS_SD_0267 */

    /** @req SWS_SD_0164 */
    entry.IndexFirstOptionRun = 0;  /* Currently always 0 in one-entry messages */
    /** @req SWS_SD_0166 */
    entry.IndexSecondOptionRun = 0; /* Currently always 0 in one-entry messages */
    /** @req SWS_SD_0168 */
    entry.NumberOfOption1 = no_of_options;
    /** @req SWS_SD_0170 */
    entry.NumberOfOption2 = 0;    /* Currently always 0 in one-entry messages */

    /* IMPROVEMENT: Handle options for multi-entry messages */

    /** @req SWS_SD_0294 */
    FillType1Entry(entry, entry_array);
}

void BuildEventGroupsEntry(Sd_EntryType entry_type, const Sd_DynClientServiceType *client, const Sd_Entry_Type2_EventGroups *subscribe_entry, uint8 event_group_index, uint8 *entry_array, uint8 no_of_options){
    Sd_Entry_Type2_EventGroups entry;

    /* Fill in entry and msg with data from client or server */

    /** @req SWS_SD_0385 */
    /** @req SWS_SD_0200 */
    /** @req SWS_SD_0289 */
    switch (entry_type)
    {
    case SD_SUBSCRIBE_EVENTGROUP:
        /** @req SWS_SD_0301 */
        /** @req SWS_SD_0312 */
        entry.Type = 0x06;
        /** @req SWS_SD_0193 **/
        entry.ServiceID = client->ClientServiceCfg->Id;
        /** @req SWS_SD_0195 **/
        entry.InstanceID = client->ClientServiceCfg->InstanceId;
        /** @req SWS_SD_0198 **/
        entry.MajorVersion = client->ClientServiceCfg->MajorVersion;
        entry.Counter = 0;  /* IMPROVEMENT:  Handle multiple subscriptions. */
        if (client->ClientServiceCfg->ConsumedEventGroup != NULL) {
            /** @req SWS_SD_0304 */
            entry.TTL = client->ClientServiceCfg->ConsumedEventGroup[event_group_index].TimerRef->TTL;
            if (client->ConsumedEventGroups[event_group_index].ConsumedEventGroupMode == SD_CONSUMED_EVENTGROUP_REQUESTED) {
                /** @req SWS_SD_0204 **/
                /** @req SWS_SD_0291 */
                entry.EventgroupID = client->ClientServiceCfg->ConsumedEventGroup[event_group_index].Id;
            }
            else{
                /* Should not occur? */
                entry.EventgroupID = 0;
            }
        }
        else {
            entry.EventgroupID = 0;
        }
        break;
    case SD_STOP_SUBSCRIBE_EVENTGROUP:
        /** @req SWS_SD_0301 */
        /** @req SWS_SD_0313 */
        entry.Type = 0x06;
        /** @req SWS_SD_0426 */
        /** @req SWS_SD_0306 */
        entry.TTL = 0;
        /** @req SWS_SD_0427 */
        /** @req SWS_SD_0193 **/
        entry.ServiceID = client->ClientServiceCfg->Id;
        /** @req SWS_SD_0195 **/
        entry.InstanceID = client->ClientServiceCfg->InstanceId;
        /** @req SWS_SD_0198 **/
        entry.MajorVersion = client->ClientServiceCfg->MajorVersion;
        entry.Counter = 0;  /* IMPROVEMENT:  Handle multiple subscriptions. */
        if (client->ClientServiceCfg->ConsumedEventGroup != NULL) {
            if (client->ConsumedEventGroups[event_group_index].ConsumedEventGroupMode == SD_CONSUMED_EVENTGROUP_REQUESTED) {
                /** @req SWS_SD_0204 **/
                /** @req SWS_SD_0291 */
                entry.EventgroupID = client->ClientServiceCfg->ConsumedEventGroup[event_group_index].Id;
            }
            else{
                /* Should not occur? */
                entry.EventgroupID = 0;
            }
        }
        else {
            entry.EventgroupID = 0;
        }
        break;
    case SD_SUBSCRIBE_EVENTGROUP_ACK:
        /** @req SWS_SD_00314 */
        entry.Type = 0x07;
        /** @req SWS_SD_00315 */
        entry.TTL = subscribe_entry->TTL;
        /** @req SWS_SD_00428 */
        /** @req SWS_SD_0193 **/
        entry.ServiceID = subscribe_entry->ServiceID;
        /** @req SWS_SD_0195 **/
        entry.InstanceID = subscribe_entry->InstanceID;
        /** @req SWS_SD_0198 **/
        entry.MajorVersion = subscribe_entry->MajorVersion;
        entry.Counter = subscribe_entry->Counter;
        /** @req SWS_SD_0204 **/
        /** @req SWS_SD_0291 */
        entry.EventgroupID = subscribe_entry->EventgroupID;
        break;
    case SD_SUBSCRIBE_EVENTGROUP_NACK:
        /** @req SWS_SD_00316 */
        entry.Type = 0x07;
        /** @req SWS_SD_00432 */
        /** @req SWS_SD_00306 */
        entry.TTL = 0;
        /** @req SWS_SD_00431 */
        /** @req SWS_SD_0193 **/
        entry.ServiceID = subscribe_entry->ServiceID;
        /** @req SWS_SD_0195 **/
        entry.InstanceID = subscribe_entry->InstanceID;
        /** @req SWS_SD_0198 **/
        entry.MajorVersion = subscribe_entry->MajorVersion;
        entry.Counter = subscribe_entry->Counter;
        /** @req SWS_SD_0204 **/
        /** @req SWS_SD_0291 */
        entry.EventgroupID = subscribe_entry->EventgroupID;
        break;
    default:
        /* Error */
        entry.Type = 0x00;
        entry.TTL = 0;
        entry.ServiceID = 0;
        entry.InstanceID = 0;
        entry.MajorVersion = 0;
        break;
    }
    entry.IndexFirstOptionRun = 0; /* IMPROVEMENT: Handling of Options */
    entry.IndexSecondOptionRun = 0;
    entry.NumberOfOption1 = 0;
    entry.NumberOfOption2 = 0;

    /** @req SWS_SD_0386 */
    entry.IndexFirstOptionRun = 0;  /* Currently always 0 in one-entry messages */
    /** @req SWS_SD_0187 */
    entry.IndexSecondOptionRun = 0; /* Currently always 0 in one-entry messages */
    /** @req SWS_SD_0188 */
    entry.NumberOfOption1 = no_of_options;
    /** @req SWS_SD_0190 */
    entry.NumberOfOption2 = 0;    /* Currently always 0 in one-entry messages */

    /* IMPROVEMENT: Handle options for multi-entry messages */

    /** @req SWS_SD_0290 */
    FillType2Entry(entry, entry_array);
}

#define IPV4ENDPOINT_OPTION_LENGTH 12
#define IPV4ENDPOINT_OPTION_TYPE 0x04
#define IPV4MULTICAST_OPTION_LENGTH 12
#define IPV4MULTICAST_OPTION_TYPE 0x14

/* Build an Ipv4 Option from the socket connection group sent as parameter */
void BuildIpv4EndpointOption(const uint16* socket_connection_group, uint8 protocol, uint32* offset, uint8 *options_array, uint32 *options_length) {

    TcpIp_SockAddrType LocalAddrPtr;
    uint8 NetmaskPtr;
    TcpIp_SockAddrType DefaultRouterPtr;

    /** @req SWS_SD_0210 */
    /** @req SWS_SD_0211 */
    /** @req SWS_SD_0213 */
    /** @req SWS_SD_0214 */
    uint8 endpoint_option[IPV4ENDPOINT_OPTION_LENGTH] = {0x00,0x00,IPV4ENDPOINT_OPTION_TYPE,0x00,0x00,0x00,0x00,0x00,0x00,protocol,0x00,0x00};

    LocalAddrPtr.domain = TCPIP_AF_INET;
    DefaultRouterPtr.domain = TCPIP_AF_INET;
    /* Get local ipaddress data from the first SoCon in the Group */
    for (uint16 i=0;i<sizeof(socket_connection_group)/sizeof(uint16); i++){
        (void)SoAd_GetLocalAddr(socket_connection_group[i], &LocalAddrPtr, &NetmaskPtr, &DefaultRouterPtr);
        if (LocalAddrPtr.domain == TCPIP_AF_INET) {
            /* Set option length = always 9 */
            /** @req SWS_SD_0209 */
            endpoint_option[1] = 0x09;
            /* Set ipv4-Address */
            /** @req SWS_SD_0212 */
            memcpy(&endpoint_option[4],LocalAddrPtr.addr,4);
            /* Set port number */
            /** @req SWS_SD_0215 */
            uint16 port = htonl16(LocalAddrPtr.port);
            memcpy(&endpoint_option[10],&port,2);
            break;
        }
    }

    if (endpoint_option[1] == 0x09){
        /* option data created. Copy it to option array */
        memcpy (&options_array[*offset], &endpoint_option[0], IPV4ENDPOINT_OPTION_LENGTH);
        *offset += IPV4ENDPOINT_OPTION_LENGTH;
        *options_length = *offset;
    }

}

/* Build an Ipv4 Multicast Option from the socket connection group sent as parameter */
void BuildIpv4MulticastOption(const uint16 socket_connection, uint32* offset, uint8 *options_array, uint32 *options_length) {
    TcpIp_SockAddrType LocalAddrPtr;
    uint8 NetmaskPtr;
    TcpIp_SockAddrType DefaultRouterPtr;

    /** @req SWS_SD_00391 */
    /** @req SWS_SD_00392 */
    /** @req SWS_SD_00394 */
    /** @req SWS_SD_00395 */
    uint8 multicast_option[IPV4MULTICAST_OPTION_LENGTH] = {0x00,0x00,IPV4MULTICAST_OPTION_TYPE,0x00,0x00,0x00,0x00,0x00,0x00,UDP_PROTO,0x00,0x00};

    LocalAddrPtr.domain = TCPIP_AF_INET;
    DefaultRouterPtr.domain = TCPIP_AF_INET;

    /* Get local ipaddress data from the first SoCon in the Group */
    (void)SoAd_GetLocalAddr(socket_connection, &LocalAddrPtr, &NetmaskPtr, &DefaultRouterPtr);
    if (LocalAddrPtr.domain == TCPIP_AF_INET) {
        /** @req SWS_SD_00390 */
        /* Set option length = always 9 */
        multicast_option[1] = 0x09;
        /** @req SWS_SD_00393 */
        /* Set ipv4-Address */
        memcpy(&multicast_option[4],LocalAddrPtr.addr,4);
        /** @req SWS_SD_00396 */
        /* Set port number */
        uint16 port = htonl16(LocalAddrPtr.port);
        memcpy(&multicast_option[10],&port,2);
    }

    if (multicast_option[1] == 0x09){
        /* option data created. Copy it to option array */
        memcpy (&options_array[*offset], &multicast_option[0], IPV4MULTICAST_OPTION_LENGTH);
        *offset += IPV4MULTICAST_OPTION_LENGTH;
        *options_length = *offset;
    }

}
/* Build hostname configuration option  */
void BuildHostNameConfigOption(uint32* offset, uint8 *options_array, uint16* totalConfigLen,const char* hostName ) {

    uint16 hostNameValueLen = strlen(hostName);
    char hostNameKey[SD_HOSTNAME_KEY_LEN] = {'h','o','s','t','n','a','m','e'};
    *offset = *offset + SD_CONFIG_OPTION_HEADER_LEN;  /* Leave it configuration header */
    options_array[*offset] =  SD_HOSTNAME_KEY_LEN + 1 + hostNameValueLen;
    memcpy (&options_array[*offset + 1], hostNameKey, SD_HOSTNAME_KEY_LEN);
    options_array[*offset + 1 + SD_HOSTNAME_KEY_LEN] = '=';
    memcpy (&options_array[*offset + 2 + SD_HOSTNAME_KEY_LEN],hostName,hostNameValueLen);
    *totalConfigLen = options_array[*offset] + 1; /* add Length byte of hostname key pair */
    *offset = (*offset + 2 + SD_HOSTNAME_KEY_LEN + hostNameValueLen);
}

void BuildClientConfigOption(uint32* offset, uint8 *options_array, uint16* totalConfigLen,Sd_EntryType entry_type, const Sd_DynClientServiceType *client, uint8 event_index){
    uint8 keyLen;
    uint8 valueLen;
    uint8 i;
    if(client->ClientServiceCfg->CapabilityRecord != NULL){
        if(*totalConfigLen == 0){
            *offset = *offset + SD_CONFIG_OPTION_HEADER_LEN;  /* Leave it for configuration header */
        }
        for(i= 0; i < client->ClientServiceCfg->SdNoOfCapabiltyRecord; i++ ){
            keyLen = strlen(client->ClientServiceCfg->CapabilityRecord[i].Key);
            valueLen = 0;
            options_array[*offset] =  keyLen;
            memcpy(&options_array[*offset+1], client->ClientServiceCfg->CapabilityRecord[i].Key, keyLen);
            options_array[*offset + 1 + keyLen] = '=';
            options_array[*offset] +=  1;
            if(client->ClientServiceCfg->CapabilityRecord[i].Value != NULL){
                valueLen = strlen(client->ClientServiceCfg->CapabilityRecord[i].Value);
                options_array[*offset] += valueLen;
                memcpy(&options_array[*offset+2+keyLen], client->ClientServiceCfg->CapabilityRecord[i].Value, valueLen);
            }
            *totalConfigLen += options_array[*offset] + 1; /* add Length byte of  key pair */
            *offset = (*offset + 2 + keyLen + valueLen);
        }
    }
    if((entry_type == SD_SUBSCRIBE_EVENTGROUP) ||
       (entry_type == SD_STOP_SUBSCRIBE_EVENTGROUP)){
        if(client->ClientServiceCfg->ConsumedEventGroup[event_index].CapabilityRecord != NULL){
            if(*totalConfigLen == 0){
                *offset = *offset + SD_CONFIG_OPTION_HEADER_LEN;  /* Leave it for configuration header */
            }
            for(i= 0; i < client->ClientServiceCfg->ConsumedEventGroup[event_index].SdNoOfCapabiltyRecord; i++ ){
                keyLen = strlen(client->ClientServiceCfg->ConsumedEventGroup[event_index].CapabilityRecord[i].Key);
                valueLen = 0;
                options_array[*offset] =  keyLen;
                memcpy(&options_array[*offset+1], client->ClientServiceCfg->ConsumedEventGroup[event_index].CapabilityRecord[i].Key, keyLen);
                options_array[*offset + 1 + keyLen] = '=';
                options_array[*offset] +=  1;
                if(client->ClientServiceCfg->ConsumedEventGroup[event_index].CapabilityRecord[i].Value != NULL){
                    valueLen = strlen(client->ClientServiceCfg->ConsumedEventGroup[event_index].CapabilityRecord[i].Value);
                    options_array[*offset] += valueLen;
                    memcpy(&options_array[*offset+2+keyLen], client->ClientServiceCfg->ConsumedEventGroup[event_index].CapabilityRecord[i].Value, valueLen);
                }
                *totalConfigLen += options_array[*offset] + 1; /* add Length byte of  key pair */
                *offset = (*offset + 2 + keyLen + valueLen);
            }
        }
    }
}

void BuildServerConfigOption(uint32* offset, uint8 *options_array, uint16* totalConfigLen,Sd_EntryType entry_type, const Sd_DynServerServiceType *server, uint8 event_index){
    uint8 keyLen;
    uint8 valueLen;
    uint8 i;
    if(server->ServerServiceCfg->CapabilityRecord != NULL){
        if(*totalConfigLen == 0){
            *offset = *offset + SD_CONFIG_OPTION_HEADER_LEN;  /* Leave it for configuration header */
        }
        for(i= 0; i < server->ServerServiceCfg->SdNoOfCapabiltyRecord; i++ ){
            keyLen = strlen(server->ServerServiceCfg->CapabilityRecord[i].Key);
            valueLen = 0;
            options_array[*offset] =  keyLen;
            memcpy(&options_array[*offset+1], server->ServerServiceCfg->CapabilityRecord[i].Key, keyLen);
            options_array[*offset + 1 + keyLen] = '=';
            options_array[*offset] +=  1;
            if(server->ServerServiceCfg->CapabilityRecord[i].Value != NULL){
                valueLen = strlen(server->ServerServiceCfg->CapabilityRecord[i].Value);
                options_array[*offset] += valueLen;
                memcpy(&options_array[*offset+2+keyLen], server->ServerServiceCfg->CapabilityRecord[i].Value, valueLen);
            }
            *totalConfigLen += options_array[*offset] + 1; /* add Length byte of hostname key pair */
            *offset = (*offset + 2 + keyLen + valueLen);
        }
    }
    if((entry_type == SD_SUBSCRIBE_EVENTGROUP_ACK) ||
       (entry_type == SD_SUBSCRIBE_EVENTGROUP_NACK)){
        if(server->ServerServiceCfg->EventHandler[event_index].CapabilityRecord != NULL){
            if(*totalConfigLen == 0){
                *offset = *offset + SD_CONFIG_OPTION_HEADER_LEN;  /* Leave it for configuration header */
            }
            for(i= 0; i < server->ServerServiceCfg->EventHandler[event_index].SdNoOfCapabiltyRecord; i++ ){
                keyLen = strlen(server->ServerServiceCfg->EventHandler[event_index].CapabilityRecord[i].Key);
                valueLen = 0;
                options_array[*offset] =  keyLen;
                memcpy(&options_array[*offset+1], server->ServerServiceCfg->EventHandler[event_index].CapabilityRecord[i].Key, keyLen);
                options_array[*offset + 1 + keyLen] = '=';
                options_array[*offset] +=  1;
                if(server->ServerServiceCfg->EventHandler[event_index].CapabilityRecord[i].Value != NULL){
                    valueLen = strlen(server->ServerServiceCfg->EventHandler[event_index].CapabilityRecord[i].Value);
                    options_array[*offset] += valueLen;
                    memcpy(&options_array[*offset+2+keyLen], server->ServerServiceCfg->EventHandler[event_index].CapabilityRecord[i].Value, valueLen);
                }
                *totalConfigLen += options_array[*offset] + 1; /* add Length byte of hostname key pair */
                *offset = (*offset + 2 + keyLen + valueLen);
            }
        }
    }
}

/** @req SWS_SD_00505 */
/** @req SWS_SD_00205 */
/** @req SWS_SD_00206 */
/** @req SWS_SD_00207 */
/** @req SWS_SD_00208 */
/** @req SWS_SD_00292 */
/** @req SWS_SD_00461 */
/** @req SWS_SD_00467 */
/** @req SWS_SD_00468 */
/** @req SWS_SD_00307 */
/** @req SWS_SD_00281 */
/** @req SWS_SD_00293 */ /* otherserv record needs to be configured from the configurator and built here accorndingly */
/* Create the option array that should be included in the message for this entry */
void BuildOptionsArray(Sd_EntryType entry_type,  const Sd_DynClientServiceType *client, const Sd_DynServerServiceType *server, uint8 event_index, uint8 *options_array, uint32 *options_length, uint8 *no_of_options,const char* hostName){
    /* Fill in options array with data from client or server */

    uint32 offset = 0;
    uint32 prev_offset = 0;
    *options_length = 0;
    *no_of_options = 0;

    /** @req SWS_SD_0477 */
    switch (entry_type)
    {
    case SD_FIND_SERVICE:
    case SD_SUBSCRIBE_EVENTGROUP_NACK:
        /* Only configuration options allowed which is created later. */
        break;
    case SD_OFFER_SERVICE:
    case SD_STOP_OFFER_SERVICE:
        /** @req SWS_SD_00416 */
        /** @req SWS_SD_00417 */
        /** @req SWS_SD_00653 */

        /* Build Ipv4Endpoint options from server config */

        prev_offset =  offset;

        /* Build Tcp options if socket connection is configured */
        if ((server->ServerServiceCfg->TcpSocketConnectionGroupId != SOCKET_CONNECTION_GROUP_NOT_SET))
        {
            BuildIpv4EndpointOption(server->ServerServiceCfg->TcpSocketConnectionGroupSocketConnectionIdsPtr, TCP_PROTO, &offset, options_array, options_length);
        }

        if (offset > prev_offset) {
            (*no_of_options)++;
            prev_offset = offset;
        }

        /* Build Udp option if socket connection is configured */
        if ((server->ServerServiceCfg->UdpSocketConnectionGroupId != SOCKET_CONNECTION_GROUP_NOT_SET))
        {
            BuildIpv4EndpointOption(server->ServerServiceCfg->UdpSocketConnectionGroupSocketConnectionIdsPtr, UDP_PROTO, &offset, options_array, options_length);
        }

        if (offset > prev_offset) {
            (*no_of_options)++;
        }
        break;
    case SD_SUBSCRIBE_EVENTGROUP:
    case SD_STOP_SUBSCRIBE_EVENTGROUP:
        /** @req SWS_SD_00655 */

        /* Build Ipv4Endpoint options from client config */

        prev_offset =  offset;

        /* Build Tcp options if socket connection is configured */
        if ((client->ClientServiceCfg->TcpSocketConnectionGroupId != SOCKET_CONNECTION_GROUP_NOT_SET))
        {
            BuildIpv4EndpointOption(client->ClientServiceCfg->TcpSocketConnectionGroupSocketConnectionIdsPtr, TCP_PROTO, &offset, options_array, options_length);
        }

        if (offset > prev_offset) {
            (*no_of_options)++;
            prev_offset = offset;
        }

        /* Build Udp option if socket connection is configured */
        if ((client->ClientServiceCfg->UdpSocketConnectionGroupId != SOCKET_CONNECTION_GROUP_NOT_SET))
        {
            BuildIpv4EndpointOption(client->ClientServiceCfg->UdpSocketConnectionGroupSocketConnectionIdsPtr, UDP_PROTO, &offset, options_array, options_length);
        }

        if (offset > prev_offset) {
            (*no_of_options)++;
        }

        break;
    case SD_SUBSCRIBE_EVENTGROUP_ACK:

        /* Build Multicast options from server config */

        prev_offset =  offset;

        /** @req SWS_SD_00659 */
        /** @req SWS_SD_00429 */
        if ( (server->ServerServiceCfg->EventHandler[event_index].Multicast != NULL) &&
                (server->ServerServiceCfg->EventHandler[event_index].Multicast->MulticastEventSoConRef != SOCKET_CONNECTION_GROUP_NOT_SET))
        {
            BuildIpv4MulticastOption(server->ServerServiceCfg->EventHandler[event_index].Multicast->MulticastEventSoConRef, &offset, options_array, options_length);
        }

        if (offset > prev_offset) {
            (*no_of_options)++;
        }

        break;
    default:
        /* Error. Should not occur */
        return;
    }
    /* Build configuration option */
    prev_offset =  offset;
    uint16 totalConfigLen = 0u;
    if(hostName != NULL ) {
        BuildHostNameConfigOption(&offset, options_array, &totalConfigLen,hostName);
    }
    if (client != NULL){
        BuildClientConfigOption(&offset, options_array,&totalConfigLen,entry_type, client, event_index);
    }
    else if (server != NULL){
        BuildServerConfigOption(&offset, options_array,&totalConfigLen,entry_type, server, event_index);
    }else{
        /* For Misra */
    }

    if (offset > prev_offset) {
        (*no_of_options)++; /* Zero-terminated Configuration String,number of options is incremented by 1 always ???? */
        totalConfigLen += 2u; /* reserved byte + Tail 0x00 byte */
        options_array[prev_offset + 0] =  (uint8) ((totalConfigLen >> 8u) & 0xFFu);
        options_array[prev_offset + 1] =  (uint8)  (totalConfigLen & 0xFFu);
        options_array[prev_offset + 2] =  SD_CONFIGURATION_OPTION_TYPE;
        options_array[prev_offset + 3] =  SD_CONFIGURATION_RESERVED_BYTE;
        options_array[offset] =           SD_CONFIGURATION_TAIL_BYTE;
        *options_length += totalConfigLen+3;
    }

}

/* Fills in an array with pointers to each option record in the option array in the message */
void OptionsReceived (uint8 *options_array, uint32 length, Sd_Entry_Type1_Services *entry1, Sd_Entry_Type2_EventGroups *entry2, uint8 **options1, uint8 **options2) {

    uint8* opt_address[MAX_OPTIONS];
    memset (opt_address, 0, (MAX_OPTIONS * 4));

    /* Assign startaddresses of all options in the Options Array */
    uint32 offset = 0;
    uint8 index = 0;
    while (offset < length) {
        uint16 current_option_length = (uint16) (options_array[offset] * 16 + options_array[offset+1]);

        opt_address[index] = (uint8 *) (options_array + offset);

        offset += (current_option_length + 3);
        index++;
    }

    /* Assign the result options array addresses */
    /** @req SWS_SD_0223 */
    /** @req SWS_SD_0224 */
    /** @req SWS_SD_0225 */
    /** @req SWS_SD_0226 */
    uint8 opt = 0;
    if (entry1 != NULL){
        for (opt = 0; opt < entry1->NumberOfOption1; opt++) {
            options1[opt] = opt_address[entry1->IndexFirstOptionRun + opt];
        }
        for (opt = 0; opt < entry1->NumberOfOption2; opt++) {
            options2[opt] = opt_address[entry1->IndexSecondOptionRun + opt];
        }
    }
    else if (entry2 != NULL){
        for (opt = 0; opt < entry2->NumberOfOption1; opt++) {
            options1[opt] = opt_address[entry2->IndexFirstOptionRun + opt];
        }
        for (opt = 0; opt < entry2->NumberOfOption2; opt++) {
            options2[opt] = opt_address[entry2->IndexSecondOptionRun + opt];
        }
    }

}
