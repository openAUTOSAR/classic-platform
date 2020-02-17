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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=NONE */
/** @tagSettings DEFAULT_ARCHITECTURE=RH850F1H|ZYNQ|JACINTO6 */

#ifndef CDD_ETHTRCV_TYPES_H_
#define CDD_ETHTRCV_TYPES_H_

#include "Std_Types.h"


typedef struct {
     /* Extendable but restricted to these parameters for the moment */
    uint8 address;
}EthTrcvAddressType;

typedef struct {
    /* Extendable but restricted to these parameters for the moment */
    EthTrcvAddressType *TrcvAdrs;
}EthTrcv_ConfigSetType;


#endif /* CDD_ETHTRCV_TYPES_H_ */
