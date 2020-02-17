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

#include "Xcp_Internal.h"

/**************************************************************************/
/**************************************************************************/
/*************************** PGM COMMANDS *********************************/
/**************************************************************************/
/**************************************************************************/
#if(XCP_FEATURE_PGM == STD_ON)

typedef struct {
    int    started;
    uint8  format;
    uint32 rem;
    uint32 len;
} XcpProgramType;

XcpProgramType Xcp_Program;

Std_ReturnType Xcp_CmdProgramStart(uint8 pid, uint8* data, PduLengthType len)
{
    uint8 commModePgm = 0x00u
#if (XCP_FEATURE_BLOCKMODE == STD_ON)
            | 1 << 0 /* MASTER_BLOCK_MODE */
            | 1 << 6 /* SLAVE_BLOCK_MODE */
#endif /*XCP_FEATURE_BLOCKMODE == STD_ON)*/
            | 0 << 1; /* INTERLEAVED_MODE */

    DEBUG(DEBUG_HIGH, "Received ProgramStart\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(len);

    FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
        SET_UINT8 (e->data, 0, XCP_PID_RES);
        SET_UINT8 (e->data, 1, 0); /* RESERVED */
        SET_UINT8 (e->data, 2, commModePgm);
        SET_UINT8 (e->data, 3, XCP_MAX_CTO); /* MAX_CTO_PGM */
        SET_UINT8 (e->data, 4, XCP_MAX_RXTX_QUEUE-1); /* MAX_BS_PGM */
        SET_UINT8 (e->data, 5, 0); /* MIN_ST_PGM [100 microseconds] */
        SET_UINT8 (e->data, 6, XCP_MAX_RXTX_QUEUE-1); /* QUEUE_SIZE_PGM */
        e->len = 7;
    }
    Xcp_Program.started = 1;

    return E_OK;
}

