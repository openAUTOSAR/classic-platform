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

#if(XCP_FEATURE_PGM)

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
#endif
            | 0 << 1; /* INTERLEAVED_MODE */

    DEBUG(DEBUG_HIGH, "Received program_start\n");
    FIFO_GET_WRITE(Xcp_FifoTx, e) {
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

    XCP_UNUSED(pid);
    XCP_UNUSED(data);
    XCP_UNUSED(len);
    return E_OK;
}

Std_ReturnType Xcp_CmdProgramClear(uint8 pid, uint8* data, PduLengthType len)
{
    uint8  mode  = GET_UINT8 (data, 0);
    uint32 range = GET_UINT32(data, 3);
    DEBUG(DEBUG_HIGH, "Received program_clear %u, %u\n", (unsigned)mode, (unsigned)range);
    if(!Xcp_Program.started) {
        RETURN_ERROR(XCP_ERR_GENERIC, "Xcp_CmdProgramClear - programming not started\n");
    }

    if(mode == 0x01) { /* functional access mode */
        if(range & 0x01) { /* clear all calibration data areas */

        }
        if(range & 0x02) { /* clear all code areas, except boot */

        }
        if(range & 0x04) { /* clear all code NVRAM areas */

        }
    }

    if(mode == 0x00) { /* absolute access mode */

    }

    XCP_UNUSED(pid);
    XCP_UNUSED(len);
    RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Xcp_CmdProgramClear - mode implemented\n");
}

Std_ReturnType Xcp_CmdProgram(uint8 pid, uint8* data, PduLengthType len)
{
    unsigned rem = GET_UINT8(data, 0) * XCP_ELEMENT_SIZE;
    unsigned off = XCP_ELEMENT_OFFSET(2) + 1;
    DEBUG(DEBUG_HIGH, "Received program %d, %d\n", pid, len);
    if(!Xcp_Program.started) {
        RETURN_ERROR(XCP_ERR_GENERIC, "Xcp_CmdProgramClear - programming not started\n");
    }

#if(!XCP_FEATURE_BLOCKMODE)
    if(rem + off > len) {
        RETURN_ERROR(XCP_ERR_OUT_OF_RANGE, "Xcp_CmdProgram - Invalid length (%d, %d, %d)\n", rem, off, len);
    }
#endif

    if(pid == XCP_PID_CMD_PGM_PROGRAM) {
        Xcp_Program.len = rem;
        Xcp_Program.rem = rem;
    }

    /* check for sequence error */
    if(Xcp_Program.rem != rem) {
        DEBUG(DEBUG_HIGH, "Xcp_CmdProgram - Invalid next state (%u, %u)\n", rem, (unsigned)Xcp_Program.rem);
        FIFO_GET_WRITE(Xcp_FifoTx, e) {
            SET_UINT8 (e->data, 0, XCP_PID_ERR);
            SET_UINT8 (e->data, 1, XCP_ERR_SEQUENCE);
            SET_UINT8 (e->data, 2, Xcp_Program.rem / XCP_ELEMENT_SIZE);
            e->len = 3;
        }
        return E_OK;
    }

    /* write what we got this packet */
    if(rem > len - off) {
        rem = len - off;
    }

    /* NOT IMPLEMENTED - write actual data to flash */

    Xcp_Program.rem -= rem;

    if(Xcp_Program.rem)
        return E_OK;

    RETURN_SUCCESS();
}

Std_ReturnType Xcp_CmdProgramReset(uint8 pid, uint8* data, PduLengthType len)
{
    XCP_UNUSED(pid);
    XCP_UNUSED(data);
    XCP_UNUSED(len);

    if(!Xcp_Program.started) {
        RETURN_ERROR(XCP_ERR_GENERIC, "Xcp_CmdProgramClear - programming not started\n");
    }

    RETURN_ERROR(XCP_ERR_CMD_UNKNOWN, "Xcp_CmdProgramReset - not implemented\n");
}

typedef enum {
    XCP_PGM_PROPERTY_ABSOLUTE_MODE         = 1 << 0,
    XCP_PGM_PROPERTY_FUNCTIONAL_MODE       = 1 << 1,
    XCP_PGM_PROPERTY_COMPRESSION_SUPPORTED = 1 << 2,
    XCP_PGM_PROPERTY_COMPRESSION_REQUIRED  = 1 << 3,
    XCP_PGM_PROPERTY_ENCRYPTION_SUPPORTED  = 1 << 4,
    XCP_PGM_PROPERTY_ENCRYPTION_REQUIRED   = 1 << 5,
    XCP_PGM_PROPERTY_NON_SEQ_PGM_SUPPORTED = 1 << 6,
    XCP_PGM_PROPERTY_NON_SEQ_PGM_REQUIRED  = 1 << 7,
} Xcp_ProgramPropertiesType;


Std_ReturnType Xcp_CmdProgramInfo(uint8 pid, uint8* data, PduLengthType len)
{
    XCP_UNUSED(pid);
    XCP_UNUSED(data);
    XCP_UNUSED(len);

    FIFO_GET_WRITE(Xcp_FifoTx, e) {
        FIFO_ADD_U8 (e, XCP_PID_RES);
        FIFO_ADD_U8 (e, XCP_PGM_PROPERTY_FUNCTIONAL_MODE
                      | XCP_PGM_PROPERTY_NON_SEQ_PGM_SUPPORTED); /* PGM_PROPERTIES */
        FIFO_ADD_U8 (e, 0); /* MAX_SECTOR */
    }

    return E_OK;
}

#endif /*XCP_FEATURE_PGM*/
