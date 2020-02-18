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

#include "FrIf.h"
#include "FrIf_Internal.h"

extern FrIf_Internal_GlobalType FrIf_Internal_Global;
extern const FrIf_ConfigType *FrIf_ConfigCachePtr;

static void markUB(sint16 pduUBOffset, FrIf_PduUpdateBitType pduUBSts, uint8 *frSduPtr);
static FrIf_PduUpdateBitType readUB(sint16 pduUBOffset,const uint8 *frSduPtr);

/**
 * @brief Mark Update bit as updated/out dated
 * @param pduUBOffset    Used to locate the update bit.
 * @param pduUBSts       Value of the update bit.
 * @param frSduPtr       Pointer to SDU.
 *
 *
 */
static void markUB(sint16 pduUBOffset, FrIf_PduUpdateBitType pduUBSts, uint8 *frSduPtr)
{
    uint8 dataByte;
    uint8 bytePos;
    uint8 bitPos;

    if (pduUBOffset > -1) {
        bytePos = (uint8)((uint16)pduUBOffset / 8u);
        bitPos = (uint16)pduUBOffset % 8u;
        dataByte = frSduPtr[bytePos];

        if (pduUBSts == PDU_UPDATED) {
            /* set update bit as updated */
            dataByte |= (uint8)(1u << bitPos);
        } else {
            /* set update bit as outdated */
            dataByte = dataByte &  ~(1u << bitPos);
        }

        /* write back Pdu update info */
        frSduPtr[bytePos] = dataByte;
    }
}

/**
 * @brief Reads the Pdu Update Bit in the sdu.
 * @param pduUBOffset    Used to locate the update bit.
 * @param frSduPtr       Pointer to SDU.
 *
 */
static FrIf_PduUpdateBitType readUB(sint16 pduUBOffset, const uint8 *frSduPtr)
{
    uint8 dataByte;
    uint8 bytePos;
    uint8 bitPos;
    FrIf_PduUpdateBitType ret;

    if (pduUBOffset > -1) {
        bytePos = (uint8)((uint16)pduUBOffset / 8u);
        bitPos = (uint16)pduUBOffset % 8u;
        dataByte = frSduPtr[bytePos];
        if ((dataByte & (1u << bitPos)) > 0) {
            ret = PDU_UPDATED;
        } else {
            ret = PDU_OUTDATED;
        }
    } else {
        /* @req FrIf05128 */
        ret = PDU_UPDATED;
    }

    return ret;
}

/**
 * @brief Get the valid length of a received Pdu (Useful in the case of dynamic segment which can possibly receive LSdu length lesser than configured LSdu length)
 * @param RxPduLength - Configured length of the composite Pdu in a LSdu
 * @param pduOffset - Starting byte position of the composite Pdu
 * @param pduUBOffset - Offset of Pdu UB position
 * @param Fr_LSduLength - Total received LSdu
 * @return Computed length of the composite Pdu (For Static segment it is RxPduLength because received LSdu length is equal to configured LSdu length)
 */
#if (FR_RX_STRINGENT_LENGTH_CHECK == STD_OFF)
uint8 getValidLength(uint8 rxPduLength, uint8 pduOffset, sint16 pduUBOffset, uint8 lSduLen);
uint8 getValidLength(uint8 rxPduLength, uint8 pduOffset, sint16 pduUBOffset, uint8 lSduLen) {

    uint8 len;

    if ((pduOffset + rxPduLength) <= lSduLen) {
        len = rxPduLength;
    } else if ((pduOffset < lSduLen) && (pduUBOffset < pduOffset)){
        /* The UB position is either before the PduOffset or after an offset of PduLength. In
         * this condition we expect it to before PduOffset */
        len = lSduLen - pduOffset;
    } else {
        len = 0;
    }
    return len;
}
#endif

/**
 * @brief Transmits FlexRay frame by getting desired Pdus from upper layer PDuR.
 * @param frIfIdx        FrIf Controller id.
 * @param frLPduIdx      Fr LPdu Index
 * @param frameCfgIdx    FrameTriggering id.
 *
 *
 */
