/*****************************************************************
 * FILE        : 56xxS_0204.h
 *
 * DESCRIPTION : This is the header file describing the register
 *               set for:
 *               MPC5606S, mask set = 0M25V
 *               SPC560S6, mask set = FS60X2
 *
 * COPYRIGHT   :(c) 2010, Freescale & STMicroelectronics
 *
 * VERSION     : 2.04
 * DATE        : February 9 2010
 * AUTHOR      : ttz778
 * HISTORY     : Original source taken from jdp_0100.h
 * 2.0  : Updates to match Reference Manual Rev 3
 * 2.01 : Correction to CGM SC_DC bitfield
 * 2.02 : Correction to DCU M_DMA_TRANS_FINISH bitfield
 * 2.03 : Amended SIU.PCR[].B.SMC bitfield; tidied embedded comments
 * 2.04 : eMIOS channel width 24->16 bits;
 *        Expanded bitfields in WKUP (use .R rather than .B.nnnn if setting/clearing more than one)
 ******************************************************************/
/*>>>>NOTE! this file is auto-generated please do not edit it!<<<<*/

/***************************************************************** 
* Example instantiation and use:
*
*  <MODULE>.<REGISTER>.B.<BIT>    = 1;
*  <MODULE>.<REGISTER>.R        = 0x10000000;
*
******************************************************************/

#ifndef _MPC5606S_H_
#define _MPC5606S_H_

#include "Compiler.h"
#include "typedefs.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __MWERKS__
#pragma push
#pragma ANSI_strict off
#endif

#include "ip_adc_mpc56xx.h"

#if 0
/****************************************************************************/
/*                             MODULE : ADC                                 */
/****************************************************************************/
    struct ADC_tag {

        union {
            vuint32_t R;
            struct {
                vuint32_t OWREN:1;
                vuint32_t WLSIDE:1;
                vuint32_t MODE:1;
                vuint32_t EDGLEV:1;
                vuint32_t TRGEN:1;
                vuint32_t EDGE:1;
                  vuint32_t:1;
                vuint32_t NSTART:1;
                  vuint32_t:1;
                vuint32_t JTRGEN:1;
                vuint32_t JEDGE:1;
                vuint32_t JSTART:1;
                  vuint32_t:11;
                vuint32_t ADCLKSEL:1;
                vuint32_t ABORTCHAIN:1;
                vuint32_t ABORT:1;
                vuint32_t ACK0:1;
                  vuint32_t:4;
                vuint32_t PWDN:1;
            } B;
        } MCR;                         /* MAIN CONFIGURATION REGISTER */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:7;
                vuint32_t NSTART:1;
                vuint32_t JABORT:1;
                  vuint32_t:2;
                vuint32_t JSTART:1;
                  vuint32_t:4;
                vuint32_t CHADDR:7;
                  vuint32_t:3;
                vuint32_t ACK0:1;
                  vuint32_t:2;
                vuint32_t ADCSTATUS:3;
            } B;
        } MSR;                         /* MAIN STATUS REGISTER */

        int32_t ADC_reserved1[2];      /* (0x008 - 0x00F)/4 = 0x02 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:28;
                vuint32_t JEOC:1;
                vuint32_t JECH:1;
                vuint32_t EOC:1;
                vuint32_t ECH:1;
            } B;
        } ISR;                         /* INTERRUPT STATUS REGISTER */


        union {
            vuint32_t R;
            struct {
                vuint32_t EOC31:1;
                vuint32_t EOC30:1;
                vuint32_t EOC29:1;
                vuint32_t EOC28:1;
                vuint32_t EOC27:1;
                vuint32_t EOC26:1;
                vuint32_t EOC25:1;
                vuint32_t EOC24:1;
                vuint32_t EOC23:1;
                vuint32_t EOC22:1;
                vuint32_t EOC21:1;
                vuint32_t EOC20:1;
                vuint32_t EOC19:1;
                vuint32_t EOC18:1;
                vuint32_t EOC17:1;
                vuint32_t EOC16:1;
                vuint32_t EOC15:1;
                vuint32_t EOC14:1;
                vuint32_t EOC13:1;
                vuint32_t EOC12:1;
                vuint32_t EOC11:1;
                vuint32_t EOC10:1;
                vuint32_t EOC9:1;
                vuint32_t EOC8:1;
                vuint32_t EOC7:1;
                vuint32_t EOC6:1;
                vuint32_t EOC5:1;
                vuint32_t EOC4:1;
                vuint32_t EOC3:1;
                vuint32_t EOC2:1;
                vuint32_t EOC1:1;
                vuint32_t EOC0:1;
            } B;
        } CEOCFR[3];                   /* Channel Pending Registers - [0] not supported */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:28;
                vuint32_t MSKJEOC:1;
                vuint32_t MSKJECH:1;
                vuint32_t MSKEOC:1;
                vuint32_t MSKECH:1;
            } B;
        } IMR;                         /* INTERRUPT MASK REGISTER */

        union {
            vuint32_t R;
            struct {
                vuint32_t CIM31:1;
                vuint32_t CIM30:1;
                vuint32_t CIM29:1;
                vuint32_t CIM28:1;
                vuint32_t CIM27:1;
                vuint32_t CIM26:1;
                vuint32_t CIM25:1;
                vuint32_t CIM24:1;
                vuint32_t CIM23:1;
                vuint32_t CIM22:1;
                vuint32_t CIM21:1;
                vuint32_t CIM20:1;
                vuint32_t CIM19:1;
                vuint32_t CIM18:1;
                vuint32_t CIM17:1;
                vuint32_t CIM16:1;
                vuint32_t CIM15:1;
                vuint32_t CIM14:1;
                vuint32_t CIM13:1;
                vuint32_t CIM12:1;
                vuint32_t CIM11:1;
                vuint32_t CIM10:1;
                vuint32_t CIM9:1;
                vuint32_t CIM8:1;
                vuint32_t CIM7:1;
                vuint32_t CIM6:1;
                vuint32_t CIM5:1;
                vuint32_t CIM4:1;
                vuint32_t CIM3:1;
                vuint32_t CIM2:1;
                vuint32_t CIM1:1;
                vuint32_t CIM0:1;
            } B;
        } CIMR[3];                     /* Channel Interrupt Mask Registers - [0] not supported */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:24;
                vuint32_t WDG3H:1;
                vuint32_t WDG2H:1;
                vuint32_t WDG1H:1;
                vuint32_t WDG0H:1;
                vuint32_t WDG3L:1;
                vuint32_t WDG2L:1;
                vuint32_t WDG1L:1;
                vuint32_t WDG0L:1;
            } B;
        } WTISR;                       /* WATCHDOG INTERRUPT THRESHOLD REGISTER was WDGTHR */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:24;
                vuint32_t MSKWDG3H:1;
                vuint32_t MSKWDG2H:1;
                vuint32_t MSKWDG1H:1;
                vuint32_t MSKWDG0H:1;
                vuint32_t MSKWDG3L:1;
                vuint32_t MSKWDG2L:1;
                vuint32_t MSKWDG1L:1;
                vuint32_t MSKWDG0L:1;
            } B;
        } WTIMR;                       /* WATCHDOG INTERRUPT MASK REGISTER was IMWDGTHR */

        int32_t ADC_reserved2[2];      /* (0x038 - 0x03F)/4 = 0x02 */

    union {
            vuint32_t R;
            struct {
                  vuint32_t:30;
                vuint32_t DCLR:1;
                vuint32_t DMAEN:1;
            } B;
        } DMAE;                        /* DMAE REGISTER */

        union {
            vuint32_t R;
            struct {
                vuint32_t DMA31:1;     //was unused [16]
                vuint32_t DMA30:1;
                vuint32_t DMA29:1;
                vuint32_t DMA28:1;
                vuint32_t DMA27:1;
                vuint32_t DMA26:1;
                vuint32_t DMA25:1;
                vuint32_t DMA24:1;
                vuint32_t DMA23:1;
                vuint32_t DMA22:1;
                vuint32_t DMA21:1;
                vuint32_t DMA20:1;
                vuint32_t DMA19:1;
                vuint32_t DMA18:1;
                vuint32_t DMA17:1;
                vuint32_t DMA16:1;
                vuint32_t DMA15:1;
                vuint32_t DMA14:1;
                vuint32_t DMA13:1;
                vuint32_t DMA12:1;
                vuint32_t DMA11:1;
                vuint32_t DMA10:1;
                vuint32_t DMA9:1;
                vuint32_t DMA8:1;
                vuint32_t DMA7:1;
                vuint32_t DMA6:1;
                vuint32_t DMA5:1;
                vuint32_t DMA4:1;
                vuint32_t DMA3:1;
                vuint32_t DMA2:1;
                vuint32_t DMA1:1;
                vuint32_t DMA0:1;
            } B;
        } DMAR[3];                     /* DMA Channel select Registers - [0] not supported */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t THREN:1;
                vuint32_t THRINV:1;
                  vuint32_t:7;
                vuint32_t THRCH:7;
            } B;
        } TRC[4];                      /* Threshold Control Registers */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:6;
                vuint32_t THRH:10;
                  vuint32_t:6;
                vuint32_t THRL:10;
            } B;
        } THRHLR[4];                   /* Threshold Registers */

        int32_t ADC_reserved3[9];      /* (0x070 - 0x08F)/4 = 0x09 */

    union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t INPLATCH:1;
                  vuint32_t:1;
                vuint32_t OFFSHIFT:2;  //!!! This field only in CTR[0]
                  vuint32_t:1;
                vuint32_t INPCMP:2;
                  vuint32_t:1;
                vuint32_t INPSAMP:8;
            } B;
        } CTR[3];                      /* Conversion Timing Register - [0] not supported */

    int32_t ADC_reserved4[1];          /* (0x0A0 - 0x0A3)/4 = 0x01 */

    union {
            vuint32_t R;
            struct {
                vuint32_t CH31:1;
                vuint32_t CH30:1;
                vuint32_t CH29:1;
                vuint32_t CH28:1;
                vuint32_t CH27:1;
                vuint32_t CH26:1;
                vuint32_t CH25:1;
                vuint32_t CH24:1;
                vuint32_t CH23:1;
                vuint32_t CH22:1;
                vuint32_t CH21:1;
                vuint32_t CH20:1;
                vuint32_t CH19:1;
                vuint32_t CH18:1;
                vuint32_t CH17:1;
                vuint32_t CH16:1;
                vuint32_t CH15:1;
                vuint32_t CH14:1;
                vuint32_t CH13:1;
                vuint32_t CH12:1;
                vuint32_t CH11:1;
                vuint32_t CH10:1;
                vuint32_t CH9:1;
                vuint32_t CH8:1;
                vuint32_t CH7:1;
                vuint32_t CH6:1;
                vuint32_t CH5:1;
                vuint32_t CH4:1;
                vuint32_t CH3:1;
                vuint32_t CH2:1;
                vuint32_t CH1:1;
                vuint32_t CH0:1;
            } B;
        } NCMR[3];                     /* NORMAL CONVERSION MASK REGISTER was [6] */

        int32_t ADC_reserved5[1];      /* (0x0B0 - 0x0B3)/4 = 0x01 */

        union {
            vuint32_t R;
            struct {
                vuint32_t CH31:1;
                vuint32_t CH30:1;
                vuint32_t CH29:1;
                vuint32_t CH28:1;
                vuint32_t CH27:1;
                vuint32_t CH26:1;
                vuint32_t CH25:1;
                vuint32_t CH24:1;
                vuint32_t CH23:1;
                vuint32_t CH22:1;
                vuint32_t CH21:1;
                vuint32_t CH20:1;
                vuint32_t CH19:1;
                vuint32_t CH18:1;
                vuint32_t CH17:1;
                vuint32_t CH16:1;
                vuint32_t CH15:1;
                vuint32_t CH14:1;
                vuint32_t CH13:1;
                vuint32_t CH12:1;
                vuint32_t CH11:1;
                vuint32_t CH10:1;
                vuint32_t CH9:1;
                vuint32_t CH8:1;
                vuint32_t CH7:1;
                vuint32_t CH6:1;
                vuint32_t CH5:1;
                vuint32_t CH4:1;
                vuint32_t CH3:1;
                vuint32_t CH2:1;
                vuint32_t CH1:1;
                vuint32_t CH0:1;
            } B;
        } JCMR[3];                     /* Injected CONVERSION MASK REGISTER */

        int32_t ADC_reserved6[1];      /* (0x0C0 - 0x0C3)/4 = 0x01 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:24;
                vuint32_t DSD:8;
            } B;
        } DSDR;                        /* DECODE SIGNALS DELAY REGISTER was DSD */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:24;
                vuint32_t PDED:8;
            } B;
        } PDEDR;                       /* POWER DOWN DELAY REGISTER was PDD */

        int32_t ADC_reserved7[13];     /* (0x0CC - 0xFF)/4 = 0x0D */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:12;
                vuint32_t VALID:1;
                vuint32_t OVERW:1;
                vuint32_t RESULT:2;
                  vuint32_t:6;
                vuint32_t CDATA:10;
            } B;
        } CDR[96];                     /* Channel 0-95 Data REGISTER - 0-31, 48-63, 72-95 not supported */


    };                                 /* end of ADC_tag */
#endif

/****************************************************************************/
/*                             MODULE : CANSP                               */
/****************************************************************************/
    struct CANSP_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t RX_COMPLETE:1;
                vuint32_t BUSY:1;
                vuint32_t ACTIVE_CK:1;
                  vuint32_t:3;
                vuint32_t MODE:1;
                vuint32_t CAN_RX_SEL:3;
                vuint32_t BRP:5;
                vuint32_t CAN_SMPLR_EN:1;
            } B;
        } CR;                          /* CANSP Control Register */

        union {
            vuint32_t R;
        } SR[12];                      /* CANSP Sample Register 0 to 11 */

    };                                 /* end of CANSP_tag */
/****************************************************************************/
/*                             MODULE : ECSM                                */
/****************************************************************************/
    struct ECSM_tag {

        union {
            vuint16_t R;
        } PCT;                         /* ECSM Processor Core Type Register */

        union {
            vuint16_t R;
        } REV;                         /* ECSM  Revision Register */

        int32_t ECSM_reserved;

        union {
            vuint32_t R;
        } IMC;                         /* IPS Configuration Register */

        int8_t ECSM_reserved1[3];

        union {
            vuint8_t R;
            struct {
                vuint8_t POR:1;
                vuint8_t DIR:1;
                  vuint8_t:6;
            } B;
        } MRSR;                        /* ECSM Miscellaneous Reset Status Register */

        int8_t ECSM_reserved2[3];

        union {
            vuint8_t R;
            struct {
                vuint8_t ENBWCR:1;
                  vuint8_t:3;
                vuint8_t PRILVL:4;
            } B;
        } MWCR;                        /* ECSM Miscellaneous Wakeup Control Register */

        int32_t ECSM_reserved3[2];
        int8_t ESCM_reserved4[3];

        union {
            vuint8_t R;
            struct {
                vuint8_t FB0AI:1;
                vuint8_t FB0SI:1;
                vuint8_t FB1AI:1;
                vuint8_t FB1SI:1;
                  vuint8_t:4;
            } B;
        } MIR;                         /* ECSM Miscellaneous Interrupt Register */

        int32_t ECSM_reserved5;

        union {
            vuint32_t R;
        } MUDCR;                       /* ECSM Miscellaneous User-Defined Control Register */

        int32_t ECSM_reserved6[6];     /* (0x040- 0x028)/4 = 0x06 */
        int8_t ECSM_reserved7[3];

        union {
            vuint8_t R;
            struct {
                  vuint8_t:2;
                vuint8_t ER1BR:1;
                vuint8_t EF1BR:1;
                  vuint8_t:2;
                vuint8_t ERNCR:1;
                vuint8_t EFNCR:1;
            } B;
        } ECR;                         /* ECSM ECC Configuration Register */

        int8_t ECSM_reserved8[3];

        union {
            vuint8_t R;
            struct {
                  vuint8_t:2;
                vuint8_t R1BC:1;
                vuint8_t F1BC:1;
                  vuint8_t:2;
                vuint8_t RNCE:1;
                vuint8_t FNCE:1;
            } B;
        } ESR;                         /* ECSM ECC Status Register */

        int16_t ECSM_reserved9;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:2;
                vuint16_t FRC1BI:1;
                vuint16_t FR11BI:1;
                  vuint16_t:2;
                vuint16_t FRCNCI:1;
                vuint16_t FR1NCI:1;
                  vuint16_t:1;
                vuint16_t ERRBIT:7;
            } B;
        } EEGR;                        /* ECSM ECC Error Generation Register */

        int32_t ECSM_reserved10;

        union {
            vuint32_t R;
        } FEAR;                        /* ECSM Flash ECC Address Register */

        int16_t ECSM_reserved11;

        union {
            vuint8_t R;
            struct {
                  vuint8_t:4;
                vuint8_t FEMR:4;
            } B;
        } FEMR;                        /* ECSM Flash ECC Master Number Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t WRITE:1;
                vuint8_t SIZE:3;
                vuint8_t PROTECTION:4;
            } B;
        } FEAT;                        /* ECSM Flash ECC Attributes Register */

        int32_t ECSM_reserved12;

        union {
            vuint32_t R;
        } FEDR;                        /* ECSM Flash ECC Data Register */

        union {
            vuint32_t R;
        } REAR;                        /* ECSM RAM ECC Address Register */

        int8_t ECSM_reserved13;

        union {
            vuint8_t R;
        } RESR;                        /* ECSM RAM ECC Address Register */

        union {
            vuint8_t R;
            struct {
                  vuint8_t:4;
                vuint8_t REMR:4;
            } B;
        } REMR;                        /* ECSM RAM ECC Master Number Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t WRITE:1;
                vuint8_t SIZE:3;
                vuint8_t PROTECTION:4;
            } B;
        } REAT;                        /* ECSM RAM ECC Attributes Register */

        int32_t ECSM_reserved14;

        union {
            vuint32_t R;
        } REDR;                        /* ECSM RAM ECC Data Register */

    };                                 /* end of ECSM_tag */
