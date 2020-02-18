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



#ifndef IOHWAB_H_
#define IOHWAB_H_

#define IOHWAB_SW_MAJOR_VERSION     1u
#define IOHWAB_SW_MINOR_VERSION     0u
#define IOHWAB_SW_PATCH_VERSION     0u

#define IOHWAB_MODULE_ID            254u
#define IOHWAB_VENDOR_ID            60u

#if defined(CFG_IOHWAB_USE_SERVICE_COMPONENT)
#include "Rte_Type.h"
#endif

#include "IoHwAb_Cfg.h"
#include "IoHwAb_Types.h"
#include "IoHwAb_Analog.h"
#include "IoHwAb_Digital.h"
#include "IoHwAb_Pwm.h"


#if (IOHWAB_USING_ADC == STD_ON)
#if defined(USE_ADC)
#include "Adc.h"
#else
#error "ADC Module is needed by IOHWAB"
#endif
#endif

#if (IOHWAB_USING_PWM == STD_ON)
#if defined(USE_PWM)
#include "Pwm.h"
#else
#error "PWM Module is needed by IOHWAB"
#endif
#endif

#if (IOHWAB_USING_DIO == STD_ON)
#if defined(USE_DIO)
#include "Dio.h"
#else
#error "DIO Module is needed by IOHWAB"
#endif
#endif


#define IOHWAB_UNLOCKED		0u
#define IOHWAB_LOCKED		1u

/******************************************** API ids *********************************************/

#define IOHWAB_INIT_ID						0x10u

#define IOHWAB_ANALOG_READ_ID				0x20u
#define IOHWAB_ANALOG_IO_CONTROL_ID         0x21u

#define IOHWAB_DIGITAL_READ_ID				0x30u
#define IOHWAB_DIGITAL_WRITE_ID				0x31u
#define IOHWAB_DIGITAL_WRITE_READBACK_ID    0x32u
#define IOHWAB_DIGITAL_IO_CONTROL_ID        0x33u

#define IOHWAB_PWMDUTY_SET_ID				0x40u
#define IOHWAB_PWMFREQUENCYANDDUTY_SET_ID	0x41u
#define IOHWAB_PWM_IO_CONTROL_ID            0x42u

#define IOHWAB_CAPTURE_GET_ID				0x50u

/***************************************** DET error ids ******************************************/

#define IOHWAB_E_INIT				0x01u

#define IOHWAB_E_PARAM_SIGNAL		0x11u
#define IOHWAB_E_PARAM_DUTY			0x12u
#define IOHWAB_E_PARAM_LEVEL        0x13u
#define IOHWAB_E_PARAM_ACTION       0x14u
#define IOHWAB_E_PARAM_PTR          0x15u

/******************************************* DET macros *******************************************/

#if (IOHWAB_DEV_ERROR_DETECT == STD_ON)

#define IOHWAB_DET_REPORT_ERROR(api, error)                   \
  			(void)Det_ReportError(IOHWAB_MODULE_ID, 0, api, error); \


#define IOHWAB_VALIDATE(expression, api, error)      \
			if ( !(expression) ) {                   \
				IOHWAB_DET_REPORT_ERROR(api, error); \
			}                                        \


#define IOHWAB_VALIDATE_RETURN(expression, api, error, rv) \
			if ( !(expression) ) {                         \
				IOHWAB_DET_REPORT_ERROR(api, error);       \
				return rv;                                 \
			}                                              \

#else  /* IOHWAB_DEV_ERROR_DETECT */

#define IOHWAB_DET_REPORT_ERROR(api, error)
#define IOHWAB_VALIDATE(expression, api, error)
#define IOHWAB_VALIDATE_RETURN(expression, api, error, rv)

#endif /* IOHWAB_DEV_ERROR_DETECT */


#define IoHwAb_LockSave(_x) 	Irq_Save(_x)
#define IoHwAb_LockRestore(_x) 	Irq_Restore(_x)

void IoHwAb_Init( void );
void IoHwAb_MainFunction( void );


#if (IOHWAB_USING_ADC == STD_ON)

/* If using ADC, IoHwAb must implement a read function for Adc values */
Adc_ValueGroupType IoHwAb_Adc_ReadSignal( Adc_GroupType group, Adc_ChannelType channel, IoHwAb_StatusType * status );

#endif


#if (IOHWAB_USING_PWM_FREQ == STD_ON)

/* If using PWM with frequency, IoHwAb must implement conversion function from frequency to period */
Pwm_PeriodType IoHwAb_Pwm_ConvertToPeriod(Pwm_ChannelType channel, IoHwAb_FrequencyType  freq);

#endif
#endif /* IOHWAB_H_ */