void FrIf_Internal_HandleDecoupledTransmission(uint8 frIfIdx,PduIdType frLPduIdx, uint8 frameCfgIdx)
{
    const FrIf_TxPduType * txPduCfgPtr;                     /* Pointer to Tx pdu cfg */
    const FrIf_FrameTriggeringType * frameTrigCfgPtr;       /* Frame Triggering cfg Ptr */
    const FrIf_FrAPIType *myFuncPtr;                        /* Pointer to Fr driver cfg */
    const FrIf_PdusIn_FrameType * pduInFramePtr;            /* Pdus in Frame cfg ptr */
    uint8 * tempBuffPtr;									/* Buffer pointer to temporary data */
    uint8 frCCIdx;                                          /* Fr CC index */
    uint8 frameStructIdx;                                   /* Frame structure index */
    FrIf_PduUpdateBitType pduSts;                           /* Base case if no PDU is updated */

    sint16 pduUBOffset;
    uint8 pduCnt;
    PduIdType pduId;
    uint8 pduOffset;
    boolean resFlag;
    Std_ReturnType ret;
    PduInfoType pduInfo;
#if (FRIF_BIG_ENDIAN_USED == STD_ON)
    uint8  tempBuffForBigEndian[FLEXRAY_FRAME_LEN_MAX] = {0};
    uint8  pduLastByteIndex = 0;
#endif

    frameTrigCfgPtr =  &(FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrameTriggeringPtr[frameCfgIdx]);
    frameStructIdx  =  frameTrigCfgPtr->FrIf_FrameStructureIdx;
    frCCIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrCtrlIdx;
    tempBuffPtr = FrIf_Internal_Global.lSduBuffer[frameStructIdx];
    pduSts = PDU_OUTDATED; /* Set initial LSdu state has outdated */

    /* @req FrIf05287 */
    /* Iterate over all PDU's in the FrameStructure */
    for (pduCnt = 0; pduCnt < frameTrigCfgPtr->FrIf_NumberPdusInFrame; pduCnt++) {

        ret = E_OK;
        pduInFramePtr = &(frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_PdusInFramePtr[pduCnt]);
        pduId = pduInFramePtr->FrIf_Pdu->FrIf_PduId;
        pduOffset = pduInFramePtr->FrIf_PduOffset;  /* Determine Pdu's offset in frame */
        pduUBOffset = pduInFramePtr->FrIf_PduUpdateBitOffset; /* @req FrIf05125 */ /* Determine bit to mark */
        txPduCfgPtr = pduInFramePtr->FrIf_Pdu->FrIf_PduDirectionPtr->FrIf_TxPduPtr;

        pduInfo.SduDataPtr = &tempBuffPtr[pduOffset];
#if (FRIF_BIG_ENDIAN_USED == STD_ON)
        if (FRIF_BIG_ENDIAN == frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_ByteOrder){
            pduInfo.SduDataPtr = & tempBuffForBigEndian[0]; /* re-ordering of buffer ptr to support big-endian */
            pduLastByteIndex = txPduCfgPtr->FrIf_TxLength-1;
        }
#endif
        pduInfo.SduLength  = txPduCfgPtr->FrIf_TxLength;

        resFlag = ((FrIf_Internal_Global.txPduStatistics[pduId].trigTxCounter > 0)
                || (TRUE == pduInFramePtr->FrIf_Pdu->FrIf_PduDirectionPtr->FrIf_TxPduPtr->FrIf_NoneMode));

        if (TRUE == resFlag) {
            /* Start trigger transmit process */
            if ((txPduCfgPtr->FrIf_UserTriggerTransmitHandle != FRIF_NO_FUNCTION_CALLOUT) &&
                    (NULL_PTR != FrIf_ConfigCachePtr->FrIf_TriggerTransmitFncs[txPduCfgPtr->FrIf_UserTriggerTransmitHandle]))
            {
                ret = FrIf_ConfigCachePtr->FrIf_TriggerTransmitFncs[txPduCfgPtr->FrIf_UserTriggerTransmitHandle](pduInFramePtr->FrIf_Pdu->FrIf_UpperLayerPduId, &pduInfo);
            }

            /* In case upper layer trigger transmit does not return E_OK reset the update-bit to "not updated" */
            if (ret != E_OK) {
                markUB(pduUBOffset, PDU_OUTDATED, tempBuffPtr);
            } else {
#if (FRIF_BIG_ENDIAN_USED == STD_ON)
                /*re-ordering is executed for big-endian format*/
                if (FRIF_BIG_ENDIAN == frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_ByteOrder){

                    for(uint8 i=0; i<=pduLastByteIndex; i++){
                        tempBuffPtr[pduOffset+i] = tempBuffForBigEndian[pduLastByteIndex-i];
                    }
                }
#endif
                /* mark this Pdu as updated */
                pduSts = PDU_UPDATED;
                /* Set update bit if configured for this PDU */
                markUB(pduUBOffset, PDU_UPDATED, tempBuffPtr);
            }
        } else {
            /* Clear update bit for the pdu and proceed with the next PDU */
            markUB(pduUBOffset, PDU_OUTDATED, tempBuffPtr);
        }
    }

    /* At least one Pdu is updated or the frame should always be transmitted */
    if ((pduSts == PDU_UPDATED) || (TRUE == frameTrigCfgPtr->FrIf_AlwaysTransmit)) {

        /* locate this driver api*/
        myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

        ret = (*myFuncPtr->Fr_TransmitTxLPdu)(frCCIdx, frLPduIdx, tempBuffPtr, frameTrigCfgPtr->FrIf_LSduLength);
        /* If transmission is OK, update trigTxCounter and txConfCounter */
        if (ret == E_OK) {

            for (pduCnt = 0; pduCnt < frameTrigCfgPtr->FrIf_NumberPdusInFrame; pduCnt++) {

                pduInFramePtr = &(frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_PdusInFramePtr[pduCnt]);
                pduId = pduInFramePtr->FrIf_Pdu->FrIf_PduId;

                /* @req FrIf05058 */ /* Decrement trigTxCounter only if trigTxCounter > 0 */
                if (FrIf_Internal_Global.txPduStatistics[pduId].trigTxCounter > 0) {
                    FrIf_Internal_Global.txPduStatistics[pduId].trigTxCounter--;
                }

                /*  Remember that a transmission for this PDU is pending if transmission confirmation is needed */
                resFlag = ((TRUE == pduInFramePtr->FrIf_Pdu->FrIf_PduDirectionPtr->FrIf_TxPduPtr->FrIf_Confirm)
                        && (FrIf_Internal_Global.txPduStatistics[pduId].txConfCounter
                                                < pduInFramePtr->FrIf_Pdu->FrIf_PduDirectionPtr->FrIf_TxPduPtr->FrIf_CounterLimit));
                if (TRUE == resFlag)  {
                    /* Only increment if counter limit has not been reached */
                    FrIf_Internal_Global.txPduStatistics[pduId].txConfCounter++;
                }
            }
        }
    }
}


