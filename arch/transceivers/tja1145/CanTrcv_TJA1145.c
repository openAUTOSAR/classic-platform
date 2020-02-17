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


#include "Spi.h"
#include "CanTrcv.h"
#include "CanTrcv_TJA1145.h"
#include "CanTrcv_Internal.h"

/* Internal functions to read and write using SPI */

Std_ReturnType spiRead(uint8 cmd, uint8 *data, const CanTrcv_SpiSequenceConfigType *SEQ_NXP);
Std_ReturnType spiWrite(uint8 cmd, uint8 data, const CanTrcv_SpiSequenceConfigType *SEQ_NXP);

static Std_ReturnType CanTrcv_Hw_ReadPowerOnStatus(boolean * failSts,const CanTrcv_SpiSequenceConfigType *spiSeq);

#define SPI_Write(regAddr, regData,spiSeq) \
	        if (E_NOT_OK == spiWrite(regAddr, regData,spiSeq)) { \
        	return E_NOT_OK; \
        }

#define SPI_Read(regAddr, regData,spiSeq) \
	        if (E_NOT_OK == spiRead(regAddr, &regData,spiSeq)) { \
        	return E_NOT_OK; \
        }


/**
 * SPI Read and Write to TJ1145
 *
 * @param cmd
 * @param data
 * @param SEQ_NXP
 * @param write
 * @return
 */
static Std_ReturnType spiReadWrite(uint8 cmd, uint8 *data, const CanTrcv_SpiSequenceConfigType *SEQ_NXP, boolean write) {
    Std_ReturnType rv;
    uint16 try = 0;

    cmd = (Spi_DataType)((Spi_DataType)cmd << 1);
    rv = Spi_SetupEB(  SEQ_NXP->CanTrcvSpiCmdChannel, &cmd, NULL, 1u );
    if( rv == E_OK ) {
        if ( write ) {
            rv = Spi_SetupEB( SEQ_NXP->CanTrcvSpiDataChannel, data, NULL, 1u);    /* Write */
        } else  {
            rv = Spi_SetupEB( SEQ_NXP->CanTrcvSpiDataChannel, NULL, data, 1u);     /* Read */
        }
        if( rv == E_OK ) {
#if defined(CFG_TJA1145_SPI_ASYNC)
            rv = Spi_AsyncTransmit(SEQ_NXP->CanTrcvSpiSequenceName);
#else
            rv = Spi_SyncTransmit(SEQ_NXP->CanTrcvSpiSequenceName);
#endif
            while( Spi_GetSequenceResult(SEQ_NXP->CanTrcvSpiSequenceName) == SPI_SEQ_PENDING ) {
                if( ++try > SPI_GET_RESULT_MAX_TRY ) {
                    rv = E_NOT_OK;
                    break;
                }

                /* Wait */
                Spi_MainFunction_Handling();
            }
        }
    }
    return rv;
}

Std_ReturnType spiRead(uint8 cmd, uint8 *data, const CanTrcv_SpiSequenceConfigType *SEQ_NXP) {
    return spiReadWrite( cmd, data,SEQ_NXP, FALSE );

}

Std_ReturnType spiWrite(uint8 cmd, uint8 data, const CanTrcv_SpiSequenceConfigType *SEQ_NXP) {
    return spiReadWrite( cmd, &data,SEQ_NXP, TRUE );
}



