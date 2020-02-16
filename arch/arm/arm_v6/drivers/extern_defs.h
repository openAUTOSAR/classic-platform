/*
 * extern_defs.h
 *
 *  Created on: Nov 21, 2014
 *      Author: avenir
 */

#ifndef _EXTERN_DEFS_H_
#define _EXTERN_DEFS_H_

//TODO: RemoveMe!!!

//extern boolean MASSAGE_ARRIVE_FLAG;
//extern boolean mail_sender, mail_receiver;
//extern boolean Message_Done;
//extern boolean ETH_ATTACHED_FLAG;
//extern boolean ETH_OPEN_FLAG;
//extern boolean ETH_READ_FLAG;

//#define SERVOCH    0
//#define SPEEDCH    1
//#define NO_CHANNEL 2

//extern boolean SPEEDEVENT;
//extern boolean SERVOEVENT;

//extern int result_channel;
//extern int result_value;

//networks
//extern struct netif netif;

extern boolean network_up;
extern boolean socket_connect;
extern boolean Tx_Buffer_Free;

extern boolean Com_Write;
extern boolean Com_Write_Linux;
extern boolean Com_Write_Autosar;
extern boolean Com_Read;
extern boolean Com_Read_Linux;
extern boolean Com_Read_Autosar;
extern uint32_t res;
boolean New_Data_Flag;
boolean new_data;

extern boolean acknowledgementVCU_ComWrite;
extern boolean new_can_message;
extern boolean ComWrite;

#endif /* _EXTERN_DEFS_H_ */
