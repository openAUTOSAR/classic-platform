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

#ifndef MCU_CFG_H_
#define MCU_CFG_H_

/* @req MCU237 */
typedef enum {
   MCU_MODE_RUN = 0,
   MCU_MODE_SLEEP = 1,
   MCU_MODE_NORMAL = 2
} Mcu_ModeType;

/* @req MCU232 */
typedef enum {
   MCU_CLOCKTYPE_MCUCLOCKSETTINGCONFIG = 0,
   MCU_NBR_OF_CLOCKS,
} Mcu_ClockType;

#endif /* MCU_CFG_H_ */
