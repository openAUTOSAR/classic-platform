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


#ifndef TC2XX_H_
#define TC2XX_H_

/* Trap flags  */
#define TRP_NOT_HANDLED	1UL
#define TRP_HANDLED		2UL
#define TRP_ADJUST_ADDR	4UL

// Trap handler function
uint32 TC2xx_HandleTrap(uint32 tin , uint32 trapClass, uint32 instrAddr );

#endif /* TC2XX_H_ */