/****************************************************************************/
/*                             MODULE : RTC                                 */
/****************************************************************************/
    struct RTC_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t SUPV:1;
                  vuint32_t:31;
            } B;
        } RTCSUPV;                     /* RTC Supervisor Control Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CNTEN:1;
                vuint32_t RTCIE:1;
                vuint32_t FRZEN:1;
                vuint32_t ROVREN:1;
                vuint32_t RTCVAL:12;
                vuint32_t APIEN:1;
                vuint32_t APIIE:1;
                vuint32_t CLKSEL:2;
                vuint32_t DIV512EN:1;
                vuint32_t DIV32EN:1;
                vuint32_t APIVAL:10;
            } B;
        } RTCC;                        /* RTC Control Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:2;
                vuint32_t RTCF:1;
                  vuint32_t:15;
                vuint32_t APIF:1;
                  vuint32_t:2;
                vuint32_t ROVRF:1;
                  vuint32_t:10;
            } B;
        } RTCS;                        /* RTC Status Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t RTCCNT:32;
            } B;
        } RTCCNT;                      /* RTC Counter Register */

    };                                 /* end of RTC_tag */
/****************************************************************************/
/*                             MODULE : SIU                                 */
/****************************************************************************/
    struct SIU_tag {

        int32_t SIU_reserved0;

        union {                        /* MCU ID Register 1 */
            vuint32_t R;
            struct {
                vuint32_t PARTNUM:16;
                vuint32_t CSP:1;
                vuint32_t PKG:5;
                  vuint32_t:2;
                vuint32_t MAJOR_MASK:4;
                vuint32_t MINOR_MASK:4;
            } B;
        } MIDR;

        union {                        /* MCU ID Register 2 */
            vuint32_t R;
            struct {
                vuint32_t SF:1;
                vuint32_t FLASH_SIZE_1:4;
                vuint32_t FLASH_SIZE_2:4;
                  vuint32_t:7;
                vuint32_t PARTNUM:8;
                  vuint32_t:3;
                vuint32_t EE:1;
                  vuint32_t:4;
            } B;
        } MIDR2;

        int32_t SIU_reserved1[2];

        union {                        /* Interrupt Status Flag Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t EIF15:1;
                vuint32_t EIF14:1;
                vuint32_t EIF13:1;
                vuint32_t EIF12:1;
                vuint32_t EIF11:1;
                vuint32_t EIF10:1;
                vuint32_t EIF9:1;
                vuint32_t EIF8:1;
                vuint32_t EIF7:1;
                vuint32_t EIF6:1;
                vuint32_t EIF5:1;
                vuint32_t EIF4:1;
                vuint32_t EIF3:1;
                vuint32_t EIF2:1;
                vuint32_t EIF1:1;
                vuint32_t EIF0:1;
            } B;
        } ISR;

        union {                        /* Interrupt Request Enable Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t IRE15:1;
                vuint32_t IRE14:1;
                vuint32_t IRE13:1;
                vuint32_t IRE12:1;
                vuint32_t IRE11:1;
                vuint32_t IRE10:1;
                vuint32_t IRE9:1;
                vuint32_t IRE8:1;
                vuint32_t IRE7:1;
                vuint32_t IRE6:1;
                vuint32_t IRE5:1;
                vuint32_t IRE4:1;
                vuint32_t IRE3:1;
                vuint32_t IRE2:1;
                vuint32_t IRE1:1;
                vuint32_t IRE0:1;
            } B;
        } IRER;

        int32_t SIU_reserved2[3];

        union {                        /* Interrupt Rising-Edge Event Enable Register */
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t IREE15:1;
                vuint32_t IREE14:1;
                vuint32_t IREE13:1;
                vuint32_t IREE12:1;
                vuint32_t IREE11:1;
                vuint32_t IREE10:1;
                vuint32_t IREE9:1;
                vuint32_t IREE8:1;
                vuint32_t IREE7:1;
                vuint32_t IREE6:1;
                vuint32_t IREE5:1;
                vuint32_t IREE4:1;
                vuint32_t IREE3:1;
                vuint32_t IREE2:1;
                vuint32_t IREE1:1;
                vuint32_t IREE0:1;
            } B;
        } IREER;

        union {                        /* Interrupt Falling-Edge Event Enable Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t IFEE15:1;
                vuint32_t IFEE14:1;
                vuint32_t IFEE13:1;
                vuint32_t IFEE12:1;
                vuint32_t IFEE11:1;
                vuint32_t IFEE10:1;
                vuint32_t IFEE9:1;
                vuint32_t IFEE8:1;
                vuint32_t IFEE7:1;
                vuint32_t IFEE6:1;
                vuint32_t IFEE5:1;
                vuint32_t IFEE4:1;
                vuint32_t IFEE3:1;
                vuint32_t IFEE2:1;
                vuint32_t IFEE1:1;
                vuint32_t IFEE0:1;
            } B;
        } IFEER;

        union {                        /* Interrupt Filter Enable Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t IFE15:1;
                vuint32_t IFE14:1;
                vuint32_t IFE13:1;
                vuint32_t IFE12:1;
                vuint32_t IFE11:1;
                vuint32_t IFE10:1;
                vuint32_t IFE9:1;
                vuint32_t IFE8:1;
                vuint32_t IFE7:1;
                vuint32_t IFE6:1;
                vuint32_t IFE5:1;
                vuint32_t IFE4:1;
                vuint32_t IFE3:1;
                vuint32_t IFE2:1;
                vuint32_t IFE1:1;
                vuint32_t IFE0:1;
            } B;
        } IFER;

        int32_t SIU_reserved3[3];

/* This register contains a bit called SMC, however, on this family SMC is also the name 
** of the Stepper Motor Controller. This is a conflict due to the macro substitution approach
** used by this header. Therefore for this family that bit is renamed "SME"
*/   
        union {                        /* Pad Configuration Registers */
            vuint16_t R;
            struct {
                  vuint16_t:1;
                vuint16_t SME:1;       /* Standard definition is vuint16_t SMC:1; */
                vuint16_t APC:1;
                  vuint16_t:1;
                vuint16_t PA:2;
                vuint16_t OBE:1;
                vuint16_t IBE:1;
                 vuint16_t:2;
                vuint16_t ODE:1;
                 vuint16_t:2;
                vuint16_t SRC:1;
                vuint16_t WPE:1;
                vuint16_t WPS:1;
            } B;
        } PCR[133];

        int16_t SIU_reserved4;         /* {0x14A} */
        int32_t SIU_reserved5[237];    /* {0x500-0x14C}/0x4 */

        union {                        /* Pad Selection for Multiplexed Input Register */
            vuint8_t R;
            struct {
                  vuint8_t:4;
                vuint8_t PADSEL:4;
            } B;
        } PSMI[43];

        int8_t SIU_reserved6;          /* {0x52B} */
        int32_t SIU_reserved7[53];     /* {0x600-0x52C}/0x4 */

        union {                        /* GPIO Pin Data Output Registers */
            vuint8_t R;
            struct {
                  vuint8_t:7;
                vuint8_t PDO:1;
            } B;
        } GPDO[133];

        int8_t SIU_reserved8[3];       /* {0x685-687} */
        int32_t SIU_reserved9[94];     /* {0x800-0x688}/0x4 */

        union {                        /* GPIO Pin Data Input Registers */
            vuint8_t R;
            struct {
                  vuint8_t:7;
                vuint8_t PDI:1;
            } B;
        } GPDI[133];

        int8_t SIU_reserved10[3];      /* {0x885-887} */
        int32_t SIU_reserved11[222];   /* {0xC00-0x888}/0x4 */

        union {                        /* Parallel GPIO Pin Data Output Register */
            vuint32_t R;
            struct {
                vuint32_t PPD0:32;
            } B;
        } PGPDO[5];

        int32_t SIU_reserved12[11];    /* {0xC40-0xC14}/0x4 */

        union {                        /* Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PPDI:32;
            } B;
        } PGPDI[5];

        int32_t SIU_reserved13[11];    /* {0xC80-0xC54}/0x4 */

        union {                        /* Masked Parallel GPIO Pin Data Out Register */
            vuint32_t R;
            struct {
                vuint32_t MASK:16;
                vuint32_t MPPDO:16;
            } B;
        } MPGPDO[9];

        int32_t SIU_reserved14[215];   /* {0x1000-0x0CA4}/0x4 */

        union {                        /* Interrupt Filter Maximum Counter Register */
            vuint32_t R;
            struct {
                  vuint32_t:28;
                vuint32_t MAXCNT:4;
            } B;
        } IFMC[16];

        int32_t SIU_reserved15[16];    /* {0x1080-0x1040}/4 = 0x10 */

        union {                        /* Interrupt Filter Clock Prescaler Register */
            vuint32_t R;
            struct {
                  vuint32_t:28;
                vuint32_t IFCP:4;
            } B;
        } IFCPR;

    };                                 /* end of SIU_tag */
/****************************************************************************/
/*                             MODULE : SSCM                                */
/****************************************************************************/
    struct SSCM_tag {
        union {
            vuint16_t R;
            struct {
                  vuint16_t:4;
                vuint16_t NXEN:1;
                vuint16_t PUB:1;
                vuint16_t SEC:1;
                  vuint16_t:1;
                vuint16_t BMODE:3;
                  vuint16_t :1;
                vuint16_t ABD:1;
                  vuint16_t:3;
            } B;
        } STATUS;                      /* Status Register */

        union {
            vuint16_t R;
            struct {
                  vuint16_t:10;
                vuint16_t IVLD:1;
                  vuint16_t:4;
                vuint16_t DVLD:1;
            } B;
        } MEMCONFIG;                   /* System Memory Configuration Register */

        int16_t SSCM_reserved;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:14;
                vuint16_t PAE:1;
                vuint16_t RAE:1;
            } B;
        } ERROR;                       /* Error Configuration Register */

        union {
            vuint16_t R;
            struct {
                  vuint16_t:13;
                vuint16_t DEBUG_MODE:3;
            } B;
        } DEBUGPORT;                   /* Debug Status Port Register */

        int16_t SSCM_reserved1;

        union {
            vuint32_t R;
            struct {
                vuint32_t PWD_HI:32;
            } B;
        } PWCMPH;                      /* Password Comparison Register High Word */

        union {
            vuint32_t R;
            struct {
                vuint32_t PWD_LO:32;
            } B;
        } PWCMPL;                      /* Password Comparison Register Low Word */

    };                                 /* end of SSCM_tag */
/****************************************************************************/
/*                             MODULE : STM                                 */
/****************************************************************************/
    struct STM_CHANNEL_tag {

        union {
            vuint32_t R;
            struct {
                  vuint32_t:31;
                vuint32_t CEN:1;
            } B;
        } CCR;                         /* STM Channel Control Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:31;
                vuint32_t CIF:1;
            } B;
        } CIR;                         /* STM Channel Interrupt Register */

        union {
            vuint32_t R;
        } CMP;                         /* STM Channel Compare Register 0 */

        int32_t STM_CHANNEL_reserved;

    };                                 /* end of STM_CHANNEL_tag */

    struct STM_tag {

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t CPS:8;
                  vuint32_t:6;
                vuint32_t FRZ:1;
                vuint32_t TEN:1;
            } B;
        } CR;                          /* STM Control Register */

        union {
            vuint32_t R;
        } CNT;                         /* STM Count Register */

        int32_t STM_reserved[2];

        struct STM_CHANNEL_tag CH[4];

    };                                 /* end of STM_tag */
/****************************************************************************/
/*                             MODULE : SWT                                 */
/****************************************************************************/
    struct SWT_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t MAP0:1;
                vuint32_t MAP1:1;
                vuint32_t MAP2:1;
                vuint32_t MAP3:1;
                  vuint32_t:19;
                vuint32_t RIA:1;
                vuint32_t WND:1;
                vuint32_t ITR:1;
                vuint32_t HLK:1;
                vuint32_t SLK:1;
                vuint32_t CSL:1;
                vuint32_t STP:1;
                vuint32_t FRZ:1;
                vuint32_t WEN:1;
            } B;
        } CR;                          /* SWT Control Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:31;
                vuint32_t TIF:1;
            } B;
        } IR;                          /* SWT Interrupt Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t WTO:32;
            } B;
        } TO;                          /* SWT Time-Out Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t WST:32;
            } B;
        } WN;                          /* SWT Window Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t WSC:16;
            } B;
        } SR;                          /* SWT Service Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CNT:32;
            } B;
        } CO;                          /* SWT Counter Output Register */

    };                                /* end of SWT_tag */
