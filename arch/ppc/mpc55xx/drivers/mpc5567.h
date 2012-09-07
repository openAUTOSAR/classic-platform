
/**************************************************************************/
/* FILE NAME: mpc5567.h                      COPYRIGHT (c) Freescale 2007 */
/* VERSION:  1.6                                  All Rights Reserved     */
/*                                                                        */
/* DESCRIPTION:                                                           */
/* This file contain all of the register and bit field definitions for    */
/* MPC5567.                                                               */
/*========================================================================*/
/* UPDATE HISTORY                                                         */
/* REV      AUTHOR      DATE       DESCRIPTION OF CHANGE                  */
/* ---   -----------  ---------    ---------------------                  */
/* 1.0   G. Emerson   03/Jan/06    Initial version.                       */
/* 1.1   G. Emerson   27/Mar/06    Fix issue with Flexcan BCC field.      */
/* 1.2   S. Mathieson 28/Jul/06    Change Flexcan BCC bit to MBFEN        */
/*                                 Add Flexcan bits WRNEN, SRXDIS,        */
/*                                 TWRNMSK, RWRNMSK,TWRNINT,RWRNINT       */
/* 1.3   S. Mathieson 30/Aug/06    SPR: L1SCR0, updated bit name from DPP */
/*                                 to DPB to align with documentation.    */
/* 1.4   S. Mathieson 26/Feb/07    eDMA TCD format updated to include     */
/*                                 alternate configuration.               */
/*                                 INTC, correction to the number of PSR  */
/*                                 registers.                             */
/*                                 Updates to bitfield sizes in MBSSUTR,  */
/*                                 MBIVEC, MBIDX & RSBIR. RSBIR, SELEC    */
/*                                 changed to SEL & RFRFCFR, FNUM changed */
/*                                 to SEL to align with documentation.    */
/*                                 Various register/ bitfield updates to  */
/*                                 correct errors (MCR, TMODE bit removed.*/
/*                                 PADR register removed. PIER1, DRDIE bit*/
/*                                 removed & PIFR1, DRDIF removed. PCR1,  */
/*                                 Filter bypass bit removed).            */
/* 1.5	 S. Mathieson 25/Apr/07    SRAM size changed from 64K to 80K.     */
/*                                                                        */
/* 1.6   G. Emerson   26/Oct/07    All module instantiations now volatile */
/*                                 All registers/bit fields now volatile  */
/**************************************************************************/
/*>>>>NOTE! this file is auto-generated please do not edit it!<<<<*/

#ifndef _MPC5567_H_
#define _MPC5567_H_

#include "Compiler.h"
#include "typedefs.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __MWERKS__
#pragma push
#pragma ANSI_strict off
#endif

/****************************************************************************/
/*                          MODULE : PBRIDGE_A Peripheral Bridge            */
/****************************************************************************/
    CC_EXTENSION struct PBRIDGE_A_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t MBW0:1;
                vuint32_t MTR0:1;
                vuint32_t MTW0:1;
                vuint32_t MPL0:1;
                vuint32_t MBW1:1;
                vuint32_t MTR1:1;
                vuint32_t MTW1:1;
                vuint32_t MPL1:1;
                vuint32_t MBW2:1;
                vuint32_t MTR2:1;
                vuint32_t MTW2:1;
                vuint32_t MPL2:1;
                vuint32_t MBW3:1;
                vuint32_t MTR3:1;
                vuint32_t MTW3:1;
                vuint32_t MPL3:1;

                vuint32_t MBW4:1;       /* FEC */
                vuint32_t MTR4:1;
                vuint32_t MTW4:1;
                vuint32_t MPL4:1;

                  vuint32_t:4;

                vuint32_t MBW6:1;       /* FLEXRAY */
                vuint32_t MTR6:1;
                vuint32_t MTW6:1;
                vuint32_t MPL6:1;

                  vuint32_t:4;
            } B;
        } MPCR;                 /* Master Privilege Control Register */

        uint32_t pbridge_a_reserved2[7];

        union {
            vuint32_t R;
            struct {
                vuint32_t BW0:1;
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;
                  vuint32_t:28;
            } B;
        } PACR0;

        uint32_t pbridge_a_reserved3[7];

        union {
            vuint32_t R;
            struct {
                vuint32_t BW0:1;
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;
                vuint32_t BW1:1;
                vuint32_t SP1:1;
                vuint32_t WP1:1;
                vuint32_t TP1:1;
                vuint32_t BW2:1;
                vuint32_t SP2:1;
                vuint32_t WP2:1;
                vuint32_t TP2:1;
                  vuint32_t:4;
                vuint32_t BW4:1;
                vuint32_t SP4:1;
                vuint32_t WP4:1;
                vuint32_t TP4:1;
                  vuint32_t:12;
            } B;
        } OPACR0;

        union {
            vuint32_t R;
            struct {

                vuint32_t BW0:1;        /* EMIOS */
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;

                  vuint32_t:28;
            } B;
        } OPACR1;

        union {
            vuint32_t R;
            struct {
                vuint32_t BW0:1;
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;
                  vuint32_t:4;
                vuint32_t BW2:1;
                vuint32_t SP2:1;
                vuint32_t WP2:1;
                vuint32_t TP2:1;
                vuint32_t BW3:1;
                vuint32_t SP3:1;
                vuint32_t WP3:1;
                vuint32_t TP3:1;
                vuint32_t BW4:1;
                vuint32_t SP4:1;
                vuint32_t WP4:1;
                vuint32_t TP4:1;
                  vuint32_t:12;
            } B;
        } OPACR2;

    };

/****************************************************************************/
/*                          MODULE : PBRIDGE_B Peripheral Bridge            */
/****************************************************************************/
    CC_EXTENSION struct PBRIDGE_B_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t MBW0:1;
                vuint32_t MTR0:1;
                vuint32_t MTW0:1;
                vuint32_t MPL0:1;
                vuint32_t MBW1:1;
                vuint32_t MTR1:1;
                vuint32_t MTW1:1;
                vuint32_t MPL1:1;
                vuint32_t MBW2:1;
                vuint32_t MTR2:1;
                vuint32_t MTW2:1;
                vuint32_t MPL2:1;
                vuint32_t MBW3:1;
                vuint32_t MTR3:1;
                vuint32_t MTW3:1;
                vuint32_t MPL3:1;

                vuint32_t MBW4:1;       /* FEC */
                vuint32_t MTR4:1;
                vuint32_t MTW4:1;
                vuint32_t MPL4:1;

                  vuint32_t:4;

                vuint32_t MBW6:1;       /* FLEXRAY */
                vuint32_t MTR6:1;
                vuint32_t MTW6:1;
                vuint32_t MPL6:1;

                  vuint32_t:4;
            } B;
        } MPCR;                 /* Master Privilege Control Register */

        uint32_t pbridge_b_reserved2[7];

        union {
            vuint32_t R;
            struct {
                vuint32_t BW0:1;
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;
                vuint32_t BW1:1;
                vuint32_t SP1:1;
                vuint32_t WP1:1;
                vuint32_t TP1:1;
                  vuint32_t:24;
            } B;
        } PACR0;

        uint32_t pbridge_b_reserved3;

        union {
            vuint32_t R;
            struct {
                vuint32_t BW0:1;
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;
                vuint32_t BW1:1;
                vuint32_t SP1:1;
                vuint32_t WP1:1;
                vuint32_t TP1:1;
                vuint32_t BW2:1;
                vuint32_t SP2:1;
                vuint32_t WP2:1;
                vuint32_t TP2:1;

                vuint32_t BW3:1;        /* FEC */
                vuint32_t SP3:1;
                vuint32_t WP3:1;
                vuint32_t TP3:1;

                  vuint32_t:16;

            } B;
        } PACR2;

        uint32_t pbridge_b_reserved4[5];

        union {
            vuint32_t R;
            struct {
                vuint32_t BW0:1;
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;
                  vuint32_t:12;

                  vuint32_t:4;

                vuint32_t BW5:1;        /* DSPI_B */
                vuint32_t SP5:1;
                vuint32_t WP5:1;
                vuint32_t TP5:1;

                vuint32_t BW6:1;
                vuint32_t SP6:1;
                vuint32_t WP6:1;
                vuint32_t TP6:1;
                vuint32_t BW7:1;
                vuint32_t SP7:1;
                vuint32_t WP7:1;
                vuint32_t TP7:1;
            } B;
        } OPACR0;

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t BW4:1;
                vuint32_t SP4:1;
                vuint32_t WP4:1;
                vuint32_t TP4:1;

                vuint32_t BW5:1;        /* ESCI_B */
                vuint32_t SP5:1;
                vuint32_t WP5:1;
                vuint32_t TP5:1;

                  vuint32_t:8;
            } B;
        } OPACR1;

        union {
            vuint32_t R;
            struct {
                vuint32_t BW0:1;
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;

                vuint32_t BW1:1;        /* CAN_B */
                vuint32_t SP1:1;
                vuint32_t WP1:1;
                vuint32_t TP1:1;

                vuint32_t BW2:1;
                vuint32_t SP2:1;
                vuint32_t WP2:1;
                vuint32_t TP2:1;

                vuint32_t BW3:1;        /* CAN_D */
                vuint32_t SP3:1;
                vuint32_t WP3:1;
                vuint32_t TP3:1;

                vuint32_t BW4:1;        /* CAN_E */
                vuint32_t SP4:1;
                vuint32_t WP4:1;
                vuint32_t TP4:1;

                  vuint32_t:12;
            } B;
        } OPACR2;

        union {
            vuint32_t R;
            struct {

                vuint32_t BW0:1;        /* FLEXRAY */
                vuint32_t SP0:1;
                vuint32_t WP0:1;
                vuint32_t TP0:1;

                  vuint32_t:24;
                vuint32_t BW7:1;
                vuint32_t SP7:1;
                vuint32_t WP7:1;
                vuint32_t TP7:1;
            } B;
        } OPACR3;

    };
/****************************************************************************/
/*                     MODULE : FMPLL                                       */
/****************************************************************************/
    CC_EXTENSION struct FMPLL_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:1;
                vuint32_t PREDIV:3;
                vuint32_t MFD:5;
                  vuint32_t:1;
                vuint32_t RFD:3;
                vuint32_t LOCEN:1;
                vuint32_t LOLRE:1;
                vuint32_t LOCRE:1;
                vuint32_t DISCLK:1;
                vuint32_t LOLIRQ:1;
                vuint32_t LOCIRQ:1;
                vuint32_t RATE:1;
                vuint32_t DEPTH:2;
                vuint32_t EXP:10;
            } B;
        } SYNCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:22;
                vuint32_t LOLF:1;
                vuint32_t LOC:1;
                vuint32_t MODE:1;
                vuint32_t PLLSEL:1;
                vuint32_t PLLREF:1;
                vuint32_t LOCKS:1;
                vuint32_t LOCK:1;
                vuint32_t LOCF:1;
                vuint32_t CALDONE:1;
                vuint32_t CALPASS:1;
            } B;
        } SYNSR;

    };
/****************************************************************************/
/*                     MODULE : External Bus Interface (EBI)                */
/****************************************************************************/
    CC_EXTENSION  struct CS_tag {
        union {                 /* Base Register Bank */
            vuint32_t R;
            struct {
                vuint32_t BA:17;
                  vuint32_t:3;
                vuint32_t PS:1;
                  vuint32_t:4;
                vuint32_t BL:1;
                vuint32_t WEBS:1;
                vuint32_t TBDIP:1;
                  vuint32_t:2;
                vuint32_t BI:1;
                vuint32_t V:1;
            } B;
        } BR;

        union {                 /* Option Register Bank */
            vuint32_t R;
            struct {
                vuint32_t AM:17;
                  vuint32_t:7;
                vuint32_t SCY:4;
                  vuint32_t:1;
                vuint32_t BSCY:2;
                  vuint32_t:1;
            } B;
        } OR;
    };

    struct CAL_CS_tag {
        union {                 /* Calibration Base Register Bank */
            vuint32_t R;
            struct {
                vuint32_t BA:17;
                  vuint32_t:3;
                vuint32_t PS:1;
                  vuint32_t:4;
                vuint32_t BL:1;
                vuint32_t WEBS:1;
                vuint32_t TBDIP:1;
                  vuint32_t:2;
                vuint32_t BI:1;
                vuint32_t V:1;
            } B;
        } BR;

        union {                 /* Calibration Option Register Bank */
            vuint32_t R;
            struct {
                vuint32_t AM:17;
                  vuint32_t:7;
                vuint32_t SCY:4;
                  vuint32_t:1;
                vuint32_t BSCY:2;
                  vuint32_t:1;
            } B;
        } OR;
    };

    struct EBI_tag {
        union {                 /* Module Configuration Register */
            vuint32_t R;
            struct {
                vuint32_t:5;
                vuint32_t SIZEEN:1;
                vuint32_t SIZE:2;
                  vuint32_t:8;
                vuint32_t ACGE:1;
                vuint32_t EXTM:1;
                vuint32_t EARB:1;
                vuint32_t EARP:2;
                  vuint32_t:4;
                vuint32_t MDIS:1;
                  vuint32_t:5;
                vuint32_t DBM:1;
            } B;
        } MCR;

        uint32_t EBI_reserved1;

        union {                 /* Transfer Error Status Register */
            vuint32_t R;
            struct {
                vuint32_t:30;
                vuint32_t TEAF:1;
                vuint32_t BMTF:1;
            } B;
        } TESR;

        union {                 /* Bus Monitor Control Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t BMT:8;
                vuint32_t BME:1;
                  vuint32_t:7;
            } B;
        } BMCR;

        struct CS_tag CS[4];

/* Calibration registers */
        uint32_t EBI_reserved2[4];
        struct CAL_CS_tag CAL_CS[4];

    };
/****************************************************************************/
/*                     MODULE : FLASH                                       */
/****************************************************************************/
    CC_EXTENSION struct FLASH_tag {
        union {                 /* Module Configuration Register */
            vuint32_t R;
            struct {
                vuint32_t:4;
                vuint32_t SIZE:4;
                  vuint32_t:1;
                vuint32_t LAS:3;
                  vuint32_t:3;
                vuint32_t MAS:1;
                vuint32_t EER:1;
                vuint32_t RWE:1;
                vuint32_t BBEPE:1;
                vuint32_t EPE:1;
                vuint32_t PEAS:1;
                vuint32_t DONE:1;
                vuint32_t PEG:1;

                  vuint32_t:2;

                vuint32_t STOP:1;
                  vuint32_t:1;
                vuint32_t PGM:1;
                vuint32_t PSUS:1;
                vuint32_t ERS:1;
                vuint32_t ESUS:1;
                vuint32_t EHV:1;
            } B;
        } MCR;

        union {                 /* LML Register */
            vuint32_t R;
            struct {
                vuint32_t LME:1;
                  vuint32_t:10;
                vuint32_t SLOCK:1;
                vuint32_t MLOCK:4;
                vuint32_t LLOCK:16;
            } B;
        } LMLR;

        union {                 /* HL Register */
            vuint32_t R;
            struct {
                vuint32_t HBE:1;
                  vuint32_t:3;
                vuint32_t HBLOCK:28;
            } B;
        } HLR;

        union {                 /* SLML Register */
            vuint32_t R;
            struct {
                vuint32_t SLE:1;
                  vuint32_t:10;
                vuint32_t SSLOCK:1;
                vuint32_t SMLOCK:4;
                vuint32_t SLLOCK:16;
            } B;
        } SLMLR;

        union {                 /* LMS Register */
            vuint32_t R;
            struct {
                vuint32_t:12;
                vuint32_t MSEL:4;
                vuint32_t LSEL:16;
            } B;
        } LMSR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:4;
                vuint32_t HBSEL:28;
            } B;
        } HSR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:10;
                vuint32_t ADDR:19;
                  vuint32_t:3;
            } B;
        } AR;

        union {
            vuint32_t R;
            struct {

                vuint32_t:9;
                vuint32_t M6PFE:1;      /* Flexray */
                  vuint32_t:1;

                vuint32_t M4PFE:1;      /* FEC */

                vuint32_t M3PFE:1;
                vuint32_t M2PFE:1;
                vuint32_t M1PFE:1;
                vuint32_t M0PFE:1;
                vuint32_t APC:3;
                vuint32_t WWSC:2;
                vuint32_t RWSC:3;

                vuint32_t DPFEN:2;
                vuint32_t IPFEN:2;

                vuint32_t PFLIM:3;
                vuint32_t BFEN:1;
            } B;
        } BIUCR;

        union {
            vuint32_t R;
            struct {

                vuint32_t:18;
                vuint32_t M6AP:2;       /* Flexray */
                  vuint32_t:2;

                vuint32_t M4AP:2;       /* FEC */

                vuint32_t M3AP:2;
                vuint32_t M2AP:2;
                vuint32_t M1AP:2;
                vuint32_t M0AP:2;
            } B;
        } BIUAPR;
    };
