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

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)

/*
 * Implementation notes:
 *   Error checks for E_OS_ID and E_OS_VALUE are not used. For
 *   the safety platform it's enough that we check that values are
 *   within application range.
 *
 *   E_OS_CALLEVEL is probably wrong in the Autosar specification,
 *   you should be able read/write from interrupt context.
 *
 * NOT Supported:
 * [SWS_Os_00806]
 *
 *
 */


/**
 * Function to check if a region should be checked at all.
 */
static boolean shouldCheckRegion( void ) {
    boolean rv = FALSE;

    ApplicationType currApp = GetApplicationID();

    /* @CODECOV:OTHER_TEST_EXIST: Tested during SIL only  */
    __CODE_COVERAGE_IGNORE__
    if( (OS_SYS_PTR->osFlags & SYSTEM_FLAGS_IN_OS) == 0u ) {
        __CODE_COVERAGE_IGNORE__
        if( currApp != INVALID_OSAPPLICATION ) {
            const OsAppConstType *aConstP = Os_ApplGetConst(currApp);
            if( aConstP->trusted == 0u ) {
                rv = TRUE;
            }
        }
    }


    return rv;
}


/* Keep MISRA happy function */
static boolean checkRange( AreaIdType  Area, uint32 Address, uint32 ReadValue, uint32 size  ) {
    boolean rv = FALSE;

    if( shouldCheckRegion() == TRUE ) {
        if( Os_MMValidPerAddressRange(Area, (uint32)Address, size) == TRUE )  {
            if( Os_ValidAddressRange((uint32)ReadValue, size ) == TRUE ) {
                rv = TRUE;
            }
        }
    } else {
        rv = TRUE;
    }
    return rv;
}
#endif

/**
 * @brief            Function to read peripheral 8-bit address
 * @details
 * @param Area
 * @param Address    The address to read
 * @param ReadValue  Content of the given memory location
 * @retval           E_OK If area is valid
 */
StatusType  Os_ReadPeripheral8 ( AreaIdType  Area,  const  uint8  * Address, uint8  * ReadValue) {

    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;

    /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.1, required], [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    OS_VALIDATE_STD( ( checkRange( Area, (uint32)Address,(uint32)ReadValue, sizeof(uint8) ) == TRUE) ,
                       E_OS_ILLEGAL_ADDRESS ,
                      OSServiceId_ReadPeripheral8 );
#endif

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

    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;

    /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.1, required], [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    OS_VALIDATE_STD( ( checkRange( Area, (uint32)Address,(uint32)ReadValue, sizeof(uint16)) == TRUE) ,
                       E_OS_ILLEGAL_ADDRESS ,
                      OSServiceId_ReadPeripheral16 );
#endif
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
    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;

    /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.1, required], [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    OS_VALIDATE_STD( ( checkRange( Area, (uint32)Address,(uint32)ReadValue, sizeof(uint32)) == TRUE) ,
                       E_OS_ILLEGAL_ADDRESS ,
                      OSServiceId_ReadPeripheral32);
#endif

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
    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    if( shouldCheckRegion() == TRUE ) {
        /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
        OS_VALIDATE_STD( (Os_MMValidPerAddressRange( Area,(uint32)Address,(uint32)sizeof(uint8)) == TRUE) ,
                           E_OS_ILLEGAL_ADDRESS , OSServiceId_WritePeripheral8);
    }
#endif
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
    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    if( shouldCheckRegion() == TRUE ) {
        /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
        OS_VALIDATE_STD( (Os_MMValidPerAddressRange( Area,(uint32)Address,(uint32)sizeof(uint16)) == TRUE) ,
                           E_OS_ILLEGAL_ADDRESS , OSServiceId_WritePeripheral16);
    }
#endif

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
    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    if( shouldCheckRegion() == TRUE ) {
        /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
        OS_VALIDATE_STD( (Os_MMValidPerAddressRange( Area,(uint32)Address,(uint32)sizeof(uint32)) == TRUE) ,
                           E_OS_ILLEGAL_ADDRESS , OSServiceId_WritePeripheral32);
    }
#endif

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
    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    if( shouldCheckRegion() == TRUE ) {
        /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
        OS_VALIDATE_STD( (Os_MMValidPerAddressRange( Area,(uint32)Address,(uint32)sizeof(uint8)) == TRUE) ,
                           E_OS_ILLEGAL_ADDRESS , OSServiceId_ModifyPeripheral8);
    }
#endif

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
    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;
#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    if( shouldCheckRegion() == TRUE ) {
        /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
        OS_VALIDATE_STD( (Os_MMValidPerAddressRange( Area,(uint32)Address,(uint32)sizeof(uint16)) == TRUE) ,
                           E_OS_ILLEGAL_ADDRESS , OSServiceId_ModifyPeripheral16);
    }
#endif
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
    /*lint -e{920} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 2.7, advisory] */
    (void)Area;

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
    StatusType rv = E_OK;

    if( shouldCheckRegion() == TRUE ) {
        /*lint -e{923} MISRA:STANDARDIZED_INTERFACE:: cast from pointer to unsigned int [MISRA 2012 Rule 11.4, advisory], [MISRA 2012 Rule 11.6, required] */
        OS_VALIDATE_STD( (Os_MMValidPerAddressRange( Area,(uint32)Address,(uint32)sizeof(uint32)) == TRUE) ,
                           E_OS_ILLEGAL_ADDRESS , OSServiceId_ModifyPeripheral32);
    }
#endif
    *Address = ((*Address & ClearMask) | SetMask);
    return E_OK;
}



