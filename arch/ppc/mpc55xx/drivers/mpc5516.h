
/**************************************************************************/
/* FILE NAME: mpc5516.h                      COPYRIGHT (c) Freescale 2007 */
/* VERSION:  1.0                                  All Rights Reserved     */
/*                                                                        */
/* DESCRIPTION:                                                           */
/* This file contain all of the register and bit field definitions for    */
/* MPC5567.                                                               */
/*========================================================================*/
/* UPDATE HISTORY                                                         */
/* REV      AUTHOR      DATE       DESCRIPTION OF CHANGE                  */
/* ---   -----------  ---------    ---------------------                  */
/* 1.0   M. Stewart   05/Feb/07    Initial version.                       */
/**************************************************************************/
/*>>>>NOTE! this file is auto-generated please do not edit it!<<<<*/

#ifndef _MPC5516_H_
#define _MPC5516_H_

#define MPC5516		1

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
/*                     MODULE : CRP                                         */
/****************************************************************************/
CC_EXTENSION struct CRP_tag {

        union {
            vuint32_t R;
            struct {
                vuint32_t:12;
                vuint32_t KIRCEN:1;
                vuint32_t XOSCEN:1;
                  vuint32_t:1;
                vuint32_t KOSCEN:1;
                vuint32_t TRIM32IRC:8;
                vuint32_t TRIMIRC:8;
            } B;
        } CLKSRC;               //Clock Source Register

        uint32_t crp_reserved1[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t CNTEN:1;
                vuint32_t RTCIE:1;
                vuint32_t RTCF:1;
                vuint32_t ROVREN:1;
                vuint32_t RTCVAL:12;
                vuint32_t APIEN:1;
                vuint32_t APIIE:1;
                vuint32_t APIF:1;
                vuint32_t CLKSEL:2;
                vuint32_t ROVRF:1;
                vuint32_t APIVAL:10;
            } B;
        } RTCSC;                //RTC Status and Control Register

        union {
            vuint32_t R;
            struct {
                vuint32_t:5;
                vuint32_t RTCCNT:27;
            } B;
        } RTCCNT;               //RTC Counter Register

        uint32_t crp_reserved2[6];

        union {
            vuint32_t R;
            struct {
                vuint32_t:27;
                vuint32_t MTEST:1;
                vuint32_t LVIDIS:1;
                vuint32_t VTRIM:3;
            } B;
        } VREGSC;               //VREG Trim Satus and Control Register

        uint32_t crp_reserved3[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t WKPSEL7:4;
                vuint32_t WKPSEL6:4;
                vuint32_t WKPSEL5:4;
                vuint32_t WKPSEL4:4;
                vuint32_t WKPSEL3:4;
                vuint32_t WKPSEL2:4;
                vuint32_t WKPSEL1:4;
                vuint32_t WKPSEL0:4;
            } B;
        } WKPINSEL;             //Wakeup Pin Source Select Register

        union {
            vuint32_t R;
            struct {
                vuint32_t WKPDET7:2;
                vuint32_t WKPDET6:2;
                vuint32_t WKPDET5:2;
                vuint32_t WKPDET4:2;
                vuint32_t WKPDET3:2;
                vuint32_t WKPDET2:2;
                vuint32_t WKPDET1:2;
                vuint32_t WKPDET0:2;
                  vuint32_t:5;
                vuint32_t RTCOVREN:1;
                vuint32_t RTCWKEN:1;		/* switched two fields JRichard 16MAY2007*/
                vuint32_t APIWKEN:1;
                  vuint32_t:7;
                vuint32_t WKCLKSEL:1;
            } B;
       } WKSE;                  //Wakeup Source Enable Register 

			/*        } WKE;                  //Wakeup Source Enable Register */
		

        uint32_t crp_reserved4[2];

        union {
            vuint32_t R;
            struct {
                vuint32_t Z1VEC:30;
                vuint32_t Z1RST:1;
                vuint32_t VLE:1;
            } B;
        } Z1VEC;                //Z1 Reset Vector Register

        union {
            vuint32_t R;
            struct {
                vuint32_t Z0VEC:30;
                vuint32_t Z0RST:1;
                  vuint32_t:1;
            } B;
        } Z0VEC;                //Z0 Reset Vector Register

        union {
            vuint32_t R;
            struct {
                vuint32_t RECPTR:30;
                vuint32_t FASTREC:1;
                  vuint32_t:1;
            } B;
        } RECPRTR;              //Reset Recovery Pointer Register

        uint32_t crp_reserved5;

        union {
            vuint32_t R;
            struct {
                vuint32_t SLEEPF:1;
                vuint32_t STOPF:1;
                  vuint32_t:3;
                vuint32_t WKRTCF:1;
                vuint32_t WKAPIF:1;
                vuint32_t WKRLLOVRF:1;
                vuint32_t PWKSRCF:8;
                vuint32_t SLEEP:1;
                vuint32_t STOP:1;
                  vuint32_t:1;
                vuint32_t PKREL:1;	  	/* JRichard */
                //vuint32_t PKREN:1;	  /* JRichard */
                
                vuint32_t STOP12EN:1;
                vuint32_t RAMSEL:3;
                vuint32_t PWKSRIE:8;
            } B;
        } PSCR;                 //Power Status and Control Register

        uint32_t crp_reserved6[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t LVI5IE:1;
                vuint32_t LVI5HIE:1;
                vuint32_t LVI5F:1;
                vuint32_t LVI5HF:1;
                vuint32_t LVILOCK:1;
                vuint32_t LVI5RE:1;
                  vuint32_t:26;
            } B;
        } LVISC;                //LVI Status and Control Register

    };
