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

#ifndef SAFEIOHWAB_H_
#define SAFEIOHWAB_H_

/***************************************** DET error ids ******************************************/

/* @req SWS_BSW_00201 Development errors should be of type uint8 */
/* @req SWS_BSW_00073 Implementation specific errors */
#define SAFEIOHWAB_E_INIT               0x01u

#define SAFEIOHWAB_E_PARAM_SIGNAL       0x11u
#define SAFEIOHWAB_E_PARAM_DUTY         0x12u
#define SAFEIOHWAB_E_PARAM_LEVEL        0x13u
#define SAFEIOHWAB_E_PARAM_ACTION       0x14u
#define SAFEIOHWAB_E_PARAM_PTR          0x15u

/******************************************** API ids *********************************************/

#define SAFEIOHWAB_INIT_ID                          0x10u
#define SAFEIOHWAB_GETVERSIONINFO_ID            	0x11u

#define SAFEIOHWAB_ANALOG_READ_ID                   0x20u
#define SAFEIOHWAB_ANALOG_NEW_VALUE_ID              0x21u

#define SAFEIOHWAB_DIGITAL_READ_ID                  0x30u
#define SAFEIOHWAB_DIGITAL_WRITE_ID                 0x31u
#define SAFEIOHWAB_DIGITAL_WRITE_READBACK_ID        0x32u

#define SAFEIOHWAB_PWMDUTY_SET_ID                   0x40u
#define SAFEIOHWAB_PWMFREQUENCYANDDUTY_SET_ID       0x41u

#define SAFEIOHWAB_CAPTURE_GET_ID                   0x50u

/**************************************************************************************************/

/* @req SWS_BSW_00059 Published information */
#define SAFEIOHWAB_SW_MAJOR_VERSION     1u
#define SAFEIOHWAB_SW_MINOR_VERSION     1u
#define SAFEIOHWAB_SW_PATCH_VERSION     0u

#define SAFEIOHWAB_AR_MAJOR_VERSION                4u
#define SAFEIOHWAB_AR_MINOR_VERSION                3u
#define SAFEIOHWAB_AR_RELEASE_REVISION_VERSION     0u

/* @req SWS_BSW_00059 Published information */
#define SAFEIOHWAB_MODULE_ID            254u
#define SAFEIOHWAB_VENDOR_ID            60u
#define SAFEIOHWAB_INSTANCE_ID          0x01u

#include "SafeIoHwAb_Cfg.h"

#if (SAFEIOHWAB_USING_ADC == STD_ON)
/* @req ARC_SWS_SafeIoHwAb_00003 SafeIoHwAb_Analog_Read prototype*/
/** 
 * @brief Function which reads a specific analog signal, returns value and status
 * @param[in]   signal Which signal that shall be read
 * @param[out]  value the value read
 * @param[out]  status the quality the signal
 * @return 		Std_ReturnType if input arguments has been valid
 */
Std_ReturnType SafeIoHwAb_Analog_Read(SafeIoHwAb_SignalType signal, SafeIoHwAb_AnalogValueType *value, SafeIoHwAb_StatusType *status);

/* @req ARC_SWS_SafeIoHwAb_00018 SafeIoHwAb_Analog_NewValue prototype*/
/** 
 * @brief		Function which stores the vlue for a specific analog signal for later readout
 * @param[in]  	signal Which signal that shall be stored
 * @param[in]  	value the value that shall be stored
 * @return 		Std_ReturnType if input arguments has been valid
 */
Std_ReturnType SafeIoHwAb_Analog_NewValue(SafeIoHwAb_SignalType signal, SafeIoHwAb_AnalogValueType value);

#endif

#if (SAFEIOHWAB_USING_PWM == STD_ON)
/* @req ARC_SWS_SafeIoHwAb_00013 SafeIoHwAb_Pwm_Set_Duty prototype*/
/** 
 * @brief		Function which first sets a specific pwm signal
 * @param[in]   signal Which signal that shall be set
 * @param[in]   duty the duty cycle which shall be set
 * @param[out]  status The status of the signal
 * @return 		Std_ReturnType if input arguments has been valid
 */
Std_ReturnType SafeIoHwAb_Pwm_Set_Duty(SafeIoHwAb_SignalType signal, SafeIoHwAb_DutyType duty, SafeIoHwAb_StatusType *status);
/* @req ARC_SWS_SafeIoHwAb_00014 SafeIoHwAb_Pwm_Set_DutyFreq prototype*/
/** 
 * @brief		Function which first sets a specific pwm signal
 * @param[in]   signal Which signal that shall be set
 * @param[in]   duty the duty cycle which shall be set
 * @param[in]   frequency the frequency which shall be set
 * @param[out]  status The status of the signal
 * @return 		Std_ReturnType if input arguments has been valid
 */
Std_ReturnType SafeIoHwAb_Pwm_Set_DutyFreq(SafeIoHwAb_SignalType signal, SafeIoHwAb_DutyType duty, SafeIoHwAb_FrequencyType frequency, SafeIoHwAb_StatusType *status);

#if (PWM_SET_PERIOD_AND_DUTY_API == STD_ON)
/* If using PWM with frequency, SafeIoHwAb must implement conversion function from frequency to period, this is the integrators responsibility */
Pwm_PeriodType SafeIoHwAb_Pwm_ConvertToPeriod(SafeIoHwAb_SignalType channel, SafeIoHwAb_FrequencyType freq);
#endif
#endif

#if (SAFEIOHWAB_USING_DIO == STD_ON)
/* @req ARC_SWS_SafeIoHwAb_00007 SafeIoHwAb_Digital_Read prototype*/
/** 
 * @brief		Function which reads a specific digital signal
 * @param[in]   signal Which signal that shall be read
 * @param[out]  value The read value
 * @param[out]  status The status of the read signal
 * @return 		Std_ReturnType if input arguments has been valid
 */
Std_ReturnType SafeIoHwAb_Digital_Read(SafeIoHwAb_SignalType signal, SafeIoHwAb_LevelType *value, SafeIoHwAb_StatusType *status);
/* @req ARC_SWS_SafeIoHwAb_00008 SafeIoHwAb_Digital_Write prototype*/
/** 
 * @brief		Function which sets a specific digital signal
 * @param[in]   signal Which signal that shall be set
 * @param[in]   newValue the value which shall be set
 * @return 		Std_ReturnType if input arguments has been valid
 */
Std_ReturnType SafeIoHwAb_Digital_Write(SafeIoHwAb_SignalType signal, SafeIoHwAb_LevelType newValue);
/* @req ARC_SWS_SafeIoHwAb_00009 SafeIoHwAb_Digital_WriteReadback prototype*/
/** 
 * @brief		Function which first sets a specific digital signal and then reads is to verify that it has been set correctly
 * @param[in]   signal Which signal that shall be set and read
 * @param[in]   newValue the value which shall be set
 * @param[out]  status The status of the read signal
 * @return 		Std_ReturnType if input arguments has been valid
 */
Std_ReturnType SafeIoHwAb_Digital_WriteReadback(SafeIoHwAb_SignalType signal, SafeIoHwAb_LevelType newValue, SafeIoHwAb_StatusType *status);
#endif


/**
 * @brief Main function which might be optional
 */
void SafeIoHwAb_MainFunction ( void );

#if ( SAFEIOHWAB_VERSION_INFO_API == STD_ON )
/** 
 * @brief Function which return the version info of SafeIoHwAb
 * @param[out]  VersionInfo Container which contains the version info
 */
void SafeIoHwAb_GetVersionInfo( Std_VersionInfoType* VersionInfo );
#endif

#endif /* SAFEIOHWAB_H_ */