/****************************************************************************/
/*                             MODULE : WKUP                                */
/****************************************************************************/
    struct WKUP_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t NIF0:1;
                vuint32_t NOVF0:1;
                  vuint32_t:30;
            } B;
        } NSR;                         /* NMI Status Register */

        int32_t WKUP_reserved;

        union {
            vuint32_t R;
            struct {
                vuint32_t NLOCK0:1;
                vuint32_t NDSS:2;
                vuint32_t NWRE:1;
                  vuint32_t:1;
                vuint32_t NREE:1;
                vuint32_t NFEE:1;
                vuint32_t NFE:1;
                  vuint32_t:24;
            } B;
        } NCR;                         /* NMI Configuration Register */

        int32_t WKUP_reserved1[2];

        union {
            vuint32_t R;
            struct {
                  vuint32_t:11;
                vuint32_t EIF20:1;
                vuint32_t EIF19:1;
                vuint32_t EIF18:1;
                vuint32_t EIF17:1;
                vuint32_t EIF16:1;
                vuint32_t EIF15:1;
                vuint32_t EIF14:1;
                vuint32_t EIF13:1;
                vuint32_t EIF12:1;
                vuint32_t EIF11:1;
                vuint32_t EIF10:1;
                vuint32_t EIF9:1;
                vuint32_t EIF8:1;
                vuint32_t EIF7:1;
                vuint32_t EIF6:1;
                vuint32_t EIF5:1;
                vuint32_t EIF4:1;
                vuint32_t EIF3:1;
                vuint32_t EIF2:1;
                vuint32_t EIF1:1;
                vuint32_t EIF0:1;
            } B;
        } WISR;                        /* Wakeup/Interrupt Status Flag Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:11;
                vuint32_t EIRE20:1;
                vuint32_t EIRE19:1;
                vuint32_t EIRE18:1;
                vuint32_t EIRE17:1;
                vuint32_t EIRE16:1;
                vuint32_t EIRE15:1;
                vuint32_t EIRE14:1;
                vuint32_t EIRE13:1;
                vuint32_t EIRE12:1;
                vuint32_t EIRE11:1;
                vuint32_t EIRE10:1;
                vuint32_t EIRE9:1;
                vuint32_t EIRE8:1;
                vuint32_t EIRE7:1;
                vuint32_t EIRE6:1;
                vuint32_t EIRE5:1;
                vuint32_t EIRE4:1;
                vuint32_t EIRE3:1;
                vuint32_t EIRE2:1;
                vuint32_t EIRE1:1;
                vuint32_t EIRE0:1;
            } B;
        } IRER;                        /* Interrupt Request Enable Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:11;
                vuint32_t WRE20:1;
                vuint32_t WRE19:1;
                vuint32_t WRE18:1;
                vuint32_t WRE17:1;
                vuint32_t WRE16:1;
                vuint32_t WRE15:1;
                vuint32_t WRE14:1;
                vuint32_t WRE13:1;
                vuint32_t WRE12:1;
                vuint32_t WRE11:1;
                vuint32_t WRE10:1;
                vuint32_t WRE9:1;
                vuint32_t WRE8:1;
                vuint32_t WRE7:1;
                vuint32_t WRE6:1;
                vuint32_t WRE5:1;
                vuint32_t WRE4:1;
                vuint32_t WRE3:1;
                vuint32_t WRE2:1;
                vuint32_t WRE1:1;
                vuint32_t WRE0:1;
            } B;
        } WRER;                        /* Wakeup Request Enable Register */

        int32_t WKUP_reserved2[2];

        union {
            vuint32_t R;
            struct {
                  vuint32_t:11;
                vuint32_t IREE20:1;
                vuint32_t IREE19:1;
                vuint32_t IREE18:1;
                vuint32_t IREE17:1;
                vuint32_t IREE16:1;
                vuint32_t IREE15:1;
                vuint32_t IREE14:1;
                vuint32_t IREE13:1;
                vuint32_t IREE12:1;
                vuint32_t IREE11:1;
                vuint32_t IREE10:1;
                vuint32_t IREE9:1;
                vuint32_t IREE8:1;
                vuint32_t IREE7:1;
                vuint32_t IREE6:1;
                vuint32_t IREE5:1;
                vuint32_t IREE4:1;
                vuint32_t IREE3:1;
                vuint32_t IREE2:1;
                vuint32_t IREE1:1;
                vuint32_t IREE0:1;
            } B;
        } WIREER;                      /* Wakeup/Interrupt Rising-Edge Event Enable Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:11;
                vuint32_t IFEE20:1;
                vuint32_t IFEE19:1;
                vuint32_t IFEE18:1;
                vuint32_t IFEE17:1;
                vuint32_t IFEE16:1;
                vuint32_t IFEE15:1;
                vuint32_t IFEE14:1;
                vuint32_t IFEE13:1;
                vuint32_t IFEE12:1;
                vuint32_t IFEE11:1;
                vuint32_t IFEE10:1;
                vuint32_t IFEE9:1;
                vuint32_t IFEE8:1;
                vuint32_t IFEE7:1;
                vuint32_t IFEE6:1;
                vuint32_t IFEE5:1;
                vuint32_t IFEE4:1;
                vuint32_t IFEE3:1;
                vuint32_t IFEE2:1;
                vuint32_t IFEE1:1;
                vuint32_t IFEE0:1;
            } B;
        } WIFEER;                      /* Wakeup/Interrupt Falling-Edge Event Enable Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:11;
                vuint32_t IFE20:1;
                vuint32_t IFE19:1;
                vuint32_t IFE18:1;
                vuint32_t IFE17:1;
                vuint32_t IFE16:1;
                vuint32_t IFE15:1;
                vuint32_t IFE14:1;
                vuint32_t IFE13:1;
                vuint32_t IFE12:1;
                vuint32_t IFE11:1;
                vuint32_t IFE10:1;
                vuint32_t IFE9:1;
                vuint32_t IFE8:1;
                vuint32_t IFE7:1;
                vuint32_t IFE6:1;
                vuint32_t IFE5:1;
                vuint32_t IFE4:1;
                vuint32_t IFE3:1;
                vuint32_t IFE2:1;
                vuint32_t IFE1:1;
                vuint32_t IFE0:1;
            } B;
        } WIFER;                       /* Wakeup/Interrupt Filter Enable Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:11;
                vuint32_t IPUE20:1;
                vuint32_t IPUE19:1;
                vuint32_t IPUE18:1;
                vuint32_t IPUE17:1;
                vuint32_t IPUE16:1;
                vuint32_t IPUE15:1;
                vuint32_t IPUE14:1;
                vuint32_t IPUE13:1;
                vuint32_t IPUE12:1;
                vuint32_t IPUE11:1;
                vuint32_t IPUE10:1;
                vuint32_t IPUE9:1;
                vuint32_t IPUE8:1;
                vuint32_t IPUE7:1;
                vuint32_t IPUE6:1;
                vuint32_t IPUE5:1;
                vuint32_t IPUE4:1;
                vuint32_t IPUE3:1;
                vuint32_t IPUE2:1;
                vuint32_t IPUE1:1;
                vuint32_t IPUE0:1;
            } B;
        } WIPUER;                      /* Wakeup/Interrupt Pullup Enable Register */

    };                                 /* end of WKUP_tag */
/****************************************************************************/
/*                             MODULE : LINFLEX                             */
/****************************************************************************/

    struct LINFLEX_tag {

        int16_t LINFLEX_reserved1;

        union {
            vuint16_t R;
            struct {
                vuint16_t CCD:1;
                vuint16_t CFD:1;
                vuint16_t LASE:1;
                vuint16_t AWUM:1;      // LCH vuint16_t AUTOWU:1;
                vuint16_t MBL:4;
                vuint16_t BF:1;
                vuint16_t SLFM:1;
                vuint16_t LBKM:1;
                vuint16_t MME:1;
                vuint16_t SBDT:1;      // LCH vuint16_t SSBL:1;
                vuint16_t RBLM:1;
                vuint16_t SLEEP:1;
                vuint16_t INIT:1;
            } B;
        } LINCR1;                      /* LINFLEX LIN Control Register 1 */

        int16_t LINFLEX_reserved2;

        union {
            vuint16_t R;
            struct {
                vuint16_t SZIE:1;
                vuint16_t OCIE:1;
                vuint16_t BEIE:1;
                vuint16_t CEIE:1;
                vuint16_t HEIE:1;
                  vuint16_t:2;
                vuint16_t FEIE:1;
                vuint16_t BOIE:1;
                vuint16_t LSIE:1;
                vuint16_t WUIE:1;
                vuint16_t DBFIE:1;
                vuint16_t DBEIE:1;
                vuint16_t DRIE:1;
                vuint16_t DTIE:1;
                vuint16_t HRIE:1;
            } B;
        } LINIER;                      /* LINFLEX LIN Interrupt Enable Register */

        int16_t LINFLEX_reserved3;

        union {
            vuint16_t R;
            struct {
                vuint16_t LINS:4;
                  vuint16_t:2;
                vuint16_t RMB:1;
                  vuint16_t:1;
                vuint16_t RBSY:1;      // LCH vuint16_t RXBUSY:1;
                vuint16_t RPS:1;       // LCH vuint16_t RDI:1;
                vuint16_t WUF:1;
                vuint16_t DBFF:1;
                vuint16_t DBEF:1;
                vuint16_t DRF:1;
                vuint16_t DTF:1;
                vuint16_t HRF:1;
            } B;
        } LINSR;                       /* LINFLEX LIN Status Register */

        int16_t LINFLEX_reserved4;

        union {
            vuint16_t R;
            struct {
                vuint16_t SZF:1;
                vuint16_t OCF:1;
                vuint16_t BEF:1;
                vuint16_t CEF:1;
                vuint16_t SFEF:1;
                vuint16_t SDEF:1;
                vuint16_t IDPEF:1;
                vuint16_t FEF:1;
                vuint16_t BOF:1;
                  vuint16_t:6;
                vuint16_t NF:1;
            } B;
        } LINESR;                      /* LINFLEX LIN Error Status Register */

        int16_t LINFLEX_reserved5;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:1;
                vuint16_t TDFL:2;
                  vuint16_t:1;
                vuint16_t RDFL:2;
                  vuint16_t:4;
                vuint16_t RXEN:1;
                vuint16_t TXEN:1;
                vuint16_t OP:1;        //LCH vuint16_t PARITYODD:1;
                vuint16_t PCE:1;
                vuint16_t WL:1;
                vuint16_t UART:1;
            } B;
        } UARTCR;                      /* LINFLEX UART Mode Control Register */

        int16_t LINFLEX_reserved6;

        union {
            vuint16_t R;
            struct {
                vuint16_t SZF:1;
                vuint16_t OCF:1;
                vuint16_t PE:4;
                vuint16_t RMB:1;
                vuint16_t FEF:1;
                vuint16_t BOF:1;
                vuint16_t RPS:1;       // LCH vuint16_t RDI:1;
                vuint16_t WUF:1;
                  vuint16_t:2;
                vuint16_t DRF:1;
                vuint16_t DTF:1;
                vuint16_t NF:1;
            } B;
        } UARTSR;                      /* LINFLEX UART Mode Status Register */

        int16_t LINFLEX_reserved7;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:5;
                vuint16_t LTOM:1;      //LCH vuint16_t MODE:1;
                vuint16_t IOT:1;
                vuint16_t TOCE:1;
                vuint16_t CNT:8;
            } B;
        } LINTCSR;                     /* LINFLEX LIN Time-Out Control Status Register */

        int16_t LINFLEX_reserved8;

        union {
            vuint16_t R;
            struct {
                vuint16_t OC2:8;
                vuint16_t OC1:8;
            } B;
        } LINOCR;                      /* LINFLEX LIN Output Compare Register */

        int16_t LINFLEX_reserved9;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:4;
                vuint16_t RTO:4;       // LCH vuint16_t RTC:4;
                  vuint16_t:1;
                vuint16_t HTO:7;       // LCH vuint16_t HTC:7;
            } B;
        } LINTOCR;                     /* LINFLEX LIN Output Compare Register */

        int16_t LINFLEX_reserved10;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:12;
                vuint16_t DIV_F:4;     // LCH vuint16_t FBR:4;
            } B;
        } LINFBRR;                     /* LINFLEX LIN Fractional Baud Rate Register */

        int16_t LINFLEX_reserved11;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DIV_M:13;    // LCH vuint16_t IBR:13;
            } B;
        } LINIBRR;                     /* LINFLEX LIN Integer Baud Rate Register */

        int16_t LINFLEX_reserved12;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:8;
                vuint16_t CF:8;
            } B;
        } LINCFR;                      /* LINFLEX LIN Checksum Field Register */

        int16_t LINFLEX_reserved13;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:1;
                vuint16_t IOBE:1;
                vuint16_t IOPE:1;
                vuint16_t WURQ:1;
                vuint16_t DDRQ:1;
                vuint16_t DTRQ:1;
                vuint16_t ABRQ:1;
                vuint16_t HTRQ:1;
                  vuint16_t:8;
            } B;
        } LINCR2;                      /* LINFLEX LIN Control Register 2 */

        int16_t LINFLEX_reserved14;

        union {
            vuint16_t R;
            struct {
                vuint16_t DFL:6;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;         // LCH vuint16_t:1;
                vuint16_t ID:6;
            } B;
        } BIDR;                        /* LINFLEX Buffer Identifier Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA3:8;
                vuint32_t DATA2:8;
                vuint32_t DATA1:8;
                vuint32_t DATA0:8;
            } B;
        } BDRL;                        /* LINFLEX Buffer Data Register Least Significant */

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA7:8;
                vuint32_t DATA6:8;
                vuint32_t DATA5:8;
                vuint32_t DATA4:8;
            } B;
        } BDRM;                        /* LINFLEX Buffer Data Register Most Significant */

        int16_t LINFLEX_reserved15;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:8;
                vuint16_t FACT:8;
            } B;
        } IFER;                        /* LINFLEX Identifier Filter Enable Register */

        int16_t LINFLEX_reserved16;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:11;
                vuint16_t IFMI:5;
            } B;
        } IFMI;                        /* LINFLEX Identifier Filter Match Index Register */

        int16_t LINFLEX_reserved17;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:8;
                vuint16_t IFM:8;
            } B;
        } IFMR;                        /* LINFLEX Identifier Filter Mode Register */

        int16_t LINFLEX_reserved18;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DFL:3;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;
                vuint16_t ID:6;
            } B;
        } IFCR0;                       /* LINFLEX Identifier Filter Control Register 0 */

        int16_t LINFLEX_reserved19;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DFL:3;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;
                vuint16_t ID:6;
            } B;
        } IFCR1;                       /* LINFLEX Identifier Filter Control Register 1 */

        int16_t LINFLEX_reserved20;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DFL:3;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;
                vuint16_t ID:6;
            } B;
        } IFCR2;                       /* LINFLEX Identifier Filter Control Register 2 */

        int16_t LINFLEX_reserved21;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DFL:3;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;
                vuint16_t ID:6;
            } B;
        } IFCR3;                       /* LINFLEX Identifier Filter Control Register 3 */

        int16_t LINFLEX_reserved22;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DFL:3;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;
                vuint16_t ID:6;
            } B;
        } IFCR4;                       /* LINFLEX Identifier Filter Control Register 4 */

        int16_t LINFLEX_reserved23;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DFL:3;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;
                vuint16_t ID:6;
            } B;
        } IFCR5;                       /* LINFLEX Identifier Filter Control Register 5 */

        int16_t LINFLEX_reserved24;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DFL:3;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;
                vuint16_t ID:6;
            } B;
        } IFCR6;                       /* LINFLEX Identifier Filter Control Register 6 */

        int16_t LINFLEX_reserved25;

        union {
            vuint16_t R;
            struct {
                  vuint16_t:3;
                vuint16_t DFL:3;
                vuint16_t DIR:1;
                vuint16_t CCS:1;
                  vuint16_t:2;
                vuint16_t ID:6;
            } B;
        } IFCR7;                       /* LINFLEX Identifier Filter Control Register 7 */

    };                                 /* end of LINFLEX_tag */
/****************************************************************************/
/*                             MODULE : ME                                  */
/****************************************************************************/
    struct ME_tag {

        union {
            vuint32_t R;
            struct {
                vuint32_t S_CURRENTMODE:4;
                vuint32_t S_MTRANS:1;
                vuint32_t S_DC:1;
                  vuint32_t:2;
                vuint32_t S_PDO:1;
                  vuint32_t:2;
                vuint32_t S_MVR:1;
                vuint32_t S_DFLA:2;
                vuint32_t S_CFLA:2;
                  vuint32_t:8;
                vuint32_t S_FMPLL1:1;
                vuint32_t S_FMPLL0:1;
                vuint32_t S_FXOSC:1;
                vuint32_t S_FIRC:1;
                vuint32_t S_SYSCLK:4;
            } B;
        } GS;                          /* Global Status Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t TARGET_MODE:4;
                  vuint32_t:12;
                vuint32_t KEY:16;
            } B;
        } MCTL;                        /* Mode Control Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:18;
                vuint32_t STANDBY:1;
                  vuint32_t:2;
                vuint32_t STOP:1;
                  vuint32_t:1;
                vuint32_t HALT:1;
                vuint32_t RUN3:1;
                vuint32_t RUN2:1;
                vuint32_t RUN1:1;
                vuint32_t RUN0:1;
                vuint32_t DRUN:1;
                vuint32_t SAFE:1;
                vuint32_t TEST:1;
                vuint32_t RESET:1;
            } B;
        } MER;                         /* Mode Enable Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:28;
                vuint32_t I_ICONF:1;
                vuint32_t I_IMODE:1;
                vuint32_t I_SAFE:1;
                vuint32_t I_MTC:1;
            } B;
        } IS;                          /* Interrupt Status Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:28;
                vuint32_t M_ICONF:1;
                vuint32_t M_IMODE:1;
                vuint32_t M_SAFE:1;
                vuint32_t M_MTC:1;
            } B;
        } IM;                          /* Interrupt Mask Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:27;
                vuint32_t S_MTI:1;
                vuint32_t S_MRI:1;
                vuint32_t S_DMA:1;
                vuint32_t S_NMA:1;
                vuint32_t S_SEA:1;
            } B;
        } IMTS;                        /* Invalid Mode Transition Status Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:8;
                vuint32_t MPH_BUSY:1;
                  vuint32_t:2;
                vuint32_t PMC_PROG:1;
                vuint32_t CORE_DBG:1;
                  vuint32_t:2;
                vuint32_t SMR:1;
                vuint32_t FMPLL1_SC:1;
                vuint32_t FMPLL0_SC:1;
                vuint32_t FXOSC_SC:1;
                vuint32_t FIRC_SC:1;
                vuint32_t SSCLK_SC:1;
                vuint32_t SYSCLK_SW:1;
                vuint32_t DFLASH_SC:1;
                vuint32_t CFLASH_SC:1;
                vuint32_t CDP_PRPH_0_143:1;
                  vuint32_t:3;
                vuint32_t CDP_PRPH_96_127:1;
                vuint32_t CDP_PRPH_64_95:1;
                vuint32_t CDP_PRPH_32_63:1;
                vuint32_t CDP_PRPH_0_31:1;
            } B;
        } DMTS;                        /* Invalid Mode Transition Status Register */

        int32_t ME_reserved0;

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t PDO:1;
                  vuint32_t:2;
                vuint32_t MVRON:1;
                vuint32_t DFLAON:2;
                vuint32_t CFLAON:2;
                  vuint32_t:8;
                vuint32_t FMPLL1ON:1;
                vuint32_t FMPLL0ON:1;
                vuint32_t FXOSCON:1;
                vuint32_t FIRCON:1;
                vuint32_t SYSCLK:4;
            } B;
        } RESET;                       /* Reset Mode Configuration Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t PDO:1;
                  vuint32_t:2;
                vuint32_t MVRON:1;
                vuint32_t DFLAON:2;
                vuint32_t CFLAON:2;
                  vuint32_t:8;
                vuint32_t FMPLL1ON:1;
                vuint32_t FMPLL0ON:1;
                vuint32_t FXOSCON:1;
                vuint32_t FIRCON:1;
                vuint32_t SYSCLK:4;
            } B;
        } TEST;                        /* Test Mode Configuration Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t PDO:1;
                  vuint32_t:2;
                vuint32_t MVRON:1;
                vuint32_t DFLAON:2;
                vuint32_t CFLAON:2;
                  vuint32_t:8;
                vuint32_t FMPLL1ON:1;
                vuint32_t FMPLL0ON:1;
                vuint32_t FXOSCON:1;
                vuint32_t FIRCON:1;
                vuint32_t SYSCLK:4;
            } B;
        } SAFE;                        /* Safe Mode Configuration Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t PDO:1;
                  vuint32_t:2;
                vuint32_t MVRON:1;
                vuint32_t DFLAON:2;
                vuint32_t CFLAON:2;
                  vuint32_t:8;
                vuint32_t FMPLL1ON:1;
                vuint32_t FMPLL0ON:1;
                vuint32_t FXOSCON:1;
                vuint32_t FIRCON:1;
                vuint32_t SYSCLK:4;
            } B;
        } DRUN;                        /* DRUN Mode Configuration Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t PDO:1;
                  vuint32_t:2;
                vuint32_t MVRON:1;
                vuint32_t DFLAON:2;
                vuint32_t CFLAON:2;
                  vuint32_t:8;
                vuint32_t FMPLL1ON:1;
                vuint32_t FMPLL0ON:1;
                vuint32_t FXOSCON:1;
                vuint32_t FIRCON:1;
                vuint32_t SYSCLK:4;
            } B;
        } RUN[4];                      /* RUN 0->4 Mode Configuration Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t PDO:1;
                  vuint32_t:2;
                vuint32_t MVRON:1;
                vuint32_t DFLAON:2;
                vuint32_t CFLAON:2;
                  vuint32_t:8;
                vuint32_t FMPLL1ON:1;
                vuint32_t FMPLL0ON:1;
                vuint32_t FXOSCON:1;
                vuint32_t FIRCON:1;
                vuint32_t SYSCLK:4;
            } B;
        } HALT;                        /* HALT0 Mode Configuration Register */

        int32_t ME_reserved1;

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t PDO:1;
                  vuint32_t:2;
                vuint32_t MVRON:1;
                vuint32_t DFLAON:2;
                vuint32_t CFLAON:2;
                  vuint32_t:8;
                vuint32_t FMPLL1ON:1;
                vuint32_t FMPLL0ON:1;
                vuint32_t FXOSCON:1;
                vuint32_t FIRCON:1;
                vuint32_t SYSCLK:4;
            } B;
        } STOP;                        /* STOP0 Mode Configuration Register */

        int32_t ME_reserved2[2];

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t PDO:1;
                  vuint32_t:2;
                vuint32_t MVRON:1;
                vuint32_t DFLAON:2;
                vuint32_t CFLAON:2;
                  vuint32_t:8;
                vuint32_t FMPLL1ON:1;
                vuint32_t FMPLL0ON:1;
                vuint32_t FXOSCON:1;
                vuint32_t FIRCON:1;
                vuint32_t SYSCLK:4;
            } B;
        } STANDBY;                     /* STANDBY0 Mode Configuration Register */

        int32_t ME_reserved3[2];