/****************************************************************************/
/*                     MODULE : SIU                                         */
/****************************************************************************/
    CC_EXTENSION struct SIU_tag {
        int32_t SIU_reserved0;

        union {                 /* MCU ID Register */
            vuint32_t R;
            struct {
                vuint32_t PARTNUM:16;
                vuint32_t MASKNUM:16;
            } B;
        } MIDR;
        int32_t SIU_reserved00;

        union {                 /* Reset Status Register */
            vuint32_t R;
            struct {
                vuint32_t PORS:1;
                vuint32_t ERS:1;
                vuint32_t LLRS:1;
                vuint32_t LCRS:1;
                vuint32_t WDRS:1;
                vuint32_t CRS:1;
                  vuint32_t:8;
                vuint32_t SSRS:1;
                vuint32_t SERF:1;
                vuint32_t WKPCFG:1;
                  vuint32_t:12;
                vuint32_t BOOTCFG:2;
                vuint32_t RGF:1;
            } B;
        } RSR;

        union {                 /* System Reset Control Register */
            vuint32_t R;
            struct {
                vuint32_t SSR:1;
                vuint32_t SER:1;
                  vuint32_t:14;
                vuint32_t CRE:1;
                  vuint32_t:15;
            } B;
        } SRCR;

        union {                 /* External Interrupt Status Register */
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
        } EISR;

        union {                 /* DMA/Interrupt Request Enable Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
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
        } DIRER;

        union {                 /* DMA/Interrupt Select Register */
            vuint32_t R;
            struct {
                vuint32_t:28;
                vuint32_t DIRS3:1;
                vuint32_t DIRS2:1;
                vuint32_t DIRS1:1;
                vuint32_t DIRS0:1;
            } B;
        } DIRSR;

        union {                 /* Overrun Status Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t OVF15:1;
                vuint32_t OVF14:1;
                vuint32_t OVF13:1;
                vuint32_t OVF12:1;
                vuint32_t OVF11:1;
                vuint32_t OVF10:1;
                vuint32_t OVF9:1;
                vuint32_t OVF8:1;
                vuint32_t OVF7:1;
                vuint32_t OVF6:1;
                vuint32_t OVF5:1;
                vuint32_t OVF4:1;
                vuint32_t OVF3:1;
                vuint32_t OVF2:1;
                vuint32_t OVF1:1;
                vuint32_t OVF0:1;
            } B;
        } OSR;

        union {                 /* Overrun Request Enable Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t ORE15:1;
                vuint32_t ORE14:1;
                vuint32_t ORE13:1;
                vuint32_t ORE12:1;
                vuint32_t ORE11:1;
                vuint32_t ORE10:1;
                vuint32_t ORE9:1;
                vuint32_t ORE8:1;
                vuint32_t ORE7:1;
                vuint32_t ORE6:1;
                vuint32_t ORE5:1;
                vuint32_t ORE4:1;
                vuint32_t ORE3:1;
                vuint32_t ORE2:1;
                vuint32_t ORE1:1;
                vuint32_t ORE0:1;
            } B;
        } ORER;

        union {                 /* External IRQ Rising-Edge Event Enable Register */
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

        union {                 /* External IRQ Falling-Edge Event Enable Register */
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

        union {                 /* External IRQ Digital Filter Register */
            vuint32_t R;
            struct {
                vuint32_t:28;
                vuint32_t DFL:4;
            } B;
        } IDFR;

        int32_t SIU_reserved1[3];

        union {                 /* Pad Configuration Registers */
            vuint16_t R;
            struct {
                vuint16_t:3;
                vuint16_t PA:3;
                vuint16_t OBE:1;
                vuint16_t IBE:1;
                vuint16_t DSC:2;
                vuint16_t ODE:1;
                vuint16_t HYS:1;
                vuint16_t SRC:2;
                vuint16_t WPE:1;
                vuint16_t WPS:1;
            } B;
        } PCR[512];

        int16_t SIU_reserved_0[224];

        union {                 /* GPIO Pin Data Output Registers */
            vuint8_t R;
            struct {
                vuint8_t:7;
                vuint8_t PDO:1;
            } B;
        } GPDO[256];

        int32_t SIU_reserved_3[64];

        union {                 /* GPIO Pin Data Input Registers */
            vuint8_t R;
            struct {
                vuint8_t:7;
                vuint8_t PDI:1;
            } B;
        } GPDI[256];

        union {                 /* IMUX Register */
            vuint32_t R;
            struct {
                vuint32_t TSEL5:2;
                vuint32_t TSEL4:2;
                vuint32_t TSEL3:2;
                vuint32_t TSEL2:2;
                vuint32_t TSEL1:2;
                vuint32_t TSEL0:2;
                  vuint32_t:20;
            } B;
        } ETISR;

        union {                 /* IMUX Register */
            vuint32_t R;
            struct {
                vuint32_t ESEL15:2;
                vuint32_t ESEL14:2;
                vuint32_t ESEL13:2;
                vuint32_t ESEL12:2;
                vuint32_t ESEL11:2;
                vuint32_t ESEL10:2;
                vuint32_t ESEL9:2;
                vuint32_t ESEL8:2;
                vuint32_t ESEL7:2;
                vuint32_t ESEL6:2;
                vuint32_t ESEL5:2;
                vuint32_t ESEL4:2;
                vuint32_t ESEL3:2;
                vuint32_t ESEL2:2;
                vuint32_t ESEL1:2;
                vuint32_t ESEL0:2;
            } B;
        } EIISR;

        union {                 /* IMUX Register */
            vuint32_t R;
            struct {
                vuint32_t SINSELA:2;
                vuint32_t SSSELA:2;
                vuint32_t SCKSELA:2;
                vuint32_t TRIGSELA:2;
                vuint32_t SINSELB:2;
                vuint32_t SSSELB:2;
                vuint32_t SCKSELB:2;
                vuint32_t TRIGSELB:2;
                vuint32_t SINSELC:2;
                vuint32_t SSSELC:2;
                vuint32_t SCKSELC:2;
                vuint32_t TRIGSELC:2;
                vuint32_t SINSELD:2;
                vuint32_t SSSELD:2;
                vuint32_t SCKSELD:2;
                vuint32_t TRIGSELD:2;
            } B;
        } DISR;

        int32_t SIU_reserved2[29];

        union {                 /* Chip Configuration Register Register */
            vuint32_t R;
            struct {
                vuint32_t:14;
                vuint32_t MATCH:1;
                vuint32_t DISNEX:1;
                  vuint32_t:16;
            } B;
        } CCR;

        union {                 /* External Clock Configuration Register Register */
            vuint32_t R;
            struct {
                vuint32_t:18;
                vuint32_t ENGDIV:6;
                  vuint32_t:4;
                vuint32_t EBTS:1;
                  vuint32_t:1;
                vuint32_t EBDF:2;
            } B;
        } ECCR;

        union {
            vuint32_t R;
        } CARH;

        union {
            vuint32_t R;
        } CARL;

        union {
            vuint32_t R;
        } CBRH;

        union {
            vuint32_t R;
        } CBRL;

    };
/****************************************************************************/
/*                          MODULE : EMIOS                                  */
/****************************************************************************/
    CC_EXTENSION struct EMIOS_tag {
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
        } MCR;                  /* Module Configuration Register */

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
        } GFR;                  /* Global FLAG Register */

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
        } OUDR;                 /* Output Update Disable Register */

        uint32_t emios_reserved[5];

        struct {
            union {
                vuint32_t R;    /* Channel A Data Register */
            } CADR;

            union {
                vuint32_t R;    /* Channel B Data Register */
            } CBDR;

            union {
                vuint32_t R;    /* Channel Counter Register */
            } CCNTR;

            union {
                vuint32_t R;
                struct {
                    vuint32_t FREN:1;
                    vuint32_t ODIS:1;
                    vuint32_t ODISSL:2;
                    vuint32_t UCPRE:2;
                    vuint32_t UCPREN:1;
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
            } CCR;              /* Channel Control Register */

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
            } CSR;              /* Channel Status Register */

            union {
                vuint32_t R;    /* Alternate Channel A Data Register */
            } ALTCADR;

            uint32_t emios_channel_reserved[2];

        } CH[24];

    };
/****************************************************************************/
/*                              MODULE :ETPU                                */
/****************************************************************************/