/****************************************************************************/
/*                          MODULE : DMAMUX                                 */
/****************************************************************************/
CC_EXTENSION struct DMAMUX_tag {
        union {
            vuint8_t R;
            struct {
                vuint8_t ENBL:1;
                vuint8_t TRIG:1;
                vuint8_t SOURCE:6;
            } B;
        } CHCONFIG[16];         /* DMA Channel Configuration Register */

    };
/****************************************************************************/
/*                          MODULE : DSPI                                   */
/****************************************************************************/
#include "ip_dspi.h"

/****************************************************************************/
/*                     MODULE : External Bus Interface (EBI)                */
/****************************************************************************/
CC_EXTENSION struct CS_tag {
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

        uint32_t EBI_reserved2[4];

        /* Calibration registers */
        struct CS_tag CAL_CS[4];

    };
/****************************************************************************/
/*                          MODULE : eDMA                                   */
/****************************************************************************/
#include "ip_edma.h"


/****************************************************************************/
/*                          MODULE : EMIOS                                  */
/****************************************************************************/
    CC_EXTENSION   struct EMIOS_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t DOZEEN:1;
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
        } GFLAG;                /* Global FLAG Register */

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
        } OUDIS;                /* Output Update Disable Register */

        union {
            vuint32_t R;
        } UCDIS;                /* Disable Channel Register */

        uint32_t emios_reserved1[4];

        struct {
            union {
                vuint32_t R;    /* Channel A Data Register */
            //} A;
            } CADR;   /* JRichard */

            union {
                vuint32_t R;    /* Channel B Data Register */
            //} B;
            } CBDR;   /* JRichard */

            union {
                vuint32_t R;    /* Channel Counter Register */
            //} CNT;
            } CCNTR;  /* JRichard */

            union {
                vuint32_t R;
                struct {
                    vuint32_t FREN:1;
                    vuint32_t ODIS:1;
                    vuint32_t ODISSL:2;
                    vuint32_t UCPRE:2;
                    vuint32_t UCPREN:1;
//                    vuint32_t UCPEN:1;		/* JRichard 16MAY2007 */
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
            } CCR;                /* Channel Control Register */
            //} C;                /* Channel Control Register */ /* JRichard*/

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
            } CSR;
            //} S;                /* JRichard Channel Status Register */

            union {
                vuint32_t R;    /* Alternate Channel A Data Register */
            } ALTA;

            uint32_t emios_channel_reserved[2];

        } CH[24];

    };
/****************************************************************************/
/*                          MODULE : EQADC                                  */
/****************************************************************************/
CC_EXTENSION  struct EQADC_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:27;
                vuint32_t ESSIE:2;
                  vuint32_t:1;
                vuint32_t DBG:2;
            } B;
        } MCR;                  /* Module Configuration Register */

        union {
            vuint32_t R;
        } TST;                  /* Test Register */

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
/*                          MODULE : eSCI                                   */
/****************************************************************************/
CC_EXTENSION     struct ESCI_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:3;
                vuint32_t SBR:13;
                vuint32_t LOOPS:1;
                  vuint32_t:1;
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
/*                     MODULE : FLASH                                       */
/****************************************************************************/
CC_EXTENSION  struct FLASH_tag {
        union {                 /* Module Configuration Register */
            vuint32_t R;
            struct {
                vuint32_t:3;
                vuint32_t SFS:1;
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
                  vuint32_t:1;
                vuint32_t PRD:1;
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
                  vuint32_t:2;
                vuint32_t MLOCK:2;
                  vuint32_t:8;
                vuint32_t LLOCK:8;
            } B;
        } LMLR;
        //} LML;			 /* JRichard 7MAR2007 */

        union {                 /* HL Register */
            vuint32_t R;
            struct {
                vuint32_t HBE:1;
                  vuint32_t:27;
                vuint32_t HBLOCK:4;
            } B;
        } HLR;
        //} HBL;										/* JRichard 7MAR2007 */

        union {                 /* SLML Register */
            vuint32_t R;
            struct {
                vuint32_t SLE:1;
                  vuint32_t:10;
                vuint32_t SSLOCK:1;
                  vuint32_t:2;
                vuint32_t SMLOCK:2;
                  vuint32_t:8;
                vuint32_t SLLOCK:8;
            } B;
        } SLMLR;
        //} SLL;			            	/* JRichard 7MAR2007 */

        union {                 /* LMS Register */
            vuint32_t R;
            struct {
                vuint32_t:14;
                vuint32_t MSEL:2;
                  vuint32_t:8;
                vuint32_t LSEL:8;
            } B;
        } LMSR;
        //} LMS; 			            	/* JRichard 7MAR2007 */

        union {
            vuint32_t R;
            struct {
                vuint32_t:28;
                vuint32_t HBSEL:4;
            } B;
        } HSR;
        //} HBS; 			            	/* JRichard 7MAR2007 */

        union {
            vuint32_t R;
            struct {
                vuint32_t:10;
                vuint32_t ADDR:19;
                  vuint32_t:3;
            } B;
        } ADR;

        union {                 /* Platform Flash Configuration Register for Port 0 */
            vuint32_t R;
            struct {
                vuint32_t LBCFG:4;
                vuint32_t ARB:1;
                vuint32_t PRI:1;
                  vuint32_t:2;
                vuint32_t M7PFE:1;
                vuint32_t M6PFE:1;
                vuint32_t M5PFE:1;
                vuint32_t M4PFE:1;
                vuint32_t M3PFE:1;
                vuint32_t M2PFE:1;
                vuint32_t M1PFE:1;
                vuint32_t M0PFE:1;
                vuint32_t APC:3;
                vuint32_t WWSC:2;
                vuint32_t RWSC:3;
                  vuint32_t:1;
                vuint32_t DPFEN:1;
                  vuint32_t:1;
                vuint32_t IPFEN:1;
                  vuint32_t:1;
                vuint32_t PFLIM:2;
                vuint32_t BFEN:1;
            } B;
        } PFCRP0;

        union {                 /* Platform Flash Configuration Register for Port 1 */
            vuint32_t R;
            struct {
                vuint32_t LBCFG:4;
                  vuint32_t:4;
                vuint32_t M7PFE:1;
                vuint32_t M6PFE:1;
                vuint32_t M5PFE:1;
                vuint32_t M4PFE:1;
                vuint32_t M3PFE:1;
                vuint32_t M2PFE:1;
                vuint32_t M1PFE:1;
                vuint32_t M0PFE:1;
                vuint32_t APC:3;
                vuint32_t WWSC:2;
                vuint32_t RWSC:3;
                  vuint32_t:1;
                vuint32_t DPFEN:1;
                  vuint32_t:1;
                vuint32_t IPFEN:1;
                  vuint32_t:1;
                vuint32_t PFLIM:2;
                vuint32_t BFEN:1;
            } B;
        } PFCRP1;

    };


