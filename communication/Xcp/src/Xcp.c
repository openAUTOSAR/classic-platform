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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.3 */

/*General requirement*/
/** @req 4.1.3/SWS_Xcp_00501 *//*It is about the code file structure of Xcp - can not be tested with conventional module tests*/
/** @req 4.1.3/SWS_Xcp_00500 *//*XCP shall access the location of the API by proper header inclusion - can not be tested with conventional module tests */
/** @req 4.1.3/SWS_Xcp_00502 *//*Header file structure - can not be tested with conventional module tests*/
/** @req 4.1.3/SWS_Xcp_00701 *//*Location of Xcp module in communication stack - can not be tested with conventional module tests*/
/** @req 4.1.3/SWS_Xcp_00702 *//*Unique PDU Id must be used for communication - can not be tested with conventional module tests*/
/** @req 4.1.3/SWS_Xcp_00703 *//*The AUTOSAR XCP Module shall support the ASAM XCP Specification Version 1.1.*/
/** @req 4.1.3/SWS_Xcp_00712 *//*For sending and receiving of calibration data, the sending and receiving APIs specified within the AUTOSAR BSW Bus Interfaces shall be used*/
/** @req 4.1.3/SWS_Xcp_00764 *//*The internal XCP states shall be available for debugging - can not be tested with conventional module tests */
/** @req 4.1.3/SWS_Xcp_00741 *//*Link-time and post-build-time configuration - can not be tested with conventional module tests*/
/** @req 4.1.3/SWS_Xcp_00742 *//*The XCP module shall support pre-compile time, link-time and post-build-time configuration - can not be tested with conventional module tests*/
/** @req 4.1.3/SWS_Xcp_00763 *//*Development errors*/
/** @req 4.1.3/SWS_Xcp_00801 *//*Imported type definitions - can not be tested with conventional module tests*/
/** @req 4.1.3/SWS_Xcp_00832 *//*Optional interface - can not be tested with conventional module tests*/


#include "Std_Types.h"
#include "Xcp_Internal.h"

static Xcp_BufferType Xcp_RxCtoBuffers[1];
static Xcp_BufferType Xcp_RxDtoBuffers[1]; // size should depend on STIM config, ignores it for now
static Xcp_BufferType Xcp_TxCtoBuffers[1];
static Xcp_FifoType   Xcp_RxCtoFifoFree;
static Xcp_FifoType   Xcp_RxDtoFifoFree;
static Xcp_FifoType   Xcp_TxCtoFifoFree;
static Xcp_FifoType   Xcp_FifoRxCto = { .free = &Xcp_RxCtoFifoFree };
       Xcp_FifoType   Xcp_FifoTxCto = { .free = &Xcp_TxCtoFifoFree };
static Xcp_DaqListType *Xcp_DaqTxList = 0;
static Xcp_DaqListType *Xcp_DaqTxListLast = 0;
static Xcp_DaqListType **XcpEventChannelTriggeredDaqListRef[XCP_EVENT_COUNT];
       uint8          Xcp_Inited;
       uint8          Xcp_Connected;
       Xcp_MtaType    Xcp_Mta;

const  Xcp_ConfigType* xcpPtr;

static uint8                 Xcp_TxOngoing = 0;
static Xcp_TransferType      Xcp_Download;
static Xcp_DaqPtrStateType   Xcp_DaqState;
static Xcp_TransferType      Xcp_Upload;
static Xcp_CmdWorkType       Xcp_Worker;

#if(XCP_FEATURE_PROTECTION == STD_ON)
static Xcp_UnlockType Xcp_Unlock;
#endif /*XCP_FEATURE_PROTECTION*/



#if (XCP_VERSION_INFO_API == STD_ON)
/**
 * Returns the version information of this module.
 *
 * ServiceId: 0x01
 *
 * @param versioninfo
 */
/** @req 4.1.3/SWS_Xcp_00807 *//*Xcp_GetVersionInfo API definition*/
/** @req 4.1.3/SWS_Xcp_00825 *//*Check whether the parameter VersioninfoPtr is a NULL pointer*/
void Xcp_GetVersionInfo(Std_VersionInfoType* versionInfo)
{
    DET_VALIDATE_NRV(versionInfo, XCP_API_ID_XCP_GETVERSIONINFO, XCP_E_INV_POINTER);

    versionInfo->moduleID = (uint16)XCP_MODULE_ID;
    versionInfo->vendorID = (uint16)XCP_VENDOR_ID;
    versionInfo->sw_major_version = (uint8)XCP_SW_MAJOR_VERSION;
    versionInfo->sw_minor_version = (uint8)XCP_SW_MINOR_VERSION;
    versionInfo->sw_patch_version = (uint8)XCP_SW_PATCH_VERSION;
}
#endif /*XCP_VERSION_INFO_API == STD_ON*/

/**
 * Initializing function
 *
 * ServiceId: 0x00
 *
 * @param Xcp_ConfigPtr
 */
/** @req 4.1.3/SWS_Xcp_00803 *//*Xcp_Init API definition*/
/** @req 4.1.3/SWS_Xcp_00802 *//*The function Xcp_Init shall internally store the configuration address to enable subsequent API calls to access the configuration.*/
/** @req 4.1.3/SWS_Xcp_00834 *//*Xcp_Init shall check the parameter Xcp_ConfigType for not being a NULL pointer*/
void Xcp_Init(const Xcp_ConfigType* Xcp_ConfigPtr)
{
    DET_VALIDATE_NRV(Xcp_ConfigPtr, XCP_API_ID_XCP_INIT, XCP_E_INV_POINTER);

    /** Save the pointer. */
    xcpPtr = Xcp_ConfigPtr;

    memset(xcpPtr->rt, 0, sizeof(*xcpPtr->rt));

    Xcp_Fifo_Init(&Xcp_RxCtoFifoFree, Xcp_RxCtoBuffers, Xcp_RxCtoBuffers + sizeof(Xcp_RxCtoBuffers) / sizeof(Xcp_RxCtoBuffers[0]));
    Xcp_Fifo_Init(&Xcp_RxDtoFifoFree, Xcp_RxDtoBuffers, Xcp_RxDtoBuffers + sizeof(Xcp_RxDtoBuffers) / sizeof(Xcp_RxDtoBuffers[0]));
    Xcp_Fifo_Init(&Xcp_TxCtoFifoFree, Xcp_TxCtoBuffers, Xcp_TxCtoBuffers + sizeof(Xcp_TxCtoBuffers) / sizeof(Xcp_TxCtoBuffers[0]));

    // init the preconfigured and the static daqs
    for (uint16 daqNr = 0; daqNr < xcpPtr->XcpDaqListSize; daqNr++) {
        Xcp_DaqListType* daq = xcpPtr->XcpDaqList + daqNr;
        Xcp_DaqListCfgType* daqCfg = xcpPtr->XcpDaqListCfg + daqNr;
        memset(daq, 0, sizeof(*daq));
        daq->EventChannel = daqCfg->FixedEventChannel;
        daq->XcpOdt = daqCfg->XcpOdt;
        daq->XcpMaxOdt = daqCfg->XcpMaxOdt;
    }
    /* Initialize XcpMaxDaq which means the currently available DAQ number*/
    xcpPtr->rt->XcpMaxDaq = xcpPtr->XcpDaqListSize;

    //initialize daq - event runtime assignment table
    for (uint16 eventNr = 0; eventNr < XCP_EVENT_COUNT; eventNr++) {
        XcpEventChannelTriggeredDaqListRef[eventNr] = 0;
    }

    //create chain for the fixed events
    for (uint16 eventNr = 0; eventNr < XCP_EVENT_COUNT; eventNr++) {

        for (uint16 daqNr = 0; daqNr < xcpPtr->XcpDaqListSize; daqNr++) {
            Xcp_DaqListType* daq = xcpPtr->XcpDaqList + daqNr;

            if (daq->EventChannel == eventNr) {
                if(XcpEventChannelTriggeredDaqListRef[eventNr] == 0) {
                    XcpEventChannelTriggeredDaqListRef[eventNr] = &daq->EventDaqListNext;
                    daq->EventDaqListNext = daq;
                } else {
                    daq->EventDaqListNext = *XcpEventChannelTriggeredDaqListRef[eventNr];
                    *XcpEventChannelTriggeredDaqListRef[eventNr] = daq;
                    XcpEventChannelTriggeredDaqListRef[eventNr] = &daq->EventDaqListNext;
                }
            }
        }
    }

    Xcp_TxOngoing = 0;

    //Initialize Communication specific channel
#if (XCP_PROTOCOL == XCP_PROTOCOL_CAN)
    Xcp_CanLayerInit();
#elif (XCP_PROTOCOL == XCP_PROTOCOL_ETHERNET)
    Xcp_SoAdLayerInit();
#endif

    Xcp_Inited = 1;
}

/**
 * Function called from lower layers (CAN/Ethernet..) containing
 * a received XCP packet.
 *
 * Can be called in interrupt context.
 *
 * @param data
 * @param len
 */
void Xcp_RxIndication(const uint8* data, PduLengthType len) {
    if (len > XCP_MAX_DTO) {
        DEBUG(DEBUG_HIGH, "Xcp_RxIndication - length %d too long\n", len);
        return;
    }

    if (len == 0) {
        return;
    }

    uint8 pid = GET_UINT8(data, 0);

    if (pid > XCP_PID_CMD_STIM_LAST) {
        /* process xcp commands */
        FIFO_GET_WRITE(Xcp_FifoRxCto, it) {
            Xcp_MemCpy(it->data, data, len);
            it->len = len;
        }
    }
}


/**
 * Xcp_TxError sends an error message back to master
 * @param code is the error code requested
 */
void Xcp_TxError(Xcp_ErrorType code) {
	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{
		SET_UINT8(e->data, 0, (uint8)XCP_PID_ERR);
		SET_UINT8(e->data, 1, (uint8)code);
		e->len = 2;
	}
}

/**
 * Xcp_TxSuccess sends a basic RES response without
 * extra data to master
 */
void Xcp_TxSuccess(void) {
	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{
		SET_UINT8(e->data, 0, XCP_PID_RES);
		e->len = 1;
	}
}