/***************************Configuration Registers**************************/

    CC_EXTENSION struct ETPU_tag {
        union {                 /* MODULE CONFIGURATION REGISTER */
            vuint32_t R;
            struct {
                vuint32_t GEC:1;        /* Global Exception Clear */
                  vuint32_t:3;
                vuint32_t MGE1:1;       /* Microcode Global Exception-ETPU_A */

                  vuint32_t:1;  /* For single ETPU implementations */

                vuint32_t ILF1:1;       /* Illegal Instruction Flag-ETPU_A */

                  vuint32_t:1;  /* For single ETPU implementations */

                  vuint32_t:3;
                vuint32_t SCMSIZE:5;    /* Shared Code Memory size */
                  vuint32_t:5;
                vuint32_t SCMMISF:1;    /* SCM MISC Flag */
                vuint32_t SCMMISEN:1;   /* SCM MISC Enable */
                  vuint32_t:2;
                vuint32_t VIS:1;        /* SCM Visability */
                  vuint32_t:5;
                vuint32_t GTBE:1;       /* Global Time Base Enable */
            } B;
        } MCR;

        union {                 /* COHERENT DUAL-PARAMETER CONTROL */
            vuint32_t R;
            struct {
                vuint32_t STS:1;        /* Start Status bit */
                vuint32_t CTBASE:5;     /* Channel Transfer Base */
                vuint32_t PBASE:10;     /* Parameter Buffer Base Address */
                vuint32_t PWIDTH:1;     /* Parameter Width */
                vuint32_t PARAM0:7;     /* Channel Parameter 0 */
                vuint32_t WR:1;
                vuint32_t PARAM1:7;     /* Channel Parameter 1 */
            } B;
        } CDCR;

        uint32_t etpu_reserved1;

        union {                 /* MISC Compare Register */
            vuint32_t R;
        } MISCCMPR;

        union {                 /* SCM off-range Date Register */
            vuint32_t R;
        } SCMOFFDATAR;

        union {                 /* ETPU_A Configuration Register */
            vuint32_t R;
            struct {
                vuint32_t FEND:1;       /* Force END */
                vuint32_t MDIS:1;       /* Low power Stop */
                  vuint32_t:1;
                vuint32_t STF:1;        /* Stop Flag */
                  vuint32_t:4;
                vuint32_t HLTF:1;       /* Halt Mode Flag */
                  vuint32_t:4;
                vuint32_t FPSCK:3;      /* Filter Prescaler Clock Control */
                vuint32_t CDFC:2;
                  vuint32_t:9;
                vuint32_t ETB:5;        /* Entry Table Base */
            } B;
        } ECR_A;
        uint32_t etpu_reserved3;        /* For single ETPU implementations */

        uint32_t etpu_reserved4;

        union {                 /* ETPU_A Timebase Configuration Register */
            vuint32_t R;
            struct {
                vuint32_t TCR2CTL:3;    /* TCR2 Clock/Gate Control */
                vuint32_t TCRCF:2;      /* TCRCLK Signal Filter Control */
                  vuint32_t:1;
                vuint32_t AM:1; /* Angle Mode */
                  vuint32_t:3;
                vuint32_t TCR2P:6;      /* TCR2 Prescaler Control */
                vuint32_t TCR1CTL:2;    /* TCR1 Clock/Gate Control */
                  vuint32_t:6;
                vuint32_t TCR1P:8;      /* TCR1 Prescaler Control */
            } B;
        } TBCR_A;

        union {                 /* ETPU_A TCR1 Visibility Register */
            vuint32_t R;
        } TB1R_A;

        union {                 /* ETPU_A TCR2 Visibility Register */
            vuint32_t R;
        } TB2R_A;

        union {                 /* ETPU_A STAC Configuration Register */
            vuint32_t R;
            struct {
                vuint32_t REN1:1;       /* Resource Enable TCR1 */
                vuint32_t RSC1:1;       /* Resource Control TCR1 */
                  vuint32_t:2;
                vuint32_t SERVER_ID1:4;
                  vuint32_t:4;
                vuint32_t SRV1:4;       /* Resource Server Slot */
                vuint32_t REN2:1;       /* Resource Enable TCR2 */
                vuint32_t RSC2:1;       /* Resource Control TCR2 */
                  vuint32_t:2;
                vuint32_t SERVER_ID2:4;
                  vuint32_t:4;
                vuint32_t SRV2:4;       /* Resource Server Slot */
            } B;
        } REDCR_A;

        uint32_t etpu_reserved5[4];
        uint32_t etpu_reserved6[4];     /* For single ETPU implementations */

        uint32_t etpu_reserved7[108];

/*****************************Status and Control Registers**************************/

        union {                 /* ETPU_A Channel Interrut Status */
            vuint32_t R;
            struct {
                vuint32_t CIS31:1;      /* Channel 31 Interrut Status */
                vuint32_t CIS30:1;      /* Channel 30 Interrut Status */
                vuint32_t CIS29:1;      /* Channel 29 Interrut Status */
                vuint32_t CIS28:1;      /* Channel 28 Interrut Status */
                vuint32_t CIS27:1;      /* Channel 27 Interrut Status */
                vuint32_t CIS26:1;      /* Channel 26 Interrut Status */
                vuint32_t CIS25:1;      /* Channel 25 Interrut Status */
                vuint32_t CIS24:1;      /* Channel 24 Interrut Status */
                vuint32_t CIS23:1;      /* Channel 23 Interrut Status */
                vuint32_t CIS22:1;      /* Channel 22 Interrut Status */
                vuint32_t CIS21:1;      /* Channel 21 Interrut Status */
                vuint32_t CIS20:1;      /* Channel 20 Interrut Status */
                vuint32_t CIS19:1;      /* Channel 19 Interrut Status */
                vuint32_t CIS18:1;      /* Channel 18 Interrut Status */
                vuint32_t CIS17:1;      /* Channel 17 Interrut Status */
                vuint32_t CIS16:1;      /* Channel 16 Interrut Status */
                vuint32_t CIS15:1;      /* Channel 15 Interrut Status */
                vuint32_t CIS14:1;      /* Channel 14 Interrut Status */
                vuint32_t CIS13:1;      /* Channel 13 Interrut Status */
                vuint32_t CIS12:1;      /* Channel 12 Interrut Status */
                vuint32_t CIS11:1;      /* Channel 11 Interrut Status */
                vuint32_t CIS10:1;      /* Channel 10 Interrut Status */
                vuint32_t CIS9:1;       /* Channel 9 Interrut Status */
                vuint32_t CIS8:1;       /* Channel 8 Interrut Status */
                vuint32_t CIS7:1;       /* Channel 7 Interrut Status */
                vuint32_t CIS6:1;       /* Channel 6 Interrut Status */
                vuint32_t CIS5:1;       /* Channel 5 Interrut Status */
                vuint32_t CIS4:1;       /* Channel 4 Interrut Status */
                vuint32_t CIS3:1;       /* Channel 3 Interrut Status */
                vuint32_t CIS2:1;       /* Channel 2 Interrut Status */
                vuint32_t CIS1:1;       /* Channel 1 Interrut Status */
                vuint32_t CIS0:1;       /* Channel 0 Interrut Status */
            } B;
        } CISR_A;
        uint32_t etpu_reserved8;        /* For single ETPU implementations */

        uint32_t etpu_reserved9[2];

        union {                 /* ETPU_A Data Transfer Request Status */
            vuint32_t R;
            struct {
                vuint32_t DTRS31:1;     /* Channel 31 Data Transfer Request Status */
                vuint32_t DTRS30:1;     /* Channel 30 Data Transfer Request Status */
                vuint32_t DTRS29:1;     /* Channel 29 Data Transfer Request Status */
                vuint32_t DTRS28:1;     /* Channel 28 Data Transfer Request Status */
                vuint32_t DTRS27:1;     /* Channel 27 Data Transfer Request Status */
                vuint32_t DTRS26:1;     /* Channel 26 Data Transfer Request Status */
                vuint32_t DTRS25:1;     /* Channel 25 Data Transfer Request Status */
                vuint32_t DTRS24:1;     /* Channel 24 Data Transfer Request Status */
                vuint32_t DTRS23:1;     /* Channel 23 Data Transfer Request Status */
                vuint32_t DTRS22:1;     /* Channel 22 Data Transfer Request Status */
                vuint32_t DTRS21:1;     /* Channel 21 Data Transfer Request Status */
                vuint32_t DTRS20:1;     /* Channel 20 Data Transfer Request Status */
                vuint32_t DTRS19:1;     /* Channel 19 Data Transfer Request Status */
                vuint32_t DTRS18:1;     /* Channel 18 Data Transfer Request Status */
                vuint32_t DTRS17:1;     /* Channel 17 Data Transfer Request Status */
                vuint32_t DTRS16:1;     /* Channel 16 Data Transfer Request Status */
                vuint32_t DTRS15:1;     /* Channel 15 Data Transfer Request Status */
                vuint32_t DTRS14:1;     /* Channel 14 Data Transfer Request Status */
                vuint32_t DTRS13:1;     /* Channel 13 Data Transfer Request Status */
                vuint32_t DTRS12:1;     /* Channel 12 Data Transfer Request Status */
                vuint32_t DTRS11:1;     /* Channel 11 Data Transfer Request Status */
                vuint32_t DTRS10:1;     /* Channel 10 Data Transfer Request Status */
                vuint32_t DTRS9:1;      /* Channel 9 Data Transfer Request Status */
                vuint32_t DTRS8:1;      /* Channel 8 Data Transfer Request Status */
                vuint32_t DTRS7:1;      /* Channel 7 Data Transfer Request Status */
                vuint32_t DTRS6:1;      /* Channel 6 Data Transfer Request Status */
                vuint32_t DTRS5:1;      /* Channel 5 Data Transfer Request Status */
                vuint32_t DTRS4:1;      /* Channel 4 Data Transfer Request Status */
                vuint32_t DTRS3:1;      /* Channel 3 Data Transfer Request Status */
                vuint32_t DTRS2:1;      /* Channel 2 Data Transfer Request Status */
                vuint32_t DTRS1:1;      /* Channel 1 Data Transfer Request Status */
                vuint32_t DTRS0:1;      /* Channel 0 Data Transfer Request Status */
            } B;
        } CDTRSR_A;
        uint32_t etpu_reserved10;       /* For single ETPU implementations */

        uint32_t etpu_reserved11[2];

        union {                 /* ETPU_A Interruput Overflow Status */
            vuint32_t R;
            struct {
                vuint32_t CIOS31:1;     /* Channel 31 Interruput Overflow Status */
                vuint32_t CIOS30:1;     /* Channel 30 Interruput Overflow Status */
                vuint32_t CIOS29:1;     /* Channel 29 Interruput Overflow Status */
                vuint32_t CIOS28:1;     /* Channel 28 Interruput Overflow Status */
                vuint32_t CIOS27:1;     /* Channel 27 Interruput Overflow Status */
                vuint32_t CIOS26:1;     /* Channel 26 Interruput Overflow Status */
                vuint32_t CIOS25:1;     /* Channel 25 Interruput Overflow Status */
                vuint32_t CIOS24:1;     /* Channel 24 Interruput Overflow Status */
                vuint32_t CIOS23:1;     /* Channel 23 Interruput Overflow Status */
                vuint32_t CIOS22:1;     /* Channel 22 Interruput Overflow Status */
                vuint32_t CIOS21:1;     /* Channel 21 Interruput Overflow Status */
                vuint32_t CIOS20:1;     /* Channel 20 Interruput Overflow Status */
                vuint32_t CIOS19:1;     /* Channel 19 Interruput Overflow Status */
                vuint32_t CIOS18:1;     /* Channel 18 Interruput Overflow Status */
                vuint32_t CIOS17:1;     /* Channel 17 Interruput Overflow Status */
                vuint32_t CIOS16:1;     /* Channel 16 Interruput Overflow Status */
                vuint32_t CIOS15:1;     /* Channel 15 Interruput Overflow Status */
                vuint32_t CIOS14:1;     /* Channel 14 Interruput Overflow Status */
                vuint32_t CIOS13:1;     /* Channel 13 Interruput Overflow Status */
                vuint32_t CIOS12:1;     /* Channel 12 Interruput Overflow Status */
                vuint32_t CIOS11:1;     /* Channel 11 Interruput Overflow Status */
                vuint32_t CIOS10:1;     /* Channel 10 Interruput Overflow Status */
                vuint32_t CIOS9:1;      /* Channel 9 Interruput Overflow Status */
                vuint32_t CIOS8:1;      /* Channel 8 Interruput Overflow Status */
                vuint32_t CIOS7:1;      /* Channel 7 Interruput Overflow Status */
                vuint32_t CIOS6:1;      /* Channel 6 Interruput Overflow Status */
                vuint32_t CIOS5:1;      /* Channel 5 Interruput Overflow Status */
                vuint32_t CIOS4:1;      /* Channel 4 Interruput Overflow Status */
                vuint32_t CIOS3:1;      /* Channel 3 Interruput Overflow Status */
                vuint32_t CIOS2:1;      /* Channel 2 Interruput Overflow Status */
                vuint32_t CIOS1:1;      /* Channel 1 Interruput Overflow Status */
                vuint32_t CIOS0:1;      /* Channel 0 Interruput Overflow Status */
            } B;
        } CIOSR_A;
        uint32_t etpu_reserved12;       /* For single ETPU implementations */

        uint32_t etpu_reserved13[2];

        union {                 /* ETPU_A Data Transfer Overflow Status */
            vuint32_t R;
            struct {
                vuint32_t DTROS31:1;    /* Channel 31 Data Transfer Overflow Status */
                vuint32_t DTROS30:1;    /* Channel 30 Data Transfer Overflow Status */
                vuint32_t DTROS29:1;    /* Channel 29 Data Transfer Overflow Status */
                vuint32_t DTROS28:1;    /* Channel 28 Data Transfer Overflow Status */
                vuint32_t DTROS27:1;    /* Channel 27 Data Transfer Overflow Status */
                vuint32_t DTROS26:1;    /* Channel 26 Data Transfer Overflow Status */
                vuint32_t DTROS25:1;    /* Channel 25 Data Transfer Overflow Status */
                vuint32_t DTROS24:1;    /* Channel 24 Data Transfer Overflow Status */
                vuint32_t DTROS23:1;    /* Channel 23 Data Transfer Overflow Status */
                vuint32_t DTROS22:1;    /* Channel 22 Data Transfer Overflow Status */
                vuint32_t DTROS21:1;    /* Channel 21 Data Transfer Overflow Status */
                vuint32_t DTROS20:1;    /* Channel 20 Data Transfer Overflow Status */
                vuint32_t DTROS19:1;    /* Channel 19 Data Transfer Overflow Status */
                vuint32_t DTROS18:1;    /* Channel 18 Data Transfer Overflow Status */
                vuint32_t DTROS17:1;    /* Channel 17 Data Transfer Overflow Status */
                vuint32_t DTROS16:1;    /* Channel 16 Data Transfer Overflow Status */
                vuint32_t DTROS15:1;    /* Channel 15 Data Transfer Overflow Status */
                vuint32_t DTROS14:1;    /* Channel 14 Data Transfer Overflow Status */
                vuint32_t DTROS13:1;    /* Channel 13 Data Transfer Overflow Status */
                vuint32_t DTROS12:1;    /* Channel 12 Data Transfer Overflow Status */
                vuint32_t DTROS11:1;    /* Channel 11 Data Transfer Overflow Status */
                vuint32_t DTROS10:1;    /* Channel 10 Data Transfer Overflow Status */
                vuint32_t DTROS9:1;     /* Channel 9 Data Transfer Overflow Status */
                vuint32_t DTROS8:1;     /* Channel 8 Data Transfer Overflow Status */
                vuint32_t DTROS7:1;     /* Channel 7 Data Transfer Overflow Status */
                vuint32_t DTROS6:1;     /* Channel 6 Data Transfer Overflow Status */
                vuint32_t DTROS5:1;     /* Channel 5 Data Transfer Overflow Status */
                vuint32_t DTROS4:1;     /* Channel 4 Data Transfer Overflow Status */
                vuint32_t DTROS3:1;     /* Channel 3 Data Transfer Overflow Status */
                vuint32_t DTROS2:1;     /* Channel 2 Data Transfer Overflow Status */
                vuint32_t DTROS1:1;     /* Channel 1 Data Transfer Overflow Status */
                vuint32_t DTROS0:1;     /* Channel 0 Data Transfer Overflow Status */
            } B;
        } CDTROSR_A;
        uint32_t etpu_reserved14;       /* For single ETPU implementations */

        uint32_t etpu_reserved15[2];

        union {                 /* ETPU_A Channel Interruput Enable */
            vuint32_t R;
            struct {
                vuint32_t CIE31:1;      /* Channel 31 Interruput Enable */
                vuint32_t CIE30:1;      /* Channel 30 Interruput Enable */
                vuint32_t CIE29:1;      /* Channel 29 Interruput Enable */
                vuint32_t CIE28:1;      /* Channel 28 Interruput Enable */
                vuint32_t CIE27:1;      /* Channel 27 Interruput Enable */
                vuint32_t CIE26:1;      /* Channel 26 Interruput Enable */
                vuint32_t CIE25:1;      /* Channel 25 Interruput Enable */
                vuint32_t CIE24:1;      /* Channel 24 Interruput Enable */
                vuint32_t CIE23:1;      /* Channel 23 Interruput Enable */
                vuint32_t CIE22:1;      /* Channel 22 Interruput Enable */
                vuint32_t CIE21:1;      /* Channel 21 Interruput Enable */
                vuint32_t CIE20:1;      /* Channel 20 Interruput Enable */
                vuint32_t CIE19:1;      /* Channel 19 Interruput Enable */
                vuint32_t CIE18:1;      /* Channel 18 Interruput Enable */
                vuint32_t CIE17:1;      /* Channel 17 Interruput Enable */
                vuint32_t CIE16:1;      /* Channel 16 Interruput Enable */
                vuint32_t CIE15:1;      /* Channel 15 Interruput Enable */
                vuint32_t CIE14:1;      /* Channel 14 Interruput Enable */
                vuint32_t CIE13:1;      /* Channel 13 Interruput Enable */
                vuint32_t CIE12:1;      /* Channel 12 Interruput Enable */
                vuint32_t CIE11:1;      /* Channel 11 Interruput Enable */
                vuint32_t CIE10:1;      /* Channel 10 Interruput Enable */
                vuint32_t CIE9:1;       /* Channel 9 Interruput Enable */
                vuint32_t CIE8:1;       /* Channel 8 Interruput Enable */
                vuint32_t CIE7:1;       /* Channel 7 Interruput Enable */
                vuint32_t CIE6:1;       /* Channel 6 Interruput Enable */
                vuint32_t CIE5:1;       /* Channel 5 Interruput Enable */
                vuint32_t CIE4:1;       /* Channel 4 Interruput Enable */
                vuint32_t CIE3:1;       /* Channel 3 Interruput Enable */
                vuint32_t CIE2:1;       /* Channel 2 Interruput Enable */
                vuint32_t CIE1:1;       /* Channel 1 Interruput Enable */
                vuint32_t CIE0:1;       /* Channel 0 Interruput Enable */
            } B;
        } CIER_A;
        uint32_t etpu_reserved16;       /* For single ETPU implementations */

        uint32_t etpu_reserved17[2];

        union {                 /* ETPU_A Channel Data Transfer Request Enable */
            vuint32_t R;
            struct {
                vuint32_t DTRE31:1;     /* Channel 31 Data Transfer Request Enable */
                vuint32_t DTRE30:1;     /* Channel 30 Data Transfer Request Enable */
                vuint32_t DTRE29:1;     /* Channel 29 Data Transfer Request Enable */
                vuint32_t DTRE28:1;     /* Channel 28 Data Transfer Request Enable */
                vuint32_t DTRE27:1;     /* Channel 27 Data Transfer Request Enable */
                vuint32_t DTRE26:1;     /* Channel 26 Data Transfer Request Enable */
                vuint32_t DTRE25:1;     /* Channel 25 Data Transfer Request Enable */
                vuint32_t DTRE24:1;     /* Channel 24 Data Transfer Request Enable */
                vuint32_t DTRE23:1;     /* Channel 23 Data Transfer Request Enable */
                vuint32_t DTRE22:1;     /* Channel 22 Data Transfer Request Enable */
                vuint32_t DTRE21:1;     /* Channel 21 Data Transfer Request Enable */
                vuint32_t DTRE20:1;     /* Channel 20 Data Transfer Request Enable */
                vuint32_t DTRE19:1;     /* Channel 19 Data Transfer Request Enable */
                vuint32_t DTRE18:1;     /* Channel 18 Data Transfer Request Enable */
                vuint32_t DTRE17:1;     /* Channel 17 Data Transfer Request Enable */
                vuint32_t DTRE16:1;     /* Channel 16 Data Transfer Request Enable */
                vuint32_t DTRE15:1;     /* Channel 15 Data Transfer Request Enable */
                vuint32_t DTRE14:1;     /* Channel 14 Data Transfer Request Enable */
                vuint32_t DTRE13:1;     /* Channel 13 Data Transfer Request Enable */
                vuint32_t DTRE12:1;     /* Channel 12 Data Transfer Request Enable */
                vuint32_t DTRE11:1;     /* Channel 11 Data Transfer Request Enable */
                vuint32_t DTRE10:1;     /* Channel 10 Data Transfer Request Enable */
                vuint32_t DTRE9:1;      /* Channel 9 Data Transfer Request Enable */
                vuint32_t DTRE8:1;      /* Channel 8 Data Transfer Request Enable */
                vuint32_t DTRE7:1;      /* Channel 7 Data Transfer Request Enable */
                vuint32_t DTRE6:1;      /* Channel 6 Data Transfer Request Enable */
                vuint32_t DTRE5:1;      /* Channel 5 Data Transfer Request Enable */
                vuint32_t DTRE4:1;      /* Channel 4 Data Transfer Request Enable */
                vuint32_t DTRE3:1;      /* Channel 3 Data Transfer Request Enable */
                vuint32_t DTRE2:1;      /* Channel 2 Data Transfer Request Enable */
                vuint32_t DTRE1:1;      /* Channel 1 Data Transfer Request Enable */
                vuint32_t DTRE0:1;      /* Channel 0 Data Transfer Request Enable */
            } B;
        } CDTRER_A;
        uint32_t etpu_reserved19;       /* For single ETPU implementations */

        uint32_t etpu_reserved20[10];
        union {                 /* ETPU_A Channel Pending Service Status */
            vuint32_t R;
            struct {
                vuint32_t SR31:1;       /* Channel 31 Pending Service Status */
                vuint32_t SR30:1;       /* Channel 30 Pending Service Status */
                vuint32_t SR29:1;       /* Channel 29 Pending Service Status */
                vuint32_t SR28:1;       /* Channel 28 Pending Service Status */
                vuint32_t SR27:1;       /* Channel 27 Pending Service Status */
                vuint32_t SR26:1;       /* Channel 26 Pending Service Status */
                vuint32_t SR25:1;       /* Channel 25 Pending Service Status */
                vuint32_t SR24:1;       /* Channel 24 Pending Service Status */
                vuint32_t SR23:1;       /* Channel 23 Pending Service Status */
                vuint32_t SR22:1;       /* Channel 22 Pending Service Status */
                vuint32_t SR21:1;       /* Channel 21 Pending Service Status */
                vuint32_t SR20:1;       /* Channel 20 Pending Service Status */
                vuint32_t SR19:1;       /* Channel 19 Pending Service Status */
                vuint32_t SR18:1;       /* Channel 18 Pending Service Status */
                vuint32_t SR17:1;       /* Channel 17 Pending Service Status */
                vuint32_t SR16:1;       /* Channel 16 Pending Service Status */
                vuint32_t SR15:1;       /* Channel 15 Pending Service Status */
                vuint32_t SR14:1;       /* Channel 14 Pending Service Status */
                vuint32_t SR13:1;       /* Channel 13 Pending Service Status */
                vuint32_t SR12:1;       /* Channel 12 Pending Service Status */
                vuint32_t SR11:1;       /* Channel 11 Pending Service Status */
                vuint32_t SR10:1;       /* Channel 10 Pending Service Status */
                vuint32_t SR9:1;        /* Channel 9 Pending Service Status */
                vuint32_t SR8:1;        /* Channel 8 Pending Service Status */
                vuint32_t SR7:1;        /* Channel 7 Pending Service Status */
                vuint32_t SR6:1;        /* Channel 6 Pending Service Status */
                vuint32_t SR5:1;        /* Channel 5 Pending Service Status */
                vuint32_t SR4:1;        /* Channel 4 Pending Service Status */
                vuint32_t SR3:1;        /* Channel 3 Pending Service Status */
                vuint32_t SR2:1;        /* Channel 2 Pending Service Status */
                vuint32_t SR1:1;        /* Channel 1 Pending Service Status */
                vuint32_t SR0:1;        /* Channel 0 Pending Service Status */
            } B;
        } CPSSR_A;
        uint32_t etpu_reserved22;       /* For single ETPU implementations */

        uint32_t etpu_reserved20a[2];

        union {                 /* ETPU_A Channel Service Status */
            vuint32_t R;
            struct {
                vuint32_t SS31:1;       /* Channel 31 Service Status */
                vuint32_t SS30:1;       /* Channel 30 Service Status */
                vuint32_t SS29:1;       /* Channel 29 Service Status */
                vuint32_t SS28:1;       /* Channel 28 Service Status */
                vuint32_t SS27:1;       /* Channel 27 Service Status */
                vuint32_t SS26:1;       /* Channel 26 Service Status */
                vuint32_t SS25:1;       /* Channel 25 Service Status */
                vuint32_t SS24:1;       /* Channel 24 Service Status */
                vuint32_t SS23:1;       /* Channel 23 Service Status */
                vuint32_t SS22:1;       /* Channel 22 Service Status */
                vuint32_t SS21:1;       /* Channel 21 Service Status */
                vuint32_t SS20:1;       /* Channel 20 Service Status */
                vuint32_t SS19:1;       /* Channel 19 Service Status */
                vuint32_t SS18:1;       /* Channel 18 Service Status */
                vuint32_t SS17:1;       /* Channel 17 Service Status */
                vuint32_t SS16:1;       /* Channel 16 Service Status */
                vuint32_t SS15:1;       /* Channel 15 Service Status */
                vuint32_t SS14:1;       /* Channel 14 Service Status */
                vuint32_t SS13:1;       /* Channel 13 Service Status */
                vuint32_t SS12:1;       /* Channel 12 Service Status */
                vuint32_t SS11:1;       /* Channel 11 Service Status */
                vuint32_t SS10:1;       /* Channel 10 Service Status */
                vuint32_t SS9:1;        /* Channel 9 Service Status */
                vuint32_t SS8:1;        /* Channel 8 Service Status */
                vuint32_t SS7:1;        /* Channel 7 Service Status */
                vuint32_t SS6:1;        /* Channel 6 Service Status */
                vuint32_t SS5:1;        /* Channel 5 Service Status */
                vuint32_t SS4:1;        /* Channel 4 Service Status */
                vuint32_t SS3:1;        /* Channel 3 Service Status */
                vuint32_t SS2:1;        /* Channel 2 Service Status */
                vuint32_t SS1:1;        /* Channel 1 Service Status */
                vuint32_t SS0:1;        /* Channel 0 Service Status */
            } B;
        } CSSR_A;
        uint32_t etpu_reserved22a;      /* For single ETPU implementations */

        uint32_t etpu_reserved23[90];

/*****************************Channels********************************/

        struct {
            union {
                vuint32_t R;    /* Channel Configuration Register */
                struct {
                    vuint32_t CIE:1;    /* Channel Interruput Enable */
                    vuint32_t DTRE:1;   /* Data Transfer Request Enable */
                    vuint32_t CPR:2;    /* Channel Priority */
                      vuint32_t:3;
                    vuint32_t ETCS:1;   /* Entry Table Condition Select */
                      vuint32_t:3;
                    vuint32_t CFS:5;    /* Channel Function Select */
                    vuint32_t ODIS:1;   /* Output disable */
                    vuint32_t OPOL:1;   /* output polarity */
                      vuint32_t:3;
                    vuint32_t CPBA:11;  /* Channel Parameter Base Address */
                } B;
            } CR;
            union {
                vuint32_t R;    /* Channel Status Control Register */
                struct {
                    vuint32_t CIS:1;    /* Channel Interruput Status */
                    vuint32_t CIOS:1;   /* Channel Interruput Overflow Status */
                      vuint32_t:6;
                    vuint32_t DTRS:1;   /* Data Transfer Status */
                    vuint32_t DTROS:1;  /* Data Transfer Overflow Status */
                      vuint32_t:6;
                    vuint32_t IPS:1;    /* Input Pin State */
                    vuint32_t OPS:1;    /* Output Pin State */
                    vuint32_t OBE:1;    /* Output Buffer Enable */
                      vuint32_t:11;
                    vuint32_t FM1:1;    /* Function mode */
                    vuint32_t FM0:1;    /* Function mode */
                } B;
            } SCR;
            union {
                vuint32_t R;    /* Channel Host Service Request Register */
                struct {
                    vuint32_t:29;       /* Host Service Request */
                    vuint32_t HSR:3;
                } B;
            } HSRR;
            uint32_t etpu_reserved23;
        } CHAN[127];

    };