/* Set Data rate */
Std_ReturnType CanTrcv_Hw_SetBaudRate(uint16 baudRate,const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;
    switch (baudRate)
    {
        case CANTRCV_BAUD_RATE_50KBPS:
            regData= REGISTER_0x26_CDR_50;
            break;
        case CANTRCV_BAUD_RATE_100KBPS:
            regData= REGISTER_0x26_CDR_100;
            break;
        case CANTRCV_BAUD_RATE_125KBPS:
            regData= REGISTER_0x26_CDR_125;
            break;
        case CANTRCV_BAUD_RATE_250KBPS:
            regData= REGISTER_0x26_CDR_250;
            break;
        case CANTRCV_BAUD_RATE_500KBPS:
            regData= REGISTER_0x26_CDR_500;
            break;
        case CANTRCV_BAUD_RATE_1000KBPS:
            regData= REGISTER_0x26_CDR_1000;
            break;
        default:
            ret = E_NOT_OK;
            break;
    } /* End of Switch Case */

    if (E_OK == ret) {
        regAddr = (uint8)REGISTER_0x26_ADDR;
        /* Send data and Command on SPI to configure Baud rate */
        SPI_Write(regAddr, regData,spiSeq);
    }

    return ret;
}

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
/* Enable selective wake up */
Std_ReturnType CanTrcv_Hw_EnablePN(const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;
    regAddr = (uint8)REGISTER_0x20_ADDR; /*CAN control register */
    regData = (uint8)(REGISTER_20h_CPNC_SET |REGISTER_20h_PNCOK_SET);
    SPI_Write(regAddr, regData,spiSeq);

    ret = CanTrcv_Hw_EnableWakeUpEvent(spiSeq);
    if (E_NOT_OK == ret){
        return ret;
    }

    regAddr = (uint8)REGISTER_0x22_ADDR; /*Trcv status register*/
    regData = 0;
    SPI_Read(regAddr, regData,spiSeq);

    if (((regData & REGISTER_22h_CPNERR_MASK) == REGISTER_22h_CPNERR_MASK) ||
            ((regData & (REGISTER_22h_CPNS_MASK )) != (REGISTER_22h_CPNS_MASK ))) {

        ret = E_NOT_OK; /*Error detected or Trcv not sucessfully setup*/
    }
    return ret;
}

/* Disable selective wake up */
Std_ReturnType CanTrcv_Hw_DisablePN(const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;
    regAddr = (uint8)REGISTER_0x20_ADDR; /*CAN control register */
    regData = 0;
    SPI_Read(regAddr, regData,spiSeq);

    regData &= (uint8)~REGISTER_20h_CPNC_SET;
    SPI_Write(regAddr, regData,spiSeq);
    return ret;
}

/* Set PN registers for wake up frame */
Std_ReturnType CanTrcv_Hw_SetupPN(const CanTrcv_PartialNetworkConfigType* partialNwConfig,const CanTrcv_SpiSequenceConfigType *spiSeq){
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;
    boolean cFlag;
    boolean idFlag;
    uint8 loopCnt;
    uint8 idxCnt;

    ret=E_OK;
    cFlag = (partialNwConfig->CanTrcvPnFrameDataMaskSize !=0);
    idFlag = (partialNwConfig->CanTrcvPnCanIdIsExtended);

    regAddr = (uint8)REGISTER_0x2F_ADDR; /* Frame control register */
    regData = idFlag?REGISTER_0x2F_IDE_EXT: REGISTER_0x2F_IDE_STD ;
    regData |= cFlag ? (REGISTER_0x2F_PNDM |partialNwConfig->CanTrcvPnFrameDlc) : 0;
    SPI_Write(regAddr, regData,spiSeq);

    if (idFlag){
        /* For extended frames update Id and Mask */

        regAddr = (uint8)REGISTER_0x27_ADDR; /* ID register 0 */
        regData = (uint8)partialNwConfig->CanTrcvPnFrameCanId;
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x28_ADDR; /*ID register 1 */
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanId >> 8u);
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x29_ADDR; /*ID register 2*/
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanId >> 16u);
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x2A_ADDR; /*ID register 3*/
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanId >> 24u) & 0x1Fu;
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x2B_ADDR; /*Mask register 0*/
        regData = (uint8)partialNwConfig->CanTrcvPnFrameCanIdMask;
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x2C_ADDR; /*Mask register 1 */
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanIdMask >> 8u);
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x2D_ADDR; /*Mask register 2*/
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanIdMask >> 16u);
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x2E_ADDR; /*Mask register 3*/
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanIdMask >> 24u) & 0x1Fu;
        SPI_Write(regAddr, regData,spiSeq);

    }
    else {
        /* For standard frames update Id and Mask */
        regAddr = (uint8)REGISTER_0x29_ADDR; /*ID register 2*/
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanId << 2u);
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x2A_ADDR; /*ID register 3 */
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanId >> 6u) & 0x1Fu;
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x2D_ADDR; /*Mask register 2 */
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanIdMask << 2u);
        SPI_Write(regAddr, regData,spiSeq);

        regAddr = (uint8)REGISTER_0x2E_ADDR; /*Mask register 3 */
        regData = (uint8)(partialNwConfig->CanTrcvPnFrameCanIdMask >> 6u) & 0x1Fu;
        SPI_Write(regAddr, regData,spiSeq);
    }

    if (cFlag) {

        idxCnt =0;
        regAddr = (uint8) REGISTER_0x6F_ADDR;
        for (loopCnt = 0; loopCnt < partialNwConfig->CanTrcvPnFrameDlc; loopCnt++,regAddr--){
            regData = 0xFF; /* Set data mask (unused must be set to zero since default value is 0xFF) */
            if ((partialNwConfig->CanTrcvPnFrameDataMaskConfig[idxCnt].CanTrcvPnFrameDataMaskIndex == loopCnt)
                    && (idxCnt < partialNwConfig->CanTrcvPnFrameDataMaskSize)){

                regData = partialNwConfig->CanTrcvPnFrameDataMaskConfig[idxCnt].CanTrcvPnFrameDataMask;
                idxCnt++;
            }
            SPI_Write(regAddr, regData,spiSeq); /*Data Mask register */
        }

    }

    return ret;
}
#endif