/** @req 4.1.3/SWS_Xcp_00709 *//*The AUTOSAR XCP Module shall support Timestamped Data Transfer (read / write access).*/
/** @req 4.1.3/SWS_Xcp_00768 *//*The ECU local time shall be derived from the AUTOSAR OS*/
static uint32 Xcp_GetTimeStamp(void) {
#if(XCP_TIMESTAMP_SIZE)
    TickType counter;

    if(GetCounterValue(XCP_COUNTER_ID, &counter)) {
        counter = 0;
    }

#if(XCP_TIMESTAMP_SIZE == 1)
    return counter % 256;
#elif(XCP_TIMESTAMP_SIZE == 2)
    return counter % (256*256);
#else /*XCP_TIMESTAMP_SIZE == 4*/
    return counter;
#endif

#else /*XCP_TIMESTAMP_SIZE == 0*/
    return 0;
#endif
}

/** @req 4.1.3/SWS_Xcp_00705 *//*The AUTOSAR XCP Module shall support Synchronous data acquisition (measurement)*/
static void Xcp_ProcessDaq_DAQ(Xcp_DaqListType* daq) {
    static Xcp_BufferType dto;
    dto.len = 0;
    Xcp_OdtType* odt = daq->NextOdtToSend;
#if   (XCP_IDENTIFICATION == XCP_IDENTIFICATION_ABSOLUTE)
	FIFO_ADD_U8(&dto, (odt - daq->XcpOdt + daq->FirstPid));
#else
    FIFO_ADD_U8(&dto, odt - daq->XcpOdt);
#endif

#if   (XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_WORD)
            FIFO_ADD_U16(&dto, daq->XcpDaqListNumber);
#elif (XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_WORD_ALIGNED)
            FIFO_ADD_U8 (&dto, 0); /* RESERVED */
            FIFO_ADD_U16(&dto, daq - xcpPtr->XcpDaqList);
#elif (XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_BYTE)
            FIFO_ADD_U8(&dto, daq - xcpPtr->XcpDaqList);
#endif
#if XCP_TIMESTAMP_SIZE != 0
    if(odt == daq->XcpOdt) {
        // only transmit timestamp on firt odt within daq
        uint8 ts = daq->Mode & XCP_DAQLIST_MODE_TIMESTAMP;
        if (ts) {
#if   (XCP_TIMESTAMP_SIZE == 1)
            FIFO_ADD_U8 (&dto, Xcp_GetTimeStamp());
#elif (XCP_TIMESTAMP_SIZE == 2)
            FIFO_ADD_U16(&dto, Xcp_GetTimeStamp());
#elif (XCP_TIMESTAMP_SIZE == 4)
            FIFO_ADD_U32(&dto, Xcp_GetTimeStamp());
#endif
        }
    }
#endif
    Xcp_OdtEntryType* ent = odt->XcpOdtEntry;
    Xcp_Fifo_Lock(); // all entries within an odt shall be consistent
	for (uint8 i = 0; i < odt->XcpOdtEntriesCount; i++) {
		uint8 len = ent->XcpOdtEntryLength;
		Xcp_MtaType mta;
		xcpPtr->XcpMtaInit(&mta, ent->XcpOdtEntryAddress, ent->XcpOdtEntryExtension);
		if (len == 0 || len + dto.len > XCP_MAX_DTO) {
			break;
		}
		Xcp_MtaRead(&mta, dto.data + dto.len, len);
		dto.len += len;
		ent++;
	}
	odt++;
	if((daq->XcpMaxOdt > odt - daq->XcpOdt) && (odt->XcpOdtEntry != 0) && (odt->XcpOdtEntry->XcpOdtEntryLength != 0)) {
		daq->NextOdtToSend = odt;
	} else {
		daq->NextOdtToSend = 0;
		Xcp_DaqTxList = daq->TxDaqListNext;
		daq->TxDaqListNext = 0;
		if(!Xcp_DaqTxList) {
			Xcp_DaqTxListLast = 0;
		}
	}

    Xcp_Fifo_Unlock();
    Std_ReturnType retVal = Xcp_Transmit(dto.data, dto.len);
    if (E_OK != retVal) {
        Xcp_TxOngoing = 0;
        DEBUG(DEBUG_HIGH, "Xcp_Transmit_Main - failed to transmit\n");
    }
}

/* Process all entries in event channel */
static void Xcp_ProcessChannel(uint16 event) {

    if(XcpEventChannelTriggeredDaqListRef[event] == 0) {
        // no daqs referenced, return
        return;
    }
    Xcp_DaqListType *daq = *XcpEventChannelTriggeredDaqListRef[event];
    do {
        if (!(daq->Mode & XCP_DAQLIST_MODE_RUNNING)) {
            continue;
        }
        daq->Timer++;
        if (daq->Timer < daq->Prescaler) {
            continue;
        }
        daq->Timer = 0;

        if(daq->NextOdtToSend) {
            // daq processing ongoing
            // send daq overrun and continue with next
            continue;
        }
        if(daq->XcpMaxOdt == 0 || daq->XcpOdt->XcpOdtEntriesCount == 0 || daq->XcpOdt->XcpOdtEntry->XcpOdtEntryLength == 0) {
            // nothing to send in daq, continue
            continue;
        }
        daq->NextOdtToSend = daq->XcpOdt;
        Xcp_Fifo_Lock();
        if(Xcp_DaqTxListLast) {
            Xcp_DaqTxListLast->TxDaqListNext = daq;
            Xcp_DaqTxListLast = daq;
            Xcp_Fifo_Unlock();
        } else {
            Xcp_DaqTxList = Xcp_DaqTxListLast = daq;
            if(!Xcp_TxOngoing) {
                Xcp_TxOngoing = 3;
                Xcp_Fifo_Unlock();
                Xcp_ProcessDaq_DAQ(Xcp_DaqTxList);
            } else {
                Xcp_Fifo_Unlock();
            }
        }
        daq = daq->EventDaqListNext;
    } while(daq != *XcpEventChannelTriggeredDaqListRef[event]);
}

/**************************************************************************/
/**************************************************************************/
/**************************** GENERIC COMMANDS ****************************/
/**************************************************************************/
/**************************************************************************/

static Std_ReturnType Xcp_CmdConnect(uint8 pid, uint8* data, PduLengthType len) {
    uint8 mode = GET_UINT8(data, 0);

    DEBUG(DEBUG_HIGH, "Received connect mode %x\n", mode);
    XCP_UNUSED(len);
    XCP_UNUSED(pid);

    if (mode != 0) {
        RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Xcp_CmdConnect\n");
    }

    uint16 endiantest = 0x100;
    uint8 endian = *((uint8*)&endiantest); /*Byte order - 1:High byte first*/

    uint8 resource = 0u;
    uint8 comModeBasic = 0u;

#if (XCP_FEATURE_CALPAG == STD_ON)
    resource |= 0x01u;
#endif
#if (XCP_FEATURE_DAQ == STD_ON)
    resource |= 0x04u;
#endif
#if (XCP_FEATURE_STIM == STD_ON)
    resource |= 0x08u;
#endif
#if (XCP_FEATURE_PGM == STD_ON)
    resource |= 0x10u;
#endif

    /* BYTE ORDER */
    comModeBasic |= endian << 0;
    /* ADDRESS_GRANULARITY */
    //comModeBasic |= 0 << 1;
#if (XCP_FEATURE_BLOCKMODE == STD_ON)
    /*SLAVE_BLOCK_MODE*/
    comModeBasic |= 0x40u;
#endif
	/* OPTIONAL */
	//comModeBasic |= 0x00u;

	if (!Xcp_Connected) {
		/* restore varius state on a new connections */
		xcpPtr->rt->XcpProtect = xcpPtr->XcpOriginalProtect;
	}

	Xcp_Connected = 1;

	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{
		FIFO_ADD_U8(e, XCP_PID_RES);
		/* RESSOURCE */
		FIFO_ADD_U8(e, resource);
		/* COMM_MODE_BASIC */
		FIFO_ADD_U8(e, comModeBasic);
		FIFO_ADD_U8(e, XCP_MAX_CTO);
		FIFO_ADD_U16(e, XCP_MAX_DTO);
		FIFO_ADD_U8(e, XCP_PROTOCOL_MAJOR_VERSION);
		FIFO_ADD_U8(e, XCP_TRANSPORT_MAJOR_VERSION);
	}

	return E_OK;

}

static Std_ReturnType Xcp_CmdGetStatus(uint8 pid, uint8* data, PduLengthType len) {

	DEBUG(DEBUG_HIGH, "Received get_status\n");
	XCP_UNUSED(pid);
	XCP_UNUSED(data); /*lint !e920 Misra 2012 1.3 cast from pointer to void */
	XCP_UNUSED(len);

	/* find if any lists are running */
	int running = 0;

	for (int i = 0; i < xcpPtr->rt->XcpMaxDaq; i++) {
		if (xcpPtr->XcpDaqList[i].Mode & XCP_DAQLIST_MODE_RUNNING) {
			running = 1;
			break;
		}
	}

    FIFO_GET_WRITE(Xcp_FifoTxCto, e)
    {
        FIFO_ADD_U8(e, XCP_PID_RES);
        FIFO_ADD_U8(e, 0u << 0u       /* STORE_CAL_REQ */
                     | 0u << 2u       /* STORE_DAQ_REQ */
                     | 0u << 3u       /* CLEAR_DAQ_REQ */
                     | running << 6u /* DAQ_RUNNING */
                     | 0u << 7u       /* RESUME */);

#if(XCP_FEATURE_PROTECTION == STD_ON)
        FIFO_ADD_U8 (e, xcpPtr->rt->XcpProtect); /* Content resource protection */
#else
        FIFO_ADD_U8(e, 0u); /* Content resource protection */
#endif
        FIFO_ADD_U8(e, 0u); /* Reserved */
        FIFO_ADD_U16(e, 0u); /* Session configuration ID */
    }

    return E_OK ;
}