/* Former declaration included to allow backwards compatibility - Peripheral Status 0->4 Register
        union {
            vuint32_t R;
            struct {
                vuint32_t PERIPH:32;
            } B;
        } PS[4];                        */

        union {
            vuint32_t R;
            struct {
                vuint32_t S_BAM:1;
                  vuint32_t:7;
                vuint32_t S_DMA_CH_MUX:1;
                  vuint32_t:5;
                vuint32_t S_FLEXCAN1:1;
                vuint32_t S_FLEXCAN0:1;
                  vuint32_t:5;
                vuint32_t S_QUADSPI:1;
                  vuint32_t:4;
                vuint32_t S_DSPI1:1;
                vuint32_t S_DSPI0:1;
                  vuint32_t:4;
            } B;
        } PS0;                         /* Peripheral Status 0 Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t S_DCU:1;
                vuint32_t S_SGL:1;
                vuint32_t S_LCD:1;
                vuint32_t S_CANSAMPLER:1;
                  vuint32_t:3;
                vuint32_t S_GAUGEDRIVER:1;
                  vuint32_t:6;
                vuint32_t S_LINFLEX1:1;
                vuint32_t S_LINFLEX0:1;
                vuint32_t S_I2C_DMA3:1;
                vuint32_t S_I2C_DMA2:1;
                vuint32_t S_I2C_DMA1:1;
                vuint32_t S_I2C_DMA0:1;
                  vuint32_t:11;
                vuint32_t S_ADC0:1;
            } B;
        } PS1;                         /* Peripheral Status 1 Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:3;
                vuint32_t S_PIT_RTI:1;
                vuint32_t S_RTC_API:1;
                vuint32_t S_MC_PCU:1;
                vuint32_t S_MC_RGM:1;
                vuint32_t S_MC_CGM:1;
                vuint32_t S_MC_ME:1;
                vuint32_t S_SSCM:1;
                  vuint32_t:9;
                vuint32_t S_CFLASH1:1;
                  vuint32_t:2;
                vuint32_t S_EMIOS1:1;
                vuint32_t S_EMIOS0:1;
                  vuint32_t:2;
                vuint32_t S_WKPU:1;
                vuint32_t S_SIUL:1;
                vuint32_t S_DFLASH0:1;
                vuint32_t S_CFLASH0:1;
                  vuint32_t:2;
            } B;
        } PS2;                         /* Peripheral Status 0 Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:24;
                vuint32_t S_CMU0:1;
                  vuint32_t:7;
            } B;
        } PS3;                         /* Peripheral Status 0 Register */

        int32_t ME_reserved4[4];

        union {
            vuint32_t R;
            struct {
                  vuint32_t:24;
                vuint32_t RUN3:1;
                vuint32_t RUN2:1;
                vuint32_t RUN1:1;
                vuint32_t RUN0:1;
                vuint32_t DRUN:1;
                vuint32_t SAFE:1;
                vuint32_t TEST:1;
                vuint32_t RESET:1;
            } B;
        } RUNPC[8];                    /* RUN Peripheral Configuration 0->7 Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:18;
                vuint32_t STANDBY:1;
                  vuint32_t:2;
                vuint32_t STOP:1;
                  vuint32_t:1;
                vuint32_t HALT:1;
                  vuint32_t:8;
            } B;
        } LPPC[8];                     /* Low Power Peripheral Configuration 0->7 Register */

        union {
            vuint8_t R;
            struct {
                  vuint8_t:1;
                vuint8_t DBG_F:1;
                vuint8_t LP_CFG:3;
                vuint8_t RUN_CFG:3;
            } B;
        } PCTL[144];                   /* Peripheral Control 0->143 Register */

    };                                 /* end of ME_tag */
/****************************************************************************/
/*                             MODULE : CGM                                 */
/****************************************************************************/
    struct CGM_tag {

    /* The CGM provides a unified register interface, enabling access to 
       all clock sources:

     Base Address |  Clock Sources
    -------------------------------
     C3FE0000      | FXOSC_CTL
     -              | Reserved
     C3FE0040      | SXOSC_CTL
     C3FE0060      | FIRC_CTL
     C3FE0080      | SIRC_CTL
     C3FE00A0      | FMPLL_0
     C3FE00C0      | FMPLL_1
     -              | Reserved
     C3FE0100      | CMU_0 & CMU_1
    */

    /***************************************/
    /* OSC_CTL @ CGM base address + 0x0000 */
    /***************************************/
        union {
            vuint32_t R;
            struct {
                vuint32_t OSCBYP:1;
                  vuint32_t:7;
                vuint32_t EOCV:8;
                vuint32_t M_OSC:1;
                  vuint32_t:2;
                vuint32_t OSCDIV:5;
                vuint32_t I_OSC:1;
                  vuint32_t:7;
            } B;
        } FXOSC_CTL;                   /* Main OSC Control Register */

    /*****************************************/
    /* LPOSC_CTL @ CGM base address + 0x0040 */
    /*****************************************/
        int32_t CGM_reserved0[15];     /* (0x040 - 0x004)/4 = 0x0F */

        union {
            vuint32_t R;
            struct {
                vuint32_t OSCBYP:1;
                  vuint32_t:7;
                vuint32_t EOCV:8;
                vuint32_t M_OSC:1;
                  vuint32_t:2;
                vuint32_t OSCDIV:5;
                vuint32_t I_OSC:1;
                  vuint32_t:5;
                vuint32_t S_OSC:1;
                vuint32_t OSCON:1;
            } B;
        } SXOSC_CTL;                   /* Low Power OSC Control Register */

    /**************************************/
    /* RC_CTL @ CGM base address + 0x0060 */
    /**************************************/
        int32_t CGM_reserved1[7];      /* (0x060 - 0x044)/4 = 0x07 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:10;
                vuint32_t RCTRIM:6;
                  vuint32_t:3;
                vuint32_t RCDIV:5;
                  vuint32_t:2;
                vuint32_t S_RC_STDBY:1;
                  vuint32_t:5;
            } B;
        } FIRC_CTL;                    /* RC OSC Control Register */

    /****************************************/
    /* LPRC_CTL @ CGM base address + 0x0080 */
    /****************************************/
        int32_t CGM_reserved2[7];      /* (0x080 - 0x064)/4 = 0x07 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:11;
                vuint32_t LRCTRIM:5;
                  vuint32_t:3;
                vuint32_t LPRCDIV:5;
                  vuint32_t:3;
                vuint32_t S_LPRC:1;
                  vuint32_t:3;
                vuint32_t LPRCON_STDBY:1;
            } B;
        } SIRC_CTL;                    /* Low Power RC OSC Control Register */

    /***************************************/
    /* FMPLL_0 @ CGM base address + 0x00A0 */
    /* FMPLL_1 @ CGM base address + 0x00C0 */
    /***************************************/
        int32_t CGM_reserved3[7];      /* (0x0A0 - 0x084)/4 = 0x07 */

        struct {
            union {
                vuint32_t R;
                 struct {
                       vuint32_t:2;
                     vuint32_t IDF:4;
                     vuint32_t ODF:2;
                      vuint32_t:1;
                    vuint32_t NDIV:7;
                      vuint32_t:7;
                    vuint32_t EN_PLL_SW:1;
                    vuint32_t MODE:1;
                    vuint32_t UNLOCK_ONCE:1;
                      vuint32_t:1;
                    vuint32_t I_LOCK:1;
                    vuint32_t S_LOCK:1;
                    vuint32_t PLL_FAIL_MASK:1;
                    vuint32_t PLL_FAIL_FLAG:1;
                      vuint32_t:1;
                } B;
            } CR;                      /* FMPLL Control Register */

            union {
                vuint32_t R;
                struct {
                    vuint32_t STRB_BYPASS:1;
                      vuint32_t:1;
                    vuint32_t SPRD_SEL:1;
                    vuint32_t MOD_PERIOD:13;
                    vuint32_t FM_EN:1;
                    vuint32_t INC_STEP:15;
                } B;
            } MR;                      /* FMPLL Modulation Register */

            int32_t CGM_reserved4[6];  /* (0x0C0 - 0x0A8)/4 = 0x06 */
                                       /* (0x0E0 - 0x0C8)/4 = 0x06 */

        } FMPLL[2];

    /************************************/
    /* CMU @ CGM base address + 0x0100  */
    /************************************/
        int32_t CGM_reserved5[8];      /* (0x100 - 0x0E0)/4 = 0x08 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t SFM:1;
                  vuint32_t:13;
                vuint32_t CLKSEL1:2;
                  vuint32_t:5;
                vuint32_t RCDIV:2;
                vuint32_t CME_A:1;
            } B;
        } CMU_CSR;                     /* Control Status Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:12;
                vuint32_t FD:20;
            } B;
        } CMU_FDR;                     /* Frequency Display Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:20;
                vuint32_t HFREF_A:12;
            } B;
        } CMU_HFREFR_A;                /* High Frequency Reference Register PLL_A Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:20;
                vuint32_t LFREF_A:12;
            } B;
        } CMU_LFREFR_A;                /* Low Frequency Reference Register PLL_A Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:29;
                vuint32_t FHHI:1;
                vuint32_t FLLI:1;
                vuint32_t OLRI:1;
            } B;
        } CMU_ISR;                     /* Interrupt Status Register */

        int32_t CGM_reserved6;         /* 0x0014 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:12;
                vuint32_t MD:20;
            } B;
        } CMU_MDR;                    /* Measurement Duration Register */


    /*****************************************************/
    /* CGM General Registers @ CGM base address + 0x0370 */
    /*****************************************************/
        int32_t CGM_reserved7[149];    /* (0x370 - 0x11C)/4 = 0x95 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:31;
                vuint32_t EN:1;
            } B;
        } OC_EN;                       /* Output Clock Enable Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:2;
                vuint32_t SELDIV:2;
                vuint32_t SELCTL:4;
                  vuint32_t:24;
            } B;
        } OCDS_SC;                     /* Output Clock Division Select Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t SELSTAT:4;
                  vuint32_t:24;
            } B;
        } SC_SS;                       /* System Clock Select Status */

        union {
            vuint8_t R;
            struct {
                vuint8_t DE:1;
                  vuint8_t:3;
                vuint8_t DIV:4;
            } B;
        } SC_DC[3];                    /* System Clock Divider Configuration 0->2 */

        int8_t CGM_reserved8;          /* 0x037F */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t SELCTL:4;
                  vuint32_t:24;
            } B;
        } AC0_SC;                      /* Aux Clock 0 Select Control */

        int32_t CGM_reserved9;         /* 0x0384 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t SELCTL:4;
                  vuint32_t:24;
            } B;
        } AC1_SC;                      /* Aux Clock 1 Select Control */

        union {
            vuint32_t R;
            struct {
                vuint32_t DE0:1;
                  vuint32_t:3;
                vuint32_t DIV0:4;
                  vuint32_t:24;
            } B;
        } AC1_DC;                      /* Aux Clock 1 Divider Configuration 0->3 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t SELCTL:4;
                  vuint32_t:24;
            } B;
        } AC2_SC;                      /* Aux Clock 2 Select Control */

        union {
            vuint32_t R;
            struct {
                vuint32_t DE0:1;
                  vuint32_t:3;
                vuint32_t DIV0:4;
                  vuint32_t:24;
            } B;
        } AC2_DC;                      /* Aux Clock 2 Divider Configuration 0->3 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t SELCTL:4;
                  vuint32_t:24;
            } B;
        } AC3_SC;                      /* Aux Clock 3 Select Control */

        union {
            vuint32_t R;
            struct {
                vuint32_t DE0:1;
                  vuint32_t:3;
                vuint32_t DIV0:4;
                  vuint32_t:24;
            } B;
        } AC3_DC;                      /* Aux Clock 3 Divider Configuration 0->3 */
  
    };                                 /* end of CGM_tag */
/****************************************************************************/
/*                             MODULE : RGM                                 */
/****************************************************************************/
    struct RGM_tag {

        union {
            vuint16_t R;
            struct {
                vuint16_t F_EXR:1;
                  vuint16_t:6;
                vuint16_t F_FLASH:1;
                vuint16_t F_LVD45:1;
                vuint16_t F_CMU_FHL:1;
                vuint16_t F_CMU_OLR:1;
                vuint16_t F_FMPLL0:1;
                vuint16_t F_CHKSTOP:1;
                vuint16_t F_SOFT:1;
                vuint16_t F_CORE:1;
                vuint16_t F_JTAG:1;
            } B;
        } FES;                         /* Functional Event Status */

        union {
            vuint16_t R;
            struct {
                vuint16_t F_POR:1;
                  vuint16_t:11;
                vuint16_t F_LVD27:1;
                vuint16_t F_SWT:1;
                vuint16_t F_LVD12_PD1:1;
                vuint16_t F_LVD12_PD0:1;
            } B;
        } DES;                         /* Destructive Event Status */

        union {
            vuint16_t R;
            struct {
                vuint16_t D_EXR:1;
                  vuint16_t:6;
                vuint16_t D_FLASH:1;
                vuint16_t D_LVD45:1;
                vuint16_t D_CMU0_FHL:1;
                vuint16_t D_CMU0_OLR:1;
                vuint16_t D_FMPLL0:1;
                vuint16_t D_CHKSTOP:1;
                vuint16_t D_SOFT:1;
                vuint16_t D_CORE:1;
                vuint16_t D_JTAG:1;
            } B;
        } FERD;                        /* Functional Event Reset Disable */

        union {
            vuint16_t R;
            struct {
                  vuint16_t:12;
                vuint16_t D_LVD27:1;
                vuint16_t D_SWT:1;
                vuint16_t D_LVD12_PD1:1;
                vuint16_t D_LVD12_PD0:1;
            } B;
        } DERD;                        /* Destructive Event Reset Disable */

        int16_t RGM_reserved0[4];

        union {
            vuint16_t R;
            struct {
                vuint16_t AR_EXR:1;
                  vuint16_t:6;
                vuint16_t AR_FLASH:1;
                vuint16_t AR_LVD45:1;
                vuint16_t AR_CMU_FHL:1;
                vuint16_t AR_CMU_OLR:1;
                vuint16_t AR_FMPLL0:1;
                vuint16_t AR_CHKSTOP:1;
                vuint16_t AR_SOFT:1;
                vuint16_t AR_CORE:1;
                vuint16_t AR_JTAG:1;
            } B;
        } FEAR;                        /* Functional Event Alternate Request */

        union {
            vuint16_t R;
            struct {
                  vuint16_t:12;
                vuint16_t AR_LVD27:1;
                vuint16_t AR_SWT:1;
                vuint16_t AR_LVD12_PD1:1;
                vuint16_t AR_LVD12_PD0:1;
            } B;
        } DEAR;                        /* Destructive Event Alternate Request */

        int16_t RGM_reserved1[2];

        union {
            vuint16_t R;
            struct {
                vuint16_t SS_EXR:1;
                  vuint16_t:6;
                vuint16_t SS_FLASH:1;
                vuint16_t SS_LVD45:1;
                vuint16_t SS_CMU0_FHL:1;
                vuint16_t SS_CMU0_OLR:1;
                vuint16_t SS_FMPLL0:1;
                vuint16_t SS_CHKSTOP:1;
                vuint16_t SS_SOFT:1;
                vuint16_t SS_CORE:1;
                vuint16_t SS_JTAG:1;
            } B;
        } FESS;                        /* Functional Event Short Sequence */

        union {
            vuint16_t R;
            struct {
                  vuint16_t:8;
                vuint16_t BOOT_FROM_BKP_RAM:1;
                  vuint16_t:7;
            } B;
        } STDBY;                       /* STANDBY reset sequence */

        union {
            vuint16_t R;
            struct {
                vuint16_t BE_EXR:1;
                  vuint16_t:6;
                vuint16_t BE_FLASH:1;
                vuint16_t BE_LVD45:1;
                vuint16_t BE_CMU0_FHL:1;
                vuint16_t BE_CMU0_OLR:1;
                vuint16_t BE_FMPLL0:1;
                vuint16_t BE_CHKSTOP:1;
                vuint16_t BE_SOFT:1;
                vuint16_t BE_CORE:1;
                vuint16_t BE_JTAG:1;
            } B;
        } FBRE;                        /* Functional Bidirectional Reset Enable */

    };                                 /* end of RGM_tag */