/****************************************************************************/
/*                          MODULE : XBAR CrossBar                          */
/****************************************************************************/
    CC_EXTENSION struct XBAR_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:4;

                vuint32_t:1;
                vuint32_t MSTR6:3;      /* FLEXRAY */

                  vuint32_t:4;

                  vuint32_t:4;

                  vuint32_t:1;
                vuint32_t MSTR3:3;      /* FEC */

                  vuint32_t:1;
                vuint32_t MSTR2:3;
                  vuint32_t:1;
                vuint32_t MSTR1:3;
                  vuint32_t:1;
                vuint32_t MSTR0:3;
            } B;
        } MPR0;                 /* Master Priority Register for Slave Port 0 */

        uint32_t xbar_reserved1[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t RO:1;
                  vuint32_t:21;
                vuint32_t ARB:2;
                  vuint32_t:2;
                vuint32_t PCTL:2;
                  vuint32_t:1;
                vuint32_t PARK:3;
            } B;
        } SGPCR0;               /* General Purpose Control Register for Slave Port 0 */

        uint32_t xbar_reserved2[59];

        union {
            vuint32_t R;
            struct {
                vuint32_t:4;

                vuint32_t:1;
                vuint32_t MSTR6:3;      /* FLEXRAY */

                  vuint32_t:4;

                  vuint32_t:4;

                  vuint32_t:1;
                vuint32_t MSTR3:3;      /* FEC */

                  vuint32_t:1;
                vuint32_t MSTR2:3;
                  vuint32_t:1;
                vuint32_t MSTR1:3;
                  vuint32_t:1;
                vuint32_t MSTR0:3;
            } B;
        } MPR1;                 /* Master Priority Register for Slave Port 1 */

        uint32_t xbar_reserved3[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t RO:1;
                  vuint32_t:21;
                vuint32_t ARB:2;
                  vuint32_t:2;
                vuint32_t PCTL:2;
                  vuint32_t:1;
                vuint32_t PARK:3;
            } B;
        } SGPCR1;               /* General Purpose Control Register for Slave Port 1 */

        uint32_t xbar_reserved4[123];

        union {
            vuint32_t R;
            struct {
                vuint32_t:4;

                vuint32_t:1;
                vuint32_t MSTR6:3;      /* FLEXRAY */

                  vuint32_t:4;

                  vuint32_t:4;

                  vuint32_t:1;
                vuint32_t MSTR3:3;      /* FEC */

                  vuint32_t:1;
                vuint32_t MSTR2:3;
                  vuint32_t:1;
                vuint32_t MSTR1:3;
                  vuint32_t:1;
                vuint32_t MSTR0:3;
            } B;
        } MPR3;                 /* Master Priority Register for Slave Port 3 */

        uint32_t xbar_reserved5[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t RO:1;
                  vuint32_t:21;
                vuint32_t ARB:2;
                  vuint32_t:2;
                vuint32_t PCTL:2;
                  vuint32_t:1;
                vuint32_t PARK:3;
            } B;
        } SGPCR3;               /* General Purpose Control Register for Slave Port 3 */
        uint32_t xbar_reserved6[187];

        union {
            vuint32_t R;
            struct {
                vuint32_t:4;

                vuint32_t:1;
                vuint32_t MSTR6:3;      /* FLEXRAY */

                  vuint32_t:4;

                  vuint32_t:4;

                  vuint32_t:1;
                vuint32_t MSTR3:3;      /* FEC */

                  vuint32_t:1;
                vuint32_t MSTR2:3;
                  vuint32_t:1;
                vuint32_t MSTR1:3;
                  vuint32_t:1;
                vuint32_t MSTR0:3;
            } B;
        } MPR6;                 /* Master Priority Register for Slave Port 6 */

        uint32_t xbar_reserved7[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t RO:1;
                  vuint32_t:21;
                vuint32_t ARB:2;
                  vuint32_t:2;
                vuint32_t PCTL:2;
                  vuint32_t:1;
                vuint32_t PARK:3;
            } B;
        } SGPCR6;               /* General Purpose Control Register for Slave Port 6 */

        uint32_t xbar_reserved8[59];

        union {
            vuint32_t R;
            struct {
                vuint32_t:4;

                vuint32_t:1;
                vuint32_t MSTR6:3;      /* FLEXRAY */

                  vuint32_t:4;

                  vuint32_t:4;

                  vuint32_t:1;
                vuint32_t MSTR3:3;      /* FEC */

                  vuint32_t:1;
                vuint32_t MSTR2:3;
                  vuint32_t:1;
                vuint32_t MSTR1:3;
                  vuint32_t:1;
                vuint32_t MSTR0:3;
            } B;
        } MPR7;                 /* Master Priority Register for Slave Port 7 */

        uint32_t xbar_reserved9[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t RO:1;
                  vuint32_t:21;
                vuint32_t ARB:2;
                  vuint32_t:2;
                vuint32_t PCTL:2;
                  vuint32_t:1;
                vuint32_t PARK:3;
            } B;
        } SGPCR7;               /* General Purpose Control Register for Slave Port 7 */

    };
/****************************************************************************/
/*                     MODULE : ECSM                                        */
/****************************************************************************/
    CC_EXTENSION struct ECSM_tag {

        uint32_t ecsm_reserved1[5];

        uint16_t ecsm_reserved2;

        union {
            vuint16_t R;
        } SWTCR;                //Software Watchdog Timer Control

        uint8_t ecsm_reserved3[3];

        union {
            vuint8_t R;
        } SWTSR;                //SWT Service Register

        uint8_t ecsm_reserved4[3];

        union {
            vuint8_t R;
        } SWTIR;                //SWT Interrupt Register

        uint32_t ecsm_reserved5a[1];

        union {
            vuint32_t R;
            struct {
                vuint32_t FSBCR0:1;
                vuint32_t FSBCR1:1;
                vuint32_t FSBCR2:1;
                vuint32_t FSBCR3:1;
                vuint32_t FSBCR4:1;
                vuint32_t FSBCR5:1;
                vuint32_t FSBCR6:1;
                vuint32_t FSBCR7:1;
                vuint32_t RBEN:1;
                vuint32_t WBEN:1;
                vuint32_t ACCERR:1;
                  vuint32_t:21;
            } B;
        } FSBMCR;               /*  FEC System Bus Master Control Register */

        uint32_t ecsm_reserved5c[6];

        uint8_t ecsm_reserved6[3];

        union {
            vuint8_t R;
            struct {
                vuint8_t:6;
                vuint8_t ERNCR:1;
                vuint8_t EFNCR:1;
            } B;
        } ECR;                  //ECC Configuration Register

        uint8_t mcm_reserved8[3];

        union {
            vuint8_t R;
            struct {
                vuint8_t:6;
                vuint8_t RNCE:1;
                vuint8_t FNCE:1;
            } B;
        } ESR;                  //ECC Status Register

        uint16_t ecsm_reserved9;

        union {
            vuint16_t R;
            struct {
                vuint16_t:6;
                vuint16_t FRCNCI:1;
                vuint16_t FR1NCI:1;
                  vuint16_t:1;
                vuint16_t ERRBIT:7;
            } B;
        } EEGR;                 //ECC Error Generation Register

        uint32_t ecsm_reserved10;

        union {
            vuint32_t R;
            struct {
                vuint32_t FEAR:32;
            } B;
        } FEAR;                 //Flash ECC Address Register

        uint16_t ecsm_reserved11;

        union {
            vuint8_t R;
            struct {
                vuint8_t:4;
                vuint8_t FEMR:4;
            } B;
        } FEMR;                 //Flash ECC Master Register

        union {
            vuint8_t R;
            struct {
                vuint8_t WRITE:1;
                vuint8_t SIZE:3;
                vuint8_t PROT0:1;
                vuint8_t PROT1:1;
                vuint8_t PROT2:1;
                vuint8_t PROT3:1;
            } B;
        } FEAT;                 //Flash ECC Attributes Register

        union {
            vuint32_t R;
            struct {
                vuint32_t FEDH:32;
            } B;
        } FEDRH;                //Flash ECC Data High Register

        union {
            vuint32_t R;
            struct {
                vuint32_t FEDL:32;
            } B;
        } FEDRL;                //Flash ECC Data Low Register

        union {
            vuint32_t R;
            struct {
                vuint32_t REAR:32;
            } B;
        } REAR;                 //RAM ECC Address

        uint8_t ecsm_reserved12[2];

        union {
            vuint8_t R;
            struct {
                vuint8_t:4;
                vuint8_t REMR:4;
            } B;
        } REMR;                 //RAM ECC Master

        union {
            vuint8_t R;
            struct {
                vuint8_t WRITE:1;
                vuint8_t SIZE:3;
                vuint8_t PROT0:1;
                vuint8_t PROT1:1;
                vuint8_t PROT2:1;
                vuint8_t PROT3:1;
            } B;
        } REAT;                 // RAM ECC Attributes Register

        union {
            vuint32_t R;
            struct {
                vuint32_t REDH:32;
            } B;
        } REDRH;                //RAM ECC Data High Register

        union {
            vuint32_t R;
            struct {
                vuint32_t REDL:32;
            } B;
        } REDRL;                //RAMECC Data Low Register

    };
/****************************************************************************/
/*                          MODULE : eDMA                                   */
/****************************************************************************/
#include "ip_edma.h"

/****************************************************************************/
/*                          MODULE : INTC                                   */
/****************************************************************************/
    CC_EXTENSION struct INTC_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:26;
                vuint32_t VTES:1;
                  vuint32_t:4;
                vuint32_t HVEN:1;
            } B;
        } MCR;                  /* Module Configuration Register */

        int32_t INTC_reserved00;

        union {
            vuint32_t R;
            struct {
                vuint32_t:28;
                vuint32_t PRI:4;
            } B;
        } CPR;                  /* Current Priority Register */

        uint32_t intc_reserved1;

        union {
            vuint32_t R;
            struct {
                vuint32_t VTBA:21;
                vuint32_t INTVEC:9;
                  vuint32_t:2;
            } B;
        } IACKR;                /* Interrupt Acknowledge Register */

        uint32_t intc_reserved2;

        union {
            vuint32_t R;
            struct {
                vuint32_t:32;
            } B;
        } EOIR;                 /* End of Interrupt Register */

        uint32_t intc_reserved3;

        union {
            vuint8_t R;
            struct {
                vuint8_t:6;
                vuint8_t SET:1;
                vuint8_t CLR:1;
            } B;
        } SSCIR[8];             /* Software Set/Clear Interruput Register */

        uint32_t intc_reserved4[6];

        union {
            vuint8_t R;
            struct {
                vuint8_t:4;
                vuint8_t PRI:4;
            } B;
        } PSR[358];             /* Software Set/Clear Interrupt Register */

    };
/****************************************************************************/
/*                          MODULE : EQADC                                  */
/****************************************************************************/
    CC_EXTENSION struct EQADC_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:27;
                vuint32_t ESSIE:2;
                  vuint32_t:1;
                vuint32_t DBG:2;
            } B;
        } MCR;                  /* Module Configuration Register */

        int32_t EQADC_reserved00;

        union {
            vuint32_t R;
            struct {
                vuint32_t:6;
                vuint32_t NMF:26;
            } B;
        } NMSFR;                /* Null Message Send Format Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:28;
                vuint32_t DFL:4;
            } B;
        } ETDFR;                /* External Trigger Digital Filter Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CFPUSH:32;
            } B;
        } CFPR[6];              /* CFIFO Push Registers */

        uint32_t eqadc_reserved1;

        uint32_t eqadc_reserved2;

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t RFPOP:16;
            } B;
        } RFPR[6];              /* Result FIFO Pop Registers */

        uint32_t eqadc_reserved3;

        uint32_t eqadc_reserved4;

        union {
            vuint16_t R;
            struct {
                vuint16_t:5;
                vuint16_t SSE:1;
                vuint16_t CFINV:1;
                  vuint16_t:1;
                vuint16_t MODE:4;
                  vuint16_t:4;
            } B;
        } CFCR[6];              /* CFIFO Control Registers */

        uint32_t eqadc_reserved5;

        union {
            vuint16_t R;
            struct {
                vuint16_t NCIE:1;
                vuint16_t TORIE:1;
                vuint16_t PIE:1;
                vuint16_t EOQIE:1;
                vuint16_t CFUIE:1;
                  vuint16_t:1;
                vuint16_t CFFE:1;
                vuint16_t CFFS:1;
                  vuint16_t:4;
                vuint16_t RFOIE:1;
                  vuint16_t:1;
                vuint16_t RFDE:1;
                vuint16_t RFDS:1;
            } B;
        } IDCR[6];              /* Interrupt and DMA Control Registers */

        uint32_t eqadc_reserved6;

        union {
            vuint32_t R;
            struct {
                vuint32_t NCF:1;
                vuint32_t TORF:1;
                vuint32_t PF:1;
                vuint32_t EOQF:1;
                vuint32_t CFUF:1;
                vuint32_t SSS:1;
                vuint32_t CFFF:1;
                  vuint32_t:5;
                vuint32_t RFOF:1;
                  vuint32_t:1;
                vuint32_t RFDF:1;
                  vuint32_t:1;
                vuint32_t CFCTR:4;
                vuint32_t TNXTPTR:4;
                vuint32_t RFCTR:4;
                vuint32_t POPNXTPTR:4;
            } B;
        } FISR[6];              /* FIFO and Interrupt Status Registers */

        uint32_t eqadc_reserved7;

        uint32_t eqadc_reserved8;

        union {
            vuint16_t R;
            struct {
                vuint16_t:5;
                vuint16_t TCCF:11;
            } B;
        } CFTCR[6];             /* CFIFO Transfer Counter Registers */

        uint32_t eqadc_reserved9;

        union {
            vuint32_t R;
            struct {
                vuint32_t CFS0:2;
                vuint32_t CFS1:2;
                vuint32_t CFS2:2;
                vuint32_t CFS3:2;
                vuint32_t CFS4:2;
                vuint32_t CFS5:2;
                  vuint32_t:5;
                vuint32_t LCFTCB0:4;
                vuint32_t TC_LCFTCB0:11;
            } B;
        } CFSSR0;               /* CFIFO Status Register 0 */

        union {
            vuint32_t R;
            struct {
                vuint32_t CFS0:2;
                vuint32_t CFS1:2;
                vuint32_t CFS2:2;
                vuint32_t CFS3:2;
                vuint32_t CFS4:2;
                vuint32_t CFS5:2;
                  vuint32_t:5;
                vuint32_t LCFTCB1:4;
                vuint32_t TC_LCFTCB1:11;
            } B;
        } CFSSR1;               /* CFIFO Status Register 1 */

        union {
            vuint32_t R;
            struct {
                vuint32_t CFS0:2;
                vuint32_t CFS1:2;
                vuint32_t CFS2:2;
                vuint32_t CFS3:2;
                vuint32_t CFS4:2;
                vuint32_t CFS5:2;
                  vuint32_t:4;
                vuint32_t ECBNI:1;
                vuint32_t LCFTSSI:4;
                vuint32_t TC_LCFTSSI:11;
            } B;
        } CFSSR2;               /* CFIFO Status Register 2 */

        union {
            vuint32_t R;
            struct {
                vuint32_t CFS0:2;
                vuint32_t CFS1:2;
                vuint32_t CFS2:2;
                vuint32_t CFS3:2;
                vuint32_t CFS4:2;
                vuint32_t CFS5:2;
                  vuint32_t:20;
            } B;
        } CFSR;

        uint32_t eqadc_reserved11;

        union {
            vuint32_t R;
            struct {
                vuint32_t:21;
                vuint32_t MDT:3;
                  vuint32_t:4;
                vuint32_t BR:4;
            } B;
        } SSICR;                /* SSI Control Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t RDV:1;
                  vuint32_t:5;
                vuint32_t RDATA:26;
            } B;
        } SSIRDR;               /* SSI Recieve Data Register */

        uint32_t eqadc_reserved12[17];

        struct {
            union {
                vuint32_t R;
                struct {
                    vuint32_t:32;
                } B;
            } R[4];

            uint32_t eqadc_reserved13[12];

        } CF[6];

        uint32_t eqadc_reserved14[32];

        struct {
            union {
                vuint32_t R;
                struct {
                    vuint32_t:32;
                } B;
            } R[4];

            uint32_t eqadc_reserved15[12];

        } RF[6];

    };