static Std_ReturnType Xcp_CmdGetCommModeInfo(uint8 pid, uint8* data, PduLengthType len) {

    uint8 commModeOptional = 0x00u
#if XCP_FEATURE_BLOCKMODE == STD_ON
            | 1u << 0u /* MASTER_BLOCK_MODE */
#endif
			| 0u << 1u; /* INTERLEAVED_MODE  */

	DEBUG(DEBUG_HIGH, "Received get_comm_mode_info\n");
	XCP_UNUSED(pid);
	XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
	XCP_UNUSED(len);

	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{
		FIFO_ADD_U8(e, XCP_PID_RES);
		FIFO_ADD_U8(e, 0); /* Reserved */
		FIFO_ADD_U8(e, commModeOptional);
		FIFO_ADD_U8(e, 0); /* Reserved */
		FIFO_ADD_U8(e, XCP_MAX_RXTX_QUEUE - 1); /* MAX_BS */
		FIFO_ADD_U8(e, 0); /* MIN_ST [100 microseconds] */
		FIFO_ADD_U8(e, XCP_MAX_RXTX_QUEUE - 1); /* QUEUE_SIZE */
		FIFO_ADD_U8(e, XCP_PROTOCOL_MAJOR_VERSION << 4 | XCP_PROTOCOL_MINOR_VERSION); /* Xcp driver version */
	}

	return E_OK;
}

static Std_ReturnType Xcp_CmdGetId(uint8 pid, uint8* data, PduLengthType len) {
    uint8 idType = GET_UINT8(data, 0);
    const char* text = NULL;

    DEBUG(DEBUG_HIGH, "Received get_id %d\n", idType);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);



    if (idType == 0) {
        text = xcpPtr->XcpInfo.XcpCaption;
    } else if (idType == 1) {
        text = xcpPtr->XcpInfo.XcpMC2File;
    } else if (idType == 2) {
        text = xcpPtr->XcpInfo.XcpMC2Path;
    } else if (idType == 3) {
        text = xcpPtr->XcpInfo.XcpMC2Url;
    } else if (idType == 4) {
        text = xcpPtr->XcpInfo.XcpMC2Upload;
    }

    uint8 text_len = 0;
    if (text) {
        text_len = strlen(text);
    }

#if (XCP_MAX_CTO > 8)
    if(text_len + 8 < XCP_MAX_CTO) {
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, 1); /* Mode */
            FIFO_ADD_U16 (e, 0); /* Reserved */
            FIFO_ADD_U32 (e, text_len); /* Length */
            if(text) {
                Xcp_MtaType mta;
                xcpPtr->XcpMtaInit(&mta, (uint32)text, XCP_MTA_EXTENSION_MEMORY_INTERNAL_FIXED);
                Xcp_MtaRead(&mta, e->data+e->len, text_len);
                e->len += text_len;
            }
        }
    } else
#endif

    {
		xcpPtr->XcpMtaInit(&Xcp_Mta, (uint32) text, XCP_MTA_EXTENSION_MEMORY_INTERNAL_FIXED);
		FIFO_GET_WRITE(Xcp_FifoTxCto, e)
		{
			FIFO_ADD_U8(e, XCP_PID_RES);
			FIFO_ADD_U8(e, 0); /* Mode */
			FIFO_ADD_U16(e, 0); /* Reserved */
			FIFO_ADD_U32(e, text_len); /* Length */
		}
	}

	return E_OK ;

}

static Std_ReturnType Xcp_CmdDisconnect(uint8 pid, uint8* data, PduLengthType len) {

    XCP_UNUSED(len);
    XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(pid);

    if (Xcp_Connected) {
        DEBUG(DEBUG_HIGH, "Received disconnect\n");
    } else {
        DEBUG(DEBUG_HIGH, "Invalid disconnect without connect\n");
    }
    Xcp_Connected = 0;

    RETURN_SUCCESS();
}

static Std_ReturnType Xcp_CmdSync(uint8 pid, uint8* data, PduLengthType len) {

    XCP_UNUSED(len);
    XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(pid);

    RETURN_ERROR(XCP_ERR_CMD_SYNCH, "Xcp_CmdSync\n");
}

static Std_ReturnType Xcp_CmdUser(uint8 pid, uint8* data, PduLengthType len) {

    XCP_UNUSED(pid);

    if (xcpPtr->XcpUserFn) {
        return xcpPtr->XcpUserFn((uint8 *) data + 1, len - 1);
    } else {
        RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Xcp_CmdUser\n");
    }
}

/**************************************************************************/
/**************************************************************************/
/*********************** UPLOAD/DOWNLOAD COMMANDS *************************/
/**************************************************************************/
/**************************************************************************/

/**
 * Worker function for blockmode uploads
 *
 * This function will be called once every main function run and send off
 * a upload package, when done it will unregister itself from main process
 *
 */
static void Xcp_CmdUpload_Worker(void) {
	PduLengthType len = Xcp_Upload.rem;
	PduLengthType off = XCP_ELEMENT_OFFSET(1);
	PduLengthType max = XCP_MAX_CTO - off - 1;

	if (len > max) {
		len = max;
	}

	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{

		SET_UINT8(e->data, 0, XCP_PID_RES);
		for (PduLengthType i = 0; i < off; i++) {
			SET_UINT8(e->data, i + 1, 0);
		}

		for (PduLengthType i = 0; i < len; i++) {
			SET_UINT8(e->data, i + 1 + off, Xcp_MtaGet(&Xcp_Mta));
		}

		e->len = len + 1 + off;
	}
	Xcp_Upload.rem -= len;

	if (Xcp_Upload.rem == 0) {
		Xcp_Worker = NULL;
	}

}

static Std_ReturnType Xcp_CmdUpload(uint8 pid, uint8* data, PduLengthType len) {

    DEBUG(DEBUG_HIGH, "Received upload\n");
    XCP_UNUSED(len);
    XCP_UNUSED(pid);

    Xcp_Upload.len = GET_UINT8(data, 0) * XCP_ELEMENT_SIZE;
    Xcp_Upload.rem = Xcp_Upload.len;

#ifndef XCP_FEATURE_BLOCKMODE
    if(Xcp_Upload.len + 1 > XCP_MAX_CTO) {
        RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Xcp_CmdUpload - Block mode not supported\n");
    }
#endif

    Xcp_Worker = Xcp_CmdUpload_Worker;
    Xcp_Worker();
    return E_OK ;
}

static Std_ReturnType Xcp_CmdShortUpload(uint8 pid, uint8* data, PduLengthType len) {

    uint8 count = GET_UINT8(data, 0);
    uint8 ext = GET_UINT8(data, 2);
    uint32 addr = GET_UINT32(data, 3);

    DEBUG(DEBUG_HIGH, "Received short upload\n");
    XCP_UNUSED(len);
    XCP_UNUSED(pid);

    if (count > XCP_MAX_CTO - XCP_ELEMENT_SIZE) {
        RETURN_ERROR(XCP_ERR_CMD_SYNTAX, "Xcp_CmdShortUpload - Too long data requested\n");
    }

    xcpPtr->XcpMtaInit(&Xcp_Mta, addr, ext);
    if (Xcp_Mta.read == NULL) {
        RETURN_ERROR(XCP_ERR_CMD_SYNTAX, "Xcp_CmdShortUpload - invalid memory address\n");
    }

    FIFO_GET_WRITE(Xcp_FifoTxCto, e)
    {
        SET_UINT8(e->data, 0, XCP_PID_RES);

#if (XCP_ELEMENT_SIZE > 1)
        memset(e->data+1, 0, XCP_ELEMENT_SIZE - 1);
#endif

        Xcp_MtaRead(&Xcp_Mta, e->data + XCP_ELEMENT_SIZE, count);
        e->len = count + XCP_ELEMENT_SIZE;
    }
    return E_OK ;
}

static Std_ReturnType Xcp_CmdSetMTA(uint8 pid, uint8* data, PduLengthType len) {
    int ext = GET_UINT8(data, 2);
    int ptr = GET_UINT32(data, 3);

    DEBUG(DEBUG_HIGH, "Received set_mta 0x%x, %d\n", ptr, ext);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    xcpPtr->XcpMtaInit(&Xcp_Mta, ptr, ext);
    RETURN_SUCCESS();
}

static Std_ReturnType Xcp_CmdDownload(uint8 pid, uint8* data, PduLengthType len) {
    uint32 rem = GET_UINT8(data, 0) * XCP_ELEMENT_SIZE;
    uint32 off = XCP_ELEMENT_OFFSET(2) + 1;
    DEBUG(DEBUG_HIGH, "Received download %d, %d\n", pid, len);

    if (!Xcp_Mta.write) {
        RETURN_ERROR(XCP_ERR_WRITE_PROTECTED, "Xcp_Download - Mta not inited\n");
    }

#if(!XCP_FEATURE_BLOCKMODE)
    if(rem + off > len) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Xcp_Download - Invalid length (%u, %u, %d)\n", rem, off, len);
    }
#endif

    if (pid == XCP_PID_CMD_CAL_DOWNLOAD) {
        Xcp_Download.len = rem;
        Xcp_Download.rem = rem;
    }

    /* check for sequence error */
    if (Xcp_Download.rem != rem) {
        DEBUG(DEBUG_HIGH, "Xcp_Download - Invalid next state (%u, %u)\n", rem, Xcp_Download.rem);
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8(e, (uint8)XCP_PID_ERR);
            FIFO_ADD_U8(e, (uint8)XCP_ERR_SEQUENCE);
            FIFO_ADD_U8(e, Xcp_Download.rem / XCP_ELEMENT_SIZE);
        }
    return E_OK ;
    }

    /* write what we got this packet */
    if (rem > len - off) {
        rem = len - off;
    }

    Xcp_MtaWrite(&Xcp_Mta, (uint8*) data + off, rem);
    Xcp_Download.rem -= rem;

    if (Xcp_Download.rem) {
        return E_OK;
    }

    Xcp_MtaFlush(&Xcp_Mta);
    RETURN_SUCCESS();

}