/****************************************************************************/
/*                             MODULE : PCU                                 */
/****************************************************************************/
    struct PCU_tag {

        union {
            vuint32_t R;
            struct {
                  vuint32_t:18;
                vuint32_t STBY0:1;
                  vuint32_t:2;
                vuint32_t STOP0:1;
                  vuint32_t:1;
                vuint32_t HALT0:1;
                vuint32_t RUN3:1;
                vuint32_t RUN2:1;
                vuint32_t RUN1:1;
                vuint32_t RUN0:1;
                vuint32_t DRUN:1;
                vuint32_t SAFE:1;
                vuint32_t TEST:1;
                vuint32_t RST:1;
            } B;
        } PCONF[3];                    /* Power domain 0-15 configuration register */

        int32_t PCU_reserved0[13];     /* (0x040 - 0x00C)/4 = 0x0D */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:29;
                vuint32_t PD2:1;
                vuint32_t PD1:1;
                vuint32_t PD0:1;
            } B;
        } PSTAT;                       /* Power Domain Status Register */

        int32_t PCU_reserved1[15];     /* {0x0080-0x0044}/0x4 = 0xF */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:31;
                vuint32_t MASK_LVDHV5:1;
            } B;
        } VREG_CTL;                    /* Voltage Regulator Control Register */

    };                                 /* end of PCU_tag */
/****************************************************************************/
/*                             MODULE : DCU                                 */
/****************************************************************************/
    struct DCU_FG_tag {

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t COLOR:24;
            } B;
        } F;                           /* Transparency layer foreground color */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t COLOR:24;
            } B;
        } B;                           /* Transparency layer foreground color */
    };
    
    struct DCU_LAYER_tag {

        union {
            vuint32_t R;
            struct {
                vuint32_t:6;
                vuint32_t HEIGHT:10;
                  vuint32_t:5;
                vuint32_t WIDTH:11;
            } B;
        } CTRLDESCL1;                  /* LAYER[X].CTRLDESCL1 */

        union {
            vuint32_t R;
            struct {
                vuint32_t:6;
                vuint32_t POSY:10;
                  vuint32_t:4;
                vuint32_t POSX:12;
            } B;
        } CTRLDESCL2;                  /* LAYER[X].CTRLDESCL2 */

        union {
            vuint32_t R;
            struct {
                vuint32_t ADDR:32;
            } B;
        } CTRLDESCL3;                  /* LAYER[X].CTRLDESCL3 */

        union {
            vuint32_t R;
            struct {
                vuint32_t EN:1;
                vuint32_t TILE_EN:1;
                vuint32_t DATA_SEL:1;
                vuint32_t SAFETY_EN:1;
                vuint32_t TRANS:8;
                vuint32_t BPP:4;
                  vuint32_t:1;
                vuint32_t LUOFFS:11;
                  vuint32_t:1;
                vuint32_t BB:1;
                vuint32_t AB:2;
            } B;
        } CTRLDESCL4;                  /* LAYER[X].CTRLDESCL4 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t CKMAX_R:8;
                vuint32_t CKMAX_G:8;
                vuint32_t CKMAX_B:8;
            } B;
        } CTRLDESCL5;                  /* LAYER[X].CTRLDESCL5 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t CKMIN_R:8;
                vuint32_t CKMIN_G:8;
                vuint32_t CKMIN_B:8;
            } B;
        } CTRLDESCL6;                  /* LAYER[X].CTRLDESCL6 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:6;
                vuint32_t TILE_VER_SIZE:10;
                  vuint32_t:8;
                vuint32_t TILE_HOR_SIZE:8;
            } B;
        } CTRLDESCL7;                  /* LAYER[X].CTRLDESCL7 */

    };                                 /* end of DCU_LAYER_tag */

    struct DCU_tag {

        /* DCU.LAYER<[x]>.CTRLDESCL<y>.R  {x = 0-15, y = 1-7} */
        /* eg DCU.LAYER[0].CTRLDESCL1.R        = 0;           */
        /*      DCU.LAYER[0].CTRLDESCL1.B.HEIGHT = 0;           */
        struct DCU_LAYER_tag LAYER[16];

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t HEIGHT:8;
                  vuint32_t:8;
                vuint32_t WIDTH:8;
            } B;
        } CTRLDESCCURSOR1;             /* Control Descriptor Cursor_1 Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:6;
                vuint32_t POSY:10;
                  vuint32_t:6;
                vuint32_t POSX:10;
            } B;
        } CTRLDESCCURSOR2;             /* Control Descriptor Cursor_2 Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CUR_EN:1;
                  vuint32_t:7;
                vuint32_t CURSOR_DEFAULT_COLOR:24;
            } B;
        } CTRLDESCCURSOR3;             /* Control Descriptor Cursor_3 Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t HWC_BLINK_OFF:8;
                  vuint32_t:7;
                vuint32_t EN_BLINK:1;
                vuint32_t HWC_BLINK_ON:8;
            } B;
        } CTRLDESCCURSOR4;             /* Control Descriptor Cursor_4 Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t DCU_SW_RESET:1;
                  vuint32_t:8;
                vuint32_t BLEND_ITER:3;
                vuint32_t PDI_SYNC_LOCK:4;
                vuint32_t PDI_INTERPOL_EN:1;        
                vuint32_t RASTER_EN:1;        
                vuint32_t PDI_EN:1;
                vuint32_t PDI_BYTE_REV:1;
                vuint32_t PDI_DE_MODE:1;
                vuint32_t PDI_NARROW_MODE:1;
                vuint32_t PDI_MODE:2;
                vuint32_t PDI_SLAVE_MODE:1;
                vuint32_t TAG_EN:1;
                vuint32_t SIG_EN:1;
                vuint32_t PDI_SYNC:1;
                vuint32_t TEST:1;
                vuint32_t EN_GAMMA:1;
                vuint32_t DCU_MODE:2;
            } B;
        } DCU_MODE;                    /* DCU Mode Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t BGND_R:8;
                vuint32_t BGND_G:8;
                vuint32_t BGND_B:8;
            } B;
        } BGND;                        /* BGND Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:6;
                vuint32_t DELTA_Y:10;
                  vuint32_t:8;
                vuint32_t DELTA_X:8;
            } B;
        } DISP_SIZE;                   /* DISP_SIZE Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:1;
                vuint32_t BP_H:9;
                  vuint32_t:2;
                vuint32_t PW_H:9;
                  vuint32_t:2;
                vuint32_t FP_H:9;
            } B;
        } HSYN_PARA;                   /* HSYNPARA Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:1;
                vuint32_t BP_V:9;
                  vuint32_t:2;
                vuint32_t PW_V:9;
                  vuint32_t:2;
                vuint32_t FP_V:9;
            } B;
        } VSYN_PARA;                   /* VSYNPARA Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:21;
                vuint32_t INV_PDI_DE:1;
                vuint32_t INV_PDI_HS:1;
                vuint32_t INV_PDI_VS:1;
                vuint32_t INV_PDI_CLK:1;
                vuint32_t INV_PXCK:1;
                vuint32_t NEG:1;
                vuint32_t BP_VS:1;
                vuint32_t BP_HS:1;
                vuint32_t INV_CS:1;
                vuint32_t INV_VS:1;
                vuint32_t INV_HS:1;
            } B;
        } SYN_POL;                     /* SYNPOL Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:6;
                vuint32_t LS_BF_VS:10;
                vuint32_t OUT_BUF_HIGH:8;
                vuint32_t OUT_BUF_LOW:8;
            } B;
        } THRESHOLD;                   /* Threshold Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:12;
                vuint32_t P4_FIFO_HI_FLAG:1;
                vuint32_t P4_FIFO_LO_FLAG:1;
                vuint32_t P3_FIFO_HI_FLAG:1;
                vuint32_t P3_FIFO_LO_FLAG:1;
                  vuint32_t:1;
                vuint32_t DMA_TRANS_FINISH:1;
                  vuint32_t:2;
                vuint32_t IPM_ERROR:1;
                vuint32_t PROG_END:1;
                vuint32_t P2_FIFO_HI_FLAG:1;
                vuint32_t P2_FIFO_LO_FLAG:1;
                vuint32_t P1_FIFO_HI_FLAG:1;
                vuint32_t P1_FIFO_LO_FLAG:1;
                vuint32_t CRC_OVERFLOW:1;
                vuint32_t CRC_READY:1;
                vuint32_t VS_BLANK:1;
                vuint32_t LS_BF_VS:1;
                vuint32_t UNDRUN:1;
                vuint32_t VSYNC:1;
            } B;
        } INT_STATUS;                    /* Interrupt Status Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:12;
                vuint32_t M_P4_FIFO_HI_FLAG:1;
                vuint32_t M_P4_FIFO_LO_FLAG:1;
                vuint32_t M_P3_FIFO_HI_FLAG:1;
                vuint32_t M_P3_FIFO_LO_FLAG:1;
                  vuint32_t:1;
                vuint32_t M_DMA_TRANS_FINISH:1;
                  vuint32_t:2;
                vuint32_t M_IPM_ERROR:1;
                vuint32_t M_PROG_END:1;
                vuint32_t M_P2_FIFO_HI_FLAG:1;
                vuint32_t M_P2_FIFO_LO_FLAG:1;
                vuint32_t M_P1_FIFO_HI_FLAG:1;
                vuint32_t M_P1_FIFO_LO_FLAG:1;
                vuint32_t M_CRC_OVERFLOW:1;
                vuint32_t M_CRC_READY:1;
                vuint32_t M_VS_BLANK:1;
                vuint32_t M_LS_BF_VS:1;
                vuint32_t M_UNDRUN:1;
                vuint32_t M_VSYNC:1;
            } B;
        } INT_MASK;                    /* Interrupt Mask Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t COLBAR_R:8;
                vuint32_t COLBAR_G:8;
                vuint32_t COLBAR_B:8;
            } B;
        } COLBAR[8];                   /* COLBAR 1-8 Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:26;
                vuint32_t DIV_RATIO:6;
            } B;
        } DIV_RATIO;                   /* Clock Divider Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:6;
                vuint32_t SIG_VER_SIZE:10;
                  vuint32_t:6;
                vuint32_t SIG_HOR_SIZE:10;
            } B;
        } SIGN_CALC_1;                 /* CRC size configuration */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:6;
                vuint32_t SIG_VER_POS:10;
                  vuint32_t:6;
                vuint32_t SIG_HOR_POS:10;
            } B;
        } SIGN_CALC_2;                 /* CRC position configuration */

        union {
            vuint32_t R;
            struct {
                vuint32_t CRC_VAL:32;
            } B;
        } CRC_VAL;                     /* CRC Result register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:22;
                vuint32_t PDI_BLANKING_ERR:1;
                vuint32_t PDI_ECC_ERR2:1;
                vuint32_t PDI_ECC_ERR1:1;
                vuint32_t PDI_LOCK_LOST:1;
                vuint32_t PDI_LOCK_DET:1;
                vuint32_t PDI_VSYNC_DET:1;
                vuint32_t PDI_HSYNC_DET:1;
                vuint32_t PDI_DE_DET:1;
                vuint32_t PDI_CLK_LOST:1;
                vuint32_t PDI_CLK_DET:1;
            } B;
        } PDI_STATUS;                  /* PDI status Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:22;
                vuint32_t M_PDI_BLANKING_ERR:1;
                vuint32_t M_PDI_ECC_ERR2:1;
                vuint32_t M_PDI_ECC_ERR1:1;
                vuint32_t M_PDI_LOCK_LOST:1;
                vuint32_t M_PDI_LOCK_DET:1;
                vuint32_t M_PDI_VSYNC_DET:1;
                vuint32_t M_PDI_HSYNC_DET:1;
                vuint32_t M_PDI_DE_DET:1;
                vuint32_t M_PDI_CLK_LOST:1;
                vuint32_t M_PDI_CLK_DET:1;
            } B;
        } MASK_PDI_STATUS;             /* PDI Status Mask Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:13;
                vuint32_t HWC_ERR:1;
                vuint32_t SIG_ERR:1;
                vuint32_t DISP_ERR:1;
                vuint32_t L_PAR_ERR:16;
            } B;
        } PARR_ERR_STATUS;             /* Parameter error status Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:13;
                vuint32_t M_HWC_ERR:1;
                vuint32_t M_SIG_ERR:1;
                vuint32_t M_DISP_ERR:1;
                vuint32_t M_L_PAR_ERR:16;
            } B;
        } MASK_PARR_ERR_STATUS;        /* Parameter error mask Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t INP_BUF_P2_HI:8;
                vuint32_t INP_BUF_P2_LO:8;
                vuint32_t INP_BUF_P1_HI:8;
                vuint32_t INP_BUF_P1_LO:8;
            } B;
        } THRESHOLD_INP1;              /* Threshold Input Buffer Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t INP_BUF_P4_HI:8;
                vuint32_t INP_BUF_P4_LO:8;
                vuint32_t INP_BUF_P3_HI:8;
                vuint32_t INP_BUF_P3_LO:8;
            } B;
        } THRESHOLD_INP2;              /* Threshold Input Buffer Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t Y_RED:10;
                  vuint32_t:1;
                vuint32_t Y_GREEN:10;
                  vuint32_t:1;
                vuint32_t Y_BLUE:10;
            } B;
        } LUMA_COMP;                   /* Luminance component Register */    

        union {
            vuint32_t R;
            struct {
                  vuint32_t:5;
                vuint32_t CR_RED:11;
                  vuint32_t:4;
                vuint32_t CB_RED:12;
            } B;
        } CHROMA_RED;                  /* Red component Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:5;
                vuint32_t CR_GREEN:11;
                  vuint32_t:4;
                vuint32_t CB_GREEN:12;
            } B;
        } CHROMA_GREEN;                /* Green component Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:5;
                vuint32_t CR_BLUE:11;
                  vuint32_t:4;
                vuint32_t CB_BLUE:12;
            } B;
        } CHROMA_BLUE;                 /* Blue component Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CRC_POS:32;
            } B;
        } CRC_POS;                     /* CRC Position Register */
    
        struct DCU_FG_tag FG[16];      /* FGCOLOR 1-16 */

        vuint32_t DCU_reserved1[0x0C]; /* 12 registers */

        union {
            vuint32_t R;
            struct {
                vuint32_t HLB:1;
                  vuint32_t:31;
            } B;
        } GPR;                         /* Global Protection Register */
    
        union {
            vuint32_t R;
            struct {
                vuint32_t WEN_L0_1:1;
                vuint32_t WEN_L0_2:1;
                vuint32_t WEN_L0_3:1;
                vuint32_t WEN_L0_4:1;
                vuint32_t SLB_L0_1:1;
                vuint32_t SLB_L0_2:1;
                vuint32_t SLB_L0_3:1;
                vuint32_t SLB_L0_4:1;
                vuint32_t WEN_L0_5:1;
                vuint32_t WEN_L0_6:1;
                vuint32_t WEN_L0_7:1;
                  vuint32_t:1;
                vuint32_t SLB_L0_5:1;
                vuint32_t SLB_L0_6:1;
                vuint32_t SLB_L0_7:1;
                  vuint32_t:17;
            } B;
        } SLR_L0;                      /* Soft Lock Register Layer 0 */
    
        union {
            vuint32_t R;
            struct {
                vuint32_t WEN_L1_1:1;
                vuint32_t WEN_L1_2:1;
                vuint32_t WEN_L1_3:1;
                vuint32_t WEN_L1_4:1;
                vuint32_t SLB_L1_1:1;
                vuint32_t SLB_L1_2:1;
                vuint32_t SLB_L1_3:1;
                vuint32_t SLB_L1_4:1;
                vuint32_t WEN_L1_5:1;
                vuint32_t WEN_L1_6:1;
                vuint32_t WEN_L1_7:1;
                  vuint32_t:1;
                vuint32_t SLB_L1_5:1;
                vuint32_t SLB_L1_6:1;
                vuint32_t SLB_L1_7:1;
                  vuint32_t:17;
            } B;
        } SLR_L1;                      /* Soft Lock Register Layer 1 */
    
        union {
            vuint32_t R;
            struct {
                vuint32_t WEN_DISP:1;
                  vuint32_t:3;
                vuint32_t SLB_DISP:1;
                  vuint32_t:27;
            } B;
        } SLR_DISP_SIZE;               /* Soft Lock Register DISP_SIZE */
    
        union {
            vuint32_t R;
            struct {
                vuint32_t WEN_HSYNC:1;
                vuint32_t WEN_VSYNC:1;
                  vuint32_t:2;
                vuint32_t SLB_HSYNC:1;
                vuint32_t SLB_VSYNC:1;
                  vuint32_t:26;
            } B;
        } SLR_HVSYNC_PARA;             /* Soft Lock Register HSYNC VSYNC PARA */
    
        union {
            vuint32_t R;
            struct {
                vuint32_t WEN_POL:1;
                  vuint32_t:3;
                vuint32_t SLB_POL:1;
                  vuint32_t:27;
            } B;
        } SLR_POL;                     /* Soft Lock Register POL */
    
        union {
            vuint32_t R;
            struct {
                vuint32_t WEN_L0_FCOLOR:1;
                vuint32_t WEN_L0_BCOLOR:1;
                  vuint32_t:2;
                vuint32_t SLB_L0_FCOLOR:1;
                vuint32_t SLB_L0_BCOLOR:1;
                  vuint32_t:26;
            } B;
        } SLR_L0TRANSP;                /* Soft Lock Register Layer 0 TRANSP */
    
        union {
            vuint32_t R;
            struct {
                vuint32_t WEN_L1_FCOLOR:1;
                vuint32_t WEN_L1_BCOLOR:1;
                  vuint32_t:2;
                vuint32_t SLB_L1_FCOLOR:1;
                vuint32_t SLB_L1_BCOLOR:1;
                  vuint32_t:26;
            } B;
        } SLR_L1TRANSP;                /* Soft Lock Register Layer 1 TRANSP */
    
        vuint32_t DCU_reserved2[0x038]; /* Unused from end of registers */

        vuint32_t CURSOR[0x0100];      /* Hardware Cursor = 256 * 32bit */

        vuint32_t GAMMARED[0x100];     /* Gamma red table = 256 * 32bit */

        vuint32_t GAMMAGREEN[0x100];   /* Gamma green table = 256 * 32bit */

        vuint32_t GAMMABLUE[0x100];    /* Gamma blue table = 256 * 32bit */

        vuint32_t DCU_reserved3[0x300]; /* Unused = 768 * 32bit */

        vuint32_t CLUT[0x0800];        /* CLUT-Pallete memory + Tile memory = 2k * 32bit */

    };                                 /* end of DCU_tag */
