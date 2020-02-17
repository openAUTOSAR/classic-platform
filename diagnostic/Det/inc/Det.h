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

/** @file Det.h
 *
 *  The Det (Default Error Tracer) Module provides functionality to collect errors
 *  in the system. In order to do so it has a function that can be called by any other
 *  module Det_RerportError. This function can then depending on configuration distribute
 *  the error within the system.
 */

/* @req SWS_Det_00004
 * The Default Error Tracer module’s source code shall offer a headerfileDet.h
 *
 * @req SWS_Det_00037
 * Det.h includes all user relevant information for the tracing of errors reported via its services.
 *
 */
#ifndef DET_H
#define DET_H

/* @req SWS_BSW_00024 Include AUTOSAR Standard Types Header in implementation header */
#include "Std_Types.h"

#define DET_MODULE_ID            15u
#define DET_VENDOR_ID            60u

/* @req SWS_BSW_00059 Published information */
/* Implementation version */
#define DET_SW_MAJOR_VERSION     2u
#define DET_SW_MINOR_VERSION     0u
#define DET_SW_PATCH_VERSION     0u

/* AUTOSAR specification document version */
#define DET_AR_MAJOR_VERSION                4u
#define DET_AR_MINOR_VERSION                3u
#define DET_AR_RELEASE_REVISION_VERSION     0u
#define DET_AR_RELEASE_MAJOR_VERSION	DET_AR_MAJOR_VERSION
#define DET_AR_RELEASE_MINOR_VERSION	DET_AR_MINOR_VERSION
/* @req SWS_BSW_00020 */
#include "Det_Cfg.h"

/* Type needed by config file. */
/* @req SWS_Det_00180 */
typedef void (*detCbk_t)( uint16 ModuleId, uint8 InstanceId , uint8 ApiId, uint8 ErrorId);

/* @req SWS_BSW_00201 Development errors should be of type uint8 */
/* @req SWS_BSW_00073 Implementation specific errors */
// Error codes
#define DET_E_PARAM_POINTER           0x01u
#define DET_E_CBK_REGISTRATION_FAILED 0x02u
#define DET_E_INDEX_OUT_OF_RANGE      0x03u

#define DET_CALLBACK_API              0xFFu

#define DET_CBK_REGISTRATION_FAILED_INDEX	0xFFu

#define DET_INIT_SERVICE_ID 0x00u
#define DET_REPORTERROR_SERVICE_ID 0x01u
#define DET_START_SERVICE_ID 0x02u
#define DET_GETVERSIONINFO_SERVICE_ID 0x03u

typedef uint8 Det_ReturnType;

#define DET_OK 					(Det_ReturnType)0u
#define DET_EMPTY 				(Det_ReturnType)1u
#define DET_ERROR   			(Det_ReturnType)2u

// Type used to store errors
typedef struct
{
  uint16 moduleId;
  uint8 instanceId;
  uint8 apiId;
  uint8 errorId;
} Det_EntryType;

typedef uint8 Det_ConfigType; /** Empty type for supporting new 4.3.0 API */

#if ( DET_ENABLE_CALLBACKS == STD_ON )

/*
 * Add a callback function to the array of callback. After a call to Det_ReportError the callback
 * is called. This can be used in for instance unit tests to verify that correct errors are
 * reported when sending invalid parameters to a function.
 *
 *
 */
/** @brief Add a callback function to the array of callback. After a call to Det_ReportError the callback
 * is called. This can be used in for instance unit tests to verify that correct errors are
 * reported when sending invalid parameters to a function.
 *
 *  @param detCbk The callback that should be used.
 *  @return This function returns the index of the callback in the array when registration is successful. If not DET_CBK_REGISTRATION_FAILED_INDEX is returned.
 */
uint8 Det_AddCbk ( detCbk_t detCbk);

/** @brief Removed a callback from the list of callbacks.
 *
 *  @param detCbkIndex The index of the callback. This index was returned from the Det_AddCbk function.
 *  @return void.
 */
void Det_RemoveCbk ( uint8 detCbkIndex);
#endif

/** @brief Initialize the module
 *   This is kept here for backward compatibility
 *  @return void.
 */
/* @req SWS_Det_00019
 * @req SWS_Det_00008
 * The DET shall provide the initialization function Det_Init
 */
void Det_Init( void /*const Det_ConfigType* ConfigPtr*/ );


/** @brief Deinitialize the module
 *  @return void.
 */
#if (DET_DEINIT_API == STD_ON)
void Det_DeInit( void );
#endif

/** @brief Start the module
 *  @return void.
 */
/* @req SWS_Det_00025
 * The Default Error Tracer shall provide the function Det_Start
 *
 * @req SWS_Det_00010
 */
void Det_Start( void );

/** @brief This service should be used by other modules or components in the software
 * to report errors.
 *  @param ModuleId The module reporting the error. Modeule IDs are defined in the module specific header files.
 *  @param InstanceId If there are more that one instance of a modeule it can be reported here.
 *  @param ApiId The API that reports the error (Module specific)
 *  @param ErrorId The error that is reporter (API specific)
 *  @return E_OK if everything was OK otherwise an internal error is reported.
 */
/* @req SWS_Det_00009 */
Std_ReturnType Det_ReportError( uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

/** @brief Return the version information of Det module.
 *
 * The function Det_GetVersionInfo shall return the version
 * information of the Det module.
 * The version information includes:
 * - Module Id
 * - Vendor Id
 * - sw_major_version
 * - sw_minor_version
 * - sw_patch_version
 *
 *  @param Std_VersionInfoType The type including Module and Vendor ID for the Det Module.
 *  @return void.
 */
/* @req SWS_BSW_00051 */
/* @req SWS_Det_00011 */
#if (DET_VERSIONINFO_API == STD_ON)
void Det_GetVersionInfo(Std_VersionInfoType* vi);
#endif

/** @brief This service is used by the Safety Monitor to read out the latest error reporter to Det
 *  @param entry Pointer to an entry that will be filled with information in the service.
 *  @return status of the call:
 *        DET_OK - The entry value will include latest error
 *        DET_EMPTY - There was no new errors reported
 *        DET_ERROR - There was an internal error inside of Det
 */
#if (DET_SAFETYMONITOR_API == STD_ON)
Det_ReturnType Det_GetNextError(Det_EntryType* entry);
#endif

#endif /*DET_H*/
