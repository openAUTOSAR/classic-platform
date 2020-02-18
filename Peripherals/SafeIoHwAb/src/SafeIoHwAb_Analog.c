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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0*/

#include "SafeIoHwAb.h"
#include "SafeIoHwAb_Internal.h"

#if (SAFEIOHWAB_USING_ADC == STD_ON)

#include <Os.h>

/* @req ARC_SWS_SafeIoHwAb_00002 reading out analog values*/
Std_ReturnType SafeIoHwAb_Analog_Read(SafeIoHwAb_SignalType signal, SafeIoHwAb_AnalogValueType* value, SafeIoHwAb_StatusType *status)
{
    /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    /* @req SWS_BSW_00212 NULL pointer check */
    SAFEIOHWAB_VALIDATE_RETURN((NULL_PTR != status), SAFEIOHWAB_ANALOG_READ_ID, SAFEIOHWAB_E_PARAM_PTR, E_NOT_OK);
    status->quality = SAFEIOHWAB_ERR;
    SAFEIOHWAB_VALIDATE_RETURN((NULL_PTR != value), SAFEIOHWAB_ANALOG_READ_ID, SAFEIOHWAB_E_PARAM_PTR, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN((signal < (SafeIoHwAb_SignalType)SAFEIOHWAB_NBR_OF_ADC_SIGNALS), SAFEIOHWAB_ANALOG_READ_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK); 

    Std_ReturnType retVal = E_OK;
    status->quality = SAFEIOHWAB_GOOD;
    Adc_ValueGroupType adcValue;

    SYS_CALL_AtomicCopy16(adcValue, SafeIoHwAb_AdcValues_Buffer[signal]);

    *value = (SafeIoHwAb_AnalogValueType)adcValue;

    return retVal;
}

/* @req ARC_SWS_SafeIoHwAb_00001 storing new analog values for later readout*/
Std_ReturnType SafeIoHwAb_Analog_NewValue(SafeIoHwAb_SignalType signal, SafeIoHwAb_AnalogValueType value)
{
    /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    SAFEIOHWAB_VALIDATE_RETURN((signal < (SafeIoHwAb_SignalType)SAFEIOHWAB_NBR_OF_ADC_SIGNALS), SAFEIOHWAB_ANALOG_NEW_VALUE_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);

    Std_ReturnType retVal = E_OK;

    SYS_CALL_AtomicCopy16(SafeIoHwAb_AdcValues_Buffer[signal], (Adc_ValueGroupType)value);

    return retVal;
}

#endif
