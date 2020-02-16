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

#include "SD.h"
#include "SD_Internal.h"

#define SD_LITTLE_ENDIAN TRUE /* IMPROVEMENT:  Can this be fetched somewhere, or made configurable? */

#define CONFIG_TYPE        0x01
#define IPV4ENDPOINT_TYPE  0x04
#define IPV4MULTICAST_TYPE 0x14

char keypair_pool[500]; /* IMPROVEMENT use dynamic allocation for keys and pairs when it can be freed after use
                           or create exact sum length from generator  */
/* calculates a random max delay time for  Initial Wait phase */
uint32 RandomDelay(uint32 min_ms, uint32 max_ms)
{
    uint32 r;
    r = min_ms +  rand() / ( RAND_MAX / (max_ms-min_ms+1) +1);

    return r;
}

/* Convert to network byte order */
/** @req 4.2.2/SWS_SD_00037 */
uint32 htonl(uint32 lValue1) {
    uint32 lValue2 = 0;

    if (SD_LITTLE_ENDIAN){
        lValue2 |= (lValue1 & 0xFF000000) >> 24;
        lValue2 |= (lValue1 & 0x00FF0000) >> 8;
        lValue2 |= (lValue1 & 0x0000FF00) << 8;
        lValue2 |= (lValue1 & 0x000000FF) << 24;
    }
    else
    {
        lValue2 = lValue1;
    }

    return (lValue2);
}

/* Convert to network byte order */
uint16 htonl16(uint16 lValue1) {
    uint16 lValue2 = 0;

    if (SD_LITTLE_ENDIAN){
        lValue2 |= (lValue1 & 0xFF00) >> 8;
        lValue2 |= (lValue1 & 0x00FF) << 8;
    }
    else
    {
        lValue2 = lValue1;
    }

    return (lValue2);
}

/* Fills the uint8 array with data from the Sd_Message
 * according to message format in chapter 7.3 of SD spec.
 *
 */
/** @req 4.2.2/SWS_SD_00030 **/
/** @req 4.2.2/SWS_SD_00031 **/
/** @req 4.2.2/SWS_SD_00032 **/
/** @req 4.2.2/SWS_SD_00158 **/
void FillMessage(Sd_Message msg, uint8* message, uint32 *length)
{

    /* Header information incl. Entries array length*/
    uint32 offset = 0;
    uint32 row = 0;

    row = htonl(msg.RequestID);
    memcpy(&message[offset], &row, 4);

    offset += 4;
    /** @req 4.2.2/SWS_SD_00140 **/
    message[offset] = msg.ProtocolVersion;
    /** @req 4.2.2/SWS_SD_00142 **/
    message[offset + 1] = msg.InterfaceVersion;
    /** @req 4.2.2/SWS_SD_00144 **/
    message[offset + 2] = msg.MessageType;
    /** @req 4.2.2/SWS_SD_00146 **/
    message[offset + 3] = msg.ReturnCode;

    offset += 4;
    /** @req 4.2.2/SWS_SD_00149 **/
    /** @req 4.2.2/SWS_SD_00155 **/
    row = (uint32) ((msg.Flags << 24) | msg.Reserved);
    row = htonl(row);
    memcpy(&message[offset], &row, 4);

    /** @req 4.2.2/SWS_SD_00157 **/
    offset += 4;
    row = htonl(msg.LengthOfEntriesArray);
    memcpy(&message[offset], &row, 4);

    /* Entries Array */
    offset += 4;
    memcpy(&message[offset], msg.EntriesArray, msg.LengthOfEntriesArray);

    /* Options array length */
    offset += msg.LengthOfEntriesArray;
    row = htonl(msg.LengthOfOptionsArray);
    memcpy(&message[offset], &row, 4);

    /* Options array */
    offset += 4;
    memcpy(&message[offset], msg.OptionsArray, msg.LengthOfOptionsArray);

    *length = offset + msg.LengthOfOptionsArray;
}

/* Fills the uint8 array with data from the Sd_Type1Entry
 * according to message format in chapter 7.3 of SD spec.
 *
 */
/** @req 4.2.2/SWS_SD_00161 */
/** @req 4.2.2/SWS_SD_00159 **/
void FillType1Entry(Sd_Entry_Type1_Services entry, uint8 *entry_array){

    uint32 row = 0;
    uint16 value = 0;

    /** @req 4.2.2/SWS_SD_0163 */
    /** @req 4.2.2/SWS_SD_0165 */
    /** @req 4.2.2/SWS_SD_0167 */
    /** @req 4.2.2/SWS_SD_0169 */
    entry_array[0] = entry.Type;
    entry_array[1] = entry.IndexFirstOptionRun;
    entry_array[2] = entry.IndexSecondOptionRun;
    entry_array[3] = (((entry.NumberOfOption1 << 4) & 0xF0) | (entry.NumberOfOption2 & 0x0F));

    /** @req 4.2.2/SWS_SD_0172 */
    value = htonl16(entry.ServiceID);
    memcpy(&entry_array[4], &value, 2);

    /** @req 4.2.2/SWS_SD_0174 */
    value = htonl16(entry.InstanceID);
    memcpy(&entry_array[6], &value, 2);

    /** @req 4.2.2/SWS_SD_0177 */
    /** @req 4.2.2/SWS_SD_0179 */
    row = htonl(entry.TTL);
    memcpy (&entry_array[8], &row, 4);
    entry_array[8] = entry.MajorVersion;

    /** @req 4.2.2/SWS_SD_0181 */
    row = htonl(entry.MinorVersion);
    memcpy (&entry_array[12], &row, 4);
}

