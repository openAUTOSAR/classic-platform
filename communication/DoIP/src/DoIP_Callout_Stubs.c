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

#include "DoIP.h"

Std_ReturnType DoIP_Arc_GetVin(uint8* Data, uint8 noOfBytes) {
    for (uint8 i = 0u; i < noOfBytes; i++) {
        Data[i] = 0x11u;
    }
    return E_OK;
}

/**
 * @brief This callout is called from DoIP whenever a routing activation is successful
 * @param id - Identifies the socket of the tcp connection */
void DoIP_Arc_TcpConnectionNotification(SoAd_SoConIdType id) {
    (void) id;
}