/****************************************************************************/
/*                             MODULE : SGL                                 */
/****************************************************************************/
    struct SGL_tag{
        union{
            vuint32_t R;
            struct{
                vuint32_t M_P:1;
                vuint32_t SOUND_CTRL:3;
                vuint32_t SDCIE:1;
                  vuint32_t:7;
                vuint32_t CH2_SEL:4;
                vuint32_t PRE:7;
                  vuint32_t:5;
                vuint32_t CH1_SEL:4;
            } B;
        }MODE_SEL;                     /* Mode selection register */

        union{
            vuint32_t R;
        }SOUND_DURATION;               /* Sound duration register */

 
        union{
            vuint32_t R;
        }HIGH_PERIOD;                  /* High period register */

        union{
            vuint32_t R;
        }LOW_PERIOD;                   /* Low period register */

        union{
            vuint32_t R;
            struct{
                  vuint32_t :31;
                vuint32_t SDCIF:1;
            } B;
        }SGL_STATUS;                   /* Status register */
    };
/****************************************************************************/
/*                             MODULE : SMC - Stepper Motor Control         */
/****************************************************************************/
    struct SMC_tag {

        union {
            vuint8_t R;
            struct {
                  vuint8_t:1;
                vuint8_t MCPRE:2;
                vuint8_t MCHME:1;
                  vuint8_t:1;
                vuint8_t DITH:1;
                  vuint8_t:1;
                vuint8_t MCTOIF:1;
            } B;
        } CTL0;                        /* Motor Controller Control Register 0 */

        union {
            vuint8_t R;
            struct {
                vuint8_t RECIRC:1;
                  vuint8_t:6;
                vuint8_t MCTOIE:1;
            } B;
        } CTL1;                        /* Motor Controller Control Register 1 */

        union {
            vuint16_t R;
            struct {
                  vuint16_t:5;
                vuint16_t PER:11;
            } B;
        } PER;                         /* Motor Controller Period Register */

        int32_t SMC_reserved0[3];      /* (0x010 - 0x004)/4 = 0x01 */

        union {
            vuint8_t R;
            struct {
                vuint8_t MCOM:2;
                vuint8_t MCAM:2;
                  vuint8_t:2;
                vuint8_t CD:2;
            } B;
        } CC[12];                      /* Motor Controller Channel Control Register 0->11 */

        int32_t SMC_reserved1;         /* (0x020 - 0x01C)/4 = 0x01 */

        union {
            vuint16_t R;
            struct {
                vuint16_t SIGN:1;
                vuint16_t SIGN30:4;
                vuint16_t DUTY:11;
            } B;
        } DC[12];                      /* Motor Controller Duty Cycle Register 0->11 */

        int32_t SMC_reserved2[2];      /* (0x040 - 0x038)/2 = 0x02 */

        union {
            vuint8_t R;
            struct {
                vuint8_t TOUT:8;
            } B;
        } SDTO;                        /* Shortcut detector time-out register  */

        int8_t SMC_reserved3[3];       /* (0x044 - 0x041) = 0x03 */

        union {
            vuint8_t R;
            struct {
                vuint8_t SDEN:8;
            } B;
        } SDE[3];                      /* Shortcut detector enable register 0->2 */

        int8_t SMC_reserved4;          /* (0x048 - 0x047) = 0x01 */

        union {
            vuint8_t R;
            struct {
                vuint8_t SDIE:8;
            } B;
        } SDIEN[3];                    /* Shortcut detector interrupt enable register 0->2 */

        int8_t SMC_reserved5;          /* (0x04C - 0x04B) = 0x01 */

        union {
            vuint8_t R;
            struct {
                vuint8_t SDIF:8;
            } B;
        } SDI[3];                      /* Shortcut detector interrupt register 0->2 */

    };                                 /* end of SMC_tag */
/****************************************************************************/
/*                             MODULE : SSD - Stepper Stall Detect          */
/****************************************************************************/
    struct SSD_tag {

        union {
            vuint16_t R;
            struct {
                vuint16_t TRIG:1;
                vuint16_t STEP:2;
                vuint16_t RCIR:1;
                vuint16_t ITGDIR:1;
                vuint16_t BLNDCL:1;
                vuint16_t ITGDCL:1;
                vuint16_t RTZE:1;
                  vuint16_t:1;
                vuint16_t BLNST:1;
                vuint16_t ITGST:1;
                  vuint16_t:3;
                vuint16_t SDCPU:1;
                vuint16_t DZDIS:1;
            } B;
        } CONTROL;                     /* Control & Status Register */

        union {
            vuint16_t R;
            struct {
                vuint16_t BLNIF:1;
                vuint16_t ITGIF:1;
                  vuint16_t:5;
                vuint16_t ACOVIF:1;
                vuint16_t BLNIE:1;
                vuint16_t ITGIE:1;
                  vuint16_t:5;
                vuint16_t ACOVIE:1;
            } B;
        } IRQ;                         /* Interrupt Flag and Enable Register */

        union {
            vuint16_t R;
            struct {
                vuint16_t ITGACC:16;
            } B;
        } ITGACC;                      /* Integrator Accumulator register */

        union {
            vuint16_t R;
            struct {
                vuint16_t DCNT:16;
            } B;
        } DCNT;                        /* Down Counter Count register */

        union {
            vuint16_t R;
            struct {
                vuint16_t BLNCNTLD:16;
            } B;
        } BLNCNTLD;                    /* Blanking Counter Load register */

        union {
            vuint16_t R;
            struct {
                vuint16_t ITGCNTLD:16;
            } B;
        } ITGCNTLD;                    /* Integration Counter Load register */

        union {
            vuint16_t R;
            struct {
                  vuint16_t:1;
                vuint16_t BLNDIV:3;
                  vuint16_t:1;
                vuint16_t ITGDIV:3;
                  vuint16_t:2;
                vuint16_t OFFCNC:2;
                  vuint16_t:1;
                vuint16_t ACDIV:3;
            } B;
        } PRESCALE;                    /* Prescaler register */

        union {
            vuint16_t R;
            struct {
                vuint16_t TMST:1;
                vuint16_t ANLOUT:1;
                vuint16_t ANLIN:1;
                vuint16_t SSDEN:1;
                vuint16_t STEP1:1;
                vuint16_t POL:1;
                vuint16_t ITG:1;
                vuint16_t DACHIZ:1;
                vuint16_t BUFHIZ:1;
                vuint16_t AMPHIZ:1;
                vuint16_t RESSHORT:1;
                vuint16_t ITSSDRV:1;
                vuint16_t ITSSDRVEN:1;
                vuint16_t REFDRV:1;
                vuint16_t REFDRVEN:1;
            } B;
        } FNTEST;                      /* Functional Test Mode register */

    };                                 /* end of SSD_tag */
/****************************************************************************/
/*                             MODULE : EMIOS                               */
/****************************************************************************/
    struct EMIOS_CHANNEL_tag {
        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t CADR:16;
            } B;
        } CADR;                        /* Channel A Data Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t CBDR:16;
            } B;
        } CBDR;                        /* Channel B Data Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t CCNTR:16;
            } B;
        } CCNTR;                       /* Channel Counter Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t FREN:1;
                vuint32_t ODIS:1;
                vuint32_t ODISSL:2;
                vuint32_t UCPRE:2;
                vuint32_t UCPEN:1;
                vuint32_t DMA:1;
                  vuint32_t:1;
                vuint32_t IF:4;
                vuint32_t FCK:1;
                vuint32_t FEN:1;
                  vuint32_t:3;
                vuint32_t FORCMA:1;
                vuint32_t FORCMB:1;
                  vuint32_t:1;
                vuint32_t BSL:2;
                vuint32_t EDSEL:1;
                vuint32_t EDPOL:1;
                vuint32_t MODE:7;
            } B;
        } CCR;                         /* Channel Control Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t OVR:1;
                  vuint32_t:15;
                vuint32_t OVFL:1;
                  vuint32_t:12;
                vuint32_t UCIN:1;
                vuint32_t UCOUT:1;
                vuint32_t FLAG:1;
            } B;
        } CSR;                         /* Channel Status Register */

        union {
            vuint32_t R;
        } ALTCADR;                     /* Alternate Channel A Data Register */

        uint32_t emios_channel_reserved[2];

    };                                 /* end of EMIOS_CHANNEL_tag */

    struct EMIOS_tag {
        union {
            vuint32_t R;
            struct {
                  vuint32_t:1;
                vuint32_t MDIS:1;
                vuint32_t FRZ:1;
                vuint32_t GTBE:1;
                vuint32_t ETB:1;
                vuint32_t GPREN:1;
                  vuint32_t:6;
                vuint32_t SRV:4;
                vuint32_t GPRE:8;
                  vuint32_t:8;
            } B;
        } MCR;                         /* Module Configuration Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t F23:1;
                vuint32_t F22:1;
                vuint32_t F21:1;
                vuint32_t F20:1;
                vuint32_t F19:1;
                vuint32_t F18:1;
                vuint32_t F17:1;
                vuint32_t F16:1;
                vuint32_t F15:1;
                vuint32_t F14:1;
                vuint32_t F13:1;
                vuint32_t F12:1;
                vuint32_t F11:1;
                vuint32_t F10:1;
                vuint32_t F9:1;
                vuint32_t F8:1;
                vuint32_t F7:1;
                vuint32_t F6:1;
                vuint32_t F5:1;
                vuint32_t F4:1;
                vuint32_t F3:1;
                vuint32_t F2:1;
                vuint32_t F1:1;
                vuint32_t F0:1;
            } B;
        } GFR;                         /* Global FLAG Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t OU23:1;
                vuint32_t OU22:1;
                vuint32_t OU21:1;
                vuint32_t OU20:1;
                vuint32_t OU19:1;
                vuint32_t OU18:1;
                vuint32_t OU17:1;
                vuint32_t OU16:1;
                vuint32_t OU15:1;
                vuint32_t OU14:1;
                vuint32_t OU13:1;
                vuint32_t OU12:1;
                vuint32_t OU11:1;
                vuint32_t OU10:1;
                vuint32_t OU9:1;
                vuint32_t OU8:1;
                vuint32_t OU7:1;
                vuint32_t OU6:1;
                vuint32_t OU5:1;
                vuint32_t OU4:1;
                vuint32_t OU3:1;
                vuint32_t OU2:1;
                vuint32_t OU1:1;
                vuint32_t OU0:1;
            } B;
        } OUDR;                        /* Output Update Disable Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t CHDIS23:1;
                vuint32_t CHDIS22:1;
                vuint32_t CHDIS21:1;
                vuint32_t CHDIS20:1;
                vuint32_t CHDIS19:1;
                vuint32_t CHDIS18:1;
                vuint32_t CHDIS17:1;
                vuint32_t CHDIS16:1;
                vuint32_t CHDIS15:1;
                vuint32_t CHDIS14:1;
                vuint32_t CHDIS13:1;
                vuint32_t CHDIS12:1;
                vuint32_t CHDIS11:1;
                vuint32_t CHDIS10:1;
                vuint32_t CHDIS9:1;
                vuint32_t CHDIS8:1;
                vuint32_t CHDIS7:1;
                vuint32_t CHDIS6:1;
                vuint32_t CHDIS5:1;
                vuint32_t CHDIS4:1;
                vuint32_t CHDIS3:1;
                vuint32_t CHDIS2:1;
                vuint32_t CHDIS1:1;
                vuint32_t CHDIS0:1;
            } B;
        } UCDIS;                       /* Disable Channel Register */

        uint32_t emios_reserved1[4];

        struct EMIOS_CHANNEL_tag CH[28];

    };                                 /* end of EMIOS_tag */
/****************************************************************************/
/*                             MODULE : PIT                                 */
/****************************************************************************/
    struct PIT_tag {
        union {
            vuint32_t R;
            struct {
                  vuint32_t:30;
                vuint32_t MDIS:1;
                vuint32_t FRZ:1;
            } B;
        } MCR;                         /* Module Control Register */

        uint32_t pit_reserved1[63];    /* (0x0100 - 0x0004)/4 = 0x3F */

        struct {
            union {
                vuint32_t R;
                struct {
                    vuint32_t TSV:32;
                 } B;
             } LDVAL;                  /* Timer Load Value Register */

             union {
                vuint32_t R;
                 struct {
                     vuint32_t TVL:32;
                 } B;
             } CVAL;                   /* Current Timer Value Register */

             union {
                vuint32_t R;
                 struct {
                       vuint32_t:30;
                     vuint32_t TIE:1;
                     vuint32_t TEN:1;
                 } B;
             } TCTRL;                  /* Timer Control Register */

             union {
                vuint32_t R;
                 struct {
                       vuint32_t:31;
                     vuint32_t TIF:1;
                 } B;
             } TFLG;                   /* Timer Flasg Register */
        } CH[4];

    };                                 /* end of PIT_tag */
/****************************************************************************/
/*                             MODULE : I2C                                 */
/****************************************************************************/
    struct I2C_tag {
        union {
            vuint8_t R;
            struct {
                vuint8_t ADR:7;
                  vuint8_t:1;
            } B;
        } IBAD;                        /* Module Bus Address Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t IBC:8;
            } B;
        } IBFD;                        /* Module Bus Frequency Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t MDIS:1;
                vuint8_t IBIE:1;
                vuint8_t MS:1;
                vuint8_t TX:1;
                vuint8_t NOACK:1;
                vuint8_t RSTA:1;
                vuint8_t DMAEN:1;
                vuint8_t IBDOZE:1;
            } B;
        } IBCR;                        /* Module Bus Control Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t TCF:1;
                vuint8_t IAAS:1;
                vuint8_t IBB:1;
                vuint8_t IBAL:1;
                  vuint8_t:1;
                vuint8_t SRW:1;
                vuint8_t IBIF:1;
                vuint8_t RXAK:1;
            } B;
        } IBSR;                        /* Module Status Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t DATA:8;
            } B;
        } IBDR;                        /* Module Data Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t BIIE:1;
                  vuint8_t:7;
            } B;
        } IBIC;                        /* Module Interrupt Configuration Register */

    };                                 /* end of I2C_tag */