/**
 * @brief Provides information about transmission status.
 * @param frIfIdx           FrIf Controller id.
 * @param frLPduIdx         LPdu id.
 * @param frameCfgIdx       Frame triggering id.
 *
 */
void FrIf_Internal_ProvideTxConfirmation(uint8 frIfIdx, PduIdType frLPduIdx, uint8 frameCfgIdx)
{

    const FrIf_TxPduType * txPduCfgPtr;                     /* Pointer to Tx pdu cfg */
    const FrIf_FrameTriggeringType * frameTrigCfgPtr;       /* Frame Triggering cfg Ptr */
    const FrIf_FrAPIType *myFuncPtr;                        /* Pointer to Fr driver cfg */
    const FrIf_PdusIn_FrameType * pduInFramePtr;            /* Pdus in Frame cfg ptr */
    uint8 frCCIdx;                                          /* Fr CC index */

    uint8 pduCnt;
    PduIdType pduId;
    Std_ReturnType ret;
    Fr_TxLPduStatusType frTxLPduStsPtr;

    frameTrigCfgPtr =  &(FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrameTriggeringPtr[frameCfgIdx]); /* Pointer to the Frame struct in the PB configuration file*/
    frCCIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrCtrlIdx;

    /* @req FrIf05288 */

    /* Locate this driver api */
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    ret = (*myFuncPtr->Fr_CheckTxLPduStatus)(frCCIdx, frLPduIdx, &frTxLPduStsPtr);

    /* If output parameter is equal to FR_TRANSMITTED, iterate over all PDUs in the frame structure */
    if ((E_OK == ret) && (frTxLPduStsPtr == FR_TRANSMITTED)) {

        for (pduCnt = 0; pduCnt < frameTrigCfgPtr->FrIf_NumberPdusInFrame; pduCnt++) {

            pduInFramePtr = &(frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_PdusInFramePtr[pduCnt]);
            pduId = pduInFramePtr->FrIf_Pdu->FrIf_PduId;

            if (FrIf_Internal_Global.txPduStatistics[pduId].txConfCounter == 0) {
                /* Do nothing */
            } else {
                txPduCfgPtr = pduInFramePtr->FrIf_Pdu->FrIf_PduDirectionPtr->FrIf_TxPduPtr;
                if (TRUE == txPduCfgPtr->FrIf_Confirm) {
                    /* Call upper layer Tx Confirmation */
                    if ((txPduCfgPtr->FrIf_TxConfirmationHandle != FRIF_NO_FUNCTION_CALLOUT) &&
                            (NULL_PTR != FrIf_ConfigCachePtr->FrIf_TxConfirmationFncs[txPduCfgPtr->FrIf_TxConfirmationHandle]))
                    {
                        FrIf_ConfigCachePtr->FrIf_TxConfirmationFncs[txPduCfgPtr->FrIf_TxConfirmationHandle](pduInFramePtr->FrIf_Pdu->FrIf_UpperLayerPduId);
                    }
                    /* Decrement txConfCounter */
                    FrIf_Internal_Global.txPduStatistics[pduId].txConfCounter--;
                } else {
                    /* Do nothing */
                }
            }
        }
    }
}