/* Fills the uint8 array with data from the Sd_Type2Entry
 * according to message format in chapter 7.3 of SD spec.
 *
 */
/** @req 4.2.2/SWS_SD_0184 */
void FillType2Entry(Sd_Entry_Type2_EventGroups entry, uint8 *entry_array){

    uint32 row = 0;
    uint16 value = 0;

    /** @req 4.2.2/SWS_SD_0163 */
    /** @req 4.2.2/SWS_SD_0165 */
    /** @req 4.2.2/SWS_SD_0167 */
    /** @req 4.2.2/SWS_SD_0169 */
    entry_array[0] = entry.Type;
    /** @req 4.2.2/SWS_SD_0185 **/
    entry_array[1] = entry.IndexFirstOptionRun;
    /** @req 4.2.2/SWS_SD_0186 **/
    entry_array[2] = entry.IndexSecondOptionRun;
    /** @req 4.2.2/SWS_SD_0387 **/
    /** @req 4.2.2/SWS_SD_0189 **/
    entry_array[3] = (((entry.NumberOfOption1 << 4) & 0xF0) | (entry.NumberOfOption2 & 0x0F));

    /** @req 4.2.2/SWS_SD_0192 **/
    value = htonl16(entry.ServiceID);
    memcpy(&entry_array[4], &value, 2);
    /** @req 4.2.2/SWS_SD_0194 **/
    value = htonl16(entry.InstanceID);
    memcpy(&entry_array[6], &value, 2);

    /** @req 4.2.2/SWS_SD_0199 **/
    row = htonl(entry.TTL);
    memcpy (&entry_array[8], &row, 4);
    /** @req 4.2.2/SWS_SD_0197 **/
    entry_array[8] = entry.MajorVersion;

    /** @req 4.2.2/SWS_SD_0201 **/
    /** @req 4.2.2/SWS_SD_0202 **/
    /** @req 4.2.2/SWS_SD_0300 */
    /** @req 4.2.2/SWS_SD_0156 */
    row = (uint32)0; /* Assure reserved field is all 0 */
    /** @req 4.2.2/SWS_SD_0203 **/
    /** @req 4.2.2/SWS_SD_0691 **/
    row = (uint32) (((entry.Counter & 0x0F) << 16) | (entry.EventgroupID));
    row = htonl(row);
    memcpy (&entry_array[12], &row, 4);

}

void DecodeType1Entry(uint8 *entries_array, Sd_Entry_Type1_Services *entry)
{

    uint32 row;

    entry->Type = entries_array[0];
    entry->IndexFirstOptionRun = entries_array[1];
    entry->IndexSecondOptionRun = entries_array[2];
    entry->NumberOfOption1 = (uint8) ((entries_array[3] & 0xF0) >> 4);
    entry->NumberOfOption2 = (uint8) (entries_array[3] & 0xF);

    entry->ServiceID = entries_array[4] * 256 + entries_array[5];
    entry->InstanceID = entries_array[6] * 256 + entries_array[7];

    entry->MajorVersion = entries_array[8];
    memcpy(&row, &entries_array[8], 4);
    row = htonl(row);
    entry->TTL = (uint32) (row & 0x00FFFFFF);

    memcpy(&row, &entries_array[12], 4);
    entry->MinorVersion = htonl(row);

}

void DecodeType2Entry(uint8 *entries_array, Sd_Entry_Type2_EventGroups *entry)
{
    uint32 row;

    entry->Type = entries_array[0];
    entry->IndexFirstOptionRun = entries_array[1];
    entry->IndexSecondOptionRun = entries_array[2];
    entry->NumberOfOption1 = (uint8) ((entries_array[3] & 0xF0) >> 4);
    entry->NumberOfOption2 = (uint8) (entries_array[3] & 0xF);

    entry->ServiceID = entries_array[4] * 256 + entries_array[5];
    entry->InstanceID = entries_array[6] * 256 + entries_array[7];

    entry->MajorVersion = entries_array[8];
    memcpy(&row, &entries_array[8], 4);
    row = htonl(row);
    entry->TTL = (uint32) (row & 0x00FFFFFF);

    memcpy(&row, &entries_array[12], 4);
    row = htonl(row);
    entry->Counter = (uint8) ((row &0x000F0000) >> 16);
    entry->EventgroupID = (uint16) (row & 0x0000FFFF);

}