/****************************************************************************/
/*                          MODULE : DSPI                                   */
/****************************************************************************/
    struct DSPI_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t MSTR:1;
                vuint32_t CONT_SCKE:1;
                vuint32_t DCONF:2;
                vuint32_t FRZ:1;
                vuint32_t MTFE:1;
                vuint32_t PCSSE:1;
                vuint32_t ROOE:1;
                  vuint32_t:2;
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
                  vuint32_t:7;
                vuint32_t HALT:1;
            } B;
        } MCR;                  /* Module Configuration Register */

        uint32_t dspi_reserved1;

        union {
            vuint32_t R;
            struct {
                vuint32_t TCNT:16;
                  vuint32_t:16;
            } B;
        } TCR;

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
        } CTAR[8];              /* Clock and Transfer Attributes Registers */

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
        } SR;                   /* Status Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t TCFRE:1;
                  vuint32_t:2;
                vuint32_t EOQFRE:1;
                vuint32_t TFUFRE:1;
                  vuint32_t:1;
                vuint32_t TFFFRE:1;
                vuint32_t TFFFDIRS:1;
                  vuint32_t:4;
                vuint32_t RFOFRE:1;
                  vuint32_t:1;
                vuint32_t RFDFRE:1;
                vuint32_t RFDFDIRS:1;
                  vuint32_t:16;
            } B;
        } RSER;                 /* DMA/Interrupt Request Select and Enable Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t CONT:1;
                vuint32_t CTAS:3;
                vuint32_t EOQ:1;
                vuint32_t CTCNT:1;
                  vuint32_t:4;
                vuint32_t PCS5:1;
                vuint32_t PCS4:1;
                vuint32_t PCS3:1;
                vuint32_t PCS2:1;
                vuint32_t PCS1:1;
                vuint32_t PCS0:1;
                vuint32_t TXDATA:16;
            } B;
        } PUSHR;                /* PUSH TX FIFO Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t RXDATA:16;
            } B;
        } POPR;                 /* POP RX FIFO Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t TXCMD:16;
                vuint32_t TXDATA:16;
            } B;
        } TXFR[4];              /* Transmit FIFO Registers */

        vuint32_t DSPI_reserved_txf[12];

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t RXDATA:16;
            } B;
        } RXFR[4];              /* Transmit FIFO Registers */

        vuint32_t DSPI_reserved_rxf[12];

        union {
            vuint32_t R;
            struct {
                vuint32_t MTOE:1;
                  vuint32_t:1;
                vuint32_t MTOCNT:6;
                  vuint32_t:4;
                vuint32_t TXSS:1;
                vuint32_t TPOL:1;
                vuint32_t TRRE:1;
                vuint32_t CID:1;
                vuint32_t DCONT:1;
                vuint32_t DSICTAS:3;
                  vuint32_t:6;
                vuint32_t DPCS5:1;
                vuint32_t DPCS4:1;
                vuint32_t DPCS3:1;
                vuint32_t DPCS2:1;
                vuint32_t DPCS1:1;
                vuint32_t DPCS0:1;
            } B;
        } DSICR;                /* DSI Configuration Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t SER_DATA:16;
            } B;
        } SDR;                  /* DSI Serialization Data Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t ASER_DATA:16;
            } B;
        } ASDR;                 /* DSI Alternate Serialization Data Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t COMP_DATA:16;
            } B;
        } COMPR;                /* DSI Transmit Comparison Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t DESER_DATA:16;
            } B;
        } DDR;                  /* DSI deserialization Data Register */

    };
/****************************************************************************/
/*                          MODULE : eSCI                                   */
/****************************************************************************/
    CC_EXTENSION struct ESCI_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:3;
                vuint32_t SBR:13;
                vuint32_t LOOPS:1;
                vuint32_t SCISDOZ:1;
                vuint32_t RSRC:1;
                vuint32_t M:1;
                vuint32_t WAKE:1;
                vuint32_t ILT:1;
                vuint32_t PE:1;
                vuint32_t PT:1;
                vuint32_t TIE:1;
                vuint32_t TCIE:1;
                vuint32_t RIE:1;
                vuint32_t ILIE:1;
                vuint32_t TE:1;
                vuint32_t RE:1;
                vuint32_t RWU:1;
                vuint32_t SBK:1;
            } B;
        } CR1;                  /* Control Register 1 */

        union {
            vuint16_t R;
            struct {
                vuint16_t MDIS:1;
                vuint16_t FBR:1;
                vuint16_t BSTP:1;
                vuint16_t IEBERR:1;
                vuint16_t RXDMA:1;
                vuint16_t TXDMA:1;
                vuint16_t BRK13:1;
                  vuint16_t:1;
                vuint16_t BESM13:1;
                vuint16_t SBSTP:1;
                  vuint16_t:2;
                vuint16_t ORIE:1;
                vuint16_t NFIE:1;
                vuint16_t FEIE:1;
                vuint16_t PFIE:1;
            } B;
        } CR2;                  /* Control Register 2 */

        union {
            vuint16_t R;
            struct {
                vuint16_t R8:1;
                vuint16_t T8:1;
                  vuint16_t:6;
                vuint8_t D;
            } B;
        } DR;                   /* Data Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t TDRE:1;
                vuint32_t TC:1;
                vuint32_t RDRF:1;
                vuint32_t IDLE:1;
                vuint32_t OR:1;
                vuint32_t NF:1;
                vuint32_t FE:1;
                vuint32_t PF:1;
                  vuint32_t:3;
                vuint32_t BERR:1;
                  vuint32_t:3;
                vuint32_t RAF:1;
                vuint32_t RXRDY:1;
                vuint32_t TXRDY:1;
                vuint32_t LWAKE:1;
                vuint32_t STO:1;
                vuint32_t PBERR:1;
                vuint32_t CERR:1;
                vuint32_t CKERR:1;
                vuint32_t FRC:1;
                  vuint32_t:7;
                vuint32_t OVFL:1;
            } B;
        } SR;                   /* Status Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t LRES:1;
                vuint32_t WU:1;
                vuint32_t WUD0:1;
                vuint32_t WUD1:1;
                vuint32_t LDBG:1;
                vuint32_t DSF:1;
                vuint32_t PRTY:1;
                vuint32_t LIN:1;
                vuint32_t RXIE:1;
                vuint32_t TXIE:1;
                vuint32_t WUIE:1;
                vuint32_t STIE:1;
                vuint32_t PBIE:1;
                vuint32_t CIE:1;
                vuint32_t CKIE:1;
                vuint32_t FCIE:1;
                  vuint32_t:7;
                vuint32_t OFIE:1;
                  vuint32_t:8;
            } B;
        } LCR;                  /* LIN Control Register */

        union {
            vuint32_t R;
        } LTR;                  /* LIN Transmit Register */

        union {
            vuint32_t R;
        } LRR;                  /* LIN Recieve Register */

        union {
            vuint32_t R;
        } LPR;                  /* LIN CRC Polynom Register  */

    };
/****************************************************************************/
/*                          MODULE : FlexCAN                                */
/****************************************************************************/
#include "ip_flexcan.h"
/****************************************************************************/
/*                          MODULE : FEC                                    */
/****************************************************************************/
    CC_EXTENSION struct FEC_tag {

        uint32_t fec_reserved_start[0x1];

        union {
            vuint32_t R;
            struct {
                vuint32_t HBERR:1;
                vuint32_t BABR:1;
                vuint32_t BABT:1;
                vuint32_t GRA:1;
                vuint32_t TXF:1;
                vuint32_t TXB:1;
                vuint32_t RXF:1;
                vuint32_t RXB:1;
                vuint32_t MII:1;
                vuint32_t EBERR:1;
                vuint32_t LC:1;
                vuint32_t RL:1;
                vuint32_t UN:1;
                  vuint32_t:19;
            } B;
        } EIR;                  /*  Interrupt Event Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t HBERRM:1;
                vuint32_t BABRM:1;
                vuint32_t BABTM:1;
                vuint32_t GRAM:1;
                vuint32_t TXFM:1;
                vuint32_t TXBM:1;
                vuint32_t RXFM:1;
                vuint32_t RXBM:1;
                vuint32_t MIIM:1;
                vuint32_t EBERRM:1;
                vuint32_t LCM:1;
                vuint32_t RLM:1;
                vuint32_t UNM:1;
                  vuint32_t:19;
            } B;
        } EIMR;                 /*  Interrupt Mask Register  */

        uint32_t fec_reserved_eimr;

        union {
            vuint32_t R;
            struct {
                vuint32_t:7;
                vuint32_t R_DES_ACTIVE:1;
                  vuint32_t:24;
            } B;
        } RDAR;                 /*  Receive Descriptor Active Register  */

        union {
            vuint32_t R;
            struct {
                vuint32_t:7;
                vuint32_t X_DES_ACTIVE:1;
                  vuint32_t:24;
            } B;
        } TDAR;                 /*  Transmit Descriptor Active Register  */

        uint32_t fec_reserved_tdar[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t:30;
                vuint32_t ETHER_EN:1;
                vuint32_t RESET:1;
            } B;
        } ECR;                  /*  Ethernet Control Register  */

        uint32_t fec_reserved_ecr[6];

        union {
            vuint32_t R;
            struct {
                vuint32_t ST:2;
                vuint32_t CP:2;
                vuint32_t PA:5;
                vuint32_t RA:5;
                vuint32_t TA:2;
                vuint32_t DATA:16;
            } B;
        } MDATA;                /* MII Data Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:24;
                vuint32_t DIS_PREAMBLE:1;
                vuint32_t MII_SPEED:6;
                  vuint32_t:1;
            } B;
        } MSCR;                 /* MII Speed Control Register */

        uint32_t fec_reserved_mscr[7];

        union {
            vuint32_t R;
            struct {
                vuint32_t MIB_DISABLE:1;
                vuint32_t MIB_IDLE:1;
                  vuint32_t:30;
            } B;
        } MIBC;                 /* MIB Control Register */

        uint32_t fec_reserved_mibc[7];

        union {
            vuint32_t R;
            struct {
                vuint32_t:5;
                vuint32_t MAX_FL:11;
                  vuint32_t:10;
                vuint32_t FCE:1;
                vuint32_t BC_REJ:1;
                vuint32_t PROM:1;
                vuint32_t MII_MODE:1;
                vuint32_t DRT:1;
                vuint32_t LOOP:1;
            } B;
        } RCR;                  /* Receive Control Register */

        uint32_t fec_reserved_rcr[15];

        union {
            vuint32_t R;
            struct {
                vuint32_t:27;
                vuint32_t RFC_PAUSE:1;
                vuint32_t TFC_PAUSE:1;
                vuint32_t FDEN:1;
                vuint32_t HBC:1;
                vuint32_t GTS:1;
            } B;
        } TCR;                  /* Transmit Control Register */

        uint32_t fec_reserved_tcr[7];

        union {
            vuint32_t R;
            struct {
                vuint32_t PADDR1:32;
            } B;
        } PALR;                 /* Physical Address Low Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t PADDR2:16;
                vuint32_t TYPE:16;
            } B;
        } PAUR;                 /* Physical Address High + Type Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t OPCODE:16;
                vuint32_t PAUSE_DUR:16;
            } B;
        } OPD;                  /* Opcode/Pause Duration Register */

        uint32_t fec_reserved_opd[10];

        union {
            vuint32_t R;
            struct {
                vuint32_t IADDR1:32;
            } B;
        } IAUR;                 /* Descriptor Individual Upper Address Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t IADDR2:32;
            } B;
        } IALR;                 /* Descriptor Individual Lower Address Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t GADDR1:32;
            } B;
        } GAUR;                 /* Descriptor Group Upper Address Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t GADDR2:32;
            } B;
        } GALR;                 /* Descriptor Group Lower Address Register */

        uint32_t fec_reserved_galr[7];

        union {
            vuint32_t R;
            struct {
                vuint32_t:30;
                vuint32_t X_WMRK:2;
            } B;
        } TFWR;                 /* FIFO Transmit FIFO Watermark Register */

        uint32_t fec_reserved_tfwr;

        union {
            vuint32_t R;
            struct {
                vuint32_t:22;
                vuint32_t R_BOUND:8;
                  vuint32_t:2;
            } B;
        } FRBR;                 /* FIFO Receive Bound Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:22;
                vuint32_t R_FSTART:8;
                  vuint32_t:2;
            } B;
        } FRSR;                 /* FIFO Receive Start Register */

        uint32_t fec_reserved_frsr[11];

        union {
            vuint32_t R;
            struct {
                vuint32_t R_DES_START:30;
                  vuint32_t:2;
            } B;
        } ERDSR;                /* Receive Descriptor Ring Start Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t X_DES_START:30;
                  vuint32_t:2;
            } B;
        } ETDSR;                /* Transmit Descriptor Ring Start Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:21;
                vuint32_t R_BUF_SIZE:7;
                  vuint32_t:4;
            } B;
        } EMRBR;                /* Receive Buffer Size Register */

        uint32_t fec_reserved_emrbr[29];

        union {
            vuint32_t R;
        } RMON_T_DROP;          /* Count of frames not counted correctly */

        union {
            vuint32_t R;
        } RMON_T_PACKETS;       /* RMON Tx packet count */

        union {
            vuint32_t R;
        } RMON_T_BC_PKT;        /* RMON Tx Broadcast Packets */

        union {
            vuint32_t R;
        } RMON_T_MC_PKT;        /* RMON Tx Multicast Packets */

        union {
            vuint32_t R;
        } RMON_T_CRC_ALIGN;     /* RMON Tx Packets w CRC/Align error */

        union {
            vuint32_t R;
        } RMON_T_UNDERSIZE;     /* RMON Tx Packets < 64 bytes, good crc */

        union {
            vuint32_t R;
        } RMON_T_OVERSIZE;      /* RMON Tx Packets > MAX_FL bytes, good crc */

        union {
            vuint32_t R;
        } RMON_T_FRAG;          /* RMON Tx Packets < 64 bytes, bad crc */

        union {
            vuint32_t R;
        } RMON_T_JAB;           /* RMON Tx Packets > MAX_FL bytes, bad crc */

        union {
            vuint32_t R;
        } RMON_T_COL;           /* RMON Tx collision count */

        union {
            vuint32_t R;
        } RMON_T_P64;           /* RMON Tx 64 byte packets */

        union {
            vuint32_t R;
        } RMON_T_P65TO127;      /* RMON Tx 65 to 127 byte packets */

        union {
            vuint32_t R;
        } RMON_T_P128TO255;     /* RMON Tx 128 to 255 byte packets */

        union {
            vuint32_t R;
        } RMON_T_P256TO511;     /* RMON Tx 256 to 511 byte packets */

        union {
            vuint32_t R;
        } RMON_T_P512TO1023;    /* RMON Tx 512 to 1023 byte packets */

        union {
            vuint32_t R;
        } RMON_T_P1024TO2047;   /* RMON Tx 1024 to 2047 byte packets */

        union {
            vuint32_t R;
        } RMON_T_P_GTE2048;     /* RMON Tx packets w > 2048 bytes */

        union {
            vuint32_t R;
        } RMON_T_OCTETS;        /* RMON Tx Octets */

        union {
            vuint32_t R;
        } IEEE_T_DROP;          /* Count of frames not counted correctly */

        union {
            vuint32_t R;
        } IEEE_T_FRAME_OK;      /* Frames Transmitted OK */

        union {
            vuint32_t R;
        } IEEE_T_1COL;          /* Frames Transmitted with Single Collision */

        union {
            vuint32_t R;
        } IEEE_T_MCOL;          /* Frames Transmitted with Multiple Collisions */

        union {
            vuint32_t R;
        } IEEE_T_DEF;           /* Frames Transmitted after Deferral Delay */

        union {
            vuint32_t R;
        } IEEE_T_LCOL;          /* Frames Transmitted with Late Collision */

        union {
            vuint32_t R;
        } IEEE_T_EXCOL;         /* Frames Transmitted with Excessive Collisions */

        union {
            vuint32_t R;
        } IEEE_T_MACERR;        /* Frames Transmitted with Tx FIFO Underrun */

        union {
            vuint32_t R;
        } IEEE_T_CSERR;         /* Frames Transmitted with Carrier Sense Error */

        union {
            vuint32_t R;
        } IEEE_T_SQE;           /* Frames Transmitted with SQE Error */

        union {
            vuint32_t R;
        } IEEE_T_FDXFC;         /* Flow Control Pause frames transmitted */

        union {
            vuint32_t R;
        } IEEE_T_OCTETS_OK;     /* Octet count for Frames Transmitted w/o Error */

        uint32_t fec_reserved_rmon_t_octets_ok[2];

        union {
            vuint32_t R;
        } RMON_R_DROP;          /*  Count of frames not counted correctly  */

        union {
            vuint32_t R;
        } RMON_R_PACKETS;       /* RMON Rx packet count */

        union {
            vuint32_t R;
        } RMON_R_BC_PKT;        /* RMON Rx Broadcast Packets */

        union {
            vuint32_t R;
        } RMON_R_MC_PKT;        /* RMON Rx Multicast Packets */

        union {
            vuint32_t R;
        } RMON_R_CRC_ALIGN;     /* RMON Rx Packets w CRC/Align error */

        union {
            vuint32_t R;
        } RMON_R_UNDERSIZE;     /* RMON Rx Packets < 64 bytes, good crc */

        union {
            vuint32_t R;
        } RMON_R_OVERSIZE;      /* RMON Rx Packets > MAX_FL bytes, good crc */

        union {
            vuint32_t R;
        } RMON_R_FRAG;          /* RMON Rx Packets < 64 bytes, bad crc */

        union {
            vuint32_t R;
        } RMON_R_JAB;           /* RMON Rx Packets > MAX_FL bytes, bad crc */

        uint32_t fec_reserved_rmon_r_jab;

        union {
            vuint32_t R;
        } RMON_R_P64;           /* RMON Rx 64 byte packets */

        union {
            vuint32_t R;
        } RMON_R_P65TO127;      /* RMON Rx 65 to 127 byte packets */

        union {
            vuint32_t R;
        } RMON_R_P128TO255;     /* RMON Rx 128 to 255 byte packets */

        union {
            vuint32_t R;
        } RMON_R_P256TO511;     /* RMON Rx 256 to 511 byte packets */

        union {
            vuint32_t R;
        } RMON_R_P512TO1023;    /* RMON Rx 512 to 1023 byte packets */

        union {
            vuint32_t R;
        } RMON_R_P1024TO2047;   /* RMON Rx 1024 to 2047 byte packets */

        union {
            vuint32_t R;
        } RMON_R_P_GTE2048;     /* RMON Rx packets w > 2048 bytes */

        union {
            vuint32_t R;
        } RMON_R_OCTETS;        /* RMON Rx Octets */

        union {
            vuint32_t R;
        } IEEE_R_DROP;          /* Count of frames not counted correctly */

        union {
            vuint32_t R;
        } IEEE_R_FRAME_OK;      /* Frames Received OK */

        union {
            vuint32_t R;
        } IEEE_R_CRC;           /* Frames Received with CRC Error */

        union {
            vuint32_t R;
        } IEEE_R_ALIGN;         /* Frames Received with Alignment Error */

        union {
            vuint32_t R;
        } IEEE_R_MACERR;        /* Receive Fifo Overflow count */

        union {
            vuint32_t R;
        } IEEE_R_FDXFC;         /* Flow Control Pause frames received */

        union {
            vuint32_t R;
        } IEEE_R_OCTETS_OK;     /* Octet count for Frames Rcvd w/o Error */

    };
