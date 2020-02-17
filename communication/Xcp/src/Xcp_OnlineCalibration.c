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
/*************************** CAL/PAG COMMANDS *****************************/
/**************************************************************************/
/**************************************************************************/
#if(XCP_FEATURE_CALPAG == STD_ON)

Std_ReturnType Xcp_CmdSetCalPage(uint8 pid, uint8* data, PduLengthType len)
{
    Xcp_ErrorType  error;
    Std_ReturnType retVal;
    uint32 mode = GET_UINT8(data, 0);
    uint32 segm = GET_UINT8(data, 1);
    uint32 page = GET_UINT8(data, 2);

    DEBUG(DEBUG_HIGH, "Received SetCalPage(0x%x, %u, %u)\n", mode, segm, page);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpSetCalPage(mode, segm, page, &error);

    if(retVal == E_OK) {
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdGetCalPage(uint8 pid, uint8* data, PduLengthType len)
{
    uint8 mode = GET_UINT8(data, 0);
    uint8 segm = GET_UINT8(data, 1);
    uint8 page = 0;
    Xcp_ErrorType error;
    Std_ReturnType retVal;

    DEBUG(DEBUG_HIGH, "Received GetCalPage(0x%x, %u)\n", mode, segm);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpGetCalPage(mode, segm, &page, &error);

    if(retVal == E_OK) {
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, 0); /* reserved */
            FIFO_ADD_U8 (e, 0); /* reserved */
            FIFO_ADD_U8 (e, page);
        }
        return E_OK; /*lint !e904 allow multiple exit */
    } else {
        RETURN_ERROR(error);
    }

}

Std_ReturnType Xcp_CmdCopyCalPage(uint8 pid, uint8* data, PduLengthType len)
{
    uint8 sourceSegm = GET_UINT8(data, 0);
    uint8 sourcePage = GET_UINT8(data, 1);
    uint8 destSegm = GET_UINT8(data, 2);
    uint8 destPage = GET_UINT8(data, 3);
    Xcp_ErrorType error;
    Std_ReturnType retVal;

    DEBUG(DEBUG_HIGH, "Received CopyCalPage(0x%x, %u)\n", mode, segm);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpCopyCalPage(sourceSegm, sourcePage, destSegm, destPage, &error);

    if(retVal == E_OK) {
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdGetPagProcessorInfo(uint8 pid, uint8* data, PduLengthType len)
{
    uint8 maxSegment;
    Xcp_ErrorType error;
    Std_ReturnType retVal;

    DEBUG(DEBUG_HIGH, "Received GetPagProcessorInfo\n");
    XCP_UNUSED(pid);
    XCP_UNUSED(data);/*lint !e920 Misra 2012 1.3 cast from pointer to void */
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpGetPagProcessorInfo(&maxSegment, &error);

    if(retVal == E_OK) {
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, maxSegment);
            FIFO_ADD_U8 (e, 0 << 0 /* FREEZE_SUPPORTED */);
        }
        return E_OK; /*lint !e904 allow multiple exit */
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdGetSegmentInfo(uint8 pid, uint8* data, PduLengthType len)
{
    uint8 mode = GET_UINT8(data, 0);
    uint8 segmentNumber = GET_UINT8(data, 1);
    Xcp_ErrorType error;

    DEBUG(DEBUG_HIGH, "Received GetSegmentInfo(%u, %u, %u, %u)\n", mode, segm, info, mapi);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    if(mode == 0) {
        uint8 segmentInfo = GET_UINT8(data, 2);
        uint32 basicInfo;

        if(E_OK == xcpPtr->XcpGetSegmentInfo_GetBasicAddress(segmentNumber,segmentInfo,&basicInfo,&error)) {

            FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
                FIFO_ADD_U8 (e, XCP_PID_RES);
                FIFO_ADD_U8 (e, 0); /* reserved */
                FIFO_ADD_U8 (e, 0); /* reserved */
                FIFO_ADD_U8 (e, 0); /* reserved */
                FIFO_ADD_U32(e, basicInfo);
            }

        } else {
            RETURN_ERROR(error);
        }

    } else if (mode == 1) {
        Xcp_SegmentStandardInfoReturnType responseData;

        if(E_OK == xcpPtr->XcpGetSegmentInfo_GetStandardInfo(segmentNumber,&responseData,&error)) {

            FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
                FIFO_ADD_U8 (e, XCP_PID_RES);
                FIFO_ADD_U8 (e, responseData.maxPages);
                FIFO_ADD_U8 (e, responseData.addressExtension);
                FIFO_ADD_U8 (e, responseData.maxMapping);
                FIFO_ADD_U8 (e, responseData.compressionMethod);
                FIFO_ADD_U8 (e, responseData.encryptionMethod);
            }
        } else {
            RETURN_ERROR(error);
        }

    } else if (mode == 2) {

        uint8 segmentInfo = GET_UINT8(data, 2);
        uint32 mappingInfo;

        if(E_OK == xcpPtr->XcpGetSegmentInfo_GetAddressMapping(segmentNumber,segmentInfo, &mappingInfo,&error)) {

            FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
                FIFO_ADD_U8 (e, XCP_PID_RES);
                FIFO_ADD_U8 (e, 0); /* reserved */
                FIFO_ADD_U8 (e, 0); /* reserved */
                FIFO_ADD_U8 (e, 0); /* reserved */
                FIFO_ADD_U32(e, mappingInfo);
            }
        } else {
            RETURN_ERROR(error);
        }

    } else {
        RETURN_ERROR(XCP_ERR_CMD_SYNTAX, "Unsupported");
    }

    return E_OK;
}

