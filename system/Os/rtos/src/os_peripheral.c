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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

#include "os_i.h"


/**
 * @brief            Function to read peripheral 8-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ReadValue  Content of the given memory location
 * @retval           E_OK If area is valid
 */
StatusType  Os_ReadPeripheral8 ( AreaIdType  Area,  const  uint8  * Address, uint8  * ReadValue) {
    (void)Area;
    *ReadValue = *Address;
    return E_OK;
}

/**
 * @brief            Function to read peripheral 16-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ReadValue  Content of the given memory location
 * @retval           E_OK If area is valid
 */

StatusType  Os_ReadPeripheral16( AreaIdType  Area,  const  uint16 * Address, uint16 * ReadValue) {
    (void)Area;
    *ReadValue = *Address;
    return E_OK;
}


/**
 * @brief            Function to read peripheral 32-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ReadValue  Content of the given memory location
 * @retval           E_OK If area is valid
 */

StatusType  Os_ReadPeripheral32( AreaIdType  Area,  const  uint32 * Address, uint32 * ReadValue) {
    (void)Area;
    *ReadValue = *Address;
    return E_OK;
}

/**
 * @brief            Function to write peripheral 8-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ReadValue  Content of the given memory location
 * @retval           E_OK If area is valid
 */
StatusType  Os_WritePeripheral8 ( AreaIdType  Area,  uint8   *Address, uint8  WriteValue) {
    (void)Area;
    *Address = WriteValue;
    return E_OK;
}

/**
 * @brief            Function to write peripheral 16-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ReadValue  Content of the given memory location
 * @retval           E_OK If area is valid
 */
StatusType  Os_WritePeripheral16( AreaIdType  Area,  uint16  *Address, uint16 WriteValue) {
    (void)Area;
    *Address = WriteValue;
    return E_OK;
}

/**
 * @brief            Function to write peripheral 16-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ReadValue  Content of the given memory location
 * @retval           E_OK If area is valid
 */
StatusType  Os_WritePeripheral32( AreaIdType  Area,  uint32  *Address, uint32 WriteValue) {
    (void)Area;
    *Address = WriteValue;
    return E_OK;
}


/**
 * @brief            Function to modify peripheral 8-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ClearMask  Address will be modified with bit-AND
 * @param SetMask    Address will be modified with bit-OR
 * @retval           E_OK If area is valid
 */
StatusType  Os_ModifyPeripheral8(  AreaIdType  Area,  uint8   *Address, uint8  ClearMask, uint8 SetMask) {
    (void)Area;
    *Address = ((*Address & ClearMask) | SetMask);
    return E_OK;
}

/**
 * @brief            Function to modify peripheral 16-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ClearMask  Address will be modified with bit-AND
 * @param SetMask    Address will be modified with bit-OR
 * @retval           E_OK If area is valid
 */
StatusType  Os_ModifyPeripheral16(  AreaIdType  Area,  uint16   *Address, uint16  ClearMask, uint16 SetMask) {
    (void)Area;
    *Address = ((*Address & ClearMask) | SetMask);
    return E_OK;
}

/**
 * @brief            Function to modify peripheral 32-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ClearMask  Address will be modified with bit-AND
 * @param SetMask    Address will be modified with bit-OR
 * @retval           E_OK If area is valid
 */
StatusType  Os_ModifyPeripheral32(  AreaIdType  Area,  uint32   *Address, uint32  ClearMask, uint32 SetMask) {
    (void)Area;
    *Address = ((*Address & ClearMask) | SetMask);
    return E_OK;
}