/****************************************************************************/
/*                             MODULE : MPU                                 */
/****************************************************************************/
    struct MPU_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t SPERR:8;
                  vuint32_t:4;
                vuint32_t HRL:4;
                vuint32_t NSP:4;
                vuint32_t NGRD:4;
                  vuint32_t:7;
                vuint32_t VLD:1;
            } B;
        } CESR;                        /* Module Control/Error Status Register */

        uint32_t mpu_reserved1[3];     /* (0x010 - 0x004)/4 = 0x03 */

        union {
            vuint32_t R;
            struct {
                vuint32_t EADDR:32;
            } B;
        } EAR0;                        /* Error Address Register port 0*/

        union {
            vuint32_t R;
            struct {
                vuint32_t EACD:16;
                vuint32_t EPID:8;
                vuint32_t EMN:4;
                vuint32_t EATTR:3;
                vuint32_t ERW:1;
            } B;
        } EDR0;                        /* Error Detail Register port 0 */

        union {
            vuint32_t R;
            struct {
                vuint32_t EADDR:32;
            } B;
        } EAR1;                        /* Error Address Register port 1 */

        union {
            vuint32_t R;
            struct {
                vuint32_t EACD:16;
                vuint32_t EPID:8;
                vuint32_t EMN:4;
                vuint32_t EATTR:3;
                vuint32_t ERW:1;
            } B;
        } EDR1;                        /* Error Detail Register port 1 */

        union {
            vuint32_t R;
            struct {
                vuint32_t EADDR:32;
            } B;
        } EAR2;                        /* Error Address Register port 2 */

        union {
            vuint32_t R;
            struct {
                vuint32_t EACD:16;
                vuint32_t EPID:8;
                vuint32_t EMN:4;
                vuint32_t EATTR:3;
                vuint32_t ERW:1;
            } B;
        } EDR2;                        /* Error Detail Register port 2 */

        union {
            vuint32_t R;
            struct {
                vuint32_t EADDR:32;
            } B;
        } EAR3;                        /* Error Address Register port 3 */

        union {
            vuint32_t R;
            struct {
                vuint32_t EACD:16;
                vuint32_t EPID:8;
                vuint32_t EMN:4;
                vuint32_t EATTR:3;
                vuint32_t ERW:1;
            } B;
        } EDR3;                        /* Error Detail Register port 3 */

        uint32_t mpu_reserved2[244];   /* (0x0400 - 0x0030)/4 = 0x0F4 */

        struct {
            union {
                vuint32_t R;
                struct {
                     vuint32_t SRTADDR:27;
                      vuint32_t:5;
                 } B;
             } WORD0;                  /* Region Descriptor n Word 0 */

             union {
                vuint32_t R;
                 struct {
                     vuint32_t ENDADDR:27;
                      vuint32_t:5;
                 } B;
             } WORD1;                   /* Region Descriptor n Word 1 */

             union {
                vuint32_t R;
                 struct {
                     vuint32_t M7RE:1;
                     vuint32_t M7WE:1;
                     vuint32_t M6RE:1;
                     vuint32_t M6WE:1;
                     vuint32_t M5RE:1;
                     vuint32_t M5WE:1;
                     vuint32_t M4RE:1;
                     vuint32_t M4WE:1;
                     vuint32_t M3PE:1;
                     vuint32_t M3SM:2;
                     vuint32_t M3UM:3;
                     vuint32_t M2PE:1;
                     vuint32_t M2SM:2;
                     vuint32_t M2UM:3;
                     vuint32_t M1PE:1;
                     vuint32_t M1SM:2;
                     vuint32_t M1UM:3;
                     vuint32_t M0PE:1;
                     vuint32_t M0SM:2;
                     vuint32_t M0UM:3;
                 } B;
             } WORD2;                  /* Region Descriptor n Word 2 */

             union {
                vuint32_t R;
                 struct {
                     vuint32_t PID:8;
                     vuint32_t PIDMASK:8;
                      vuint32_t:15;
                     vuint32_t VLD:1;
                 } B;
             } WORD3;                  /* Region Descriptor n Word 3 */

        } RGD[16];

        uint32_t mpu_reserved3[192];   /* (0x0800 - 0x0500)/4 = 0x0C0 */

        union {
            vuint32_t R;
            struct {
                vuint32_t M7RE:1;
                vuint32_t M7WE:1;
                vuint32_t M6RE:1;
                vuint32_t M6WE:1;
                vuint32_t M5RE:1;
                vuint32_t M5WE:1;
                vuint32_t M4RE:1;
                vuint32_t M4WE:1;
                vuint32_t M3PE:1;
                vuint32_t M3SM:2;
                vuint32_t M3UM:3;
                vuint32_t M2PE:1;
                vuint32_t M2SM:2;
                vuint32_t M2UM:3;
                vuint32_t M1PE:1;
                vuint32_t M1SM:2;
                vuint32_t M1UM:3;
                vuint32_t M0PE:1;
                vuint32_t M0SM:2;
                vuint32_t M0UM:3;
            } B;
        } RGDAAC[16];                  /* Region Descriptor Alternate Access Control n */

    };                                 /* end of MPU_tag */
/****************************************************************************/
/*                             MODULE : eDMA                                */
/****************************************************************************/

#include "ip_edma.h"

/****************************************************************************/
/*                             MODULE : INTC                                */
/****************************************************************************/
    struct INTC_tag {
        union {
            vuint32_t R;
            struct {
                  vuint32_t:26;
                vuint32_t VTES:1;
                  vuint32_t:4;
                vuint32_t HVEN:1;
            } B;
        } MCR;                         /* Module Configuration Register */

        int32_t INTC_reserved1;        /* (0x008 - 0x004)/4 = 0x01 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:28;
                vuint32_t PRI:4;
            } B;
        } CPR;                         /* Current Priority Register */

        int32_t INTC_reserved2;        /* (0x010 - 0x00C)/4 = 0x01 */

        union {
            vuint32_t R;
            struct {
                vuint32_t VTBA:21;
                vuint32_t INTVEC:9;
                  vuint32_t:2;
            } B;
        } IACKR;                       /* Interrupt Acknowledge Register */

        int32_t INTC_reserved3;        /* (0x018 - 0x014)/4 = 0x01 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:32;
            } B;
        } EOIR;                        /* End of Interrupt Register */

        int32_t INTC_reserved4;        /* (0x020 - 0x01C)/4 = 0x01 */

        union {
            vuint8_t R;
            struct {
                  vuint8_t:6;
                vuint8_t SET:1;
                vuint8_t CLR:1;
            } B;
        } SSCIR[8];                    /* Software Set/Clear Interruput Register */

        uint32_t intc_reserved5[6];    /* (0x040 - 0x028)/4 = 0x06 */

        union {
            vuint8_t R;
            struct {
                  vuint8_t:4;
                vuint8_t PRI:4;
            } B;
        } PSR[207];                    /* Software Set/Clear Interrupt Register */

    };                                 /* end of INTC_tag */
/****************************************************************************/
/*                             MODULE : DSPI                                */
/****************************************************************************/
#include "ip_dspi.h"

/****************************************************************************/
/*                             MODULE : QUADSPI                             */
/****************************************************************************/
    struct QUADSPI_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t MSTR:1;
                vuint32_t CONT_SCKE:1;
                  vuint32_t:2;
                vuint32_t FRZ:1;
                vuint32_t MTFE:1;
                vuint32_t PCSSE:1;
                vuint32_t ROOE:1;
                vuint32_t PCSIS7:1;
                vuint32_t PCSIS6:1;
                vuint32_t PCSIS5:1;
                vuint32_t PCSIS4:1;
                vuint32_t PCSIS3:1;
                vuint32_t PCSIS2:1;
                vuint32_t PCSIS1:1;
                vuint32_t PCSIS0:1;
                vuint32_t DOZE:1;
                vuint32_t MDIS:1;
                vuint32_t DIS_TXF:1;
                vuint32_t DIS_RXF:1;
                vuint32_t CLR_TXF:1;
                vuint32_t CLR_RXF:1;
                vuint32_t SMPL_PT:2;
                vuint32_t QMODE:1;
                vuint32_t VMID:4;
                  vuint32_t:2;
                vuint32_t HALT:1;
            } B;
        } MCR;                         /* Module Configuration Register */

        uint32_t quadspi_reserved1;        /* BASE + 0x004 */

        vuint32_t TCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t DBR:1;
                vuint32_t FMSZ:4;
                vuint32_t CPOL:1;
                vuint32_t CPHA:1;
                vuint32_t LSBFE:1;
                vuint32_t PCSSCK:2;
                vuint32_t PASC:2;
                vuint32_t PDT:2;
                vuint32_t PBR:2;
                vuint32_t CSSCK:4;
                vuint32_t ASC:4;
                vuint32_t DT:4;
                vuint32_t BR:4;
            } B;
        } CTAR[2];                     /* Clock and Transfer Attributes Registers */

        uint32_t quadspi_reserved2[6]; /* {0x002C-0x0014}/0x4 = 0x06 */

        union {
            vuint32_t R;
            struct {
                vuint32_t TCF:1;
                vuint32_t TXRXS:1;
                  vuint32_t:1;
                vuint32_t EOQF:1;
                vuint32_t TFUF:1;
                  vuint32_t:1;
                vuint32_t TFFF:1;
                  vuint32_t:5;
                vuint32_t RFOF:1;
                  vuint32_t:1;
                vuint32_t RFDF:1;
                  vuint32_t:1;
                vuint32_t TXCTR:4;
                vuint32_t TXNXTPTR:4;
                vuint32_t RXCTR:4;
                vuint32_t POPNXTPTR:4;
            } B;
        } SPISR;                       /* SPI Status Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t TCF_IE:1;
                  vuint32_t:2;
                vuint32_t EOQF_IE:1;
                vuint32_t TFUF_IE:1;
                  vuint32_t:1;
                vuint32_t TFFF_RE:1;
                vuint32_t TFFF_DIRS:1;
                  vuint32_t:4;
                vuint32_t RFOF_IE:1;
                  vuint32_t:1;
                vuint32_t RFDF_RE:1;
                vuint32_t RFDF_DIRS:1;
                  vuint32_t:16;
            } B;
        } SPIRSER;                     /* SPI Interrupt and DMA Request Select and Enable Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CONT:1;
                vuint32_t CTAS:3;
                vuint32_t EOQ:1;
                vuint32_t CTCNT:1;
                  vuint32_t:2;
                vuint32_t PCS7:1;
                vuint32_t PCS6:1;
                vuint32_t PCS5:1;
                vuint32_t PCS4:1;
                vuint32_t PCS3:1;
                vuint32_t PCS2:1;
                vuint32_t PCS1:1;
                vuint32_t PCS0:1;
                vuint32_t TXDATA:16;
            } B;
        } PUSHR;                       /* PUSH TX FIFO Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t RXDATA:16;
            } B;
        } POPR;                        /* POP RX FIFO Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t TXCMD:16;
                vuint32_t TXDATA:16;
            } B;
        } TXFR[15];                    /* Transmit FIFO Registers 0 - 14 */

        uint32_t quadspi_reserved3;    /* {0x0078-0x0078}/0x4 = 0x01 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:16;
                vuint32_t RXDATA:16;
            } B;
        } RXFR[15];                    /* RX FIFO Registers 0 - 14 */

        uint32_t quadspi_reserved4[18]; /* {0x0100-0x00B8}/0x4 = 0x12 */

        vuint32_t SFAR;                /* Serial Flash Address Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t ICO:24;
                vuint32_t IC:8;
            } B;
        } ICR;                         /* Instruction Code Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:25;
                vuint32_t FSDLY:1;
                vuint32_t FSPHS:1;
                  vuint32_t:2;
                vuint32_t HSDLY:1;
                vuint32_t HSPHS:1;
                vuint32_t HSENA:1;
            } B;
        } SMPR;                        /* Sampling Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t RDCTR:16;
                vuint32_t RDBFL:8;
                  vuint32_t:8;
            } B;
        } RBSR;                        /* RX Buffer Status Register */

        vuint32_t RBDR[15];            /* RX Buffer Data Registers 0 - 14 */

        uint32_t quadspi_reserved5;    /* {0x0150-0x014C}/0x4 = 0x01 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:7;
                vuint32_t TRCTR:9;
                vuint32_t TRBFL:8;
                  vuint32_t:8;
            } B;
        } TBSR;                        /* TX Buffer Status Register */

        vuint32_t TBDR;                /* TX Buffer Data Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:8;
                vuint32_t ARMB:8;
                  vuint32_t:1;
                vuint32_t ARSZ:5;
                  vuint32_t:2;
                vuint32_t ARIC:8;
            } B;
        } ACR;                         /* AMBA Control Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t TXFULL:1;
                  vuint32_t:2;
                vuint32_t TXNE:1;
                  vuint32_t:4;
                vuint32_t RXFULL:1;
                  vuint32_t:2;
                vuint32_t RXNE:1;
                  vuint32_t:4;
                vuint32_t AHBFULL:1;
                  vuint32_t:2;
                vuint32_t AHBNE:1;
                  vuint32_t:4;
                vuint32_t CONTMODE:1;
                vuint32_t AHB_ACC:1;
                vuint32_t IP_ACC:1;
                vuint32_t BUSY:1;
            } B;
        } SFMSR;                       /* Serial Flash Mode Status Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t TBFF:1;
                vuint32_t TBUF:1;
                  vuint32_t:8;
                vuint32_t RBOF:1;
                vuint32_t RBDF:1;
                  vuint32_t:6;
                vuint32_t ABOF:1;
                  vuint32_t:1;
                vuint32_t IPAEF:1;
                vuint32_t IPIEF:1;
                vuint32_t ICEF:1;
                  vuint32_t:4;
                vuint32_t TFF:1;
            } B;
        } SFMFR;                       /* Serial Flash Mode Flag Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t TBFIE:1;
                vuint32_t TBUIE:1;
                  vuint32_t:4;
                vuint32_t RBDDE:1;
                  vuint32_t:3;
                vuint32_t RBOIE:1;
                vuint32_t RBDIE:1;
                  vuint32_t:6;
                vuint32_t ABOIE:1;
                  vuint32_t:1;
                vuint32_t IPAEIE:1;
                vuint32_t IPIEIE:1;
                vuint32_t ICEIE:1;
                  vuint32_t:4;
                vuint32_t TFIE:1;
            } B;
        } SFMRSER;                     /* SFM Interrupt and DMA Request Select and Enable Register */

        uint32_t quadspi_reserved6[38]; /* {0x0200-0x0168}/0x4 = 0x26 */

    };                            /* end of QUADSPI_tag */
/****************************************************************************/
/*                             MODULE : FlexCAN                             */
/****************************************************************************/
#include "ip_flexcan.h"


/****************************************************************************/
/*                             MODULE : DMAMUX                              */
/****************************************************************************/
    struct DMAMUX_tag {
        union {
            vuint8_t R;
            struct {
                vuint8_t ENBL:1;
                vuint8_t TRIG:1;
                vuint8_t SOURCE:6;
            } B;
        } CHCONFIG[16];                /* DMA Channel Configuration Register */

    };                                 /* end of DMAMUX_tag */