#include "ip_flexcan.h"


CC_EXTENSION     struct FMPLL_tag {
        union {             /* JRichard */
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
        //int32_t FMPLL_reserved1; /* JRichard */

        union {                 /* Synthesiser Status Register */
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
        
				
				
        union {
            vuint32_t R;
            struct {
                vuint32_t EMODE:1;
                vuint32_t CLKCFG:3;
                  vuint32_t:8;
                vuint32_t EPREDIV:4;
                  vuint32_t:8;
                vuint32_t EMFD:8;
            } B;
        } ESYNCR1;

        union {
            vuint32_t R;
            struct {
                vuint32_t:8;
                vuint32_t LOCEN:1;
                vuint32_t LOLRE:1;
                vuint32_t LOCRE:1;
                vuint32_t LOLIRQ:1;
                vuint32_t LOCIRQ:1;
                  vuint32_t:1;
                vuint32_t ERATE:2;
                  vuint32_t:5;
                vuint32_t EDEPTH:3;
                  vuint32_t:2;
                vuint32_t ERFD:6;
            } B;
        } ESYNCR2;

    };
/****************************************************************************/
/*                          MODULE : i2c                                    */
/****************************************************************************/
CC_EXTENSION     struct I2C_tag {
        union {
            vuint8_t R;
            struct {
                vuint8_t ADR:7;
                  vuint8_t:1;
            } B;
        } IBAD;                 /* Module Bus Address Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t IBC:8;
            } B;
        } IBFD;                 /* Module Bus Frequency Register */

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
        } IBCR;                 /* Module Bus Control Register */

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
        } IBSR;                 /* Module Status Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t DATA:8;
            } B;
        } IBDR;                 /* Module Data Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t BIIE:1;
                  vuint8_t:7;
            } B;
        } IBIC;                 /* Module Interrupt Configuration Register */

    };
/****************************************************************************/
/*                          MODULE : INTC                                   */
/****************************************************************************/
CC_EXTENSION     struct INTC_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:18;
                vuint32_t VTES_PRC1:1;
                  vuint32_t:4;
                vuint32_t HVEN_PRC1:1;
                  vuint32_t:2;
                vuint32_t VTES_PRC0:1;
                  vuint32_t:4;
                vuint32_t HVEN_PRC0:1;
            } B;
        } MCR;                  /* Module Configuration Register */

        int32_t INTC_reserved1;

        union {
            vuint32_t R;
            struct {
                vuint32_t:28;
                vuint32_t PRI:4;
            } B;
        } CPR_PRC0;             /* Processor 0 Current Priority Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:28;
                vuint32_t PRI:4;
            } B;
        } CPR_PRC1;             /* Processor 1 Current Priority Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t VTBA_PRC0:21;
                vuint32_t INTVEC_PRC0:9;
                  vuint32_t:2;
            } B;
        } IACKR_PRC0;           /* Processor 0 Interrupt Acknowledge Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t VTBA_PRC1:21;
                vuint32_t INTVEC_PRC1:9;
                  vuint32_t:2;
            } B;
        } IACKR_PRC1;           /* Processor 1 Interrupt Acknowledge Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:32;
            } B;
        } EOIR_PRC0;            /* Processor 0 End of Interrupt Register */

        union {
            vuint32_t R;
            struct {
                vuint32_t:32;
            } B;
        } EOIR_PRC1;            /* Processor 1 End of Interrupt Register */

        union {
            vuint8_t R;
            struct {
                vuint8_t:6;
                vuint8_t SET:1;
                vuint8_t CLR:1;
            } B;
        } SSCIR[8];             /* Software Set/Clear Interruput Register */

        uint32_t intc_reserved2[6];

        union {
            vuint8_t R;
            struct {
                vuint8_t PRC_SEL:2;
                  vuint8_t:2;
                vuint8_t PRI:4;
            } B;
        } PSR[294];             /* Software Set/Clear Interrupt Register */

    };
