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

#if (SAFEIOHWAB_USING_PWM == STD_ON)

#define IS_VALID_DUTY_CYCLE(duty) (duty <= SAFEIOHWAB_DUTY_MAX)

/* @req ARC_SWS_SafeIoHwAb_00011 setting pwm duty cycle*/
Std_ReturnType SafeIoHwAb_Pwm_Set_Duty(SafeIoHwAb_SignalType signal, SafeIoHwAb_DutyType duty, SafeIoHwAb_StatusType *status) 
{
    /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
	/* @req SWS_BSW_00212 NULL pointer check */	
    SAFEIOHWAB_VALIDATE_RETURN((NULL_PTR != status), SAFEIOHWAB_PWMDUTY_SET_ID, SAFEIOHWAB_E_PARAM_PTR, E_NOT_OK);
    status->quality = SAFEIOHWAB_ERR;
    SAFEIOHWAB_VALIDATE_RETURN((signal < (SafeIoHwAb_SignalType)SAFEIOHWAB_NBR_OF_PWM_SIGNALS), SAFEIOHWAB_PWMDUTY_SET_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN((SafeIoHwAb_Signal_Pwm_Type[signal] == SAFE_PWM_DUTY), SAFEIOHWAB_PWMDUTY_SET_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN(IS_VALID_DUTY_CYCLE(duty), SAFEIOHWAB_PWMDUTY_SET_ID, SAFEIOHWAB_E_PARAM_DUTY, E_NOT_OK);

    Std_ReturnType rv = E_OK;
    status->quality = SAFEIOHWAB_GOOD;

    Pwm_SetDutyCycle(SafeIoHwAb_Signal_Pwm_Channel[signal], (uint16)duty);
    
    return rv;
}

#if (PWM_SET_PERIOD_AND_DUTY_API == STD_ON)

/* @req ARC_SWS_SafeIoHwAb_00012 setting pwm frequency and duty cycle*/
Std_ReturnType SafeIoHwAb_Pwm_Set_DutyFreq(SafeIoHwAb_SignalType signal, SafeIoHwAb_DutyType duty, SafeIoHwAb_FrequencyType frequency, SafeIoHwAb_StatusType *status)
{
    /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */

	/* @req SWS_BSW_00212 NULL pointer check */
    SAFEIOHWAB_VALIDATE_RETURN((NULL_PTR != status), SAFEIOHWAB_PWMFREQUENCYANDDUTY_SET_ID, SAFEIOHWAB_E_PARAM_PTR, E_NOT_OK);
    status->quality = SAFEIOHWAB_ERR;
    SAFEIOHWAB_VALIDATE_RETURN((signal < (SafeIoHwAb_SignalType)SAFEIOHWAB_NBR_OF_PWM_SIGNALS), SAFEIOHWAB_PWMFREQUENCYANDDUTY_SET_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN((SafeIoHwAb_Signal_Pwm_Type[signal] == SAFE_PWM_DUTYFREQ), SAFEIOHWAB_PWMFREQUENCYANDDUTY_SET_ID, SAFEIOHWAB_E_PARAM_SIGNAL, E_NOT_OK);
    SAFEIOHWAB_VALIDATE_RETURN(IS_VALID_DUTY_CYCLE(duty), SAFEIOHWAB_PWMFREQUENCYANDDUTY_SET_ID, SAFEIOHWAB_E_PARAM_DUTY, E_NOT_OK);

    Std_ReturnType rv = E_OK;
    status->quality = SAFEIOHWAB_GOOD;
    Pwm_PeriodType pwmPeriod = SafeIoHwAb_Pwm_ConvertToPeriod(signal, frequency);
    Pwm_SetPeriodAndDuty(SafeIoHwAb_Signal_Pwm_Channel[signal], pwmPeriod, (uint16)duty);
    
    return rv;
}

#endif

#endif
