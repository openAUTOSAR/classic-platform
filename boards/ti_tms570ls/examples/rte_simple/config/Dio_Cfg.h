/*
* Configuration of module: Dio (Dio_Cfg.h)
*
* Created by:              
* Copyright:               
*
* Configured for (MCU):    TMS570
*
* Module vendor:           ArcCore
* Generator version:       2.0.0
*
* Generated by Arctic Studio (http://arccore.com) 
*           on Thu Jun 30 17:29:03 CEST 2011
*/


#if !(((DIO_SW_MAJOR_VERSION == 1) && (DIO_SW_MINOR_VERSION == 0)) )
#error Dio: Configuration file expected BSW module version to be 1.0.*
#endif


#ifndef DIO_CFG_H_
#define DIO_CFG_H_
#include "Port.h"

#define DIO_VERSION_INFO_API    STD_OFF
#define DIO_DEV_ERROR_DETECT    STD_OFF

#define DIO_END_OF_LIST  -1

// Physical ports
typedef enum
{
  DIO_PORT_A = 0,
  DIO_PORT_B = 1,
} Dio_PortTypesType;


// Channels	
#define DIO_CHANNEL_NAME_LED_CHANNEL	PORT_PAD_3

// Channel group

// Ports
#define DIO_PORT_NAME_LED_PORT 		(DIO_PORT_A)




// Pointers for convenience.
// Channels	
extern const Dio_ChannelType DioChannelConfigData[];
// Channel group
extern const Dio_ChannelGroupType DioConfigData[];
// Port
extern const Dio_PortType DioPortConfigData[];

#endif /*DIO_CFG_H_*/