/**
 * @brief Receive a message and store it.
 * @param frIfIdx         FrIf controller id.
 * @param frLPduIdx       LPdu id.
 * @param frameCfgIdx     Frame triggering id.
 */
void FrIf_Internal_HandleReceiveAndStore(uint8 frIfIdx, PduIdType frLPduIdx, uint8 frameCfgIdx)
{
    const FrIf_FrameTriggeringType * frameTrigCfgPtr;       /* Frame Triggering cfg Ptr */
    const FrIf_FrAPIType  *myFuncPtr;						/* Driver pointer */
    const FrIf_PdusIn_FrameType * pduInFramePtr;            /* Pdus in Frame cfg ptr */
    const FrIf_RxPduType * rxPduCfgPtr;                     /* Pointer to Rx pdu cfg */
    uint8 * tempBuffPtr;									/* Pointer to temporary buffer */

    PduIdType pduId;
    sint16 pduUBOffset;
    uint8 pduOffset;
    Fr_RxLPduStatusType frRxLPduStsPtr; /* Declare the output parameter for Fr_ReceiveRxLPdu */
    uint8 frameStructIdx;                                   /* Frame structure index */
    Std_ReturnType ret;
    uint8 pduCnt;
    uint8 frCCIdx;                                          /* Fr CC index */
    uint8 rxLen;
    uint8 pduLen;
    uint8 * sduBuffPtr;

    /* @req FrIf05290 */
    frameTrigCfgPtr =  &(FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrameTriggeringPtr[frameCfgIdx]);
    frameStructIdx  =  frameTrigCfgPtr->FrIf_FrameStructureIdx;
    frCCIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrCtrlIdx;
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr); /* Locate this driver api */
    sduBuffPtr = FrIf_Internal_Global.lSduBuffer[frameStructIdx];

    tempBuffPtr = sduBuffPtr;
#if (FRIF_BIG_ENDIAN_USED == STD_ON)
    uint8 tempBuffForBigEndian[FLEXRAY_FRAME_LEN_MAX];

    if (frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_ByteOrder == FRIF_BIG_ENDIAN) {
       tempBuffPtr = &tempBuffForBigEndian[0];
    }
