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

/**
  * @file    os_mm.h
  * @brief   Memory management interface for the OS
  *
  * @addtogroup os
  * @{
  */

#ifndef OS_MM_H_
#define OS_MM_H_

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)

/**
 * @brief   Initialize the the MPU/MMU.
 * @details This normally involves setting the MPU entries for
 *          privileged mode.
 */
void Os_MMInit( void );

/**
 * @brief Set application MPU settings.
 * @param aP
 */
void Os_MMSetApplUserMode(  const OsAppVarType *aP );


/**
 * @brief Set user mode for an ISR
 *
 * @param isrPtr    Pointer to a task
 */
void Os_MMSetUserModeIsr( const OsIsrVarType *isrPtr  );

/**
 * @brief Set user mode for a task
 * @param pcbPtr
 */
void Os_MMSetUserMode( const OsTaskVarType *pcbPtr );

/**
 * @brief Check if an address (addr) is in the peripheral address
 *        range of the application
 * @param pcbPtr
 */

boolean Os_MMValidPerAddressRange( AreaIdType area, uint32 addr, uint32 size );


#endif /* defined(OS_SC3 == STD_ON) || defined(OS_SC4 == STD_ON) */

#endif /* OS_MM_H_*/

/** @} */