/****************************************************************************/
/*                     MODULE : MCM                                         */
/****************************************************************************/
CC_EXTENSION     struct MCM_tag {

        uint32_t mcm_reserved1[5];

        uint16_t mcm_reserved2;

        union {
            vuint16_t R;
            struct {
                vuint16_t RO:1;
                  vuint16_t:6;
                vuint16_t SWRWH:1;
                vuint16_t SWE:1;
                vuint16_t SWRI:2;
                vuint16_t SWT:5;
            } B;
        } SWTCR;                //Software Watchdog Timer Control

        uint8_t mcm_reserved3[3];

        union {
            vuint8_t R;
        } SWTSR;                //SWT Service Register

        uint8_t mcm_reserved4[3];

        union {
            vuint8_t R;
            struct {
                vuint8_t:7;
                vuint8_t SWTIC:1;
            } B;
        } SWTIR;                //SWT Interrupt Register

        uint32_t mcm_reserved5[1];

        union {
            vuint32_t R;
            struct {
                vuint32_t PRI:1;
                  vuint32_t:31;
            } B;
        } MUDCR;                //Misc. User Defined Control Register

        uint32_t mcm_reserved6[6];
        uint8_t mcm_reserved7[3];

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

        uint16_t mcm_reserved9;

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

        uint32_t mcm_reserved10;

        union {
            vuint32_t R;
        } FEAR;                 //Flash ECC Address Register

        uint16_t mcm_reserved11;

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
                vuint8_t PROTECTION:4;
            } B;
        } FEAT;                 //Flash ECC Attributes Register

        union {
            vuint32_t R;
        } FEDRH;                //Flash ECC Data High Register

        union {
            vuint32_t R;
        } FEDRL;                //Flash ECC Data Low Register

        union {
            vuint32_t R;
            struct {
                vuint32_t REAR:32;
            } B;
        } REAR;                 //RAM ECC Address

        uint8_t mcm_reserved12;

        union {
            vuint8_t R;
            struct {
                vuint8_t RESR:8;
            } B;
        } RESR;                 //RAM ECC Syndrome

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
                vuint8_t PROTECTION:1;
            } B;
        } REAT;                 // RAM ECC Attributes Register

        union {
            vuint32_t R;
        } REDRH;                //RAM ECC Data High Register

        union {
            vuint32_t R;
        } REDRL;                //RAMECC Data Low Register

    };
/****************************************************************************/
/*                          MODULE : MPU                                    */
/****************************************************************************/
CC_EXTENSION     struct MPU_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t MSTR:8;
                  vuint32_t:4;
                vuint32_t HRL:4;
                vuint32_t NSP:4;
                vuint32_t NGRD:4;
                  vuint32_t:7;
                vuint32_t VLD:1;
            } B;
        } CESR;                 /* Module Control/Error Status Register */

        uint32_t mpu_reserved1[3];

        union {
            vuint32_t R;
            struct {
                vuint32_t EADDR:32;
            } B;
        } EAR0;

        union {
            vuint32_t R;
            struct {
                vuint32_t EACD:16;
                vuint32_t EPID:8;
                vuint32_t EMN:4;
                vuint32_t EATTR:3;
                vuint32_t ERW:1;
            } B;
        } EDR0;

        union {
            vuint32_t R;
            struct {
                vuint32_t EADDR:32;
            } B;
        } EAR1;

        union {
            vuint32_t R;
            struct {
                vuint32_t EACD:16;
                vuint32_t EPID:8;
                vuint32_t EMN:4;
                vuint32_t EATTR:3;
                vuint32_t ERW:1;
            } B;
        } EDR1;

        union {
            vuint32_t R;
            struct {
                vuint32_t EADDR:32;
            } B;
        } EAR2;

        union {
            vuint32_t R;
            struct {
                vuint32_t EACD:16;
                vuint32_t EPID:8;
                vuint32_t EMN:4;
                vuint32_t EATTR:3;
                vuint32_t ERW:1;
            } B;
        } EDR2;

        uint32_t mpu_reserved2[246];

        struct {
            union {
                vuint32_t R;
                struct {
                    vuint32_t SRTADDR:27;
                      vuint32_t:5;
                } B;
            } WORD0;            /* Region Descriptor n Word 0 */

            union {
                vuint32_t R;
                struct {
                    vuint32_t ENDADDR:27;
                      vuint32_t:5;
                } B;
            } WORD1;            /* Region Descriptor n Word 1 */

            union {
                vuint32_t R;
                struct {
                    vuint32_t:6;
                    vuint32_t M4RE0:1;
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
            } WORD2;            /* Region Descriptor n Word 2 */

            union {
                vuint32_t R;
                struct {
                    vuint32_t PID:8;
                    vuint32_t PIDMASK:8;
                      vuint32_t:15;
                    vuint32_t VLD:1;
                } B;
            } WORD3;            /* Region Descriptor n Word 3 */

        } RGD[16];

        uint32_t mpu_reserved3[192];

        union {
            vuint32_t R;
            struct {
                vuint32_t:6;
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
        } RGDAAC[16];           /* Region Descriptor Alternate Access Control n */
    };
/****************************************************************************/
/*                          MODULE : pit                                    */
/****************************************************************************/
CC_EXTENSION     struct PIT_tag {
        union {
            vuint32_t R;
            struct {
                vuint32_t:8;
                vuint32_t TSV:24;
            } B;
        } TLVAL0;

        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } TLVAL1;

        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } TLVAL2;

        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } TLVAL3;

        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } TLVAL4;

        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } TLVAL5;

        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } TLVAL6;

        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } TLVAL7;

        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } TLVAL8;

        uint32_t pit_reserved1[23];

        union {
            vuint32_t R;
            struct {
                vuint32_t:8;
                vuint32_t TVL:24;
            } B;
        } TVAL0;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } TVAL1;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } TVAL2;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } TVAL3;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } TVAL4;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } TVAL5;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } TVAL6;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } TVAL7;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } TVAL8;

        uint32_t pit_reserved2[23];

        union {
            vuint32_t R;
            struct {
                vuint32_t:27;
                vuint32_t TIF4:1;
                vuint32_t TIF3:1;
                vuint32_t TIF2:1;
                vuint32_t TIF1:1;
                vuint32_t RTIF:1;
            } B;
        } FLG;

        union {
            vuint32_t R;
            struct {
                vuint32_t:27;
                vuint32_t TIE4:1;
                vuint32_t TIE3:1;
                vuint32_t TIE2:1;
                vuint32_t TIE1:1;
                vuint32_t RTIE:1;
            } B;
        } INTEN;

        union {
            vuint32_t R;
            struct {
                vuint32_t:27;
                vuint32_t ISEL4:1;
                vuint32_t ISEL3:1;
                vuint32_t ISEL2:1;
                vuint32_t ISEL1:1;
                  vuint32_t:1;
            } B;
        } INTSEL;

        union {
            vuint32_t R;
            struct {
                vuint32_t:21;
                vuint32_t PEN10:1;
                vuint32_t PEN9:1;
                vuint32_t PEN8:1;
                vuint32_t PEN7:1;
                vuint32_t PEN6:1;
                vuint32_t PEN5:1;
                vuint32_t PEN4:1;
                vuint32_t PEN3:1;
                vuint32_t PEN2:1;
                vuint32_t PEN1:1;
                vuint32_t PEN0:1;
            } B;
        } EN;

        union {
            vuint32_t R;
            struct {
                vuint32_t:6;
                vuint32_t DOZE:1;
                vuint32_t MDIS:1;
                  vuint32_t:24;
            } B;
        } CTRL;

    };