Std_ReturnType Xcp_CmdProgramClear(uint8 pid, uint8* data, PduLengthType len)
{
    Std_ReturnType retVal;
    Xcp_ErrorType error;
    uint8  mode  = GET_UINT8 (data, 0);
    uint32 clearRange = GET_UINT32(data, 3);

    DEBUG(DEBUG_HIGH, "Received ProgramClear %u, %u\n", mode, clearRange);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if(!Xcp_Program.started) {
        RETURN_ERROR(XCP_ERR_GENERIC, "Xcp_CmdProgramClear - programming not started\n");
    }

    if(mode == 0x01) { /* functional access mode */
        retVal = xcpPtr->XcpProgramClear_FunctionalAccess(clearRange, &error);

    }
    else if(mode == 0x00) { /* absolute access mode */
        retVal = xcpPtr->XcpProgramClear_AbsoluteAccess(&Xcp_Mta, clearRange, &error);
    }
    else {
        RETURN_ERROR(XCP_ERR_CMD_SYNTAX, "Xcp_CmdProgramClear - Mode argument is invalid\n");
    }

    if(retVal == E_OK) {
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdProgram(uint8 pid, uint8* data, PduLengthType len)
{
    Std_ReturnType retVal;
    Xcp_ErrorType error;
    uint32 rem = GET_UINT8(data, 0) * XCP_ELEMENT_SIZE;
    uint32 off = XCP_ELEMENT_OFFSET(2) + 1;

    DEBUG(DEBUG_HIGH, "Received Program %d, %d\n", pid, len);

    if(!Xcp_Program.started) {
        RETURN_ERROR(XCP_ERR_GENERIC, "Xcp_CmdProgram - programming not started\n");
    }

#if (XCP_FEATURE_BLOCKMODE == STD_OFF)
    if(rem + off > len) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Xcp_CmdProgram - Invalid length (%d, %d, %d)\n", rem, off, len);
    }
#endif /*XCP_FEATURE_BLOCKMODE == STD_OFF*/

    if(pid == XCP_PID_CMD_PGM_PROGRAM) {
        Xcp_Program.len = rem;
        Xcp_Program.rem = rem;
    }

    /* check for sequence error */
    if(Xcp_Program.rem != rem) {
        DEBUG(DEBUG_HIGH, "Xcp_CmdProgram - Invalid next state (%u, %u)\n", rem, (unsigned)Xcp_Program.rem);
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            SET_UINT8 (e->data, 0, XCP_PID_ERR);
            SET_UINT8 (e->data, 1, XCP_ERR_SEQUENCE);
            SET_UINT8 (e->data, 2, Xcp_Program.rem / XCP_ELEMENT_SIZE);
            e->len = 3;
        }
        return E_OK; /*lint !e904 allow multiple exit */
    }

    /* write what we got this packet */
    if(rem > len - off) {
        rem = len - off;
    }

    /* write actual data to flash via callout function */
    retVal = xcpPtr->XcpProgram(&Xcp_Mta,(uint8*) data + off, rem, &error);

    if(retVal == E_OK) {
        Xcp_Program.rem -= rem;
        if(Xcp_Program.rem) {
            return E_OK; /*lint !e904 allow multiple exit */
        }
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdProgramReset(uint8 pid, uint8* data, PduLengthType len)
{
    Xcp_ErrorType error;
    Std_ReturnType retVal;

    XCP_UNUSED(pid);
    XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(len);

    if(!Xcp_Program.started) {
        RETURN_ERROR(XCP_ERR_GENERIC, "Xcp_CmdProgramReset - programming not started\n");
    }

    Xcp_Program.started = 0;
    Xcp_Connected = 0;

    retVal = xcpPtr->XcpProgramReset(&error);

    if(retVal == E_OK) {
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdGetPgmProcessorInfo(uint8 pid, uint8* data, PduLengthType len)
{
    Xcp_ErrorType error;
    Std_ReturnType retVal;
    uint8 pgmProperties;
    uint8 maxSector

    DEBUG(DEBUG_HIGH, "Received GetPgmProcessorInfo");

    XCP_UNUSED(pid);
    XCP_UNUSED(len);
    XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */

    retVal = xcpPtr->XcpGetPgmProcessorInfo(&pgmProperties, &maxSector, &error);

    if(E_OK == retVal) {
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, pgmProperties);
            FIFO_ADD_U8 (e, maxSector);
        }
        return E_OK; /*lint !e904 allow multiple exit */
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdGetSectorInfo(uint8 pid, uint8* data, PduLengthType len)
{
    Xcp_ErrorType error;
    uint8 mode  = GET_UINT8(data, 0);
    uint8 sectorNumber = GET_UINT8(data, 1);

    DEBUG(DEBUG_HIGH, "Received GetSectorInfo(%u, %u)\n", mode,sectorNumber);

    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if ((mode == 0) || (mode == 1)) {
        Xcp_GetSectorInfo_Mode0_1_ReturnType responseData;
        if(E_OK == xcpPtr->XcpGetSectorInfo_Mode0_1(
                mode, sectorNumber, &responseData, &error)) {
            FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
                FIFO_ADD_U8 (e, XCP_PID_RES);
                FIFO_ADD_U8 (e, responseData.clearSeqNum);
                FIFO_ADD_U8 (e, responseData.programSeqNum);
                FIFO_ADD_U8 (e, responseData.programMethod);
                FIFO_ADD_U32 (e, responseData.sectorInfo);
            }
            return E_OK; /*lint !e904 allow multiple exit */
        } else {
            RETURN_ERROR(error);
        }
    } else if (mode == 2) {
        uint8 sectorNameLength;
        if(E_OK == xcpPtr->XcpGetSectorInfo_Mode2(&sectorNameLength, &error)) {
            FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
                FIFO_ADD_U8 (e, XCP_PID_RES);
                FIFO_ADD_U8 (e, sectorNameLength);
            }
            return E_OK; /*lint !e904 allow multiple exit */
        } else {
            RETURN_ERROR(error);
        }
    } else {
        RETURN_ERROR(XCP_ERR_CMD_SYNTAX, "Xcp_CmdGetSectorInfo - Mode argument is invalid\n");
    }
}

Std_ReturnType Xcp_CmdProgramPrepare(uint8 pid, uint8* data, PduLengthType len)
{
    Xcp_ErrorType error;
    Std_ReturnType retVal;
    uint16 codeSize = GET_UINT16(data, 1);

    DEBUG(DEBUG_HIGH, "Received ProgramPrepare(%u)\n", codeSize);

    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpProgramPrepare(&Xcp_Mta, codeSize, &error);

    if(retVal == E_OK) {
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdProgramFormat(uint8 pid, uint8* data, PduLengthType len)
{
    Xcp_ErrorType error;
    Std_ReturnType retVal;
    uint8 compressionMethod = GET_UINT8(data, 0);
    uint8 encryptionMethod  = GET_UINT8(data, 1);
    uint8 programmingMethod = GET_UINT8(data, 2);
    uint8 accessMethod      = GET_UINT8(data, 3);

    DEBUG(DEBUG_HIGH, "Received ProgramFormat(%u, %u, %u, %u)\n",
            compressionMethod,encryptionMethod,programmingMethod,accessMethod);

    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpProgramFormat(compressionMethod,encryptionMethod,programmingMethod,accessMethod, &error);

    if(retVal == E_OK) {
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdProgramVerify(uint8 pid, uint8* data, PduLengthType len)
{
    Xcp_ErrorType error;
    Std_ReturnType retVal;
    uint8  mode = GET_UINT8(data, 0);
    uint16 type = GET_UINT16(data, 1);
    uint32 value = GET_UINT32(data, 3);
    DEBUG(DEBUG_HIGH, "Received ProgramVerify(%u, %u, %u)\n", mode, type, value);

    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpProgramVerify(mode, type, value, &error);

    if(retVal == E_OK) {
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

#endif /*XCP_FEATURE_PGM == STD_ON*/
