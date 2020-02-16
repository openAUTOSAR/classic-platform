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

/*
 * Development Error Tracer driver
 *
 * Specification: Autosar v3.2.0, Final
 *
 */

#include "Std_Types.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Cpu.h"
#include "MemMap.h" /* @req 4.0.3/DET006 */

#define DEBUG_LVL 1
#include "debug.h"

/* @req 4.0.3/DET036 */
/* ----------------------------[Version check]------------------------------*/
#if !(((DET_SW_MAJOR_VERSION == 1) && (DET_SW_MINOR_VERSION == 0)) )
#error Det: Expected BSW module version to be 1.0.*
#endif

#if !(((DET_AR_MAJOR_VERSION == 4) && (DET_AR_MINOR_VERSION == 0)) )
#error Det: Expected AUTOSAR version to be 4.0.*
#endif

/* ----------------------------[private define]------------------------------*/
typedef enum
{
    DET_UNINITIALIZED = 0,
    DET_INITIALIZED,
    DET_STARTED
} Det_StateType;

static Det_StateType detState = DET_UNINITIALIZED;

#if ( DET_USE_RAMLOG == STD_ON )
// Ram log variables in uninitialized memory
SECTION_RAMLOG uint32 Det_RamlogIndex;
/*lint -esym(552,Det_RamLog)*/ /* PC-Lint OK. supress lintwarning about Det_Ramlog not being accessed */
SECTION_RAMLOG Det_EntryType Det_RamLog[DET_RAMLOG_SIZE] ;
#endif

#if ( DET_ENABLE_CALLBACKS == STD_ON )
detCbk_t detCbk_List[DET_NUMBER_OF_CALLBACKS];

uint8 Det_AddCbk(detCbk_t detCbk)
{
	uint8 rv = DET_CBK_REGISTRATION_FAILED_INDEX;	 // Return DET_CBK_REGISTRATION_FAILED_INDEX if the registration fails

    if (detState != DET_UNINITIALIZED)
    {
        for (uint8 i = 0; i < DET_NUMBER_OF_CALLBACKS; i++)
        {
            if (NULL == detCbk_List[i])
            {
                detCbk_List[i] = detCbk;
                rv = i;
                break;
            }
        }
    }

    if (rv == DET_CBK_REGISTRATION_FAILED_INDEX)
    {
        Det_ReportError(DET_MODULE_ID, 0, DET_CALLBACK_API, DET_E_CBK_REGISTRATION_FAILED);
    }

    return rv;
}


void Det_RemoveCbk(uint8 detCbkIndex)
{
    // Validate the index
    if (detCbkIndex >= DET_NUMBER_OF_CALLBACKS)
    {
        Det_ReportError(DET_MODULE_ID, 0, DET_CALLBACK_API, DET_E_INDEX_OUT_OF_RANGE);
    }
    else
    {
    	detCbk_List[detCbkIndex]=NULL;
    }
}
#endif

/* @req 4.0.3/DET019 */
/* @req 4.0.3/DET020 */
void Det_Init(void)
{

#if ( DET_ENABLE_CALLBACKS == STD_ON )
    for (uint32 i=0; i<DET_NUMBER_OF_CALLBACKS; i++)
    {
        detCbk_List[i]=NULL;
    }
#endif

#if ( DET_USE_RAMLOG == STD_ON )
    for(uint32 i=0; i < DET_RAMLOG_SIZE; i++)
    {
        Det_RamLog[i].moduleId = 0;
        Det_RamLog[i].instanceId = 0;
        Det_RamLog[i].apiId = 0;
        Det_RamLog[i].errorId = 0;
    }
    Det_RamlogIndex = 0;
#endif

    detState = DET_INITIALIZED;
}

#if DET_DEINIT_API == STD_ON
void Det_DeInit( void )
{
    detState = DET_UNINITIALIZED;
}
#endif