Std_ReturnType Xcp_CmdGetPageInfo(uint8 pid, uint8* data, PduLengthType len)
{
    uint8 segmentNumber = GET_UINT8(data, 1);
    uint8 pageNumber = GET_UINT8(data, 2);
    uint8 pageProperties;
    uint8 initSegment;
    Xcp_ErrorType error;
    Std_ReturnType retVal;

    DEBUG(DEBUG_HIGH, "Received GetPageInfo(%u, %u)\n", segmentNumber, pageNumber);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpGetPageInfo(segmentNumber,pageNumber,&pageProperties,&initSegment,&error);

    if(retVal == E_OK) {
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, pageProperties);
            FIFO_ADD_U8 (e, initSegment);
        }
        return E_OK; /*lint !e904 allow multiple exit */
    } else {
        RETURN_ERROR(error);
    }

}

Std_ReturnType Xcp_CmdSetSegmentMode(uint8 pid, uint8* data, PduLengthType len)
{
    uint8 mode = GET_UINT8(data, 0);
    uint8 segmentNumber = GET_UINT8(data, 1);
    Xcp_ErrorType error;
    Std_ReturnType retVal;

    DEBUG(DEBUG_HIGH, "Received SetSegmentMode(%u, %u)\n", mode, segmentNumber);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpSetSegmentMode(mode, segmentNumber, &error);

    if(retVal == E_OK) {
        RETURN_SUCCESS();
    } else {
        RETURN_ERROR(error);
    }
}

Std_ReturnType Xcp_CmdGetSegmentMode(uint8 pid, uint8* data, PduLengthType len)
{
    uint8 segmentNumber = GET_UINT8(data, 1);
    uint8 mode;
    Xcp_ErrorType error;
    Std_ReturnType retVal;

    DEBUG(DEBUG_HIGH, "Received GetSegmentMode(%u)\n", segmentNumber);
    XCP_UNUSED(pid);
    XCP_UNUSED(len);

    retVal = xcpPtr->XcpGetSegmentMode(segmentNumber,&mode, &error);

    if(retVal == E_OK) {
        FIFO_GET_WRITE(Xcp_FifoTxCto, e) {
            FIFO_ADD_U8 (e, XCP_PID_RES);
            FIFO_ADD_U8 (e, 0); /*reserved*/
            FIFO_ADD_U8 (e, mode);
        }
        return E_OK; /*lint !e904 allow multiple exit */
    } else {
        RETURN_ERROR(error);
    }
}


#endif /*XCP_FEATURE_CALPAG == STD_ON*/