/* Set transceiver mode */
Std_ReturnType CanTrcv_Hw_SetupMode(CanTrcv_TrcvModeType mode,const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    regAddr = (uint8)0x3;
    regData = 0;
    SPI_Read(regAddr, regData,spiSeq);

    ret=E_OK;
    regAddr = (uint8)REGISTER_0x01_ADDR; /*Mode control register*/
    switch(mode)
    {
        case CANTRCV_TRCVMODE_NORMAL:
            regData = REGISTER_0x01_MC_NORMAL;
            break;
        case CANTRCV_TRCVMODE_SLEEP:
            regData = REGISTER_0x01_MC_SLEEP;
            break;
        case CANTRCV_TRCVMODE_STANDBY:
            regData = REGISTER_0x01_MC_STANDBY;
            break;
        default:
            ret = E_NOT_OK;
            break;
    }

    if (E_OK == ret){
        SPI_Write(regAddr, regData,spiSeq);

        if (CANTRCV_TRCVMODE_NORMAL == mode) /*Set the CAN to active mode*/
        {
            regAddr = (uint8)0x3;
            regData = 0;
            SPI_Read(regAddr, regData,spiSeq);

            regAddr = (uint8)REGISTER_0x20_ADDR; /* CAN control register */
            regData = 0;
            SPI_Read(regAddr, regData,spiSeq);

            regData |=REGISTER_20h_CMC_SET;
            SPI_Write(regAddr, regData,spiSeq);

            regAddr = (uint8)REGISTER_0x22_ADDR; /*Trcv status register*/
            regData = 0;
            SPI_Read(regAddr, regData,spiSeq);

            if ((regData&REGISTER_22h_CTS_MASK)==0) {
                ret = E_NOT_OK;
                while( 1 ) {};
            }
        }
    }

    return ret;
}

/* Read transceiver mode */
Std_ReturnType CanTrcv_Hw_ReadCurrMode(CanTrcv_TrcvModeType* mode,const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;
    regAddr = (uint8)REGISTER_0x01_ADDR; /* Mode control register */
    regData = 0;
    SPI_Read(regAddr, regData,spiSeq);

    switch(regData&REGISTER_0x01_MODE_MASK){
        case REGISTER_0x01_MC_NORMAL:
            *mode = CANTRCV_TRCVMODE_NORMAL;
            break;
        case REGISTER_0x01_MC_SLEEP:
            *mode = CANTRCV_TRCVMODE_SLEEP;
            break;
        case REGISTER_0x01_MC_STANDBY:
            *mode = CANTRCV_TRCVMODE_STANDBY;
            break;
        default:
            ret = E_NOT_OK;
            break;

    }
    return ret;

}

/* Check for power on status */
Std_ReturnType CanTrcv_Hw_ReadPowerOnStatus(boolean * resetSts,const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;
    regAddr = (uint8)REGISTER_0x61_ADDR; /*system event status register */
    regData = 0;
    SPI_Read(regAddr, regData,spiSeq);
    if ((regData & REGISTER_0x61_PO_SET) == REGISTER_0x61_PO_SET) {
    	 *resetSts = TRUE;
    } else {
    	*resetSts = FALSE;
    }

    return ret;
}

/* Check if PNC is properly configured */
Std_ReturnType CanTrcv_Hw_ReadPNConfigurationStatus(boolean * errConfigSts,const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;
    regAddr = (uint8)REGISTER_0x22_ADDR; /*Trcv status register*/
    regData = 0;
    SPI_Read(regAddr, regData,spiSeq);

    if ((regData&REGISTER_22h_CPNS_MASK) == REGISTER_22h_CPNS_MASK) {
    	*errConfigSts = FALSE;
    } else {
    	*errConfigSts = TRUE;/*PN configuration error*/
    }

    return ret;

}

