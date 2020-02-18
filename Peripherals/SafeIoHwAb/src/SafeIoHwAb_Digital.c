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

#if (SAFEIOHWAB_USING_DIO == STD_ON)

#define IS_VALID_DIO_LEVEL(_x) ((STD_LOW == (_x)) || (STD_HIGH == (_x)))

/* @req ARC_SWS_SafeIoHwAb_00005 setting digital values*/
Std_ReturnType SafeIoHwAb_Digital_Write(SafeIoHwAb_SignalType signal, SafeIoHwAb_LevelType newValue)
{
    /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    SAFEIOHWAB_VALIDATE_RETURN((signal < (SafeIoHwAb_SignalType)SAFEIOHWAB_NBR_OF_DIO_SIGNALS), SAFEIOHWAB_DIGITAL_WRITE_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN((SafeIoHwAb_Signal_Digital_Type[signal] == SAFE_DIO_WRITE), SAFEIOHWAB_DIGITAL_WRITE_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN(IS_VALID_DIO_LEVEL((Dio_LevelType)newValue), SAFEIOHWAB_DIGITAL_WRITE_ID, SAFEIOHWAB_E_PARAM_LEVEL, E_NOT_OK);
    
    /* @req ARC_SWS_SafeIoHwAb_00010 Inversion of DIO signal*/
    if ((boolean)TRUE == SafeIoHwAb_Signal_Digital_Inverted[signal]) {
        /*lint -e9044 MISRA:OTHER:rule is advisory and this way simplifies readability:[MISRA 2012 Rule 17.8, advisory] */
        newValue = ((STD_LOW == newValue) ? STD_HIGH:STD_LOW);
     }
    Std_ReturnType ret = E_OK;

    Dio_WriteChannel(SafeIoHwAb_Signal_Digital_Channel[signal], (Dio_LevelType)newValue);
    
    return ret;
}

/* @req ARC_SWS_SafeIoHwAb_00004 reading digital values*/
Std_ReturnType SafeIoHwAb_Digital_Read(SafeIoHwAb_SignalType signal, SafeIoHwAb_LevelType *value, SafeIoHwAb_StatusType *status)
{
    /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
	/* @req SWS_BSW_00212 NULL pointer check */	
    SAFEIOHWAB_VALIDATE_RETURN((NULL_PTR != status), SAFEIOHWAB_DIGITAL_READ_ID, SAFEIOHWAB_E_PARAM_PTR, E_NOT_OK);
    status->quality = SAFEIOHWAB_ERR;
    SAFEIOHWAB_VALIDATE_RETURN((signal < (SafeIoHwAb_SignalType)SAFEIOHWAB_NBR_OF_DIO_SIGNALS), SAFEIOHWAB_DIGITAL_READ_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN((SafeIoHwAb_Signal_Digital_Type[signal] == SAFE_DIO_READ), SAFEIOHWAB_DIGITAL_READ_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN((NULL_PTR != value), SAFEIOHWAB_DIGITAL_READ_ID, SAFEIOHWAB_E_PARAM_PTR, E_NOT_OK);

    Std_ReturnType ret = E_OK;
    Dio_LevelType retVal;
    retVal = Dio_ReadChannel(SafeIoHwAb_Signal_Digital_Channel[signal]);

    /* @req ARC_SWS_SafeIoHwAb_00010 Inversion of DIO signal*/
    if ((boolean)TRUE == SafeIoHwAb_Signal_Digital_Inverted[signal]) {
        retVal = ((STD_LOW == retVal) ? STD_HIGH:STD_LOW);
    }

    if (retVal == STD_LOW) {
        *value = SAFEIOHWAB_LOW;
    } else {
        *value = SAFEIOHWAB_HIGH;
    }

    status->quality = SAFEIOHWAB_GOOD;

    return ret;
}

/* @req ARC_SWS_SafeIoHwAb_00006 writing and reading digital values*/
Std_ReturnType SafeIoHwAb_Digital_WriteReadback(SafeIoHwAb_SignalType signal, SafeIoHwAb_LevelType newValue, SafeIoHwAb_StatusType *status)
{
    /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */

	/* @req SWS_BSW_00212 NULL pointer check */
    SAFEIOHWAB_VALIDATE_RETURN((NULL_PTR != status), SAFEIOHWAB_DIGITAL_WRITE_READBACK_ID, SAFEIOHWAB_E_PARAM_PTR, E_NOT_OK);
    status->quality = SAFEIOHWAB_ERR;
    SAFEIOHWAB_VALIDATE_RETURN((signal < (SafeIoHwAb_SignalType)SAFEIOHWAB_NBR_OF_DIO_SIGNALS), SAFEIOHWAB_DIGITAL_WRITE_READBACK_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN((SafeIoHwAb_Signal_Digital_Type[signal] == SAFE_DIO_WRITEREADBACK), SAFEIOHWAB_DIGITAL_WRITE_READBACK_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN(IS_VALID_DIO_LEVEL((Dio_LevelType)newValue), SAFEIOHWAB_DIGITAL_WRITE_READBACK_ID, SAFEIOHWAB_E_PARAM_LEVEL, E_NOT_OK);
    SafeIoHwAb_LevelType readLevel;
    Std_ReturnType ret = E_OK;
    
    /* @req ARC_SWS_SafeIoHwAb_00010 Inversion of DIO signal*/
    if ((boolean)TRUE == SafeIoHwAb_Signal_Digital_Inverted[signal]) {
        /*lint -e9044 MISRA:OTHER:rule is advisory and this way simplifies readability:[MISRA 2012 Rule 17.8, advisory] */
        newValue = ((STD_LOW == newValue) ? STD_HIGH:STD_LOW);

    }

    Dio_WriteChannel(SafeIoHwAb_Signal_Digital_Channel[signal], (Dio_LevelType)newValue);
    
    /* @req ARC_SWS_SafeIoHwAb_00010 Inversion of DIO signal*/
    readLevel = (SafeIoHwAb_LevelType)Dio_ReadChannel(SafeIoHwAb_Signal_Digital_Channel[signal]);
    if ((boolean)TRUE == SafeIoHwAb_Signal_Digital_Inverted[signal]) {
        readLevel = ((STD_LOW == readLevel) ? STD_HIGH:STD_LOW);
    }
    if( readLevel != newValue ) {
        status->quality = SAFEIOHWAB_ERR; 
    } else {
        status->quality = SAFEIOHWAB_GOOD;
    }

    return ret;
}

#endif