/** @req 4.2.2/SWS_SD_0479 **/
void DecodeMessage(Sd_Message *msg, uint8* message, uint32 length)
{
    uint32 offset = 0;
    uint32 row = 0;

    memcpy (&row, &message[offset], 4);
    msg->RequestID = htonl(row);

    offset += 4;
    msg->ProtocolVersion = message[offset];
    msg->InterfaceVersion = message[offset + 1];
    msg->MessageType = message[offset + 2];
    msg->ReturnCode = message[offset + 3];

    offset += 4;
    memcpy (&row, &message[offset], 4);
    row = htonl(row);
    msg->Flags = (row & 0xFF000000) >> 24;
    msg->Reserved = (row & 0x00FFFFFF);

    offset += 4;
    memcpy (&row, &message[offset], 4);
    msg->LengthOfEntriesArray = htonl(row);

    offset += 4;
    msg->EntriesArray = (uint8 *)&message[offset];

    const uint32 OptionsArrayOffset = (msg->LengthOfEntriesArray + 16);

    if (length > OptionsArrayOffset){
        memcpy (&row, &message[OptionsArrayOffset], 4);
        msg->LengthOfOptionsArray = htonl(row);

        if (msg->LengthOfOptionsArray > 0) {
            msg->OptionsArray = (uint8 *)&message[OptionsArrayOffset + 4];
        }
        else {
            msg->OptionsArray = NULL;
        }
    }
    else
    {
        msg->LengthOfOptionsArray = 0;
        msg->OptionsArray = NULL;
    }
}

void DecodeOptionConfiguration (uint8 *options[], Sd_CapabilityRecordType capabilty_record[], uint8 *no_of_capabilty_records )
{
    uint8 index = 0;
    uint8 len = 0;
    uint8 offset = 4u;
    uint8 separator_pos = 0;
    uint8 i;
    uint8 pool_index = 0;

    for (uint8 opt=0;opt<MAX_OPTIONS;opt++){
        if (options[opt] != NULL){
            uint8 type = options[opt][2];
            if (type == CONFIG_TYPE){
                for(;options[opt][offset] != 0x00;){
                    len =  options[opt][offset];
                    offset = offset+1;
                    separator_pos = len;
                    for (i = 0; i < len; i++){
                        if( options[opt][offset+i] == '='){
                            separator_pos = i;
                            break;
                        }
                    }
                    capabilty_record[index].Key = &keypair_pool[pool_index];
                    pool_index += separator_pos+1;
                    memcpy(capabilty_record[index].Key, &options[opt][offset], separator_pos);
                    capabilty_record[index].Key[separator_pos] = '\0';

                    if((separator_pos != len) || ((len-separator_pos) != 1)){/* separator may not be there or value may not be there */
                        capabilty_record[index].Value = &keypair_pool[pool_index];
                        pool_index += len-separator_pos;
                        memcpy(capabilty_record[index].Value, &options[opt][offset+separator_pos+1], len-separator_pos-1);
                        capabilty_record[index].Value[len-separator_pos-1] = '\0';
                    }
                    index++;
                    if(options[opt][len+offset] == 0x00){
                        break;
                    }
                    offset += len;
                    len = 0;
                }
            }
        }
    }
    *no_of_capabilty_records = index;
}


void DecodeOptionIpv4Endpoint (uint8 *options[], Ipv4Endpoint endpoint[], uint8 *no_of_endpoints)
{
    uint8 index = 0;
    uint32 row = 0;
    uint16 value = 0;

    for (uint8 opt=0;opt<MAX_OPTIONS;opt++){
        if (options[opt] != NULL){
            uint8 type = options[opt][2];
            if (type == IPV4ENDPOINT_TYPE){
                memcpy(&row, &options[opt][4], 4);
                row = htonl(row);
                endpoint[index].IPv4Address = row;
                endpoint[index].Protocol = options[opt] [9];
                memcpy(&value, &options[opt][10], 2);
                value = htonl16(value);
                endpoint[index].PortNumber = value;
                index++;
            }
        }
    }
    *no_of_endpoints = index;
}

void DecodeOptionIpv4Multicast (uint8 *options[], Ipv4Multicast multicast[], uint8 *no_of_multicasts)
{
    uint8 index = 0;
    uint32 row = 0;
    uint16 value = 0;

    for (uint8 opt=0;opt<MAX_OPTIONS;opt++){
        if (options[opt] != NULL){
            uint8 type = options[opt][2];
            if (type == IPV4MULTICAST_TYPE){
                memcpy(&row, &options[opt][4], 4);
                row = htonl(row);
                multicast[index].IPv4Address = row;
                multicast[index].Protocol = options[opt] [9];
                memcpy(&value, &options[opt][10], 2);
                value = htonl16(value);
                multicast[index].PortNumber = value;
                index++;
            }
        }
    }
    *no_of_multicasts = index;
}
