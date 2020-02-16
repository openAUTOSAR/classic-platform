#ifndef WDGM_H
#define WDGM_H

#include "Std_Types.h"

#define WDGM_AR_RELEASE_MAJOR_VERSION   	4
#define WDGM_AR_RELEASE_MINOR_VERSION   	0
#define WDGM_AR_RELEASE_REVISION_VERSION    3

#define WDGM_VENDOR_ID          60
#define WDGM_MODULE_ID          MODULE_ID_WDGM
#define WDGM_AR_MAJOR_VERSION   WDGM_AR_RELEASE_MAJOR_VERSION
#define WDGM_AR_MINOR_VERSION   WDGM_AR_RELEASE_MINOR_VERSION
#define WDGM_AR_PATCH_VERSION   WDGM_AR_RELEASE_REVISION_VERSION

#define WDGM_SW_MAJOR_VERSION   1
#define WDGM_SW_MINOR_VERSION   0
#define WDGM_SW_PATCH_VERSION   0


/* globally fulfilled requirements */
/** @req WDGM038 */
/** @req WDGM042 */
/** @req WDGM047 */
/** @req WDGM260 */
/** @req WDGM015 */
/** @req WDGM048 */
/** @req WDGM200 */
/** @req WDGM085 */
/** @req WDGM293 */
/** @req WDGM267 */
/** @req WDGM345 */
/** @req WDGM127 */
/** @req WDGM014.partially */

#define WDGM_SID_INIT					0x00
#define WDGM_SID_DEINIT					0x01
#define WDGM_SID_GETVERSIONINFO 		0x02
#define WDGM_SID_SETMODE				0x03
#define WDGM_SID_GETMODE				0x0B
#define WDGM_SID_CHECKPOINTREACHED 		0x0E
#define WDGM_SID_UPDATEALIVECOUNTER		0x04
#define WDGM_SID_GETLOCALSTATUS			0x0C
#define WDGM_SID_GETGLOBALSTATUS		0x0D
#define WDGM_SID_PERFORMRESET			0x0F
#define WDGM_SID_GETFIRSTEXPIREDSEID	0x10
#define WDGM_SID_MAINFUNCTION			0x08

/** @req WDGM259 */
/** @req WDGM004 */
#define WDGM_E_NO_INIT					0x10
#define WDGM_E_PARAM_CONFIG				0x11
#define WDGM_E_PARAM_MODE				0x12
#define WDGM_E_PARAM_SEID				0x13
#define WDGM_E_INV_POINTER				0x14
#define WDGM_E_DISABLE_NOT_ALLOWED		0x15
#define WDGM_E_CPID						0x16
#define WDGM_E_DEPRECATED				0x17
#define WDGM_E_SEDEACTIVATED			0x19
#define WDGM_E_AMBIGIOUS				0x18

/* the following datatypes are defined globally in order to allow debugging as mentioned in the spec */
#include "WdgM_ConfigTypes.h"
#include "WdgM_internalTypes.h"
/* since all is public every debugging-req is fulfilled */
/** @req WDGM234 */
/** @req WDGM235 */
/** @req WDGM236 */
/** @req WDGM237 */
/** @req WDGM002 */
/** @req WDGM011 */
/** @req WDGM038 */
/** @req WDGM042 */
/** @req WDGM161 */
/** @req WDGM162 */

typedef struct
{
	const WdgM_ConfigType		*ConfigPtr; 		/** @req WDGM238 */ /** @req WDGM240 */ /** @req WDGM241 */ /** @req WDGM242 */
	WdgM_internal_RuntimeData	*internalRuntimeDataPtr;
	boolean						isInitiated;
	WdgM_GlobalStatusType		GlobalState;					/** @req WDGM239 */ /** @req WDGM213 */
	uint32						CurrentTime;
	const WdgM_Mode				*CurrentMode;
	boolean						ResetInitiated;
	uint16						firstExpiredSEID;
	boolean						isFirstExpiredSet;
} WdgM_debuggable_internalData;

extern WdgM_debuggable_internalData	WdgM_instance;

/** @req WDGM151 */
void WdgM_Init(const WdgM_ConfigType *ConfigPtr);

/** @req WDGM261 */
void WdgM_DeInit( void );

/** @req WDGM153 */
/** @req WDGM262 */
/** @req WDGM110 */
#if ( WDGM_VERSION_INFO_API == STD_ON )
#define WdgM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,WDGM)
#endif


/** @req WDGM154 */
Std_ReturnType WdgM_SetMode( WdgM_ModeType Mode, uint16 CallerID);

/** @req WDGM168 */
Std_ReturnType WdgM_GetMode( WdgM_ModeType *Mode);

/** @req WDGM263 */
Std_ReturnType WdgM_CheckpointReached( WdgM_SupervisedEntityIdType SEID, WdgM_CheckpointIdType CheckpointID);

/** @req WDGM155 */
Std_ReturnType WdgM_UpdateAliveCounter( WdgM_SupervisedEntityIdType SEID);

/** @req WDGM169 */
Std_ReturnType WdgM_GetLocalStatus( WdgM_SupervisedEntityIdType SEID, WdgM_LocalStatusType *Status);

/** @req WDGM175 */
Std_ReturnType WdgM_GetGlobalStatus( WdgM_GlobalStatusType *Status);

/** @req WDGM264 */
Std_ReturnType WdgM_PerformReset( void );

/** @req WDGM346 */
Std_ReturnType WdgM_GetFirstExpiredSEID( WdgM_SupervisedEntityIdType *SEID);

/** @req WDGM159 */
void WdgM_MainFunction( void );

#endif