/* @req 4.0.3/DET207 */
/* @req 4.0.3/DET027 */
/* @req 4.0.3/DET013 */
/* @req 4.0.3/DET039 */
/* @req 4.0.3/DET026 */
/* @req 4.0.3/DET050 */
void Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
	/* @req 4.0.3/DET024 */
    if (detState == DET_STARTED) // No action is taken if the module is not started
    {

    	// Call static error hooks here
    	/* @req 4.0.3/DET035 */
    	/* @req 4.0.3/DET014 */
#if ( DET_USE_STATIC_CALLBACKS == STD_ON )
        uint32 old1; // 586 PC-Lint OK: fattar inte att den anv�nds i macrot.
        Irq_Save(old1);

        /* @req 4.0.3/DET018 */
        /* @req 4.0.3/DET017 */
        for (uint32 i=0; i<DET_NUMBER_OF_STATIC_CALLBACKS; i++)
        {
        	(*DetStaticHooks[i])(ModuleId, InstanceId, ApiId, ErrorId);
        }
        Irq_Restore(old1);
#endif

        /* @req 4.0.3/DET015 */
#if ( DET_ENABLE_CALLBACKS == STD_ON )
        uint32 old2; // 586 PC-Lint OK: fattar inte att den anv�nds i macrot.
        Irq_Save(old2);
        for (uint32 i=0; i<DET_NUMBER_OF_CALLBACKS; i++)
        {
            if (NULL!=detCbk_List[i])
            {
                (void)(*detCbk_List[i])(ModuleId, InstanceId, ApiId, ErrorId); /* Ignoring return value */
            }
        }
        Irq_Restore(old2);
#endif


#if ( DET_USE_RAMLOG == STD_ON )
        uint32 old3;
        Irq_Save(old3);
        if (Det_RamlogIndex < DET_RAMLOG_SIZE)
        {
            Det_RamLog[Det_RamlogIndex].moduleId = ModuleId;
            Det_RamLog[Det_RamlogIndex].instanceId = InstanceId;
            Det_RamLog[Det_RamlogIndex].apiId = ApiId;
            Det_RamLog[Det_RamlogIndex].errorId = ErrorId;
            Det_RamlogIndex++;
#if ( DET_WRAP_RAMLOG == STD_ON )
            if (Det_RamlogIndex == DET_RAMLOG_SIZE){
                Det_RamlogIndex = 0;
            }
#endif
        }
        Irq_Restore(old3);
#endif

#if ( DET_USE_STDERR == STD_ON )
        printf("Det Error: ModuleId=%d, InstanceId=%d, ApiId=%d, ErrorId=%d\n", ModuleId, InstanceId, ApiId, ErrorId);        //fprintf(stderr, "Det Error: ModuleId=%d, InstanceId=%d, ApiId=%d, ErrorId=%d\n", ModuleId, InstanceId, ApiId, ErrorId);
#endif
    }
}

/* @req 4.0.3/DET025 */
void Det_Start(void)
{
    detState = DET_STARTED;
}

/* @req 4.0.3/DET028 */
#if (DET_VERSIONINFO_API == STD_ON)
void Det_GetVersionInfo(Std_VersionInfoType* vi)
{
	/* @req 4.0.3/DET051 */
	if(vi != NULL) {
		vi->vendorID = DET_VENDOR_ID;
		vi->moduleID = DET_MODULE_ID;
		vi->sw_major_version = DET_SW_MAJOR_VERSION;
		vi->sw_minor_version = DET_SW_MINOR_VERSION;
		vi->sw_patch_version = DET_SW_PATCH_VERSION;
		vi->ar_major_version = DET_AR_MAJOR_VERSION;
		vi->ar_minor_version = DET_AR_MINOR_VERSION;
		vi->ar_patch_version = DET_AR_RELEASE_REVISION_VERSION;
	} else {
		/* @req 4.0.3/DET023 */
		/* @req 4.0.3/DET052 */
		Det_ReportError(DET_MODULE_ID, 0, DET_GETVERSIONINFO_SERVICE_ID, DET_E_PARAM_POINTER);
	}
}
#endif