/****************************************************************************/
/*                          MODULE : FlexRay                                */
/****************************************************************************/

    CC_EXTENSION typedef union uMVR {
        vuint16_t R;
        struct {
            vuint16_t CHIVER:8; /* CHI Version Number */
            vuint16_t PEVER:8;  /* PE Version Number */
        } B;
    } MVR_t;

    CC_EXTENSION typedef union uMCR {
        vuint16_t R;
        struct {
            vuint16_t MEN:1;    /* module enable */
              vuint16_t:1;
            vuint16_t SCMD:1;   /* single channel mode */
            vuint16_t CHB:1;    /* channel B enable */
            vuint16_t CHA:1;    /* channel A enable */
            vuint16_t SFFE:1;   /* synchronization frame filter enable */
              vuint16_t:5;
            vuint16_t CLKSEL:1; /* protocol engine clock source select */
            vuint16_t PRESCALE:3;       /* protocol engine clock prescaler */
              vuint16_t:1;
        } B;
    } MCR_t;
    CC_EXTENSION typedef union uSTBSCR {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;    /* write mode */
            vuint16_t STBSSEL:7;        /* strobe signal select */
              vuint16_t:3;
            vuint16_t ENB:1;    /* strobe signal enable */
              vuint16_t:2;
            vuint16_t STBPSEL:2;        /* strobe port select */
        } B;
    } STBSCR_t;
    CC_EXTENSION typedef union uSTBPCR {
        vuint16_t R;
        struct {
            vuint16_t:12;
            vuint16_t STB3EN:1; /* strobe port enable */
            vuint16_t STB2EN:1; /* strobe port enable */
            vuint16_t STB1EN:1; /* strobe port enable */
            vuint16_t STB0EN:1; /* strobe port enable */
        } B;
    } STBPCR_t;

    CC_EXTENSION typedef union uMBDSR {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t MBSEG2DS:7;       /* message buffer segment 2 data size */
              vuint16_t:1;
            vuint16_t MBSEG1DS:7;       /* message buffer segment 1 data size */
        } B;
    } MBDSR_t;
    CC_EXTENSION typedef union uMBSSUTR {
        vuint16_t R;
        struct {

            vuint16_t:1;
            vuint16_t LAST_MB_SEG1:7;   /* last message buffer control register for message buffer segment 1 */
              vuint16_t:1;
            vuint16_t LAST_MB_UTIL:7;   /* last message buffer utilized */
        } B;
    } MBSSUTR_t;

    CC_EXTENSION typedef union uPOCR {
        vuint16_t R;
        vuint8_t byte[2];
        struct {
            vuint16_t WME:1;    /* write mode external correction command */
              vuint16_t:3;
            vuint16_t EOC_AP:2; /* external offset correction application */
            vuint16_t ERC_AP:2; /* external rate correction application */
            vuint16_t BSY:1;    /* command write busy / write mode command */
              vuint16_t:3;
            vuint16_t POCCMD:4; /* protocol command */
        } B;
    } POCR_t;
/* protocol commands */
    CC_EXTENSION typedef union uGIFER {
        vuint16_t R;
        struct {
            vuint16_t MIF:1;    /* module interrupt flag */
            vuint16_t PRIF:1;   /* protocol interrupt flag */
            vuint16_t CHIF:1;   /* CHI interrupt flag */
            vuint16_t WKUPIF:1; /* wakeup interrupt flag */
            vuint16_t FNEBIF:1; /* receive FIFO channel B not empty interrupt flag */
            vuint16_t FNEAIF:1; /* receive FIFO channel A not empty interrupt flag */
            vuint16_t RBIF:1;   /* receive message buffer interrupt flag */
            vuint16_t TBIF:1;   /* transmit buffer interrupt flag */
            vuint16_t MIE:1;    /* module interrupt enable */
            vuint16_t PRIE:1;   /* protocol interrupt enable */
            vuint16_t CHIE:1;   /* CHI interrupt enable */
            vuint16_t WKUPIE:1; /* wakeup interrupt enable */
            vuint16_t FNEBIE:1; /* receive FIFO channel B not empty interrupt enable */
            vuint16_t FNEAIE:1; /* receive FIFO channel A not empty interrupt enable */
            vuint16_t RBIE:1;   /* receive message buffer interrupt enable */
            vuint16_t TBIE:1;   /* transmit buffer interrupt enable */
        } B;
    } GIFER_t;
    CC_EXTENSION typedef union uPIFR0 {
        vuint16_t R;
        struct {
            vuint16_t FATLIF:1; /* fatal protocol error interrupt flag */
            vuint16_t INTLIF:1; /* internal protocol error interrupt flag */
            vuint16_t ILCFIF:1; /* illegal protocol configuration flag */
            vuint16_t CSAIF:1;  /* cold start abort interrupt flag */
            vuint16_t MRCIF:1;  /* missing rate correctio interrupt flag */
            vuint16_t MOCIF:1;  /* missing offset correctio interrupt flag */
            vuint16_t CCLIF:1;  /* clock correction limit reached interrupt flag */
            vuint16_t MXSIF:1;  /* max sync frames detected interrupt flag */
            vuint16_t MTXIF:1;  /* media access test symbol received flag */
            vuint16_t LTXBIF:1; /* pdLatestTx violation on channel B interrupt flag */
            vuint16_t LTXAIF:1; /* pdLatestTx violation on channel A interrupt flag */
            vuint16_t TBVBIF:1; /* Transmission across boundary on channel B Interrupt Flag */
            vuint16_t TBVAIF:1; /* Transmission across boundary on channel A Interrupt Flag */
            vuint16_t TI2IF:1;  /* timer 2 expired interrupt flag */
            vuint16_t TI1IF:1;  /* timer 1 expired interrupt flag */
            vuint16_t CYSIF:1;  /* cycle start interrupt flag */
        } B;
    } PIFR0_t;
    CC_EXTENSION typedef union uPIFR1 {
        vuint16_t R;
        struct {
            vuint16_t EMCIF:1;  /* error mode changed interrupt flag */
            vuint16_t IPCIF:1;  /* illegal protocol command interrupt flag */
            vuint16_t PECFIF:1; /* protocol engine communication failure interrupt flag */
            vuint16_t PSCIF:1;  /* Protocol State Changed Interrupt Flag */
            vuint16_t SSI3IF:1; /* slot status counter incremented interrupt flag */
            vuint16_t SSI2IF:1; /* slot status counter incremented interrupt flag */
            vuint16_t SSI1IF:1; /* slot status counter incremented interrupt flag */
            vuint16_t SSI0IF:1; /* slot status counter incremented interrupt flag */
              vuint16_t:2;
            vuint16_t EVTIF:1;  /* even cycle table written interrupt flag */
            vuint16_t ODTIF:1;  /* odd cycle table written interrupt flag */
              vuint16_t:4;
        } B;
    } PIFR1_t;
    CC_EXTENSION typedef union uPIER0 {
        vuint16_t R;
        struct {
            vuint16_t FATLIE:1; /* fatal protocol error interrupt enable */
            vuint16_t INTLIE:1; /* internal protocol error interrupt interrupt enable  */
            vuint16_t ILCFIE:1; /* illegal protocol configuration interrupt enable */
            vuint16_t CSAIE:1;  /* cold start abort interrupt enable */
            vuint16_t MRCIE:1;  /* missing rate correctio interrupt enable */
            vuint16_t MOCIE:1;  /* missing offset correctio interrupt enable */
            vuint16_t CCLIE:1;  /* clock correction limit reached interrupt enable */
            vuint16_t MXSIE:1;  /* max sync frames detected interrupt enable */
            vuint16_t MTXIE:1;  /* media access test symbol received interrupt enable */
            vuint16_t LTXBIE:1; /* pdLatestTx violation on channel B interrupt enable */
            vuint16_t LTXAIE:1; /* pdLatestTx violation on channel A interrupt enable */
            vuint16_t TBVBIE:1; /* Transmission across boundary on channel B Interrupt enable */
            vuint16_t TBVAIE:1; /* Transmission across boundary on channel A Interrupt enable */
            vuint16_t TI2IE:1;  /* timer 2 expired interrupt enable */
            vuint16_t TI1IE:1;  /* timer 1 expired interrupt enable */
            vuint16_t CYSIE:1;  /* cycle start interrupt enable */
        } B;
    } PIER0_t;
    CC_EXTENSION typedef union uPIER1 {
        vuint16_t R;
        struct {
            vuint16_t EMCIE:1;  /* error mode changed interrupt enable */
            vuint16_t IPCIE:1;  /* illegal protocol command interrupt enable */
            vuint16_t PECFIE:1; /* protocol engine communication failure interrupt enable */
            vuint16_t PSCIE:1;  /* Protocol State Changed Interrupt enable */
            vuint16_t SSI3IE:1; /* slot status counter incremented interrupt enable */
            vuint16_t SSI2IE:1; /* slot status counter incremented interrupt enable */
            vuint16_t SSI1IE:1; /* slot status counter incremented interrupt enable */
            vuint16_t SSI0IE:1; /* slot status counter incremented interrupt enable */
              vuint16_t:2;
            vuint16_t EVTIE:1;  /* even cycle table written interrupt enable */
            vuint16_t ODTIE:1;  /* odd cycle table written interrupt enable */
              vuint16_t:4;
        } B;
    } PIER1_t;
    CC_EXTENSION typedef union uCHIERFR {
        vuint16_t R;
        struct {
            vuint16_t FRLBEF:1; /* flame lost channel B error flag */
            vuint16_t FRLAEF:1; /* frame lost channel A error flag */
            vuint16_t PCMIEF:1; /* command ignored error flag */
            vuint16_t FOVBEF:1; /* receive FIFO overrun channel B error flag */
            vuint16_t FOVAEF:1; /* receive FIFO overrun channel A error flag */
            vuint16_t MSBEF:1;  /* message buffer search error flag */
            vuint16_t MBUEF:1;  /* message buffer utilization error flag */
            vuint16_t LCKEF:1;  /* lock error flag */
            vuint16_t DBLEF:1;  /* double transmit message buffer lock error flag */
            vuint16_t SBCFEF:1; /* system bus communication failure error flag */
            vuint16_t FIDEF:1;  /* frame ID error flag */
            vuint16_t DPLEF:1;  /* dynamic payload length error flag */
            vuint16_t SPLEF:1;  /* static payload length error flag */
            vuint16_t NMLEF:1;  /* network management length error flag */
            vuint16_t NMFEF:1;  /* network management frame error flag */
            vuint16_t ILSAEF:1; /* illegal access error flag */
        } B;
    } CHIERFR_t;
    CC_EXTENSION typedef union uMBIVEC {
        vuint16_t R;
        struct {

            vuint16_t:1;
            vuint16_t TBIVEC:7; /* transmit buffer interrupt vector */
              vuint16_t:1;
            vuint16_t RBIVEC:7; /* receive buffer interrupt vector */
        } B;
    } MBIVEC_t;

    CC_EXTENSION typedef union uPSR0 {
        vuint16_t R;
        struct {
            vuint16_t ERRMODE:2;        /* error mode */
            vuint16_t SLOTMODE:2;       /* slot mode */
              vuint16_t:1;
            vuint16_t PROTSTATE:3;      /* protocol state */
            vuint16_t SUBSTATE:4;       /* protocol sub state */
              vuint16_t:1;
            vuint16_t WAKEUPSTATUS:3;   /* wakeup status */
        } B;
    } PSR0_t;

