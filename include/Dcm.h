/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/








#ifndef DCM_H_
#define DCM_H_

#define DCM_MODULE_ID			MODULE_ID_DCM /** @req DCM052 */
#define DCM_VENDOR_ID			1

#define DCM_SW_MAJOR_VERSION    1
#define DCM_SW_MINOR_VERSION   	0
#define DCM_SW_PATCH_VERSION    0
#define DCM_AR_MAJOR_VERSION    3
#define DCM_AR_MINOR_VERSION    0
#define DCM_AR_PATCH_VERSION    1

#include "Dcm_Types.h"
#include "Dcm_Cfg.h"
#include "Dcm_Lcfg.h"
#include "ComStack_Types.h"
#include "Dcm_Cbk.h"

#if (DCM_DEV_ERROR_DETECT == STD_ON)
// Error codes produced by this module
#define DCM_E_CONFIG_INVALID				0x41
#define DCM_E_UNEXPECTED_PARAM				0x42
#define DCM_E_NOT_SUPPORTED					0xfe
#define DCM_E_NOT_IMPLEMENTED_YET			0xff

// Service ID in this module
#define DCM_INIT_ID							0x01
#define DCM_HANDLE_RESPONSE_TRANSMISSION	0x80
#define DCM_UDS_READ_DTC_INFO				0x81
#define DCM_UDS_RESET						0x82
#define DCM_GLOBAL_ID						0xff

#endif

/*
 * Interfaces for BSW components (8.3.1)
 */
#if ( DCM_VERSION_INFO_API == STD_ON ) /** @req DCM337 */
#define Dcm_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,DCM) /** @req DCM065 */ /** @req DCM335 */ /** @req DCM336 */
#endif /* DCM_VERSION_INFO_API */

void Dcm_Init( void ); /** @req DCM037 */


/*
 * Interfaces for BSW modules and to SW-Cs (8.3.2)
 */
Std_ReturnType Dcm_GetSecurityLevel(Dcm_SecLevelType *secLevel); /** @req DCM338 */
Std_ReturnType Dcm_GetSesCtrlType(Dcm_SesCtrlType *sesCtrlType); /** @req DCM339 */
Std_ReturnType Dcm_GetActiveProtocol(Dcm_ProtocolType *activeProtocol); /** @req DCM340 */

/*
 * Interface for basic software scheduler (8.5)
 */
void Dcm_MainFunction( void ); /** @req DCM053 */


#endif /*DCM_H_*/