/****************************************************************************/
/*                          MODULE : sem4                                   */
/****************************************************************************/
CC_EXTENSION     struct SEMA4_tag {
        union {
            vuint8_t R;
            struct {
                vuint8_t:6;
                vuint8_t GTFSM:2;
            } B;
        } GATE[16];             /* Gate n Register */

        uint32_t sema4_reserved1[12];

        union {
            vuint32_t R;
            struct {
                vuint32_t INE0:1;
                vuint32_t INE1:1;
                vuint32_t INE2:1;
                vuint32_t INE3:1;
                vuint32_t INE4:1;
                vuint32_t INE5:1;
                vuint32_t INE6:1;
                vuint32_t INE7:1;
                vuint32_t INE8:1;
                vuint32_t INE9:1;
                vuint32_t INE10:1;
                vuint32_t INE11:1;
                vuint32_t INE12:1;
                vuint32_t INE13:1;
                vuint32_t INE14:1;
                vuint32_t INE15:1;
                  vuint32_t:16;
            } B;
        } CP0INE;

        uint32_t sema4_reserved2[1];

        union {
            vuint32_t R;
            struct {
                vuint32_t INE0:1;
                vuint32_t INE1:1;
                vuint32_t INE2:1;
                vuint32_t INE3:1;
                vuint32_t INE4:1;
                vuint32_t INE5:1;
                vuint32_t INE6:1;
                vuint32_t INE7:1;
                vuint32_t INE8:1;
                vuint32_t INE9:1;
                vuint32_t INE10:1;
                vuint32_t INE11:1;
                vuint32_t INE12:1;
                vuint32_t INE13:1;
                vuint32_t INE14:1;
                vuint32_t INE15:1;
                  vuint32_t:16;
            } B;
        } CP1INE;

        uint32_t sema4_reserved3[13];

        union {
            vuint32_t R;
            struct {
                vuint32_t GN0:1;
                vuint32_t GN1:1;
                vuint32_t GN2:1;
                vuint32_t GN3:1;
                vuint32_t GN4:1;
                vuint32_t GN5:1;
                vuint32_t GN6:1;
                vuint32_t GN7:1;
                vuint32_t GN8:1;
                vuint32_t GN9:1;
                vuint32_t GN10:1;
                vuint32_t GN11:1;
                vuint32_t GN12:1;
                vuint32_t GN13:1;
                vuint32_t GN14:1;
                vuint32_t GN15:1;
                  vuint32_t:16;
            } B;
        } CP0NTF;

        uint32_t sema4_reserved4[1];

        union {
            vuint32_t R;
            struct {
                vuint32_t GN0:1;
                vuint32_t GN1:1;
                vuint32_t GN2:1;
                vuint32_t GN3:1;
                vuint32_t GN4:1;
                vuint32_t GN5:1;
                vuint32_t GN6:1;
                vuint32_t GN7:1;
                vuint32_t GN8:1;
                vuint32_t GN9:1;
                vuint32_t GN10:1;
                vuint32_t GN11:1;
                vuint32_t GN12:1;
                vuint32_t GN13:1;
                vuint32_t GN14:1;
                vuint32_t GN15:1;
                  vuint32_t:16;
            } B;
        } CP1NTF;

        uint32_t sema4_reserved5[29];

        union {
            vuint32_t R;
            struct {
                vuint32_t:2;
                vuint32_t RSTGSM:2;
                  vuint32_t:1;
                vuint32_t RSTGMS:3;
                vuint32_t RSTGTN:8;
                  vuint32_t:16;
            } B;
        } RSTGT;

        union {
            vuint32_t R;
            struct {
                vuint32_t:2;
                vuint32_t RSTNSM:2;
                  vuint32_t:1;
                vuint32_t RSTNMS:3;
                vuint32_t RSTNTN:8;
                  vuint32_t:16;
            } B;
        } RSTNTF;
    };