/****************************************************************************/
/*                             MODULE : LCD                                 */
/****************************************************************************/
    struct LCD_tag {

        union {
            vuint32_t R;
            struct {
                vuint32_t LCDEN:1;
                vuint32_t LCDRST:1;
                vuint32_t LCDRCS:1;
                vuint32_t DUTY:3;
                vuint32_t BIAS:1;
                vuint32_t VLCDS:1;
                vuint32_t PWR:2;
                vuint32_t BSTEN:1;
                vuint32_t BSTSEL:1;
                vuint32_t BSTAO:1;
                vuint32_t LCDOCS:1;
                vuint32_t LCDINT:1;
                vuint32_t EOFF:1;
                vuint32_t NOF:8;
                  vuint32_t:2;
                vuint32_t LCDBPA:1;
                  vuint32_t:2;
                vuint32_t LCDBPS:3;
            } B;
        } CR;                          /* LCD Control Register */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:4;
                vuint32_t LCLK:4;
                  vuint32_t:24;
            } B;
        } PCR;                         /* LCD Prescaler Control Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CCEN:1;
                  vuint32_t:4;
                vuint32_t LCC:11;
                  vuint32_t:16;
            } B;
        } CCR;                         /* LCD Contrast Control Register */

        int32_t LCD_reserved1;         /* (0x10 - 0x0C)/4 = 0x01 */

        union {
            vuint32_t R;
            struct {
                vuint32_t FP31EN:1;
                vuint32_t FP30EN:1;
                vuint32_t FP29EN:1;
                vuint32_t FP28EN:1;
                vuint32_t FP27EN:1;
                vuint32_t FP26EN:1;
                vuint32_t FP25EN:1;
                vuint32_t FP24EN:1;
                vuint32_t FP23EN:1;
                vuint32_t FP22EN:1;
                vuint32_t FP21EN:1;
                vuint32_t FP20EN:1;
                vuint32_t FP19EN:1;
                vuint32_t FP18EN:1;
                vuint32_t FP17EN:1;
                vuint32_t FP16EN:1;
                vuint32_t FP15EN:1;
                vuint32_t FP14EN:1;
                vuint32_t FP13EN:1;
                vuint32_t FP12EN:1;
                vuint32_t FP11EN:1;
                vuint32_t FP10EN:1;
                vuint32_t FP9EN:1;
                vuint32_t FP8EN:1;
                vuint32_t FP7EN:1;
                vuint32_t FP6EN:1;
                vuint32_t FP5EN:1;
                vuint32_t FP4EN:1;
                vuint32_t FP3EN:1;
                vuint32_t FP2EN:1;
                vuint32_t FP1EN:1;
                vuint32_t FP0EN:1;
            } B;
        } FPENR0;                      /* LCD Frontplane Enable Register 0 */

        union {
            vuint32_t R;
            struct {
                vuint32_t FP63EN:1;
                vuint32_t FP62EN:1;
                vuint32_t FP61EN:1;
                vuint32_t FP60EN:1;
                vuint32_t FP59EN:1;
                vuint32_t FP58EN:1;
                vuint32_t FP57EN:1;
                vuint32_t FP56EN:1;
                vuint32_t FP55EN:1;
                vuint32_t FP54EN:1;
                vuint32_t FP53EN:1;
                vuint32_t FP52EN:1;
                vuint32_t FP51EN:1;
                vuint32_t FP50EN:1;
                vuint32_t FP49EN:1;
                vuint32_t FP48EN:1;
                vuint32_t FP47EN:1;
                vuint32_t FP46EN:1;
                vuint32_t FP45EN:1;
                vuint32_t FP44EN:1;
                vuint32_t FP43EN:1;
                vuint32_t FP42EN:1;
                vuint32_t FP41EN:1;
                vuint32_t FP40EN:1;
                vuint32_t FP39EN:1;
                vuint32_t FP38EN:1;
                vuint32_t FP37EN:1;
                vuint32_t FP36EN:1;
                vuint32_t FP35EN:1;
                vuint32_t FP34EN:1;
                vuint32_t FP33EN:1;
                vuint32_t FP32EN:1;
            } B;
        } FPENR1;                      /* LCD Frontplane Enable Register 1 */

        int32_t LCD_reserved2[2];      /* (0x20 - 0x18)/4 = 0x02 */

        union {
            vuint32_t R;
            struct {
                  vuint32_t:2;
                vuint32_t FP0BP5:1;
                vuint32_t FP0BP4:1;
                vuint32_t FP0BP3:1;
                vuint32_t FP0BP2:1;
                vuint32_t FP0BP1:1;
                vuint32_t FP0BP0:1;
                  vuint32_t:2;
                vuint32_t FP1BP5:1;
                vuint32_t FP1BP4:1;
                vuint32_t FP1BP3:1;
                vuint32_t FP1BP2:1;
                vuint32_t FP1BP1:1;
                vuint32_t FP1BP0:1;
                  vuint32_t:2;
                vuint32_t FP2BP5:1;
                vuint32_t FP2BP4:1;
                vuint32_t FP2BP3:1;
                vuint32_t FP2BP2:1;
                vuint32_t FP2BP1:1;
                vuint32_t FP2BP0:1;
                  vuint32_t:2;
                vuint32_t FP3BP5:1;
                vuint32_t FP3BP4:1;
                vuint32_t FP3BP3:1;
                vuint32_t FP3BP2:1;
                vuint32_t FP3BP1:1;
                vuint32_t FP3BP0:1;
            } B;
        } RAM[16];                     /* LCD RAM Register */
    };                                 /* end of LCD_tag */
/****************************************************************************/
/*                             MODULE : DFLASH                              */
/****************************************************************************/
    struct DFLASH_tag {
        union {                        /* Module Configuration Register */
            vuint32_t R;
            struct {
                vuint32_t EDC:1;
                  vuint32_t:4;
                vuint32_t SIZE:3;
                  vuint32_t:1;
                vuint32_t LAS:3;
                  vuint32_t:3;
                vuint32_t MAS:1;
                vuint32_t EER:1;
                vuint32_t RWE:1;
                  vuint32_t:1;
                  vuint32_t:1;
                vuint32_t PEAS:1;
                vuint32_t DONE:1;
                vuint32_t PEG:1;
                  vuint32_t:4;
                vuint32_t PGM:1;
                vuint32_t PSUS:1;
                vuint32_t ERS:1;
                vuint32_t ESUS:1;
                vuint32_t EHV:1;
            } B;
        } MCR;

        union {                        /* LML Register */
            vuint32_t R;
            struct {
                vuint32_t LME:1;
                  vuint32_t:10;
                vuint32_t TSLK:1;
                  vuint32_t:2;
                vuint32_t MLK:2;
                vuint32_t LLK:16;
            } B;
        } LML;

        union {                        /* HBL Register */
            vuint32_t R;
            struct {
                vuint32_t HBE:1;
                  vuint32_t:23;
                vuint32_t HLK:8;
            } B;
        } HBL;

        union {                        /* SLML Register */
            vuint32_t R;
            struct {
                vuint32_t SLE:1;
                  vuint32_t:10;
                vuint32_t STSLK:1;
                  vuint32_t:2;
                vuint32_t SMK:2;
                vuint32_t SLK:16;
            } B;
        } SLL;

        union {                        /* LMS Register */
            vuint32_t R;
            struct {
                  vuint32_t:14;
                vuint32_t MSL:2;
                vuint32_t LSL:16;
            } B;
        } LMS;

        union {                        /* High Address Space Block Select Register */
            vuint32_t R;
            struct {
                  vuint32_t:26;
                vuint32_t HSL:6;
            } B;
        } HBS;

        union {                        /* Address Register */
            vuint32_t R;
            struct {
                  vuint32_t:10;
                vuint32_t ADD:19;
                  vuint32_t:3;
            } B;
        } ADR;

        int32_t Dflash_reserved0[8];   /* {0x003C-0x001C}/0x4 = 0x08 */

        union {                        /* User Test Register 0 */
            vuint32_t R;
            struct {
                vuint32_t UTE:1;
                  vuint32_t:7;
                vuint32_t DSI:8;
                  vuint32_t:10;
                vuint32_t MRE:1;
                vuint32_t MRV:1;
                vuint32_t EIE:1;
                vuint32_t AIS:1;
                vuint32_t AIE:1;
                vuint32_t AID:1;
            } B;
        } UT0;

        union {                        /* User Test Register 1 */
            vuint32_t R;
            struct {
                vuint32_t DAI:32;
            } B;
        } UT1;

        union {                        /* User Test Register 2 */
            vuint32_t R;
            struct {
                vuint32_t DAI:32;
            } B;
        } UT2;

        union {                        /* User Multiple Input Signature Register 0-4 */
            vuint32_t R;
            struct {
                vuint32_t MS:32;
            } B;
        } UMISR[5];

    };                                 /* end of Dflash_tag */
/****************************************************************************/
/*                             MODULE : CFLASH                              */
/****************************************************************************/
    struct CFLASH_tag {
        union {                        /* Module Configuration Register */
            vuint32_t R;
            struct {
                vuint32_t EDC:1;
                  vuint32_t:4;
                vuint32_t SIZE:3;
                  vuint32_t:1;
                vuint32_t LAS:3;
                  vuint32_t:3;
                vuint32_t MAS:1;
                vuint32_t EER:1;
                vuint32_t RWE:1;
                  vuint32_t:1;
                  vuint32_t:1;
                vuint32_t PEAS:1;
                vuint32_t DONE:1;
                vuint32_t PEG:1;
                  vuint32_t:4;
                vuint32_t PGM:1;
                vuint32_t PSUS:1;
                vuint32_t ERS:1;
                vuint32_t ESUS:1;
                vuint32_t EHV:1;
            } B;
        } MCR;

        union {                        /* LML Register */
            vuint32_t R;
            struct {
                vuint32_t LME:1;
                  vuint32_t:10;
                vuint32_t TSLK:1;
                  vuint32_t:2;
                vuint32_t MLK:2;
                vuint32_t LLK:16;
            } B;
        } LML;

        union {                        /* HBL Register */
            vuint32_t R;
            struct {
                vuint32_t HBE:1;
                  vuint32_t:23;
                vuint32_t HLK:8;
            } B;
        } HBL;

        union {                        /* SLML Register */
            vuint32_t R;
            struct {
                vuint32_t SLE:1;
                  vuint32_t:10;
                vuint32_t STSLK:1;
                  vuint32_t:2;
                vuint32_t SMK:2;
                vuint32_t SLK:16;
            } B;
        } SLL;

        union {                        /* LMS Register */
            vuint32_t R;
            struct {
                  vuint32_t:14;
                vuint32_t MSL:2;
                vuint32_t LSL:16;
            } B;
        } LMS;

        union {                        /* High Address Space Block Select Register */
            vuint32_t R;
            struct {
                  vuint32_t:26;
                vuint32_t HSL:6;
            } B;
        } HBS;

        union {                        /* Address Register */
            vuint32_t R;
            struct {
                  vuint32_t:10;
                vuint32_t ADD:19;
                  vuint32_t:3;
            } B;
        } ADR;

        union {                        /* CFLASH Configuration Register 0 */
            vuint32_t R;
            struct {
                vuint32_t B02_APC:5;
                vuint32_t B02_WWSC:5;
                vuint32_t B02_RWSC:5;
                vuint32_t B02_RWWC2:1;
                vuint32_t B02_RWWC1:1;
                vuint32_t B02_P1_BCFG:2;
                vuint32_t B02_P1_DPFE:1;
                vuint32_t B02_P1_IPFE:1;
                vuint32_t B02_P1_PFLM:2;
                vuint32_t B02_P1_BFE:1;
                vuint32_t B02_RWWC0:1;
                vuint32_t B02_P0_BCFG:2;
                vuint32_t B02_P0_DPFE:1;
                vuint32_t B02_P0_IPFE:1;
                vuint32_t B02_P0_PFLM:2;
                vuint32_t B02_P0_BFE:1;
            } B;
        } PFCR0;

        union {                        /* CFLASH Configuration Register 1 */
            vuint32_t R;
            struct {
                vuint32_t B1_APC:5;
                vuint32_t B1_WWSC:5;
                vuint32_t B1_RWSC:5;
                vuint32_t B1_RWWC2:1;
                vuint32_t B1_RWWC1:1;
                  vuint32_t:6;
                vuint32_t B1_P1_BFE:1;
                vuint32_t B1_RWWC0:1;
                  vuint32_t:6;
                vuint32_t B1_P0_BFE:1;
            } B;
        } PFCR1;

        union {                        /* cflash Access Protection Register */
            vuint32_t R;
            struct {
                  vuint32_t:6;
                vuint32_t ARBM:2;
                vuint32_t M7PFD:1;
                vuint32_t M6PFD:1;
                vuint32_t M5PFD:1;
                vuint32_t M4PFD:1;
                vuint32_t M3PFD:1;
                vuint32_t M2PFD:1;
                vuint32_t M1PFD:1;
                vuint32_t M0PFD:1;
                vuint32_t M7AP:2;
                vuint32_t M6AP:2;
                vuint32_t M5AP:2;
                vuint32_t M4AP:2;
                vuint32_t M3AP:2;
                vuint32_t M2AP:2;
                vuint32_t M1AP:2;
                vuint32_t M0AP:2;
            } B;
        } FAPR;

        int32_t CFLASH_reserved0[5];   /* {0x003C-0x0028}/0x4 = 0x05 */

        union {                        /* User Test Register 0 */
            vuint32_t R;
            struct {
                vuint32_t UTE:1;
                  vuint32_t:7;
                vuint32_t DSI:8;
                  vuint32_t:10;
                vuint32_t MRE:1;
                vuint32_t MRV:1;
                vuint32_t EIE:1;
                vuint32_t AIS:1;
                vuint32_t AIE:1;
                vuint32_t AID:1;
            } B;
        } UT0;

        union {                        /* User Test Register 1 */
            vuint32_t R;
            struct {
                vuint32_t DAI:32;
            } B;
        } UT1;

        union {                        /* User Test Register 2 */
            vuint32_t R;
            struct {
                vuint32_t DAI:32;
            } B;
        } UT2;

        union {                        /* User Multiple Input Signature Register 0-4 */
            vuint32_t R;
            struct {
                vuint32_t MS:32;
            } B;
        } UMISR[5];

    };                                 /* end of CFLASH_tag */
/****************************************************************** 
| defines and macros (scope: module-local) 
|-----------------------------------------------------------------*/
/* Define instances of modules */
#define CFLASH0         (*(volatile struct CFLASH_tag *)    0xC3F88000UL)
#define DFLASH          (*(volatile struct DFLASH_tag *)    0xC3F8C000UL)
#define SIU             (*(volatile struct SIU_tag *)       0xC3F90000UL)
#define WKUP            (*(volatile struct WKUP_tag *)      0xC3F94000UL)
#define EMIOS_0         (*(volatile struct EMIOS_tag *)     0xC3FA0000UL)
#define EMIOS_1         (*(volatile struct EMIOS_tag *)     0xC3FA4000UL)
#define CFLASH1         (*(volatile struct CFLASH_tag *)    0xC3FB0000UL)
#define SSCM            (*(volatile struct SSCM_tag *)      0xC3FD8000UL)
#define ME              (*(volatile struct ME_tag *)        0xC3FDC000UL)
#define CGM             (*(volatile struct CGM_tag *)       0xC3FE0000UL)
#define RGM             (*(volatile struct RGM_tag *)       0xC3FE4000UL)
#define PCU             (*(volatile struct PCU_tag *)       0xC3FE8000UL)
#define RTC             (*(volatile struct RTC_tag *)       0xC3FEC000UL)
#define PIT             (*(volatile struct PIT_tag *)       0xC3FF0000UL)
#define ADC_0           (*(volatile struct ADC_tag *)       0xFFE00000UL)
#define I2C_0           (*(volatile struct I2C_tag *)       0xFFE30000UL)
#define I2C_1           (*(volatile struct I2C_tag *)       0xFFE34000UL)
#define I2C_2           (*(volatile struct I2C_tag *)       0xFFE38000UL)
#define I2C_3           (*(volatile struct I2C_tag *)       0xFFE3C000UL)
#define LINFLEX_0       (*(volatile struct LINFLEX_tag *)   0xFFE40000UL)
#define LINFLEX_1       (*(volatile struct LINFLEX_tag *)   0xFFE44000UL)
#define SMC             (*(volatile struct SMC_tag *)       0xFFE60000UL)
#define SSD_0           (*(volatile struct SSD_tag *)       0xFFE61000UL)
#define SSD_1           (*(volatile struct SSD_tag *)       0xFFE61800UL)
#define SSD_2           (*(volatile struct SSD_tag *)       0xFFE62000UL)
#define SSD_3           (*(volatile struct SSD_tag *)       0xFFE62800UL)
#define SSD_4           (*(volatile struct SSD_tag *)       0xFFE63000UL)
#define SSD_5           (*(volatile struct SSD_tag *)       0xFFE63800UL)
#define CANSP           (*(volatile struct CANSP_tag *)     0xFFE70000UL)
#define LCD             (*(volatile struct LCD_tag *)       0xFFE74000UL)
#define SGL             (*(volatile struct SGL_tag *)       0xFFE78000UL)
#define DCU             (*(volatile struct DCU_tag *)       0xFFE7C000UL)
#define MPU             (*(volatile struct MPU_tag *)       0xFFF10000UL)
#define SWT             (*(volatile struct SWT_tag *)       0xFFF38000UL)
#define STM             (*(volatile struct STM_tag *)       0xFFF3C000UL)
#define ECSM            (*(volatile struct ECSM_tag *)      0xFFF40000UL)
#define EDMA            (*(volatile struct EDMA_tag *)      0xFFF44000UL)
#define INTC            (*(volatile struct INTC_tag *)      0xFFF48000UL)
#define DSPI_0          (*(volatile struct DSPI_tag *)      0xFFF90000UL)
#define DSPI_1          (*(volatile struct DSPI_tag *)      0xFFF94000UL)
#define QUADSPI_0       (*(volatile struct QUADSPI_tag *)   0xFFFA8000UL)
#define CAN_0           (*(volatile struct FLEXCAN_tag *)   0xFFFC0000UL)
#define CAN_1           (*(volatile struct FLEXCAN_tag *)   0xFFFC4000UL)
#define DMAMUX          (*(volatile struct DMAMUX_tag *)    0xFFFDC000UL)

#ifdef __MWERKS__
#pragma pop
#endif

#ifdef  __cplusplus
}
#endif
#endif                            /* ifdef _JDP_H */
/* End of file */