static Std_ReturnType Xcp_CmdBuildChecksum(uint8 pid, uint8* data, PduLengthType len) {

	Xcp_ChecksumType  type;
	uint32            response;
	uint32            block = GET_UINT32(data, 3);

	DEBUG(DEBUG_HIGH, "Received build_checksum %ul\n", (uint32) block);
	XCP_UNUSED(pid);
	XCP_UNUSED(len);

	if (!xcpPtr->XcpBuildChecksum) {
		/*No CRC user function is defined*/
		RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Xcp_CmdBuildChecksum\n");
	}

	if (!Xcp_Mta.get) {
		RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Xcp_CmdBuildChecksum - Mta not inited\n");
	}

	/*callout function is used to calculate CRC*/
	xcpPtr->XcpBuildChecksum(&Xcp_Mta,block,&type,&response);

	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{
		FIFO_ADD_U8(e, XCP_PID_RES);
		FIFO_ADD_U8(e, (uint8)type);
		FIFO_ADD_U8(e, 0); /* reserved */
		FIFO_ADD_U8(e, 0); /* reserved */
		FIFO_ADD_U32(e, response);
	}
	return E_OK;
}

/**************************************************************************/
/**************************************************************************/
/*************************** DAQ/STIM COMMANDS ****************************/
/**************************************************************************/
/**************************************************************************/

static Std_ReturnType Xcp_CmdClearDaqList(uint8 pid, uint8* data, PduLengthType len) {

    uint16 daqListNumber = GET_UINT16(data, 1);

    DEBUG(DEBUG_HIGH, "Received ClearDaqList\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if (daqListNumber >= xcpPtr->rt->XcpMaxDaq) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: daq list number out of range\n");
    }

    Xcp_DaqListType* daq = xcpPtr->XcpDaqList + daqListNumber;

    if (daq->Mode & XCP_DAQLIST_MODE_RUNNING) {
        daq->Mode &= ~XCP_DAQLIST_MODE_RUNNING; // ensure that the code isn't using running mode as indicator if daq list is in ready queue.
    }

#if XCP_MIN_DAQ > 0
    if(daqListNumber > XCP_MIN_DAQ)
#endif
    {
        for (int i = 0; i < daq->XcpMaxOdt; i++) {
            Xcp_OdtType* odt = daq->XcpOdt + i;
            for (int j = 0; j < odt->XcpOdtEntriesCount; j++) {
                Xcp_OdtEntryType* entry = odt->XcpOdtEntry + j;
                entry->XcpOdtEntryAddress = 0;
                entry->XcpOdtEntryExtension = 0;
                entry->XcpOdtEntryLength = 0;
                entry->BitOffSet = 0xFF;
            }
        }
    }
    RETURN_SUCCESS();
}

static Std_ReturnType Xcp_CmdSetDaqPtr(uint8 pid, uint8* data, PduLengthType len) {

    uint16 daqListNumber = GET_UINT16(data, 1);
    uint8 odtNumber = GET_UINT8(data, 3);
    uint8 odtEntryNumber = GET_UINT8(data, 4);

    DEBUG(DEBUG_HIGH, "Received SetDaqPtr %u, %u, %u\n", daqListNumber,odtNumber, odtEntryNumber);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if (daqListNumber >= xcpPtr->rt->XcpMaxDaq) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: daq list number out of range\n");
    }

    Xcp_DaqListType* daq = xcpPtr->XcpDaqList + daqListNumber;

    if (daq->Mode & XCP_DAQLIST_MODE_RUNNING) {
        RETURN_ERROR(XCP_ERR_DAQ_ACTIVE, "Error: DAQ running\n");
    }

    if (odtNumber >= daq->XcpMaxOdt) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: odt number out of range (%u, %u)\n", odtNumber, daq->XcpMaxOdt);
    }

    Xcp_OdtType* odt = daq->XcpOdt + odtNumber;

    if (odtEntryNumber >= odt->XcpOdtEntriesCount) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: odt entry number out of range\n");
    }

    Xcp_OdtEntryType* odtEntry = odt->XcpOdtEntry + odtEntryNumber;

    Xcp_DaqState.daq = daq;
    Xcp_DaqState.odt = odt;
    Xcp_DaqState.ptr = odtEntry;

    RETURN_SUCCESS();
}

static Std_ReturnType Xcp_CmdWriteDaq(uint8 pid, uint8* data, PduLengthType len) {

    DEBUG(DEBUG_HIGH, "Received WriteDaq\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if (Xcp_DaqState.ptr == NULL) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: No more ODT entries in this ODT\n");
    }

#if (XCP_MIN_DAQ > 0)
    /* Check if DAQ list is write protected */
    if (Xcp_DaqState.daq - xcpPtr->XcpDaqList < XCP_MIN_DAQ) {
        RETURN_ERROR(XCP_ERR_WRITE_PROTECTED, "Error: DAQ-list is read only\n");
    }
#endif

    if (Xcp_DaqState.daq->Mode & XCP_DAQLIST_MODE_RUNNING) {
        RETURN_ERROR(XCP_ERR_DAQ_ACTIVE, "Error: DAQ running\n");
    }

    uint8 maxOdtEntrySize;
    uint8 granularityOdtEntrySize;

    if (Xcp_DaqState.daq->Mode & XCP_DAQLIST_MODE_STIM) /* Get DAQ list Direction */
    {
        maxOdtEntrySize = XCP_MAX_ODT_ENTRY_SIZE_STIM;
        granularityOdtEntrySize = XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM;
    } else {
        maxOdtEntrySize = XCP_MAX_ODT_ENTRY_SIZE_DAQ;
        granularityOdtEntrySize = XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ;
    }

    uint8 daqElemSize = GET_UINT8(data, 1);

    if (daqElemSize > maxOdtEntrySize) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: DAQ list element size is invalid\n");
    }

    uint8 bitOffSet = GET_UINT8(data, 0);

    if (bitOffSet <= 0x1F) {
        if (daqElemSize == granularityOdtEntrySize) {
            Xcp_DaqState.ptr->BitOffSet = bitOffSet;
        } else {
            RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: Element size and granularity don't match\n");
        }
    } else {
        Xcp_DaqState.ptr->BitOffSet = 0xFF;
    }

    Xcp_DaqState.ptr->XcpOdtEntryExtension = GET_UINT8(data, 2);
    Xcp_DaqState.ptr->XcpOdtEntryAddress = GET_UINT32(data, 3);

    Xcp_DaqState.ptr->XcpOdtEntryLength = daqElemSize;

    Xcp_DaqState.ptr++;
    if(Xcp_DaqState.odt->XcpOdtEntriesCount <= Xcp_DaqState.ptr - Xcp_DaqState.odt->XcpOdtEntry) {
        Xcp_DaqState.ptr = NULL;
        Xcp_DaqState.daq = NULL;
        Xcp_DaqState.odt = NULL;
    }

    RETURN_SUCCESS();
}

static void Xcp_CmdSetDaqListMode_EventChannel(Xcp_DaqListType* daq, uint16 newEventChannelNumber) {

    if(daq->EventChannel == newEventChannelNumber) {
        // already allocated to event, return
        return;
    }

    if (daq->EventChannel < XCP_EVENT_COUNT /* !=0xFFFF*/) {

        //Remove daq ref from XcpEventChannelTriggeredDaqListRef
        if(daq->EventDaqListNext == daq) {
            // this is the only daq allocated to event, clear event list ptr
            XcpEventChannelTriggeredDaqListRef[daq->EventChannel] = 0;

        //Remove daq ref from chain, point the XcpEventChannelTriggeredDaqListRef to the rest
        } else {
            Xcp_DaqListType *daqP = daq->EventDaqListNext;
            while(daqP->EventDaqListNext != daq) {
                daqP = daqP->EventDaqListNext;
            }
            daqP->EventDaqListNext = daq->EventDaqListNext;
            XcpEventChannelTriggeredDaqListRef[daq->EventChannel] = &daqP->EventDaqListNext;
        }
    }

    if(newEventChannelNumber < XCP_EVENT_COUNT /* !=0xFFFF*/) {
        // add daq to new event channel
        if(XcpEventChannelTriggeredDaqListRef[newEventChannelNumber] == 0) {
            XcpEventChannelTriggeredDaqListRef[newEventChannelNumber] = &daq->EventDaqListNext;
            daq->EventDaqListNext = daq;
        } else {
            daq->EventDaqListNext = *XcpEventChannelTriggeredDaqListRef[newEventChannelNumber];
            *XcpEventChannelTriggeredDaqListRef[newEventChannelNumber] = daq;
            XcpEventChannelTriggeredDaqListRef[newEventChannelNumber] = &daq->EventDaqListNext;
        }
    }
    daq->EventChannel = newEventChannelNumber;
}

static Std_ReturnType Xcp_CmdSetDaqListMode(uint8 pid, uint8* data, PduLengthType len) {

	uint16 list = GET_UINT16(data, 1);
	uint16 event = GET_UINT16(data, 3);
	uint8 prio = GET_UINT8(data, 6);
	uint8 mode = GET_UINT8(data, 0);

	DEBUG(DEBUG_HIGH, "Received SetDaqListMode\n");
	XCP_UNUSED(pid);
	XCP_UNUSED(len);

	if (list >= xcpPtr->rt->XcpMaxDaq || event >= XCP_EVENT_COUNT) {
		RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: daq list number out of range\n");
	}

	Xcp_DaqListType *daq = xcpPtr->XcpDaqList + list;

	if (daq->Mode & XCP_DAQLIST_MODE_RUNNING) {
		RETURN_ERROR(XCP_ERR_DAQ_ACTIVE, "Error: DAQ running\n");
	}
	/* Check to see if the event channel supports the direction of the DAQ list.
	 * Can DAQ list be set to requested direction.
	 * Is the DAQ Predefined or Event_fixed
	 * */
	if(list < xcpPtr->XcpDaqListSize) {
		// this is a static or predefined list
		Xcp_DaqListCfgType *daqCfg = xcpPtr->XcpDaqListCfg + list;
		if (!( (    (mode & XCP_DAQLIST_MODE_STIM)
				 && (daqCfg->Properties & XCP_DAQLIST_PROPERTY_STIM)
			   ) ||
			   (    (!(mode & XCP_DAQLIST_MODE_STIM))
				 && (daqCfg->Properties & XCP_DAQLIST_PROPERTY_DAQ)
			   )) ) {
			RETURN_ERROR(XCP_ERR_CMD_SYNTAX, "Error: direction not allowed.\n");
		}
		if ((daqCfg->Properties & XCP_DAQLIST_PROPERTY_EVENTFIXED)
				&& (event != daqCfg->FixedEventChannel)) {
			RETURN_ERROR(XCP_ERR_DAQ_CONFIG, "Error: DAQ list has a fixed event channel\n");
		}
	}
	if (!( (    (mode & XCP_DAQLIST_MODE_STIM)
			 && ((uint8)xcpPtr->XcpEventChannel[event].XcpEventChannelProperties & (uint8)XCP_EVENTCHANNEL_PROPERTY_STIM)
		   ) ||
		   (    (!(mode & XCP_DAQLIST_MODE_STIM))
			 && ((uint8)xcpPtr->XcpEventChannel[event].XcpEventChannelProperties & (uint8)XCP_EVENTCHANNEL_PROPERTY_DAQ)
		   )) ) {
		RETURN_ERROR(XCP_ERR_CMD_SYNTAX, "Error: direction not allowed.\n");
	}

#if (XCP_MIN_DAQ > 0)
    if (list < XCP_MIN_DAQ) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: DAQ list is Predefined\n");
    }