#endif

    /* Call Driver's APT function */
    ret = (*myFuncPtr->Fr_ReceiveRxLPdu)(frCCIdx, frLPduIdx, tempBuffPtr, &frRxLPduStsPtr, &rxLen);

    /* If a LPDU was received iterate over all PDUs in the frame structure */
    if ((E_OK == ret) && (frRxLPduStsPtr == FR_RECEIVED)) {

        /* Iterate over all PDU's in the FrameStructure */
        for (pduCnt = 0; pduCnt < frameTrigCfgPtr->FrIf_NumberPdusInFrame; pduCnt++) {

            pduInFramePtr = &(frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_PdusInFramePtr[pduCnt]);
            pduOffset = pduInFramePtr->FrIf_PduOffset;  /* Determine Pdu's offset in frame */
            pduId = pduInFramePtr->FrIf_Pdu->FrIf_PduId;/* Get the ID of the PDU in the frame */
            /* @req FrIf05125 */
            pduUBOffset = pduInFramePtr->FrIf_PduUpdateBitOffset; /* Determine UB pos  */
            rxPduCfgPtr = pduInFramePtr->FrIf_Pdu->FrIf_PduDirectionPtr->FrIf_RxPduPtr;

#if (FRIF_BIG_ENDIAN_USED == STD_ON)
            if (frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_ByteOrder == FRIF_BIG_ENDIAN) {
                FrIf_Internal_SwapToBigEndian(&tempBuffPtr[pduOffset],&sduBuffPtr[pduOffset], rxPduCfgPtr->RxPduLength);
            }
#endif

#if (FR_RX_STRINGENT_LENGTH_CHECK == STD_OFF)
            pduLen = getValidLength(rxPduCfgPtr->RxPduLength, pduOffset, pduUBOffset, rxLen);
#else
            pduLen = rxPduCfgPtr->RxPduLength;
#endif
            /* @req FrIf05056 */
            if ((readUB(pduUBOffset, tempBuffPtr) == PDU_UPDATED) && (pduLen > 0)) {
                /* Mark the PDU-related static buffer as up-to-date */
                FrIf_Internal_Global.rxPduStatistics[pduId].pduUpdated = TRUE;
                FrIf_Internal_Global.rxPduStatistics[pduId].pduRxLen = pduLen;
            }
        }
    }
}

/**
 * @brief Receives an FlexRay frame and indicates upper layer PduR by callback.
 * @param frIfIdx                            FrIf Controller id
 * @param frLPduIdx                          LPdu id
 * @param frameCfgIdx                        Frame Triggering id
 * @param maxLoop                            configured max number of loops for Receive and Indicate
 *
 */
