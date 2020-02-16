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


#warning "This default file may only be used as an example!"

#ifndef CANTP_CFG_H_
#define CANTP_CFG_H_

#include "CanTp_Types.h"

#define CANTP_MAIN_FUNCTION_PERIOD_TIME_MS	1000
#define CANTP_CONVERT_MS_TO_MAIN_CYCLES(x) 	(x)/CANTP_MAIN_FUNCTION_PERIOD_TIME_MS

#define CANTP_NSDU_CONFIG_LIST_SIZE		4
#define CANTP_NSDU_RUNTIME_LIST_SIZE	4

#define FRTP_CANCEL_TRANSMIT_REQUEST 	STD_ON
#define CANTP_VERSION_INFO_API          STD_ON   /**< Build version info API */
#define CANTP_DEV_ERROR_DETECT          STD_ON

extern CanTp_ConfigType CanTpConfig;
extern const CanTp_NSduType CanTpNSduConfigList[];

#endif /* CANTP_CFG_H_ */