#endif

    daq->Mode = (Xcp_DaqListModeEnum)((mode & 0x32) | (daq->Mode & ~0x32));
    Xcp_CmdSetDaqListMode_EventChannel(daq, event);
    daq->Prescaler = GET_UINT8(data, 5);
    daq->Priority = prio;

    RETURN_SUCCESS();
}

static Std_ReturnType Xcp_CmdGetDaqListMode(uint8 pid, uint8* data, PduLengthType len) {

    uint16 daqListNumber = GET_UINT16(data, 1);

    DEBUG(DEBUG_HIGH, "Received GetDaqListMode\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if (daqListNumber >= xcpPtr->rt->XcpMaxDaq) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: DAQ list number out of range\n");
    }

    Xcp_DaqListType* daq = xcpPtr->XcpDaqList + daqListNumber;

	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{

		FIFO_ADD_U8(e, XCP_PID_RES);
		FIFO_ADD_U8(e, daq->Mode); /* Mode */
		FIFO_ADD_U16(e, 0); /* Reserved */
		FIFO_ADD_U16(e, daq->EventChannel); /* Current Event Channel Number */
		FIFO_ADD_U8(e, daq->Prescaler); /* Current Prescaler */
		FIFO_ADD_U8(e, daq->Priority); /* Current DAQ list Priority */
	}
	return E_OK ;
}

static Std_ReturnType Xcp_CmdStartStopDaqList(uint8 pid, uint8* data, PduLengthType len) {

	uint16 daqListNumber = GET_UINT16(data, 1);

	DEBUG(DEBUG_HIGH, "Received StartStopDaqList\n");
	XCP_UNUSED(pid);
	XCP_UNUSED(len);

	if (daqListNumber >= xcpPtr->rt->XcpMaxDaq) {
		RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: daq list number out of range\n");
	}
	Xcp_DaqListType* daq = xcpPtr->XcpDaqList + daqListNumber;
	daq->FirstPid = 0;
	for(int i = 0; i < daqListNumber; i++) {
		daq->FirstPid += xcpPtr->XcpDaqList[i].XcpMaxOdt;
	}

	uint8 mode = GET_UINT8(data, 0);
	if (mode == 0) {
		/* STOP */
		daq->Mode &= ~XCP_DAQLIST_MODE_RUNNING;
	} else if (mode == 1) {
		/* START */
		daq->Mode |= XCP_DAQLIST_MODE_RUNNING;
	} else if (mode == 2) {
		/* SELECT */
		daq->Mode |= XCP_DAQLIST_MODE_SELECTED;
	} else {
		RETURN_ERROR(XCP_ERR_MODE_NOT_VALID, "Error mode not valid\n");
	}

	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{
		FIFO_ADD_U8(e, XCP_PID_RES);
		FIFO_ADD_U8(e, daq->FirstPid);
	}
	return E_OK ;
}

static Std_ReturnType Xcp_CmdStartStopSynch(uint8 pid, uint8* data, PduLengthType len) {

    uint8 mode = GET_UINT8(data, 0);

    DEBUG(DEBUG_HIGH, "Received StartStopSynch %u\n", mode);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    Xcp_DaqListType* daq = xcpPtr->XcpDaqList;

    if (mode == 0) {
        /* STOP ALL */
        for (int i = 0; i < xcpPtr->rt->XcpMaxDaq; i++) {
            daq[i].Mode &= ~XCP_DAQLIST_MODE_RUNNING;
            daq[i].Mode &= ~XCP_DAQLIST_MODE_SELECTED;
        }
    } else if (mode == 1) {
        /* START SELECTED */
        for (int i = 0; i < xcpPtr->rt->XcpMaxDaq; i++) {
            if (daq[i].Mode & XCP_DAQLIST_MODE_SELECTED) {
                daq[i].Mode |= XCP_DAQLIST_MODE_RUNNING;
                daq[i].Mode &= ~XCP_DAQLIST_MODE_SELECTED;
            }
        }
    } else if (mode == 2) {
        /* STOP SELECTED */
        for (int i = 0; i < xcpPtr->rt->XcpMaxDaq; i++) {
            if (daq[i].Mode & XCP_DAQLIST_MODE_SELECTED) {
                daq[i].Mode &= ~XCP_DAQLIST_MODE_RUNNING;
                daq[i].Mode &= ~XCP_DAQLIST_MODE_SELECTED;
            }
        }
    } else {
        RETURN_ERROR(XCP_ERR_MODE_NOT_VALID, "Error mode not valid\n");
    }
    RETURN_SUCCESS();
}

static Std_ReturnType Xcp_CmdGetDaqClock(uint8 pid, uint8* data, PduLengthType len) {

	DEBUG(DEBUG_HIGH, "Received GetDaqClock\n");
	XCP_UNUSED(pid);
	XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
	XCP_UNUSED(len);

	FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
		FIFO_ADD_U8(e, XCP_PID_RES);
		FIFO_ADD_U8(e, 0); /* Alignment */
		FIFO_ADD_U8(e, 0); /* Alignment */
		FIFO_ADD_U8(e, 0); /* Alignment */
		FIFO_ADD_U32(e, Xcp_GetTimeStamp());
	}
	return E_OK;

}

static Std_ReturnType Xcp_CmdReadDaq(uint8 pid, uint8* data, PduLengthType len) {

	DEBUG(DEBUG_HIGH, "Received ReadDaq\n");
	XCP_UNUSED(pid);
	XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
	XCP_UNUSED(len);

	if (!Xcp_DaqState.ptr) {
		RETURN_ERROR(XCP_ERR_DAQ_CONFIG, "Error: No more ODT entries in this ODT\n");
	}

	FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
		FIFO_ADD_U8(e, XCP_PID_RES);
		FIFO_ADD_U8(e, Xcp_DaqState.ptr->BitOffSet);
		FIFO_ADD_U8(e, Xcp_DaqState.ptr->XcpOdtEntryLength);
		FIFO_ADD_U8(e, Xcp_DaqState.ptr->XcpOdtEntryExtension);
		FIFO_ADD_U32(e, Xcp_DaqState.ptr->XcpOdtEntryAddress);
	}

	Xcp_DaqState.ptr++;
	if(Xcp_DaqState.odt->XcpOdtEntriesCount <= Xcp_DaqState.ptr - Xcp_DaqState.odt->XcpOdtEntry) {
		Xcp_DaqState.ptr = NULL;
		Xcp_DaqState.daq = NULL;
		Xcp_DaqState.odt = NULL;
	}

	return E_OK;

}

static Std_ReturnType Xcp_CmdGetDaqProcessorInfo(uint8 pid, uint8* data, PduLengthType len) {

    uint8 daqProperties = 0x00u
#if (XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC)
            | 1 << 0 /* DAQ_CONFIG_TYPE     */
#endif
#if (XCP_TIMESTAMP_SIZE > 0)
            | 1 << 4 /* TIMESTAMP_SUPPORTED */
#endif
			| 1 << 1 /* PRESCALER_SUPPORTED */
			| 0 << 2 /* RESUME_SUPPORTED    */
			| 0 << 3 /* BIT_STIM_SUPPORTED  */
			| 0 << 5 /* PID_OFF_SUPPORTED   */
			| 0 << 6 /* OVERLOAD_MSB        */
			| 0 << 7 /* OVERLOAD_EVENT      */;

	DEBUG(DEBUG_HIGH, "Received GetDaqProcessorInfo\n");
	XCP_UNUSED(pid);
	XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
	XCP_UNUSED(len);

	FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
		FIFO_ADD_U8(e, XCP_PID_RES);
		FIFO_ADD_U8(e, daqProperties);
		FIFO_ADD_U16(e, xcpPtr->rt->XcpMaxDaq);
		FIFO_ADD_U16(e, XCP_EVENT_COUNT);
		FIFO_ADD_U8(e, XCP_MIN_DAQ);
		FIFO_ADD_U8(e, 0 << 0 /* Optimisation_Type_0 */
		| 0 << 1 /* Optimisation_Type_1 */
		| 0 << 2 /* Optimisation_Type_2 */
		| 0 << 3 /* Optimisation_Type_3 */
		| 0 << 4 /* Address_Extension_ODT */
		| 0 << 5 /* Address_Extension_DAQ */
		| XCP_IDENTIFICATION << 6 /* Identification_Field_Type_0 and 1  */);
	}
	return E_OK;
}