/****************************************************************************/
/*                     MODULE : SIU                                         */
/****************************************************************************/
CC_EXTENSION     struct SIU_tag {
        int32_t SIU_reserved0;

        union {                 /* MCU ID Register */
            vuint32_t R;
            struct {
                vuint32_t PARTNUM:16;
                vuint32_t PKG:4;
                //vuint32_t MASKNUM:16;
                vuint32_t MASKNUM:12;
			} B;
        } MIDR;

        int32_t SIU_reserved1;

        union {                 /* Reset Status Register */
            vuint32_t R;
            struct {
                vuint32_t PORS:1;
                vuint32_t ERS:1;
                vuint32_t LLRS:1;
                vuint32_t LCRS:1;
                vuint32_t WDRS:1;
                vuint32_t CRS0:1;
                vuint32_t CRS1:1;
                  vuint32_t:7;
                vuint32_t SSRS:1;
                  vuint32_t:14;
                vuint32_t BOOTCFG:2;
                  vuint32_t:1;
            } B;
        } RSR;

        union {                 /* System Reset Control Register */
            vuint32_t R;
            struct {
                vuint32_t SSR:1;
                  vuint32_t:15;
                vuint32_t CRE0:1;
                vuint32_t CRE1:1;
                  vuint32_t:6;
                vuint32_t SSRL:1;
                  vuint32_t:7;
            } B;
        } SRCR;

        union {                 /* External Interrupt Status Register */
            vuint32_t R;
            struct {
                vuint32_t NMI0:1;
                vuint32_t NMI1:1;
                  vuint32_t:14;
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
                vuint32_t NRE0:1;
                vuint32_t NRE1:1;
                  vuint32_t:14;
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
                vuint32_t NREE0:1;
                vuint32_t NREE1:1;
                  vuint32_t:14;
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
                vuint32_t NFEE0:1;
                vuint32_t NFEE1:1;
                  vuint32_t:14;
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

        union {                 /* External IRQ Filtered Input Register */
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t FI15:1;
                vuint32_t FI14:1;
                vuint32_t FI13:1;
                vuint32_t FI12:1;
                vuint32_t FI11:1;
                vuint32_t FI10:1;
                vuint32_t FI9:1;
                vuint32_t FI8:1;
                vuint32_t FI7:1;
                vuint32_t FI6:1;
                vuint32_t FI5:1;
                vuint32_t FI4:1;
                vuint32_t FI3:1;
                vuint32_t FI2:1;
                vuint32_t FI1:1;
                vuint32_t FI0:1;
            } B;
        } IFIR;

        int32_t SIU_reserved2;
		int32_t SIU_reserved11;

        union {                 /* Pad Configuration Registers */
            vuint16_t R;
            struct {
                vuint16_t:4;
                vuint16_t PA:2;
                vuint16_t OBE:1;
                vuint16_t IBE:1;
                  vuint16_t:2;
                vuint16_t ODE:1;
                vuint16_t HYS:1;
                vuint16_t SEC:2;
                vuint16_t WPE:1;
                vuint16_t WPS:1;
            } B;
        } PCR[146];

        int32_t SIU_reserved3[295];

        union {                 /* GPIO Pin Data Output Registers */
            vuint8_t R;
            struct {
                vuint8_t:7;
                vuint8_t PDO:1;
            } B;
        } GPDO[146];

        int32_t SIU_reserved4[91];

        union {                 /* GPIO Pin Data Input Registers */
            vuint8_t R;
            struct {
                vuint8_t:7;
                vuint8_t PDI:1;
            } B;
        } GPDI[146];

        int32_t SIU_reserved5[27];

        union {                 /* IMUX Register */
            vuint32_t R;
            struct {
                vuint32_t TSEL3:2;
                vuint32_t TSEL2:2;
                vuint32_t TSEL1:2;
                vuint32_t TSEL0:2;
                  vuint32_t:24;
            } B;
        } ISEL0;

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
        } ISEL1;

        union {                 /* IMUX Register */
            vuint32_t R;
            struct {
                vuint32_t SELEMIOS15:2;
                vuint32_t SELEMIOS14:2;
                vuint32_t SELEMIOS13:2;
                vuint32_t SELEMIOS12:2;
                vuint32_t SELEMIOS11:2;
                vuint32_t SELEMIOS10:2;
                vuint32_t SELEMIOS9:2;
                vuint32_t SELEMIOS8:2;
                vuint32_t SELEMIOS7:2;
                vuint32_t SELEMIOS6:2;
                vuint32_t SELEMIOS5:2;
                vuint32_t SELEMIOS4:2;
                vuint32_t SELEMIOS3:2;
                vuint32_t SELEMIOS2:2;
                vuint32_t SELEMIOS1:2;
                vuint32_t SELEMIOS0:2;
            } B;
        } ISEL2;

        int32_t SIU_reserved6[29];

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
                vuint32_t:30;
                vuint32_t EBDF:2;
            } B;
        } ECCR;

        union {                 /* Compare A High Register */
            vuint32_t R;
        } CMPAH;

        union {                 /* Compare A Low Register */
            vuint32_t R;
        } CMPAL;

        union {                 /* Compare B High Register */
            vuint32_t R;
        } CMPBH;

        union {                 /* Compare B Low Register */
            vuint32_t R;
        } CMPBL;

        int32_t SIU_reserved7[2];

        union {                 /* System CLock Register */
            vuint32_t R;
            struct {
                vuint32_t SYSCLKSEL:2;
                vuint32_t SYSCLKDIV:2;
                vuint32_t SWTCLKSEL:1;
                  vuint32_t:11;
                vuint32_t LPCLKDIV7:2;
                vuint32_t LPCLKDIV6:2;
                vuint32_t LPCLKDIV5:2;
                vuint32_t LPCLKDIV4:2;
                vuint32_t LPCLKDIV3:2;
                vuint32_t LPCLKDIV2:2;
                vuint32_t LPCLKDIV1:2;
                vuint32_t LPCLKDIV0:2;
            } B;
        } SYSCLK;

        union {                 /* Halt Register */
            vuint32_t R;
        } HLT;

        union {                 /* Halt Acknowledge Register */
            vuint32_t R;
        } HLTACK;

        int32_t SIU_reserved8[149];

        union {                 /* Parallel GPIO Pin Data Output Register */
            vuint32_t R;
            struct {
                vuint32_t PA:16;
                vuint32_t PB:16;
            } B;
        } PGPDO0;

        union {                 /* Parallel GPIO Pin Data Output Register */
            vuint32_t R;
            struct {
                vuint32_t PC:16;
                vuint32_t PD:16;
            } B;
        } PGPDO1;

        union {                 /* Parallel GPIO Pin Data Output Register */
            vuint32_t R;
            struct {
                vuint32_t PE:16;
                vuint32_t PF:16;
            } B;
        } PGPDO2;

        union {                 /* Parallel GPIO Pin Data Output Register */
            vuint32_t R;
            struct {
                vuint32_t PG:16;
                vuint32_t PH:16;
            } B;
        } PGPDO3;

        union {                 /* Parallel GPIO Pin Data Output Register */
            vuint32_t R;
            struct {
                vuint32_t PJ:16;
                vuint32_t PK:2;
                  vuint32_t:14;
            } B;
        } PGPDO4;

        int32_t SIU_reserved9[11];

        union {                 /* Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PA:16;
                vuint32_t PB:16;
            } B;
        } PGPDI0;

        union {                 /* Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PC:16;
                vuint32_t PD:16;
            } B;
        } PGPDI1;

        union {                 /* Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PE:16;
                vuint32_t PF:16;
            } B;
        } PGPDI2;

        union {                 /* Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PG:16;
                vuint32_t PH:16;
            } B;
        } PGPDI3;

        union {                 /* Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PJ:16;
                vuint32_t PK:2;
                  vuint32_t:14;
            } B;
        } PGPDI4;

        int32_t SIU_reserved10[11];

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PA_MASK:16;
                vuint32_t PA:16;
            } B;
        } MPGPDO0;

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PB_MASK:16;
                vuint32_t PB:16;
            } B;
        } MPGPDO1;

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PC_MASK:16;
                vuint32_t PC:16;
            } B;
        } MPGPDO2;

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PD_MASK:16;
                vuint32_t PD:16;
            } B;
        } MPGPDO3;

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PE_MASK:16;
                vuint32_t PE:16;
            } B;
        } MPGPDO4;

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PF_MASK:16;
                vuint32_t PF:16;
            } B;
        } MPGPDO5;

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PG_MASK:16;
                vuint32_t PG:16;
            } B;
        } MPGPDO6;

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PH_MASK:16;
                vuint32_t PH:16;
            } B;
        } MPGPDO7;

        union {                 /* Masked Parallel GPIO Pin Data Input Register */
            vuint32_t R;
            struct {
                vuint32_t PJ_MASK:16;
                vuint32_t PJ:16;
            } B;
        } MPGPDO8;

    };
