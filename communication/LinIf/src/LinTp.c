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

#include "LinTp.h"



/* @req SWS_LinIf_00498 */
#if (LINIF_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

/*lint -e{551} */  /* ( [Temporary] Remove this exception after LinTp_Init() is filled up  */
static LinTp_StatusType LinTpStatus = LINTP_UNINIT;


/*lint -emacro(904,VALIDATE,VALIDATE_W_RV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/

/** @req LINIF269*/ /** @req LINIF271*/
/* Development error macros. */
/* Development error macros. */
#if ( LINIF_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(LINIF_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(LINIF_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }

#define DET_REPORT_ERROR(_api,_err) (void)Det_ReportError(LINIF_MODULE_ID, 0, _api, _err);

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#define DET_REPORT_ERROR(_api,_err)
#endif



/** @req SWS_LinIf_00350 */
void LinTp_Init( const LinTp_ConfigType* ConfigPtr )
{
	/*lint -e715 */ /* ( [Temporary] Remove this exception after LinTp_Init() is filled up  */

	/** @req SWS_LinIf_00483 */
	LinTpStatus = LINTP_INIT;
}


/** @req SWS_LinIf_00351 */
Std_ReturnType LinTp_Transmit(PduIdType LinTxPduId,const PduInfoType* PduInfoPtr)
{
    (void)LinTxPduId;
    //lint -estring(920,pointer)  /* cast to void */
    (void)PduInfoPtr;
    //lint +estring(920,pointer)  /* cast to void */
    /* Sporadic frames not supported in this release */
    /*printf("LinIf_Transmit received request. Id: %d, Data: %d\n", LinTxPduId, *(PduInfoPtr->SduDataPtr));*/
    return E_OK;
}



/* @req SWS_LinIf_00639 */
/* @req SWS_LinIf_00352 */
/** @brief Return the version information of LinTp module.
 *
 * The function LinTp_GetVersionInfo shall return the version
 * information of the LinTp module.
 * The version information includes:
 * - Module Id
 * - Vendor Id
 * - sw_major_version
 * - sw_minor_version
 * - sw_patch_version
 *
 * The function LinTp_GetVersionInfo raises a development error "LINIF_E_PARAM_POINTER."
 * if versioninfo has an invalid value
 *
 *  @param Std_VersionInfoType The type including Module and Vendor ID for the LinTP Module.
 *  @return void.
 */
void LinTp_GetVersionInfo(Std_VersionInfoType* versioninfo) {

	VALIDATE((versioninfo != NULL_PTR),LINTP_GETVERSIONINFO_SERVICE_ID,LINIF_E_PARAM_POINTER);

	versioninfo->vendorID = LINTP_VENDOR_ID;
	versioninfo->moduleID = LINTP_MODULE_ID;
	versioninfo->sw_major_version = LINTP_SW_MAJOR_VERSION;
	versioninfo->sw_minor_version = LINTP_SW_MINOR_VERSION;
	versioninfo->sw_patch_version = LINTP_SW_PATCH_VERSION;

}


/** @req SWS_LinIf_00355 */
void  LinTp_Shutdown(void)
{
	/* @req SWS_LinIf_00484 */
	LinTpStatus = LINTP_UNINIT;
}


/** @req SWS_LinIf_00501 */
Std_ReturnType  LinTp_ChangeParameter(PduIdType id,TPParameterType parameter,uint16 value)
{
	return E_OK;
}


/** @req SWS_LinIf_00625 */
Std_ReturnType  LinTp_CancelReceive(PduIdType RxPduId)
{
	return E_OK;
}

/** @req SWS_LinIf_00500 */
Std_ReturnType  LinTp_CancelTransmit(PduIdType TxPduId)
{
	return E_OK;
}

#ifdef HOST_TEST
LinTp_StatusType readInternal_LinTpStatus(void);

LinTp_StatusType readInternal_LinTpStatus(void){

    return LinTpStatus;

}
#endif