static Std_ReturnType Xcp_CmdGetDaqResolutionInfo(uint8 pid, uint8* data, PduLengthType len) {

    DEBUG(DEBUG_HIGH, "Received GetDaqResolutionInfo\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(len);

    FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
        SET_UINT8(e->data, 0, XCP_PID_RES);
        SET_UINT8(e->data, 1, XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ); /* GRANULARITY_ODT_ENTRY_SIZE_DAQ */
        SET_UINT8(e->data, 2, XCP_MAX_ODT_ENTRY_SIZE_DAQ); /* MAX_ODT_ENTRY_SIZE_DAQ */
        SET_UINT8(e->data, 3, XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM); /* GRANULARITY_ODT_ENTRY_SIZE_STIM */
        SET_UINT8(e->data, 4, XCP_MAX_ODT_ENTRY_SIZE_STIM); /* MAX_ODT_ENTRY_SIZE_STIM */

#if(XCP_TIMESTAMP_SIZE)
            SET_UINT8 (e->data, 5, XCP_TIMESTAMP_SIZE << 0 /* TIMESTAMP_SIZE  */
                        | 0 << 3 /* TIMESTAMP_FIXED */
                        | XCP_TIMESTAMP_UNIT << 4 /* TIMESTAMP_UNIT  */);
            SET_UINT16(e->data, 6, 1); /* TIMESTAMP_TICKS */
#else
            SET_UINT8(e->data, 5, 0); /* TIMESTAMP_MODE  */
            SET_UINT16(e->data, 6, 0); /* TIMESTAMP_TICKS */
#endif

        e->len = 8;
    }
    return E_OK ;
}

static Std_ReturnType Xcp_CmdGetDaqListInfo(uint8 pid, uint8* data, PduLengthType len) {

    uint16 daqListNumber = GET_UINT16(data, 1);
    /*temporary variable to calculate the maximum OdtEntry value of a given Daq*/
    uint8 maxMaxOdtEntry = 0U;
    uint8 Properties = 0;

    DEBUG(DEBUG_HIGH, "Received GetDaqListInfo\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if (daqListNumber >= xcpPtr->rt->XcpMaxDaq) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: Xcp_GetDaqListInfo list number out of range\n");
    }

    Xcp_DaqListType* daq = xcpPtr->XcpDaqList+daqListNumber;

    for (uint8 odtNr = 0; odtNr < daq->XcpMaxOdt; odtNr++) {
        Xcp_OdtType* odt = daq->XcpOdt + odtNr;

        if (odt->XcpOdtEntriesCount > maxMaxOdtEntry) {
            maxMaxOdtEntry = odt->XcpOdtEntriesCount;
        }
    }

#if  XCP_MIN_DAQ > 0
    if(daqListNumber < XCP_MIN_DAQ) {
        Properties |= XCP_DAQLIST_PROPERTY_PREDEFINED;
    }
#endif
    if(daqListNumber < xcpPtr->XcpDaqListSize) {
        Properties |= XCP_DAQLIST_PROPERTY_EVENTFIXED;
        Properties |= xcpPtr->XcpDaqListCfg[daqListNumber].Properties & 0xC;
    } else {
        Properties |= XCP_DAQLIST_PROPERTY_DAQ_STIM;
    }

	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{

		SET_UINT8(e->data, 0, XCP_PID_RES);
		SET_UINT8(e->data, 1, Properties);
		SET_UINT8(e->data, 2, daq->XcpMaxOdt); /* MAX_ODT */
		SET_UINT8(e->data, 3, maxMaxOdtEntry); /* MAX_ODT_ENTRIES */
		SET_UINT16(e->data, 4, daq->EventChannel); /* FIXED_EVENT */
		e->len = 6;
	}
	return E_OK ;
}

static Std_ReturnType Xcp_CmdGetDaqEventInfo(uint8 pid, uint8* data, PduLengthType len) {

	uint16 eventChannelNumber = GET_UINT16(data, 1);

	DEBUG(DEBUG_HIGH, "Received GetDaqEventInfo\n");
	XCP_UNUSED(pid);
	XCP_UNUSED(len);

	if (eventChannelNumber >= XCP_EVENT_COUNT) {
		RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Error: Xcp_CmdGetDaqEventInfo event channel number out of range\n");
	}

	const Xcp_EventChannelType* eventChannel = xcpPtr->XcpEventChannel + eventChannelNumber;

	uint8 namelen = 0;
	if (eventChannel->XcpEventChannelName) {
		namelen = strlen(eventChannel->XcpEventChannelName);
		xcpPtr->XcpMtaInit(&Xcp_Mta, (uint32) eventChannel->XcpEventChannelName,XCP_MTA_EXTENSION_MEMORY_INTERNAL_FIXED);
	}

	FIFO_GET_WRITE(Xcp_FifoTxCto, e)
	{
		SET_UINT8(e->data, 0, XCP_PID_RES);
		SET_UINT8(e->data, 1, (uint8)eventChannel->XcpEventChannelProperties);
		SET_UINT8(e->data, 2, eventChannel->XcpEventChannelMaxDaqList);
		SET_UINT8(e->data, 3, namelen); /* Name length */
		SET_UINT8(e->data, 4, eventChannel->XcpEventChannelRate); /* Cycle time  */
		SET_UINT8(e->data, 5, (uint8)eventChannel->XcpEventChannelUnit); /* Time unit   */
		SET_UINT8(e->data, 6, eventChannel->XcpEventChannelPriority); /* Event channel priority */
		e->len = 7;
	}

	return E_OK ;
}

#if(XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC)

static Std_ReturnType Xcp_CmdFreeDaq(uint8 pid, uint8* data, PduLengthType len) {
    DEBUG(DEBUG_HIGH, "Received FreeDaq\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(len);

    Xcp_Fifo_Lock();
    // free daqs from tx list
    Xcp_DaqListType **Xcp_DaqTxListPtr = &Xcp_DaqTxList;
    Xcp_DaqTxListLast = 0;
    while(*Xcp_DaqTxListPtr != 0) {
        if(*Xcp_DaqTxListPtr - xcpPtr->XcpDaqList >= XCP_MIN_DAQ) {
            // this is a dynamic daq, free it
            *Xcp_DaqTxListPtr = (*Xcp_DaqTxListPtr)->TxDaqListNext;
        } else {
            Xcp_DaqTxListLast = *Xcp_DaqTxListPtr;
            Xcp_DaqTxListPtr = &(*Xcp_DaqTxListPtr)->TxDaqListNext;
        }
    }
    // free daqs from event lists
    for(int event = 0; event < XCP_EVENT_COUNT; event++) {
        if(XcpEventChannelTriggeredDaqListRef[event] != 0) {
            Xcp_DaqListType *daqFirst = *XcpEventChannelTriggeredDaqListRef[event];
            Xcp_DaqListType *daq = daqFirst->EventDaqListNext;
            Xcp_DaqListType *daqNext;
            do {
                daqNext = daq->EventDaqListNext;
                if(daqNext - xcpPtr->XcpDaqList  >= XCP_MIN_DAQ) {
                    daq->EventDaqListNext = daqNext->EventDaqListNext;
                } else {
                    daq = daqNext;
                }
            }while(daqFirst != daqNext);
            if(daq - xcpPtr->XcpDaqList  >= XCP_MIN_DAQ) {
                // list is empty, clear ptr
                XcpEventChannelTriggeredDaqListRef[event] = 0;
            } else {
                XcpEventChannelTriggeredDaqListRef[event] = &daq->EventDaqListNext;
            }
        }
    }
    Xcp_Fifo_Unlock();

    /* we now only have minimum number of daq lists */
    xcpPtr->rt->XcpMaxDaq = XCP_MIN_DAQ;
    xcpPtr->rt->XcpNumDynWordsAllocated = 0;

    Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_FREE_DAQ;
    RETURN_SUCCESS();
}

static uint8* Xcp_AllocDyn(uint32 bytes) {

    uint8* retPtr;

    uint32 numToAllocate = (bytes + 3) / 4;

    if(xcpPtr->dynamicRamBufferSize < xcpPtr->rt->XcpNumDynWordsAllocated + numToAllocate) {
        retPtr = NULL;
    } else {
        retPtr = (uint8*)&xcpPtr->ptrDynamicRamBuffer[xcpPtr->rt->XcpNumDynWordsAllocated];
        xcpPtr->rt->XcpNumDynWordsAllocated += numToAllocate;
        memset(retPtr, 0, numToAllocate * 4);
    }

    return retPtr;
}

static Std_ReturnType Xcp_CmdAllocDaq(uint8 pid, uint8* data, PduLengthType len) {

    DEBUG(DEBUG_HIGH, "Received AllocDaq\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if (!(Xcp_DaqState.dyn == XCP_DYNAMIC_STATE_FREE_DAQ)) {
        Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_UNDEFINED;
        RETURN_ERROR(XCP_ERR_SEQUENCE, " ");
    }

    uint16 nrDaqs = GET_UINT16(data, 1);
    Xcp_DaqListType *daqDyn = (Xcp_DaqListType*) Xcp_AllocDyn(sizeof(Xcp_DaqListType)*nrDaqs);

    if (daqDyn == NULL) {
        RETURN_ERROR(XCP_ERR_MEMORY_OVERFLOW, "Error, memory overflow");
    }

    for (uint16 daqNr = 0; daqNr < nrDaqs; daqNr++) {
        Xcp_DaqListType* daq = daqDyn + daqNr;
        daq->EventChannel = 0xFFFF;
    }
    /*lint -e{920} Do not take care of return value.*/
    xcpPtr->rt->XcpMaxDaq = XCP_MIN_DAQ + nrDaqs;

    Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_ALLOC_DAQ;
    RETURN_SUCCESS();
}

static Std_ReturnType Xcp_CmdAllocOdt(uint8 pid, uint8* data, PduLengthType len) {

    DEBUG(DEBUG_HIGH, "Received AllocOdt\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if (!(Xcp_DaqState.dyn == XCP_DYNAMIC_STATE_ALLOC_DAQ || Xcp_DaqState.dyn == XCP_DYNAMIC_STATE_ALLOC_ODT)) {
        Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_UNDEFINED;
        RETURN_ERROR(XCP_ERR_SEQUENCE, " ");
    }
    Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_ALLOC_ODT;

    uint16 daqNr = GET_UINT16(data, 1);
    uint8 nrOdts = GET_UINT8(data, 3);

    if (daqNr >= xcpPtr->rt->XcpMaxDaq) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Requested allocation to predefined daq list %u", daqNr);
    }

#if (XCP_MIN_DAQ > 0)
    if (daqNr < XCP_MIN_DAQ) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Requested allocation to predefined daq list %u", daqNr);
    }