/****************************************************************************/
/*                          MODULE : FlexRay                                */
/****************************************************************************/

CC_EXTENSION     typedef union uMVR {
        vuint16_t R;
        struct {
            vuint16_t CHIVER:8; /* CHI Version Number */
            vuint16_t PEVER:8;  /* PE Version Number */
        } B;
    } MVR_t;

CC_EXTENSION     typedef union uMCR {
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
CC_EXTENSION     typedef union uSTBSCR {
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
CC_EXTENSION     typedef union uSTBPCR {
        vuint16_t R;
        struct {
            vuint16_t:12;
            vuint16_t STB3EN:1; /* strobe port enable */
            vuint16_t STB2EN:1; /* strobe port enable */
            vuint16_t STB1EN:1; /* strobe port enable */
            vuint16_t STB0EN:1; /* strobe port enable */
        } B;
    } STBPCR_t;

CC_EXTENSION     typedef union uMBDSR {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t MBSEG2DS:7;       /* message buffer segment 2 data size */
              vuint16_t:1;
            vuint16_t MBSEG1DS:7;       /* message buffer segment 1 data size */
        } B;
    } MBDSR_t;

CC_EXTENSION     typedef union uMBSSUTR {
        vuint16_t R;
        struct {

            vuint16_t:2;
            vuint16_t LAST_MB_SEG1:6;   /* last message buffer control register for message buffer segment 1 */
              vuint16_t:2;
            vuint16_t LAST_MB_UTIL:6;   /* last message buffer utilized */
        } B;
    } MBSSUTR_t;

CC_EXTENSION     typedef union uPOCR {
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
CC_EXTENSION     typedef union uGIFER {
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

CC_EXTENSION     typedef union uPIFR0 {
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

    CC_EXTENSION     typedef union uPIFR1 {
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
CC_EXTENSION     typedef union uPIER0 {
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
CC_EXTENSION     typedef union uPIER1 {
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

            vuint16_t:2;
            vuint16_t TBIVEC:6; /* transmit buffer interrupt vector */
              vuint16_t:2;
            vuint16_t RBIVEC:6; /* receive buffer interrupt vector */
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
              vuint16_t:5;
            vuint16_t RSBIDX:7; /* receive shadow buffer index */
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
                vuint16_t:9;
                vuint16_t MBIDX:7;      /* message buffer index */
            } B;
        } MBIDXR;
    } MSG_BUFF_CCS_t;
    CC_EXTENSION typedef union uSYSBADHR {
        vuint16_t R;
    } SYSBADHR_t;
    CC_EXTENSION typedef union uSYSBADLR {
        vuint16_t R;
    } SYSBADLR_t;
    CC_EXTENSION typedef union uPADR {
        vuint16_t R;
    } PADR_t;
    CC_EXTENSION typedef union uPDAR {
        vuint16_t R;
    } PDAR_t;
    CC_EXTENSION typedef union uCASERCR {
        vuint16_t R;
    } CASERCR_t;
    CC_EXTENSION typedef union uCBSERCR {
        vuint16_t R;
    } CBSERCR_t;
    CC_EXTENSION typedef union uCYCTR {
        vuint16_t R;
    } CYCTR_t;
    CC_EXTENSION typedef union uMTCTR {
        vuint16_t R;
    } MTCTR_t;
    CC_EXTENSION typedef union uSLTCTAR {
        vuint16_t R;
    } SLTCTAR_t;
    CC_EXTENSION typedef union uSLTCTBR {
        vuint16_t R;
    } SLTCTBR_t;
    CC_EXTENSION typedef union uRTCORVR {
        vuint16_t R;
    } RTCORVR_t;
    CC_EXTENSION typedef union uOFCORVR {
        vuint16_t R;
    } OFCORVR_t;
    CC_EXTENSION typedef union uSFTOR {
        vuint16_t R;
    } SFTOR_t;
    CC_EXTENSION typedef union uSFIDAFVR {
        vuint16_t R;
    } SFIDAFVR_t;
    CC_EXTENSION typedef union uSFIDAFMR {
        vuint16_t R;
    } SFIDAFMR_t;
    CC_EXTENSION typedef union uNMVR {
        vuint16_t R;
    } NMVR_t;
    CC_EXTENSION typedef union uNMVLR {
        vuint16_t R;
    } NMVLR_t;
    CC_EXTENSION typedef union uT1MTOR {
        vuint16_t R;
    } T1MTOR_t;
    CC_EXTENSION typedef union uTI2CR0 {
        vuint16_t R;
    } TI2CR0_t;
    CC_EXTENSION typedef union uTI2CR1 {
        vuint16_t R;
    } TI2CR1_t;
    CC_EXTENSION typedef union uSSCR {
        vuint16_t R;
    } SSCR_t;
    CC_EXTENSION typedef union uRFSR {
        vuint16_t R;
    } RFSR_t;
    CC_EXTENSION typedef union uRFSIR {
        vuint16_t R;
    } RFSIR_t;
    CC_EXTENSION typedef union uRFARIR {
        vuint16_t R;
    } RFARIR_t;
    CC_EXTENSION typedef union uRFBRIR {
        vuint16_t R;
    } RFBRIR_t;
    CC_EXTENSION typedef union uRFMIDAFVR {
        vuint16_t R;
    } RFMIDAFVR_t;
    CC_EXTENSION typedef union uRFMIAFMR {
        vuint16_t R;
    } RFMIAFMR_t;
    CC_EXTENSION typedef union uRFFIDRFVR {
        vuint16_t R;
    } RFFIDRFVR_t;
    CC_EXTENSION typedef union uRFFIDRFMR {
        vuint16_t R;
    } RFFIDRFMR_t;
    CC_EXTENSION typedef union uLDTXSLAR {
        vuint16_t R;
    } LDTXSLAR_t;
    CC_EXTENSION typedef union uLDTXSLBR {
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
        volatile PADR_t PADR;   /*PE address register *//*10 */
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
#define SRAM_SIZE      0x10000
#define SRAM_END    0x4000FFFF

#define FLASH_START         0x0
#define FLASH_SIZE      0x100000
#define FLASH_END       0xFFFFF
#endif

/* Define instances of modules */
//#define PBRIDGE_A (*( volatile struct PBRIDGE_A_tag *) 0xC3F00000)
#define MPU       (*( volatile struct MPU_tag *)       0xFFF10000)
#define SEMA4     (*( volatile struct SEMA4_tag *)     0xFFF14000)

#define MCM       (*( volatile struct MCM_tag *)       0xFFF40000)
#define EDMA      (*( volatile struct EDMA_tag *)      0xFFF44000)
#define INTC      (*( volatile struct INTC_tag *)      0xFFF48000)

#define EQADC     (*( volatile struct EQADC_tag *)     0xFFF80000)

#define I2C       (*( volatile struct I2C_tag *)       0xFFF88000)

#define DSPI_A    (*( volatile struct DSPI_tag *)      0xFFF90000)
#define DSPI_B    (*( volatile struct DSPI_tag *)      0xFFF94000)
#define DSPI_C    (*( volatile struct DSPI_tag *)      0xFFF98000)
#define DSPI_D    (*( volatile struct DSPI_tag *)      0xFFF9C000)

#define ESCI_A    (*( volatile struct ESCI_tag *)      0xFFFA0000)
#define ESCI_B    (*( volatile struct ESCI_tag *)      0xFFFA4000)
#define ESCI_C    (*( volatile struct ESCI_tag *)      0xFFFA8000)
#define ESCI_D    (*( volatile struct ESCI_tag *)      0xFFFAC000)
#define ESCI_E    (*( volatile struct ESCI_tag *)      0xFFFB0000)
#define ESCI_F    (*( volatile struct ESCI_tag *)      0xFFFB4000)
#define ESCI_G    (*( volatile struct ESCI_tag *)      0xFFFB8000)
#define ESCI_H    (*( volatile struct ESCI_tag *)      0xFFFBC000)

#define CAN_A     (*( volatile struct FLEXCAN_tag *)  0xFFFC0000)
#define CAN_B     (*( volatile struct FLEXCAN_tag *)  0xFFFC4000)
#define CAN_C     (*( volatile struct FLEXCAN_tag *)  0xFFFC8000)
#define CAN_D     (*( volatile struct FLEXCAN_tag *)  0xFFFCC000)
#define CAN_E     (*( volatile struct FLEXCAN_tag *)  0xFFFD0000)
#define CAN_F     (*( volatile struct FLEXCAN_tag *)  0xFFFD4000)
#define FR        (*( volatile struct FR_tag *)       0xFFFD8000)
#define DMAMUX    (*( volatile struct DMAMUX_tag *)   0xFFFDC000)
#define PIT       (*( volatile struct PIT_tag *)      0xFFFE0000)
#define EMIOS     (*( volatile struct EMIOS_tag *)    0xFFFE4000)
#define SIU       (*( volatile struct SIU_tag *)      0xFFFE8000)
#define CRP       (*( volatile struct CRP_tag *)      0xFFFEC000)
#define FMPLL     (*( volatile struct FMPLL_tag *)    0xFFFF0000)
#define EBI       (*( volatile struct EBI_tag *)      0xFFFF4000)
#define FLASH     (*( volatile struct FLASH_tag *)    0xFFFF8000)

#ifdef __MWERKS__
#pragma pop
#endif

#ifdef  __cplusplus
}
#endif
#endif                          /* ifdef _MPC5516_H */