/* protocol states */
/* protocol sub-states */
/* wakeup status */
    CC_EXTENSION typedef union uPSR1 {
        vuint16_t R;
        struct {
            vuint16_t CSAA:1;   /* cold start attempt abort flag */
            vuint16_t SCP:1;    /* cold start path */
              vuint16_t:1;
            vuint16_t REMCSAT:5;        /* remanining coldstart attempts */
            vuint16_t CPN:1;    /* cold start noise path */
            vuint16_t HHR:1;    /* host halt request pending */
            vuint16_t FRZ:1;    /* freeze occured */
            vuint16_t APTAC:5;  /* allow passive to active counter */
        } B;
    } PSR1_t;
    CC_EXTENSION typedef union uPSR2 {
        vuint16_t R;
        struct {
            vuint16_t NBVB:1;   /* NIT boundary violation on channel B */
            vuint16_t NSEB:1;   /* NIT syntax error on channel B */
            vuint16_t STCB:1;   /* symbol window transmit conflict on channel B */
            vuint16_t SBVB:1;   /* symbol window boundary violation on channel B */
            vuint16_t SSEB:1;   /* symbol window syntax error on channel B */
            vuint16_t MTB:1;    /* media access test symbol MTS received on channel B */
            vuint16_t NBVA:1;   /* NIT boundary violation on channel A */
            vuint16_t NSEA:1;   /* NIT syntax error on channel A */
            vuint16_t STCA:1;   /* symbol window transmit conflict on channel A */
            vuint16_t SBVA:1;   /* symbol window boundary violation on channel A */
            vuint16_t SSEA:1;   /* symbol window syntax error on channel A */
            vuint16_t MTA:1;    /* media access test symbol MTS received on channel A */
            vuint16_t CLKCORRFAILCNT:4; /* clock correction failed counter */
        } B;
    } PSR2_t;
    CC_EXTENSION typedef union uPSR3 {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t WUB:1;    /* wakeup symbol received on channel B */
            vuint16_t ABVB:1;   /* aggregated boundary violation on channel B */
            vuint16_t AACB:1;   /* aggregated additional communication on channel B */
            vuint16_t ACEB:1;   /* aggregated content error on channel B */
            vuint16_t ASEB:1;   /* aggregated syntax error on channel B */
            vuint16_t AVFB:1;   /* aggregated valid frame on channel B */
              vuint16_t:2;
            vuint16_t WUA:1;    /* wakeup symbol received on channel A */
            vuint16_t ABVA:1;   /* aggregated boundary violation on channel A */
            vuint16_t AACA:1;   /* aggregated additional communication on channel A */
            vuint16_t ACEA:1;   /* aggregated content error on channel A */
            vuint16_t ASEA:1;   /* aggregated syntax error on channel A */
            vuint16_t AVFA:1;   /* aggregated valid frame on channel A */
        } B;
    } PSR3_t;
    CC_EXTENSION typedef union uCIFRR {
        vuint16_t R;
        struct {
            vuint16_t:8;
            vuint16_t MIFR:1;   /* module interrupt flag */
            vuint16_t PRIFR:1;  /* protocol interrupt flag */
            vuint16_t CHIFR:1;  /* CHI interrupt flag */
            vuint16_t WUPIFR:1; /* wakeup interrupt flag */
            vuint16_t FNEBIFR:1;        /* receive fifo channel B no empty interrupt flag */
            vuint16_t FNEAIFR:1;        /* receive fifo channel A no empty interrupt flag */
            vuint16_t RBIFR:1;  /* receive message buffer interrupt flag */
            vuint16_t TBIFR:1;  /* transmit buffer interrupt flag */
        } B;
    } CIFRR_t;
    CC_EXTENSION typedef union uSFCNTR {
        vuint16_t R;
        struct {
            vuint16_t SFEVB:4;  /* sync frames channel B, even cycle */
            vuint16_t SFEVA:4;  /* sync frames channel A, even cycle */
            vuint16_t SFODB:4;  /* sync frames channel B, odd cycle */
            vuint16_t SFODA:4;  /* sync frames channel A, odd cycle */
        } B;
    } SFCNTR_t;

    CC_EXTENSION typedef union uSFTCCSR {
        vuint16_t R;
        struct {
            vuint16_t ELKT:1;   /* even cycle tables lock and unlock trigger */
            vuint16_t OLKT:1;   /* odd cycle tables lock and unlock trigger */
            vuint16_t CYCNUM:6; /* cycle number */
            vuint16_t ELKS:1;   /* even cycle tables lock status */
            vuint16_t OLKS:1;   /* odd cycle tables lock status */
            vuint16_t EVAL:1;   /* even cycle tables valid */
            vuint16_t OVAL:1;   /* odd cycle tables valid */
              vuint16_t:1;
            vuint16_t OPT:1;    /*one pair trigger */
            vuint16_t SDVEN:1;  /* sync frame deviation table enable */
            vuint16_t SIDEN:1;  /* sync frame ID table enable */
        } B;
    } SFTCCSR_t;
    CC_EXTENSION typedef union uSFIDRFR {
        vuint16_t R;
        struct {
            vuint16_t:6;
            vuint16_t SYNFRID:10;       /* sync frame rejection ID */
        } B;
    } SFIDRFR_t;

    CC_EXTENSION typedef union uTICCR {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t T2CFG:1;  /* timer 2 configuration */
            vuint16_t T2REP:1;  /* timer 2 repetitive mode */
              vuint16_t:1;
            vuint16_t T2SP:1;   /* timer 2 stop */
            vuint16_t T2TR:1;   /* timer 2 trigger */
            vuint16_t T2ST:1;   /* timer 2 state */
              vuint16_t:3;
            vuint16_t T1REP:1;  /* timer 1 repetitive mode */
              vuint16_t:1;
            vuint16_t T1SP:1;   /* timer 1 stop */
            vuint16_t T1TR:1;   /* timer 1 trigger */
            vuint16_t T1ST:1;   /* timer 1 state */

        } B;
    } TICCR_t;
    CC_EXTENSION typedef union uTI1CYSR {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t TI1CYCVAL:6;      /* timer 1 cycle filter value */
              vuint16_t:2;
            vuint16_t TI1CYCMSK:6;      /* timer 1 cycle filter mask */

        } B;
    } TI1CYSR_t;

    CC_EXTENSION typedef union uSSSR {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;    /* write mode */
              vuint16_t:1;
            vuint16_t SEL:2;    /* static slot number */
              vuint16_t:1;
            vuint16_t SLOTNUMBER:11;    /* selector */
        } B;
    } SSSR_t;

    CC_EXTENSION typedef union uSSCCR {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;    /* write mode */
              vuint16_t:1;
            vuint16_t SEL:2;    /* selector */
              vuint16_t:1;
            vuint16_t CNTCFG:2; /* counter configuration */
            vuint16_t MCY:1;    /* multi cycle selection */
            vuint16_t VFR:1;    /* valid frame selection */
            vuint16_t SYF:1;    /* sync frame selection */
            vuint16_t NUF:1;    /* null frame selection  */
            vuint16_t SUF:1;    /* startup frame selection */
            vuint16_t STATUSMASK:4;     /* slot status mask */
        } B;
    } SSCCR_t;
    CC_EXTENSION typedef union uSSR {
        vuint16_t R;
        struct {
            vuint16_t VFB:1;    /* valid frame on channel B */
            vuint16_t SYB:1;    /* valid sync frame on channel B */
            vuint16_t NFB:1;    /* valid null frame on channel B */
            vuint16_t SUB:1;    /* valid startup frame on channel B */
            vuint16_t SEB:1;    /* syntax error on channel B */
            vuint16_t CEB:1;    /* content error on channel B */
            vuint16_t BVB:1;    /* boundary violation on channel B */
            vuint16_t TCB:1;    /* tx conflict on channel B */
            vuint16_t VFA:1;    /* valid frame on channel A */
            vuint16_t SYA:1;    /* valid sync frame on channel A */
            vuint16_t NFA:1;    /* valid null frame on channel A */
            vuint16_t SUA:1;    /* valid startup frame on channel A */
            vuint16_t SEA:1;    /* syntax error on channel A */
            vuint16_t CEA:1;    /* content error on channel A */
            vuint16_t BVA:1;    /* boundary violation on channel A */
            vuint16_t TCA:1;    /* tx conflict on channel A */
        } B;
    } SSR_t;
    CC_EXTENSION typedef union uMTSCFR {
        vuint16_t R;
        struct {
            vuint16_t MTE:1;    /* media access test symbol transmission enable */
              vuint16_t:1;
            vuint16_t CYCCNTMSK:6;      /* cycle counter mask */
              vuint16_t:2;
            vuint16_t CYCCNTVAL:6;      /* cycle counter value */
        } B;
    } MTSCFR_t;
    CC_EXTENSION typedef union uRSBIR {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;    /* write mode */
              vuint16_t:1;
            vuint16_t SEL:2;    /* selector */
              vuint16_t:4;
            vuint16_t RSBIDX:8; /* receive shadow buffer index */
        } B;
    } RSBIR_t;
    CC_EXTENSION typedef union uRFDSR {
        vuint16_t R;
        struct {
            vuint16_t FIFODEPTH:8;      /* fifo depth */
              vuint16_t:1;
            vuint16_t ENTRYSIZE:7;      /* entry size */
        } B;
    } RFDSR_t;

    CC_EXTENSION typedef union uRFRFCFR {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;    /* write mode */
            vuint16_t IBD:1;    /* interval boundary */
            vuint16_t SEL:2;    /* filter number */
              vuint16_t:1;
            vuint16_t SID:11;   /* slot ID */
        } B;
    } RFRFCFR_t;

    CC_EXTENSION typedef union uRFRFCTR {
        vuint16_t R;
        struct {
            vuint16_t:4;
            vuint16_t F3MD:1;   /* filter mode */
            vuint16_t F2MD:1;   /* filter mode */
            vuint16_t F1MD:1;   /* filter mode */
            vuint16_t F0MD:1;   /* filter mode */
              vuint16_t:4;
            vuint16_t F3EN:1;   /* filter enable */
            vuint16_t F2EN:1;   /* filter enable */
            vuint16_t F1EN:1;   /* filter enable */
            vuint16_t F0EN:1;   /* filter enable */
        } B;
    } RFRFCTR_t;
    CC_EXTENSION typedef union uPCR0 {
        vuint16_t R;
        struct {
            vuint16_t ACTION_POINT_OFFSET:6;
            vuint16_t STATIC_SLOT_LENGTH:10;
        } B;
    } PCR0_t;

    CC_EXTENSION typedef union uPCR1 {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t MACRO_AFTER_FIRST_STATIC_SLOT:14;
        } B;
    } PCR1_t;

    CC_EXTENSION typedef union uPCR2 {
        vuint16_t R;
        struct {
            vuint16_t MINISLOT_AFTER_ACTION_POINT:6;
            vuint16_t NUMBER_OF_STATIC_SLOTS:10;
        } B;
    } PCR2_t;

    CC_EXTENSION typedef union uPCR3 {
        vuint16_t R;
        struct {
            vuint16_t WAKEUP_SYMBOL_RX_LOW:6;
            vuint16_t MINISLOT_ACTION_POINT_OFFSET:5;
            vuint16_t COLDSTART_ATTEMPTS:5;
        } B;
    } PCR3_t;

    CC_EXTENSION typedef union uPCR4 {
        vuint16_t R;
        struct {
            vuint16_t CAS_RX_LOW_MAX:7;
            vuint16_t WAKEUP_SYMBOL_RX_WINDOW:9;
        } B;
    } PCR4_t;

    CC_EXTENSION typedef union uPCR5 {
        vuint16_t R;
        struct {
            vuint16_t TSS_TRANSMITTER:4;
            vuint16_t WAKEUP_SYMBOL_TX_LOW:6;
            vuint16_t WAKEUP_SYMBOL_RX_IDLE:6;
        } B;
    } PCR5_t;

    CC_EXTENSION typedef union uPCR6 {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t SYMBOL_WINDOW_AFTER_ACTION_POINT:8;
            vuint16_t MACRO_INITIAL_OFFSET_A:7;
        } B;
    } PCR6_t;

    CC_EXTENSION typedef union uPCR7 {
        vuint16_t R;
        struct {
            vuint16_t DECODING_CORRECTION_B:9;
            vuint16_t MICRO_PER_MACRO_NOM_HALF:7;
        } B;
    } PCR7_t;

    CC_EXTENSION typedef union uPCR8 {
        vuint16_t R;
        struct {
            vuint16_t MAX_WITHOUT_CLOCK_CORRECTION_FATAL:4;
            vuint16_t MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE:4;
            vuint16_t WAKEUP_SYMBOL_TX_IDLE:8;
        } B;
    } PCR8_t;

    CC_EXTENSION typedef union uPCR9 {
        vuint16_t R;
        struct {
            vuint16_t MINISLOT_EXISTS:1;
            vuint16_t SYMBOL_WINDOW_EXISTS:1;
            vuint16_t OFFSET_CORRECTION_OUT:14;
        } B;
    } PCR9_t;

    CC_EXTENSION typedef union uPCR10 {
        vuint16_t R;
        struct {
            vuint16_t SINGLE_SLOT_ENABLED:1;
            vuint16_t WAKEUP_CHANNEL:1;
            vuint16_t MACRO_PER_CYCLE:14;
        } B;
    } PCR10_t;

    CC_EXTENSION typedef union uPCR11 {
        vuint16_t R;
        struct {
            vuint16_t KEY_SLOT_USED_FOR_STARTUP:1;
            vuint16_t KEY_SLOT_USED_FOR_SYNC:1;
            vuint16_t OFFSET_CORRECTION_START:14;
        } B;
    } PCR11_t;

    CC_EXTENSION typedef union uPCR12 {
        vuint16_t R;
        struct {
            vuint16_t ALLOW_PASSIVE_TO_ACTIVE:5;
            vuint16_t KEY_SLOT_HEADER_CRC:11;
        } B;
    } PCR12_t;

    CC_EXTENSION typedef union uPCR13 {
        vuint16_t R;
        struct {
            vuint16_t FIRST_MINISLOT_ACTION_POINT_OFFSET:6;
            vuint16_t STATIC_SLOT_AFTER_ACTION_POINT:10;
        } B;
    } PCR13_t;

    CC_EXTENSION typedef union uPCR14 {
        vuint16_t R;
        struct {
            vuint16_t RATE_CORRECTION_OUT:11;
            vuint16_t LISTEN_TIMEOUT_H:5;
        } B;
    } PCR14_t;

    CC_EXTENSION typedef union uPCR15 {
        vuint16_t R;
        struct {
            vuint16_t LISTEN_TIMEOUT_L:16;
        } B;
    } PCR15_t;

    CC_EXTENSION typedef union uPCR16 {
        vuint16_t R;
        struct {
            vuint16_t MACRO_INITIAL_OFFSET_B:7;
            vuint16_t NOISE_LISTEN_TIMEOUT_H:9;
        } B;
    } PCR16_t;

    CC_EXTENSION typedef union uPCR17 {
        vuint16_t R;
        struct {
            vuint16_t NOISE_LISTEN_TIMEOUT_L:16;
        } B;
    } PCR17_t;

    CC_EXTENSION typedef union uPCR18 {
        vuint16_t R;
        struct {
            vuint16_t WAKEUP_PATTERN:6;
            vuint16_t KEY_SLOT_ID:10;
        } B;
    } PCR18_t;

    CC_EXTENSION typedef union uPCR19 {
        vuint16_t R;
        struct {
            vuint16_t DECODING_CORRECTION_A:9;
            vuint16_t PAYLOAD_LENGTH_STATIC:7;
        } B;
    } PCR19_t;

    CC_EXTENSION typedef union uPCR20 {
        vuint16_t R;
        struct {
            vuint16_t MICRO_INITIAL_OFFSET_B:8;
            vuint16_t MICRO_INITIAL_OFFSET_A:8;
        } B;
    } PCR20_t;

    CC_EXTENSION typedef union uPCR21 {
        vuint16_t R;
        struct {
            vuint16_t EXTERN_RATE_CORRECTION:3;
            vuint16_t LATEST_TX:13;
        } B;
    } PCR21_t;

    CC_EXTENSION typedef union uPCR22 {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t COMP_ACCEPTED_STARTUP_RANGE_A:11;
            vuint16_t MICRO_PER_CYCLE_H:4;
        } B;
    } PCR22_t;

    CC_EXTENSION typedef union uPCR23 {
        vuint16_t R;
        struct {
            vuint16_t micro_per_cycle_l:16;
        } B;
    } PCR23_t;

    CC_EXTENSION typedef union uPCR24 {
        vuint16_t R;
        struct {
            vuint16_t CLUSTER_DRIFT_DAMPING:5;
            vuint16_t MAX_PAYLOAD_LENGTH_DYNAMIC:7;
            vuint16_t MICRO_PER_CYCLE_MIN_H:4;
        } B;
    } PCR24_t;

    CC_EXTENSION typedef union uPCR25 {
        vuint16_t R;
        struct {
            vuint16_t MICRO_PER_CYCLE_MIN_L:16;
        } B;
    } PCR25_t;

    CC_EXTENSION typedef union uPCR26 {
        vuint16_t R;
        struct {
            vuint16_t ALLOW_HALT_DUE_TO_CLOCK:1;
            vuint16_t COMP_ACCEPTED_STARTUP_RANGE_B:11;
            vuint16_t MICRO_PER_CYCLE_MAX_H:4;
        } B;
    } PCR26_t;

    CC_EXTENSION typedef union uPCR27 {
        vuint16_t R;
        struct {
            vuint16_t MICRO_PER_CYCLE_MAX_L:16;
        } B;
    } PCR27_t;

    CC_EXTENSION typedef union uPCR28 {
        vuint16_t R;
        struct {
            vuint16_t DYNAMIC_SLOT_IDLE_PHASE:2;
            vuint16_t MACRO_AFTER_OFFSET_CORRECTION:14;
        } B;
    } PCR28_t;

    CC_EXTENSION typedef union uPCR29 {
        vuint16_t R;
        struct {
            vuint16_t EXTERN_OFFSET_CORRECTION:3;
            vuint16_t MINISLOTS_MAX:13;
        } B;
    } PCR29_t;

    CC_EXTENSION typedef union uPCR30 {
        vuint16_t R;
        struct {
            vuint16_t:12;
            vuint16_t SYNC_NODE_MAX:4;
        } B;
    } PCR30_t;

    CC_EXTENSION typedef struct uMSG_BUFF_CCS {
        union {
            vuint16_t R;
            struct {
                vuint16_t:1;
                vuint16_t MCM:1;        /* message buffer commit mode */
                vuint16_t MBT:1;        /* message buffer type */
                vuint16_t MTD:1;        /* message buffer direction */
                vuint16_t CMT:1;        /* commit for transmission */
                vuint16_t EDT:1;        /* enable / disable trigger */
                vuint16_t LCKT:1;       /* lock request trigger */
                vuint16_t MBIE:1;       /* message buffer interrupt enable */
                  vuint16_t:3;
                vuint16_t DUP:1;        /* data updated  */
                vuint16_t DVAL:1;       /* data valid */
                vuint16_t EDS:1;        /* lock status */
                vuint16_t LCKS:1;       /* enable / disable status */
                vuint16_t MBIF:1;       /* message buffer interrupt flag */
            } B;
        } MBCCSR;
        union {
            vuint16_t R;
            struct {
                vuint16_t MTM:1;        /* message buffer transmission mode */
                vuint16_t CHNLA:1;      /* channel assignement */
                vuint16_t CHNLB:1;      /* channel assignement */
                vuint16_t CCFE:1;       /* cycle counter filter enable */
                vuint16_t CCFMSK:6;     /* cycle counter filter mask */
                vuint16_t CCFVAL:6;     /* cycle counter filter value */
            } B;
        } MBCCFR;
        union {
            vuint16_t R;
            struct {
                vuint16_t:5;
                vuint16_t FID:11;       /* frame ID */
            } B;
        } MBFIDR;
        union {
            vuint16_t R;
            struct {
                vuint16_t:8;
                vuint16_t MBIDX:8;      /* message buffer index */
            } B;
        } MBIDXR;
    } MSG_BUFF_CCS_t;
    typedef union uSYSBADHR {
        vuint16_t R;
    } SYSBADHR_t;
    typedef union uSYSBADLR {
        vuint16_t R;
    } SYSBADLR_t;
    typedef union uPDAR {
        vuint16_t R;
    } PDAR_t;
    typedef union uCASERCR {
        vuint16_t R;
    } CASERCR_t;
    typedef union uCBSERCR {
        vuint16_t R;
    } CBSERCR_t;
    typedef union uCYCTR {
        vuint16_t R;
    } CYCTR_t;
    typedef union uMTCTR {
        vuint16_t R;
    } MTCTR_t;
    typedef union uSLTCTAR {
        vuint16_t R;
    } SLTCTAR_t;
    typedef union uSLTCTBR {
        vuint16_t R;
    } SLTCTBR_t;
    typedef union uRTCORVR {
        vuint16_t R;
    } RTCORVR_t;
    typedef union uOFCORVR {
        vuint16_t R;
    } OFCORVR_t;
    typedef union uSFTOR {
        vuint16_t R;
    } SFTOR_t;
    typedef union uSFIDAFVR {
        vuint16_t R;
    } SFIDAFVR_t;
    typedef union uSFIDAFMR {
        vuint16_t R;
    } SFIDAFMR_t;
    typedef union uNMVR {
        vuint16_t R;
    } NMVR_t;
    typedef union uNMVLR {
        vuint16_t R;
    } NMVLR_t;
    typedef union uT1MTOR {
        vuint16_t R;
    } T1MTOR_t;
    typedef union uTI2CR0 {
        vuint16_t R;
    } TI2CR0_t;
    typedef union uTI2CR1 {
        vuint16_t R;
    } TI2CR1_t;
    typedef union uSSCR {
        vuint16_t R;
    } SSCR_t;
    typedef union uRFSR {
        vuint16_t R;
    } RFSR_t;
    typedef union uRFSIR {
        vuint16_t R;
    } RFSIR_t;
    typedef union uRFARIR {
        vuint16_t R;
    } RFARIR_t;
    typedef union uRFBRIR {
        vuint16_t R;
    } RFBRIR_t;
    typedef union uRFMIDAFVR {
        vuint16_t R;
    } RFMIDAFVR_t;
    typedef union uRFMIAFMR {
        vuint16_t R;
    } RFMIAFMR_t;
    typedef union uRFFIDRFVR {
        vuint16_t R;
    } RFFIDRFVR_t;
    typedef union uRFFIDRFMR {
        vuint16_t R;
    } RFFIDRFMR_t;
    typedef union uLDTXSLAR {
        vuint16_t R;
    } LDTXSLAR_t;
    typedef union uLDTXSLBR {
        vuint16_t R;
    } LDTXSLBR_t;

    typedef struct FR_tag {
        volatile MVR_t MVR;     /*module version register *//*0  */
        volatile MCR_t MCR;     /*module configuration register *//*2  */
        volatile SYSBADHR_t SYSBADHR;   /*system memory base address high register *//*4        */
        volatile SYSBADLR_t SYSBADLR;   /*system memory base address low register *//*6         */
        volatile STBSCR_t STBSCR;       /*strobe signal control register *//*8      */
        volatile STBPCR_t STBPCR;       /*strobe port control register *//*A        */
        volatile MBDSR_t MBDSR; /*message buffer data size register *//*C  */
        volatile MBSSUTR_t MBSSUTR;     /*message buffer segment size and utilization register *//*E  */
        vuint16_t reserved3a[1];        /*10 */
        volatile PDAR_t PDAR;   /*PE data register *//*12 */
        volatile POCR_t POCR;   /*Protocol operation control register *//*14 */
        volatile GIFER_t GIFER; /*global interrupt flag and enable register *//*16 */
        volatile PIFR0_t PIFR0; /*protocol interrupt flag register 0 *//*18 */
        volatile PIFR1_t PIFR1; /*protocol interrupt flag register 1 *//*1A */
        volatile PIER0_t PIER0; /*protocol interrupt enable register 0 *//*1C */
        volatile PIER1_t PIER1; /*protocol interrupt enable register 1 *//*1E */
        volatile CHIERFR_t CHIERFR;     /*CHI error flag register *//*20 */
        volatile MBIVEC_t MBIVEC;       /*message buffer interrupt vector register *//*22 */
        volatile CASERCR_t CASERCR;     /*channel A status error counter register *//*24 */
        volatile CBSERCR_t CBSERCR;     /*channel B status error counter register *//*26 */
        volatile PSR0_t PSR0;   /*protocol status register 0 *//*28 */
        volatile PSR1_t PSR1;   /*protocol status register 1 *//*2A */
        volatile PSR2_t PSR2;   /*protocol status register 2 *//*2C */
        volatile PSR3_t PSR3;   /*protocol status register 3 *//*2E */
        volatile MTCTR_t MTCTR; /*macrotick counter register *//*30 */
        volatile CYCTR_t CYCTR; /*cycle counter register *//*32 */
        volatile SLTCTAR_t SLTCTAR;     /*slot counter channel A register *//*34 */
        volatile SLTCTBR_t SLTCTBR;     /*slot counter channel B register *//*36 */
        volatile RTCORVR_t RTCORVR;     /*rate correction value register *//*38 */
        volatile OFCORVR_t OFCORVR;     /*offset correction value register *//*3A */
        volatile CIFRR_t CIFRR; /*combined interrupt flag register *//*3C */
        vuint16_t reserved3[1]; /*3E */
        volatile SFCNTR_t SFCNTR;       /*sync frame counter register *//*40 */
        volatile SFTOR_t SFTOR; /*sync frame table offset register *//*42 */
        volatile SFTCCSR_t SFTCCSR;     /*sync frame table configuration, control, status register *//*44 */
        volatile SFIDRFR_t SFIDRFR;     /*sync frame ID rejection filter register *//*46 */
        volatile SFIDAFVR_t SFIDAFVR;   /*sync frame ID acceptance filter value regiater *//*48 */
        volatile SFIDAFMR_t SFIDAFMR;   /*sync frame ID acceptance filter mask register *//*4A */
        volatile NMVR_t NMVR[6];        /*network management vector registers (12 bytes) *//*4C */
        volatile NMVLR_t NMVLR; /*network management vector length register *//*58 */
        volatile TICCR_t TICCR; /*timer configuration and control register *//*5A */
        volatile TI1CYSR_t TI1CYSR;     /*timer 1 cycle set register *//*5C */
        volatile T1MTOR_t T1MTOR;       /*timer 1 macrotick offset register *//*5E */
        volatile TI2CR0_t TI2CR0;       /*timer 2 configuration register 0 *//*60 */
        volatile TI2CR1_t TI2CR1;       /*timer 2 configuration register 1 *//*62 */
        volatile SSSR_t SSSR;   /*slot status selection register *//*64 */
        volatile SSCCR_t SSCCR; /*slot status counter condition register *//*66 */
        volatile SSR_t SSR[8];  /*slot status registers 0-7 *//*68 */
        volatile SSCR_t SSCR[4];        /*slot status counter registers 0-3 *//*78 */
        volatile MTSCFR_t MTSACFR;      /*mts a config register *//*80 */
        volatile MTSCFR_t MTSBCFR;      /*mts b config register *//*82 */
        volatile RSBIR_t RSBIR; /*receive shadow buffer index register *//*84 */
        volatile RFSR_t RFSR;   /*receive fifo selection register *//*86 */
        volatile RFSIR_t RFSIR; /*receive fifo start index register *//*88 */
        volatile RFDSR_t RFDSR; /*receive fifo depth and size register *//*8A */
        volatile RFARIR_t RFARIR;       /*receive fifo a read index register *//*8C */
        volatile RFBRIR_t RFBRIR;       /*receive fifo b read index register *//*8E */
        volatile RFMIDAFVR_t RFMIDAFVR; /*receive fifo message ID acceptance filter value register *//*90 */
        volatile RFMIAFMR_t RFMIAFMR;   /*receive fifo message ID acceptance filter mask register *//*92 */
        volatile RFFIDRFVR_t RFFIDRFVR; /*receive fifo frame ID rejection filter value register *//*94 */
        volatile RFFIDRFMR_t RFFIDRFMR; /*receive fifo frame ID rejection filter mask register *//*96 */
        volatile RFRFCFR_t RFRFCFR;     /*receive fifo range filter configuration register *//*98 */
        volatile RFRFCTR_t RFRFCTR;     /*receive fifo range filter control register *//*9A */
        volatile LDTXSLAR_t LDTXSLAR;   /*last dynamic transmit slot channel A register *//*9C */
        volatile LDTXSLBR_t LDTXSLBR;   /*last dynamic transmit slot channel B register *//*9E */
        volatile PCR0_t PCR0;   /*protocol configuration register 0 *//*A0 */
        volatile PCR1_t PCR1;   /*protocol configuration register 1 *//*A2 */
        volatile PCR2_t PCR2;   /*protocol configuration register 2 *//*A4 */
        volatile PCR3_t PCR3;   /*protocol configuration register 3 *//*A6 */
        volatile PCR4_t PCR4;   /*protocol configuration register 4 *//*A8 */
        volatile PCR5_t PCR5;   /*protocol configuration register 5 *//*AA */
        volatile PCR6_t PCR6;   /*protocol configuration register 6 *//*AC */
        volatile PCR7_t PCR7;   /*protocol configuration register 7 *//*AE */
        volatile PCR8_t PCR8;   /*protocol configuration register 8 *//*B0 */
        volatile PCR9_t PCR9;   /*protocol configuration register 9 *//*B2 */
        volatile PCR10_t PCR10; /*protocol configuration register 10 *//*B4 */
        volatile PCR11_t PCR11; /*protocol configuration register 11 *//*B6 */
        volatile PCR12_t PCR12; /*protocol configuration register 12 *//*B8 */
        volatile PCR13_t PCR13; /*protocol configuration register 13 *//*BA */
        volatile PCR14_t PCR14; /*protocol configuration register 14 *//*BC */
        volatile PCR15_t PCR15; /*protocol configuration register 15 *//*BE */
        volatile PCR16_t PCR16; /*protocol configuration register 16 *//*C0 */
        volatile PCR17_t PCR17; /*protocol configuration register 17 *//*C2 */
        volatile PCR18_t PCR18; /*protocol configuration register 18 *//*C4 */
        volatile PCR19_t PCR19; /*protocol configuration register 19 *//*C6 */
        volatile PCR20_t PCR20; /*protocol configuration register 20 *//*C8 */
        volatile PCR21_t PCR21; /*protocol configuration register 21 *//*CA */
        volatile PCR22_t PCR22; /*protocol configuration register 22 *//*CC */
        volatile PCR23_t PCR23; /*protocol configuration register 23 *//*CE */
        volatile PCR24_t PCR24; /*protocol configuration register 24 *//*D0 */
        volatile PCR25_t PCR25; /*protocol configuration register 25 *//*D2 */
        volatile PCR26_t PCR26; /*protocol configuration register 26 *//*D4 */
        volatile PCR27_t PCR27; /*protocol configuration register 27 *//*D6 */
        volatile PCR28_t PCR28; /*protocol configuration register 28 *//*D8 */
        volatile PCR29_t PCR29; /*protocol configuration register 29 *//*DA */
        volatile PCR30_t PCR30; /*protocol configuration register 30 *//*DC */
        vuint16_t reserved2[17];
        volatile MSG_BUFF_CCS_t MBCCS[128];     /* message buffer configuration, control & status registers 0-31 *//*100 */
    } FR_tag_t;

    CC_EXTENSION typedef union uF_HEADER     /* frame header */
    {
        struct {
            vuint16_t:5;
            vuint16_t HDCRC:11; /* Header CRC */
              vuint16_t:2;
            vuint16_t CYCCNT:6; /* Cycle Count */
              vuint16_t:1;
            vuint16_t PLDLEN:7; /* Payload Length */
              vuint16_t:1;
            vuint16_t PPI:1;    /* Payload Preamble Indicator */
            vuint16_t NUF:1;    /* Null Frame Indicator */
            vuint16_t SYF:1;    /* Sync Frame Indicator */
            vuint16_t SUF:1;    /* Startup Frame Indicator */
            vuint16_t FID:11;   /* Frame ID */
        } B;
        vuint16_t WORDS[3];
    } F_HEADER_t;
    CC_EXTENSION typedef union uS_STSTUS     /* slot status */
    {
        struct {
            vuint16_t VFB:1;    /* Valid Frame on channel B */
            vuint16_t SYB:1;    /* Sync Frame Indicator channel B */
            vuint16_t NFB:1;    /* Null Frame Indicator channel B */
            vuint16_t SUB:1;    /* Startup Frame Indicator channel B */
            vuint16_t SEB:1;    /* Syntax Error on channel B */
            vuint16_t CEB:1;    /* Content Error on channel B */
            vuint16_t BVB:1;    /* Boundary Violation on channel B */
            vuint16_t CH:1;     /* Channel */
            vuint16_t VFA:1;    /* Valid Frame on channel A */
            vuint16_t SYA:1;    /* Sync Frame Indicator channel A */
            vuint16_t NFA:1;    /* Null Frame Indicator channel A */
            vuint16_t SUA:1;    /* Startup Frame Indicator channel A */
            vuint16_t SEA:1;    /* Syntax Error on channel A */
            vuint16_t CEA:1;    /* Content Error on channel A */
            vuint16_t BVA:1;    /* Boundary Violation on channel A */
              vuint16_t:1;
        } RX;
        struct {
            vuint16_t VFB:1;    /* Valid Frame on channel B */
            vuint16_t SYB:1;    /* Sync Frame Indicator channel B */
            vuint16_t NFB:1;    /* Null Frame Indicator channel B */
            vuint16_t SUB:1;    /* Startup Frame Indicator channel B */
            vuint16_t SEB:1;    /* Syntax Error on channel B */
            vuint16_t CEB:1;    /* Content Error on channel B */
            vuint16_t BVB:1;    /* Boundary Violation on channel B */
            vuint16_t TCB:1;    /* Tx Conflict on channel B */
            vuint16_t VFA:1;    /* Valid Frame on channel A */
            vuint16_t SYA:1;    /* Sync Frame Indicator channel A */
            vuint16_t NFA:1;    /* Null Frame Indicator channel A */
            vuint16_t SUA:1;    /* Startup Frame Indicator channel A */
            vuint16_t SEA:1;    /* Syntax Error on channel A */
            vuint16_t CEA:1;    /* Content Error on channel A */
            vuint16_t BVA:1;    /* Boundary Violation on channel A */
            vuint16_t TCA:1;    /* Tx Conflict on channel A */
        } TX;
        vuint16_t R;
    } S_STATUS_t;

    typedef struct uMB_HEADER   /* message buffer header */
    {
        F_HEADER_t FRAME_HEADER;
        vuint16_t DATA_OFFSET;
        S_STATUS_t SLOT_STATUS;
    } MB_HEADER_t;