#endif /* XCP_MIN_DAQ > 0 */


    Xcp_DaqListType* daq = xcpPtr->XcpDaqList + daqNr;
    if(daq->XcpOdt) {
        Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_UNDEFINED;
        RETURN_ERROR(XCP_ERR_SEQUENCE, " ");
    }

    daq->XcpOdt = (Xcp_OdtType *)Xcp_AllocDyn(sizeof(daq->XcpOdt[0])*nrOdts);
    if (daq->XcpOdt == NULL) {
        RETURN_ERROR(XCP_ERR_MEMORY_OVERFLOW, "Error, memory overflow");
    }

    daq->XcpMaxOdt = nrOdts;
    RETURN_SUCCESS();
}

static Std_ReturnType Xcp_CmdAllocOdtEntry(uint8 pid, uint8* data, PduLengthType len)
{
    DEBUG(DEBUG_HIGH, "Received AllocOdtEntry\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if (!(Xcp_DaqState.dyn == XCP_DYNAMIC_STATE_ALLOC_ODT || Xcp_DaqState.dyn == XCP_DYNAMIC_STATE_ALLOC_ODT_ENTRY)) {
        Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_UNDEFINED;
        RETURN_ERROR(XCP_ERR_SEQUENCE, " ");
    }
    Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_ALLOC_ODT_ENTRY;

    uint16 daqNr = GET_UINT16(data, 1);
    uint8 odtNr = GET_UINT8(data, 3);
    uint8 odtEntriesCount = GET_UINT8(data, 4);

    if (daqNr >= xcpPtr->rt->XcpMaxDaq) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Requested allocation to predefined daq list %u", daqNr);
    }

#if (XCP_MIN_DAQ > 0)
    if (daqNr < XCP_MIN_DAQ) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Requested allocation to predefined daq list %u", daqNr);
    }
#endif /* XCP_MIN_DAQ > 0 */


    Xcp_DaqListType* daq = xcpPtr->XcpDaqList + daqNr;

    if (odtNr >= daq->XcpMaxOdt) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Requested allocation to invalid odt for daq %u, odt %u", daqNr, odtNr);
    }

    Xcp_OdtType* odt = daq->XcpOdt + odtNr;
    if(odt->XcpOdtEntry) {
        Xcp_DaqState.dyn = XCP_DYNAMIC_STATE_UNDEFINED;
        RETURN_ERROR(XCP_ERR_SEQUENCE, " ");
    }
    odt->XcpOdtEntry = (Xcp_OdtEntryType *)Xcp_AllocDyn(sizeof(odt->XcpOdtEntry[0])*odtEntriesCount);
    if (odt->XcpOdtEntry == NULL) {
        RETURN_ERROR(XCP_ERR_MEMORY_OVERFLOW, "Error, memory overflow");
    }
    odt->XcpOdtEntriesCount = odtEntriesCount;
    RETURN_SUCCESS();
}
#endif

/**************************************************************************/
/**************************************************************************/
/****************************** SEED & KEY ********************************/
/**************************************************************************/
/**************************************************************************/
#if(XCP_FEATURE_PROTECTION == STD_ON)

static Std_ReturnType Xcp_CmdGetSeed(uint8 pid, uint8* data, PduLengthType len)
{
	uint8 mode = GET_UINT8(data, 0);
	uint8 res = GET_UINT8(data, 1);

	DEBUG(DEBUG_HIGH, "Received GetSeed(%u, %u)\n", mode, res);
	XCP_UNUSED(pid);
	XCP_UNUSED(len);

	if(mode == 0) {
		if(    res != XCP_PROTECT_CALPAG
			&& res != XCP_PROTECT_DAQ
		    && res != XCP_PROTECT_STIM
		    && res != XCP_PROTECT_PGM) {
			RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Requested invalid resource");
		}

		Xcp_Unlock.res = (Xcp_ProtectType)res;
		Xcp_Unlock.key_len = 0;
		Xcp_Unlock.key_rem = 0;
		Xcp_Unlock.seed_len = xcpPtr->XcpSeedFn((Xcp_ProtectType)res, Xcp_Unlock.seed);
		Xcp_Unlock.seed_rem = Xcp_Unlock.seed_len;
	} else if(mode == 1) {
		if(Xcp_Unlock.res == XCP_PROTECT_NONE) {
			RETURN_ERROR(XCP_ERR_SEQUENCE, "Requested second part of seed before first");
		}
	} else {
		RETURN_ERROR(XCP_ERR_GENERIC, "Requested invalid mode");
	}

	uint8 rem;

	if(Xcp_Unlock.seed_rem > XCP_MAX_CTO - 2) {
		rem = XCP_MAX_CTO - 2;
	} else {
		rem = Xcp_Unlock.seed_rem;
	}

	FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
		FIFO_ADD_U8(e, XCP_PID_RES);
		FIFO_ADD_U8(e, Xcp_Unlock.seed_rem);
		Xcp_MemCpy(   e->data+e->len
				, Xcp_Unlock.seed + Xcp_Unlock.seed_len - Xcp_Unlock.seed_rem
				, rem);

		e->len += rem;
		Xcp_Unlock.seed_rem -= rem;
	}

	return E_OK;

}

static Std_ReturnType Xcp_CmdUnlock(uint8 pid, uint8* data, PduLengthType len)
{

	uint8 rem = GET_UINT8(data, 0);
	DEBUG(DEBUG_HIGH, "Received Unlock(%u)\n", rem);
	XCP_UNUSED(pid);

	if(Xcp_Unlock.res == XCP_PROTECT_NONE) {
		RETURN_ERROR(XCP_ERR_SEQUENCE, "Requested unlock without requesting a seed");
	}

	/* if this is first call, setup state */
	if(Xcp_Unlock.key_len == 0) {
		Xcp_Unlock.key_len = rem;
		Xcp_Unlock.key_rem = rem;
	}

	/* validate that we are in correct sync */
	if(Xcp_Unlock.key_rem != rem) {
		FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
			FIFO_ADD_U8 (e, (uint8)XCP_PID_ERR);
			FIFO_ADD_U8 (e, (uint8)XCP_ERR_SEQUENCE);
			FIFO_ADD_U8 (e, Xcp_Unlock.key_rem);
		}
		return E_OK; /*lint !e904 allow multiple exit */
	}

	if(rem > len - 1)
	rem = len - 1;

	Xcp_MemCpy(   Xcp_Unlock.key + Xcp_Unlock.key_len - Xcp_Unlock.key_rem
			, (uint8 *)data+1
			, rem);

	Xcp_Unlock.key_rem -= rem;

	if(Xcp_Unlock.key_rem == 0) {
		if(xcpPtr->XcpUnlockFn == NULL) {
			RETURN_ERROR(XCP_ERR_GENERIC, "No unlock function defines");
		}

		if(xcpPtr->XcpUnlockFn( Xcp_Unlock.res
						         , Xcp_Unlock.seed
						         , Xcp_Unlock.seed_len
						         , Xcp_Unlock.key
						         , Xcp_Unlock.key_len) == E_OK) {
			xcpPtr->rt->XcpProtect &= ~Xcp_Unlock.res;
		} else {
			RETURN_ERROR(XCP_ERR_ACCESS_LOCKED, "Failed to unlock resource");
		}

	}
	RETURN_SUCCESS();
}
#endif

/**************************************************************************/
/**************************************************************************/
/*************************** COMMAND PROCESSOR ****************************/
/**************************************************************************/
/**************************************************************************/

/**
 * Structure holding a map between command codes and the function
 * implementing the command
 */
static const Xcp_CmdListType Xcp_CmdList[256-XCP_PID_CMD_STIM_LAST] =
{ 	    [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_CONNECT] =    {.fun = Xcp_CmdConnect, .len = 1 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_DISCONNECT] = {.fun = Xcp_CmdDisconnect, .len = 0 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_GET_STATUS] = {.fun = Xcp_CmdGetStatus, .len = 0 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_GET_ID] =     {.fun = Xcp_CmdGetId, .len = 1 },

        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_UPLOAD] =       {.fun = Xcp_CmdUpload, .len = 1 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_SHORT_UPLOAD] = {.fun = Xcp_CmdShortUpload, .len = 8 },

        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_SET_MTA] =             {.fun = Xcp_CmdSetMTA, .len = 3 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_SYNCH] =               {.fun = Xcp_CmdSync, .len = 0 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_GET_COMM_MODE_INFO] =  {.fun = Xcp_CmdGetCommModeInfo, .len = 0 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_BUILD_CHECKSUM] =      {.fun = Xcp_CmdBuildChecksum, .len = 8 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_TRANSPORT_LAYER_CMD] = {.fun = Xcp_CmdTransportLayer, .len = 1 },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_USER_CMD] =            {.fun = Xcp_CmdUser, .len = 0 },

#if(XCP_FEATURE_PROTECTION == STD_ON)
        /** @req 4.1.3/SWS_Xcp_00766 *//*The AUTOSAR XCP Module shall support the feature Seed & Key*/
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_GET_SEED] = {.fun = Xcp_CmdGetSeed, .len = 0},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_STD_UNLOCK] =   {.fun = Xcp_CmdUnlock,  .len = 3},
#endif

#if(XCP_FEATURE_PGM == STD_ON)
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_PROGRAM_START] =          {.fun = Xcp_CmdProgramStart,        .len = 0, .lock = XCP_PROTECT_PGM},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_PROGRAM_CLEAR] =          {.fun = Xcp_CmdProgramClear,        .len = 7, .lock = XCP_PROTECT_PGM},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_PROGRAM] =                {.fun = Xcp_CmdProgram,             .len = 2, .lock = XCP_PROTECT_PGM},
#if (XCP_FEATURE_BLOCKMODE == STD_ON)
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_PROGRAM_NEXT] =           {.fun = Xcp_CmdProgram,             .len = 2, .lock = XCP_PROTECT_PGM},
#endif /* XCP_FEATURE_BLOCKMODE == STD_ON */
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_PROGRAM_RESET] =          {.fun = Xcp_CmdProgramReset,        .len = 0, .lock = XCP_PROTECT_PGM},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_GET_PGM_PROCESSOR_INFO] = {.fun = Xcp_CmdGetPgmProcessorInfo, .len = 0, .lock = XCP_PROTECT_PGM},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_GET_SECTOR_INFO] =        {.fun = Xcp_CmdGetSectorInfo,       .len = 2, .lock = XCP_PROTECT_PGM},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_PROGRAM_PREPARE] =        {.fun = Xcp_CmdProgramPrepare,      .len = 3, .lock = XCP_PROTECT_PGM},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_PROGRAM_FORMAT] =         {.fun = Xcp_CmdProgramFormat,       .len = 4, .lock = XCP_PROTECT_PGM},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PGM_PROGRAM_VERIFY] =         {.fun = Xcp_CmdProgramVerify,       .len = 7, .lock = XCP_PROTECT_PGM},
#endif /* XCP_FEATURE_PGM == STD_ON */