void FrIf_Internal_HandleReceiveAndIndicate(uint8 frIfIdx, PduIdType frLPduIdx, uint8 frameCfgIdx, uint16 maxLoop )
{
    const FrIf_FrameTriggeringType * frameTrigCfgPtr;       /* Frame Triggering cfg Ptr */
    const FrIf_FrAPIType  *myFuncPtr;						/* Driver pointer */
    const FrIf_PdusIn_FrameType * pduInFramePtr;            /* Pdus in Frame cfg ptr */
    const FrIf_RxPduType * rxPduCfgPtr;                     /* Pointer to Rx pdu cfg */
    uint8 * tempBuffPtr;									/* Pointer to temporary buffer */
    uint16 ComOpLoopCounter;								/* Loop counter */
    sint16 pduUBOffset;
    uint8 frameStructIdx;                                   /* Frame structure index */

    PduInfoType pduInfo;
    Std_ReturnType ret;
    uint8 pduCnt;
    uint8 pduOffset;
    Fr_RxLPduStatusType frRxLPduStsPtr; /* Declare the output parameter for Fr_ReceiveRxLPdu */
    uint8 frCCIdx;
    uint8 rxLen;
    uint8 pduLen;
    uint8 * sduBuffPtr;
    uint16 ChannelAStatusPtr;
    uint16 ChannelBStatusPtr;

    frameTrigCfgPtr =  &(FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrameTriggeringPtr[frameCfgIdx]); /* Pointer to the Frame struct in the PB configuration file*/
    frameStructIdx  =  frameTrigCfgPtr->FrIf_FrameStructureIdx;
    frCCIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrCtrlIdx;
    sduBuffPtr = FrIf_Internal_Global.lSduBuffer[frameStructIdx];

    tempBuffPtr = sduBuffPtr;
#if (FRIF_BIG_ENDIAN_USED == STD_ON)
    uint8 tempBuffForBigEndian[FLEXRAY_FRAME_LEN_MAX];

    if (frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_ByteOrder == FRIF_BIG_ENDIAN) {
        tempBuffPtr = &tempBuffForBigEndian[0];
    }
#endif

    /* @req FrIf05293 */

    /* Locate this driver api */
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);
    ComOpLoopCounter = 0;

    do {
        ret = (*myFuncPtr->Fr_ReceiveRxLPdu)(frCCIdx, frLPduIdx, tempBuffPtr, &frRxLPduStsPtr, &rxLen);

        if ((E_OK == ret) && (frRxLPduStsPtr != FR_NOT_RECEIVED)) {

            /* Iterate over all PDU's in the FrameStructure */
            for (pduCnt = 0; pduCnt < frameTrigCfgPtr->FrIf_NumberPdusInFrame; pduCnt++) {

                pduInFramePtr = &(frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_PdusInFramePtr[pduCnt]);
                pduOffset = pduInFramePtr->FrIf_PduOffset;  /* Determine Pdu's offset in frame */
                /* @req FrIf05125 */
                pduUBOffset = pduInFramePtr->FrIf_PduUpdateBitOffset; /* Determine UB pos  */
                rxPduCfgPtr = pduInFramePtr->FrIf_Pdu->FrIf_PduDirectionPtr->FrIf_RxPduPtr;

#if (FRIF_BIG_ENDIAN_USED == STD_ON)
                if (frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_ByteOrder == FRIF_BIG_ENDIAN) {
                    FrIf_Internal_SwapToBigEndian(&tempBuffPtr[pduOffset],&sduBuffPtr[pduOffset], rxPduCfgPtr->RxPduLength);
                }
#endif

#if (FR_RX_STRINGENT_LENGTH_CHECK == STD_OFF)
                pduLen = getValidLength(rxPduCfgPtr->RxPduLength, pduOffset, pduUBOffset, rxLen);
#else
                pduLen = rxPduCfgPtr->RxPduLength;
#endif
                /* @req FrIf05056 */
                if ((readUB(pduUBOffset, tempBuffPtr) == PDU_UPDATED) && (pduLen > 0)) {

                      pduInfo.SduDataPtr = &sduBuffPtr[pduOffset];
                    pduInfo.SduLength  = pduLen;

                    if ((rxPduCfgPtr->FrIf_RxIndicationHandle != FRIF_NO_FUNCTION_CALLOUT) &&
                            (NULL_PTR != FrIf_ConfigCachePtr->FrIf_RxIndicationFncs[rxPduCfgPtr->FrIf_RxIndicationHandle]))
                    {
                        FrIf_ConfigCachePtr->FrIf_RxIndicationFncs[rxPduCfgPtr->FrIf_RxIndicationHandle](pduInFramePtr->FrIf_Pdu->FrIf_UpperLayerPduId, &pduInfo);
                    }
                }
            }
        } else {
            /* The communication operation is finished.*/

        	if (ret == E_NOT_OK) {
        		/* Recover from ACS (Aggregated Channel Status) errors.
        		 * Fr_GetChannelStatus resets the ACS information */
        		(void) Fr_GetChannelStatus(frCCIdx, &ChannelAStatusPtr, &ChannelBStatusPtr);
        	}

        	break;
        }
        if (frRxLPduStsPtr == FR_RECEIVED_MORE_DATA_AVAILABLE) {
            ComOpLoopCounter++;
        } else {
            /*lint -e{9011} we need to terminate loop*/
            break;
        }
    } while (ComOpLoopCounter < maxLoop);
}




/**
 * @brief Calls upper layer functions if there is an updated frame.
 * @param frIfIdx        FrIf Controller id.
 * @param frLPduIdx      LPdu id.
 * @param frameCfgIdx    Frame triggering id.
 */