/* Clear wake up events */
Std_ReturnType CanTrcv_Hw_ClearWakeUpEventStatus(const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;
    regAddr = (uint8)REGISTER_0x63_ADDR; /*Trcv event status register*/
    regData = (uint8)(REGISTER_0x63_CW_SET |REGISTER_0x63_PNFDE_SET);
    SPI_Write(regAddr, regData,spiSeq);


    regAddr = (uint8)REGISTER_0x61_ADDR; /*system event status register */
    regData = (uint8)REGISTER_0x61_PO_SET;
    SPI_Write(regAddr, regData,spiSeq);

    regAddr = (uint8)REGISTER_0x64_ADDR; /* Wake pin event status register*/
    regData = (uint8)(REGISTER_0x64_WPR_SET|REGISTER_0x64_WPF_SET);
    SPI_Write(regAddr, regData,spiSeq);


    return ret;
}

/* Enable wake up on CAN bus (standard wake up pattern or selective wake up frame ) */
Std_ReturnType CanTrcv_Hw_EnableWakeUpEvent(const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;
    /* Wake up enable */
    regAddr = (uint8)REGISTER_0x23_ADDR; /*capture enable register */
    regData = (uint8) (REGISTER_0x23_CWE_SET); /* Wakeup detection (CFE is ignored because it is not a fault on the bus but fault at TxD pin due to MCU)*/
    SPI_Write(regAddr, regData,spiSeq);

#if 0
    /*Enable wake up on pin*/
    regAddr = (uint8)REGISTER_0x4c_ADDR; /*Wake pin event enable register*/
    regData = (uint8) (REGISTER_0x4C_WPRE_SET|REGISTER_0x4C_WPFE_SET );
    SPI_Write(regAddr, regData,spiSeq);
#endif
    return ret;
}

/* Read wake up event status */
Std_ReturnType CanTrcv_Hw_ReadWakeupEventStatus(CanTrcv_TrcvWakeupReasonType* reason,boolean * wakeupDetected,const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;
    boolean resetSts;

    ret=E_OK;
    resetSts = FALSE;
    ret = CanTrcv_Hw_ReadPowerOnStatus(&resetSts,spiSeq);
    if ((E_OK == ret) && resetSts)
    {
        *reason = CANTRCV_WU_POWER_ON;
        *wakeupDetected = TRUE;
    } else {

        regAddr = (uint8)REGISTER_0x63_ADDR; /*Trcv event status register*/
        regData = 0;
        SPI_Read(regAddr, regData,spiSeq);

        if (regData & REGISTER_0x63_CW_SET){
            *reason = CANTRCV_WU_BY_BUS;
            *wakeupDetected = TRUE;
        }
        if (regData & REGISTER_0x63_PNFDE_SET){
            *reason = CANTRCV_WU_BY_SYSERR;
            *wakeupDetected = TRUE;
        }
#if 0
        else {
            regAddr = (uint8)REGISTER_0x64_ADDR; /* Wake pin event status register*/
            regData = 0;
            SPI_Read(regAddr, regData,spiSeq);
            if ((regData&(REGISTER_0x64_WPR_SET|REGISTER_0x64_WPF_SET)) != 0)
            {
                *reason = CANTRCV_WU_BY_PIN;
                *wakeupDetected = TRUE;
            }
        }
#endif
    }

    return ret;
}

/* Check for bus activity */
Std_ReturnType CanTrcv_Hw_ReadSilenceFlag(CanTrcv_TrcvFlagStateType *flag,const CanTrcv_SpiSequenceConfigType *spiSeq)
{
    Std_ReturnType ret;
    uint8 regData;
    uint8 regAddr;

    ret=E_OK;

    regAddr =  (uint8)REGISTER_0x22_ADDR; /*Trcv status register*/
    regData = 0;
    SPI_Read(regAddr, regData,spiSeq);

    if ((regData&REGISTER_22h_CBSS_MASK)!= 0u)/*REGISTER_22h_CBSS_MASK*/
    {
        *flag = CANTRCV_FLAG_CLEARED; /* Currently silent */
    }
    else {
        *flag = CANTRCV_FLAG_SET; /* CAN activity on-going */
    }
    return ret;
}