/* Define memories */

#if 0
#define SRAM_START  0x40000000
#define SRAM_SIZE      0x14000
#define SRAM_END    0x40013FFF

#define FLASH_START         0x0
#define FLASH_SIZE      0x200000
#define FLASH_END       0x1FFFFF
#endif

/* Define instances of modules */
#define PBRIDGE_A (*( volatile struct PBRIDGE_A_tag *) 0xC3F00000)
#define FMPLL     (*( volatile struct FMPLL_tag *)     0xC3F80000)
#define EBI       (*( volatile struct EBI_tag *)       0xC3F84000)
#define FLASH     (*( volatile struct FLASH_tag *)     0xC3F88000)
#define SIU       (*( volatile struct SIU_tag *)       0xC3F90000)

#define EMIOS     (*( volatile struct EMIOS_tag *)     0xC3FA0000)
#define ETPU      (*( volatile struct ETPU_tag *)      0xC3FC0000)
#define ETPU_DATA_RAM  (*( uint32_t *)        0xC3FC8000)
#define ETPU_DATA_RAM_EXT  (*( uint32_t *)    0xC3FCC000)
#define ETPU_DATA_RAM_END  0xC3FC89FC
#define CODE_RAM       (*( uint32_t *)        0xC3FD0000)
#define ETPU_CODE_RAM  (*( uint32_t *)        0xC3FD0000)

#define PBRIDGE_B (*( volatile struct PBRIDGE_B_tag *) 0xFFF00000)
#define XBAR      (*( volatile struct XBAR_tag *)      0xFFF04000)
#define ECSM      (*( volatile struct ECSM_tag *)      0xFFF40000)
#define EDMA      (*( volatile struct EDMA_tag *)      0xFFF44000)
#define INTC      (*( volatile struct INTC_tag *)      0xFFF48000)

#define EQADC     (*( volatile struct EQADC_tag *)     0xFFF80000)

#define DSPI_B    (*( volatile struct DSPI_tag *)      0xFFF94000)
#define DSPI_C    (*( volatile struct DSPI_tag *)      0xFFF98000)
#define DSPI_D    (*( volatile struct DSPI_tag *)      0xFFF9C000)

#define ESCI_A    (*( volatile struct ESCI_tag *)      0xFFFB0000)
#define ESCI_B    (*( volatile struct ESCI_tag *)      0xFFFB4000)

#define CAN_A     (*( volatile struct FLEXCAN2_tag *)  0xFFFC0000)
#define CAN_B     (*( volatile struct FLEXCAN2_tag *)  0xFFFC4000)
#define CAN_C     (*( volatile struct FLEXCAN2_tag *)  0xFFFC8000)
#define CAN_D     (*( volatile struct FLEXCAN2_tag *)  0xFFFCC000)
#define CAN_E     (*( volatile struct FLEXCAN2_tag *)  0xFFFD0000)

#define FEC     (*( volatile struct FEC_tag *)  0xFFF4C000)

#define FR     (*( volatile struct FR_tag *)  0xFFFE0000)

#ifdef __MWERKS__
#pragma pop
#endif

#ifdef  __cplusplus
}
#endif
#endif                          /* ifdef _MPC5567_H */
/*********************************************************************
 *
 * Copyright:
 *	Freescale Semiconductor, INC. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is
 *  retained without alteration in any modified and/or redistributed
 *  versions, and that such modified versions are clearly identified
 *  as such. No licenses are granted by implication, estoppel or
 *  otherwise under any patents or trademarks of Freescale
 *  Semiconductor, Inc. This software is provided on an "AS IS"
 *  basis and without warranty.
 *
 *  To the maximum extent permitted by applicable law, Freescale
 *  Semiconductor DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,
 *  INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *  PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
 *  REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF)
 *  AND ANY ACCOMPANYING WRITTEN MATERIALS.
 *
 *  To the maximum extent permitted by applicable law, IN NO EVENT
 *  SHALL Freescale Semiconductor BE LIABLE FOR ANY DAMAGES WHATSOEVER
 *  (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
 *  BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER
 *  PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.
 *
 *  Freescale Semiconductor assumes no responsibility for the
 *  maintenance and support of this software
 *
 ********************************************************************/