void FrIf_Internal_ProvideRxIndication(uint8 frIfIdx, PduIdType frLPduIdx, uint8 frameCfgIdx )
{
    const FrIf_FrameTriggeringType * frameTrigCfgPtr;       /* Frame Triggering cfg Ptr */
    const FrIf_PdusIn_FrameType * pduInFramePtr;            /* Pdus in Frame cfg ptr */
    const FrIf_RxPduType * rxPduCfgPtr;                     /* Pointer to Rx pdu cfg */
    uint8 * tempBuffPtr;									/* Pointer to temporary buffer */
    uint8 frameStructIdx;                                   /* Frame structure index */

    PduIdType pduId;
    uint8 pduOffset;
    PduInfoType pduInfo;
    uint8 pduCnt;

    /* @req FrIf05291 */

    frameTrigCfgPtr =  &(FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrameTriggeringPtr[frameCfgIdx]);
    frameStructIdx  =  frameTrigCfgPtr->FrIf_FrameStructureIdx;

    tempBuffPtr = FrIf_Internal_Global.lSduBuffer[frameStructIdx];

    /* Iterate over all PDU's in the FrameStructure */
    for (pduCnt = 0; pduCnt < frameTrigCfgPtr->FrIf_NumberPdusInFrame; pduCnt++) {

        pduInFramePtr = &(frameTrigCfgPtr->FrIf_FrameStructureRef->FrIf_PdusInFramePtr[pduCnt]);
        pduId = pduInFramePtr->FrIf_Pdu->FrIf_PduId;/* Get the ID of the PDU in the frame */

        /* Check if static buffer is marked as outdated */
        if (TRUE == FrIf_Internal_Global.rxPduStatistics[pduId].pduUpdated) {

            pduOffset = pduInFramePtr->FrIf_PduOffset;  /* Determine Pdu's offset in frame */
            rxPduCfgPtr = pduInFramePtr->FrIf_Pdu->FrIf_PduDirectionPtr->FrIf_RxPduPtr;

            pduInfo.SduDataPtr = &tempBuffPtr[pduOffset];
            pduInfo.SduLength  = FrIf_Internal_Global.rxPduStatistics[pduId].pduRxLen;

            /* Call the upper layer's function <UL>_RxIndication */
            if ((rxPduCfgPtr->FrIf_RxIndicationHandle != FRIF_NO_FUNCTION_CALLOUT) &&
                    (NULL_PTR != FrIf_ConfigCachePtr->FrIf_RxIndicationFncs[rxPduCfgPtr->FrIf_RxIndicationHandle]))
            {
                FrIf_ConfigCachePtr->FrIf_RxIndicationFncs[rxPduCfgPtr->FrIf_RxIndicationHandle](pduInFramePtr->FrIf_Pdu->FrIf_UpperLayerPduId, &pduInfo);
            } else {
                /* Do nothing */
            }
            /* Mark the PDU-related static buffer as outdated */
            FrIf_Internal_Global.rxPduStatistics[pduId].pduUpdated = FALSE;
        }
    }
    (void)frLPduIdx;
}

/**
 * @brief wraps the Fr_PrepareLPdu
 * @param frIfIdx  FrIf Ctrl Idx
 * @param frLPduIdx Fr LPdu Idx
 */
void FrIf_Internal_PrepareLPdu(uint8 frIfIdx, PduIdType frLPduIdx)
{
    const FrIf_FrAPIType  *myFuncPtr;						/* Driver pointer */
    uint8 frCCIdx;                                          /* Fr CC index */
    /* @req FrIf05294 */
    /* Locate this driver api */
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    frCCIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[frIfIdx].FrIf_FrCtrlIdx;

    if (myFuncPtr->Fr_PrepareLPdu != NULL) {
        (void)(*myFuncPtr->Fr_PrepareLPdu)(frCCIdx, frLPduIdx);
    }

}

#ifdef HOST_TEST
FrIf_Internal_txPduStatisticsType readPduStatistics(PduIdType pduId)
{
    return FrIf_Internal_Global.txPduStatistics[pduId];
}
#endif


#if (FRIF_BIG_ENDIAN_USED == STD_ON)
/**
 * Used to swap the byte order.
 * @param srcBuffer
 * @param destBuffer
 * @param length
 */
void FrIf_Internal_SwapToBigEndian(const uint8 *srcBuffer,uint8 *destBuffer, uint8 length) {
    uint8 i;

    for (i = 0; i < length; i++) {
        destBuffer[i] = srcBuffer[(length - 1U)-i];
    }
}
#endif