#if(XCP_FEATURE_CALPAG == STD_ON)
        /** @req 4.1.3/SWS_Xcp_00708 *//*The AUTOSAR XCP Module shall support Online memory calibration (read / write access).*/
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PAG_SET_CAL_PAGE] =           {.fun = Xcp_CmdSetCalPage          , .len = 3, .lock = XCP_PROTECT_CALPAG},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PAG_GET_CAL_PAGE] =           {.fun = Xcp_CmdGetCalPage          , .len = 2, .lock = XCP_PROTECT_CALPAG},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PAG_GET_PAG_PROCESSOR_INFO] = {.fun = Xcp_CmdGetPagProcessorInfo , .len = 0, .lock = XCP_PROTECT_CALPAG},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PAG_GET_SEGMENT_INFO] =       {.fun = Xcp_CmdGetSegmentInfo      , .len = 3, .lock = XCP_PROTECT_CALPAG},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PAG_GET_PAGE_INFO] =          {.fun = Xcp_CmdGetPageInfo         , .len = 3, .lock = XCP_PROTECT_CALPAG},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PAG_SET_SEGMENT_MODE] =       {.fun = Xcp_CmdSetSegmentMode      , .len = 2, .lock = XCP_PROTECT_CALPAG},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PAG_GET_SEGMENT_MODE] =       {.fun = Xcp_CmdGetSegmentMode      , .len = 2, .lock = XCP_PROTECT_CALPAG},
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_PAG_COPY_CAL_PAGE] =          {.fun = Xcp_CmdCopyCalPage         , .len = 4, .lock = XCP_PROTECT_CALPAG},
#endif /* XCP_FEATURE_CALPAG == STD_ON */

        /** @req 4.1.3/SWS_Xcp_00708 *//*The AUTOSAR XCP Module shall support Online memory calibration (read / write access).*/
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_CAL_DOWNLOAD] = { .fun = Xcp_CmdDownload, .len = 3, .lock = XCP_PROTECT_CALPAG },
#if(XCP_FEATURE_BLOCKMODE == STD_ON)
        /** @req 4.1.3/SWS_Xcp_00711 *//*The AUTOSAR XCP Module shall support the feature Block communication mode*/
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_CAL_DOWNLOAD_NEXT] = { .fun = Xcp_CmdDownload, .len = 3, .lock = XCP_PROTECT_CALPAG },
#endif

        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_CLEAR_DAQ_LIST] =          { .fun = Xcp_CmdClearDaqList, .len = 3, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_SET_DAQ_PTR] =             { .fun = Xcp_CmdSetDaqPtr, .len = 5, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_WRITE_DAQ] =               { .fun = Xcp_CmdWriteDaq, .len = 7, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_SET_DAQ_LIST_MODE] =       { .fun = Xcp_CmdSetDaqListMode, .len = 7, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_GET_DAQ_LIST_MODE ] =      { .fun = Xcp_CmdGetDaqListMode, .len = 3, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_START_STOP_DAQ_LIST] =     { .fun = Xcp_CmdStartStopDaqList, .len = 3, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_START_STOP_SYNCH] =        { .fun = Xcp_CmdStartStopSynch, .len = 1, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_GET_DAQ_CLOCK] =           { .fun = Xcp_CmdGetDaqClock, .len = 0, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_READ_DAQ] =                { .fun = Xcp_CmdReadDaq, .len = 0, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_GET_DAQ_PROCESSOR_INFO] =  { .fun = Xcp_CmdGetDaqProcessorInfo, .len = 0, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_GET_DAQ_RESOLUTION_INFO] = { .fun = Xcp_CmdGetDaqResolutionInfo, .len = 0, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_GET_DAQ_LIST_INFO] =       { .fun = Xcp_CmdGetDaqListInfo, .len = 3, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_GET_DAQ_EVENT_INFO] =      { .fun = Xcp_CmdGetDaqEventInfo, .len = 3, .lock = XCP_PROTECT_DAQ },
#if(XCP_DAQ_CONFIG_TYPE == DAQ_DYNAMIC)
        /** @req 4.1.3/SWS_Xcp_00706 *//*The AUTOSAR XCP Module shall support Dynamic DAQ Configuration.*/
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_FREE_DAQ] =        { .fun = Xcp_CmdFreeDaq, .len = 0, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_ALLOC_DAQ] =       { .fun = Xcp_CmdAllocDaq, .len = 3, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_ALLOC_ODT] =       { .fun = Xcp_CmdAllocOdt, .len = 4, .lock = XCP_PROTECT_DAQ },
        [-XCP_PID_CMD_STIM_LAST+XCP_PID_CMD_DAQ_ALLOC_ODT_ENTRY] = { .fun = Xcp_CmdAllocOdtEntry, .len = 5, .lock = XCP_PROTECT_DAQ }
#endif
        };

/**
 * Xcp_Recieve_Main is the main process that executes all received commands.
 *
 * The function queues up replies for transmission. Which will be sent
 * when Xcp_Transmit_Main function is called.
 */
static void Xcp_Recieve_Main(void) {
    FIFO_FOR_READ(Xcp_FifoRxCto, it) {
        uint8 pid = GET_UINT8(it->data, 0);

        /* ignore commands when we are not connected */
        if (!Xcp_Connected && pid != XCP_PID_CMD_STD_CONNECT && pid != XCP_PID_CMD_STD_TRANSPORT_LAYER_CMD) {
            continue;
        }

        /* process standard commands */
        const Xcp_CmdListType* cmd = Xcp_CmdList + (pid - XCP_PID_CMD_STIM_LAST);
        if (pid > XCP_PID_CMD_STIM_LAST && cmd->fun) {

#if(XCP_FEATURE_PROTECTION == STD_ON)
		    if(cmd->lock & xcpPtr->rt->XcpProtect) {
				Xcp_TxError(XCP_ERR_ACCESS_LOCKED);
				continue;
			}
#endif /*XCP_FEATURE_PROTECTION == STD_ON*/

		   if (cmd->len && it->len < cmd->len) {
				DEBUG(DEBUG_HIGH, "Xcp_RxIndication_Main - Len %d to short for %u\n", it->len, pid);
				return; /*lint !e904 allow multiple exit */
			}
		   (void) cmd->fun(pid, it->data + 1, it->len - 1);
		} else {
			Xcp_TxError(XCP_ERR_CMD_UNKNOWN);
		}
	}
}


/**
 * Xcp_TxConfirmation function is responsible to initiate a new message sending.
 * Xcp communication layer (XcpOnCan or XcpOnEth) store that message and handles
 * the sending to the communication channel and wait for the confirmation.
 */
void Xcp_TxConfirmation(void) {

    Xcp_BufferType* it;
    Std_ReturnType retVal;

    if (NULL != (it = Xcp_Fifo_Get(&Xcp_FifoTxCto))) {
    	Xcp_TxOngoing = 3;
        retVal = Xcp_Transmit(it->data, it->len);
        Xcp_Fifo_Free(&Xcp_FifoTxCto, it);
        if (E_OK != retVal) {
            Xcp_TxOngoing = 0;
            DEBUG(DEBUG_HIGH, "Xcp_Transmit_Main - failed to transmit\n");
        }
    } else if (Xcp_DaqTxList) {
    	Xcp_TxOngoing = 3;
        Xcp_ProcessDaq_DAQ(Xcp_DaqTxList);
    } else {
        Xcp_TxOngoing = 0;
    }
}

/**
 * Scheduled function of the event channel
 * @param channel
 */
void Xcp_MainFunction_Channel(uint32 channel) {

    if(!Xcp_Inited)
    {
        DET_REPORTERROR(XCP_API_ID_XCP_MAINFUNCTION,XCP_E_NOT_INITIALIZED);
        return; /*lint !e904 allow multiple exit */
    }
    else
    {
        DET_VALIDATE_NRV(channel < XCP_EVENT_COUNT, XCP_API_ID_XCP_MAINFUNCTION, XCP_E_INVALID_EVENT);
        Xcp_ProcessChannel(channel);
    }
}

/**
 * Scheduled function of the XCP module
 *
 * ServiceId: 0x04
 *
 */
/** @req 4.1.3/SWS_Xcp_00823 *//*Xcp_MainFunction API definition*/
/** @req 4.1.3/SWS_Xcp_00824 *//*Xcp_MainFunction shall be called cyclically - can not be tested with conventional module tests*/
void Xcp_MainFunction(void) {
    DET_VALIDATE_NRV(Xcp_Inited, XCP_API_ID_XCP_MAINFUNCTION, XCP_E_NOT_INITIALIZED);

    /* check if we have some queued worker */
    if (Xcp_Worker) {
        Xcp_Worker();
    } else {
        Xcp_Recieve_Main();
    }

    Xcp_Fifo_Lock();
    if(!Xcp_TxOngoing && Xcp_FifoTxCto.front) { // this is an uggly fix to ensure that command responses are sent, move to commands instead
        Xcp_Fifo_Unlock();
        Xcp_TxConfirmation();
    } else {
    	if(Xcp_TxOngoing){
    		Xcp_TxOngoing--;
    		if(Xcp_TxOngoing == 0) {
#if (XCP_PROTOCOL == XCP_PROTOCOL_CAN)
    			Xcp_CancelTxRequests();
#endif
    		}
    	}
        Xcp_Fifo_Unlock();
    }
}

