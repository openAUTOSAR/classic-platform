
/**************************************************************************
 * FILE NAME: $RCSfile: mpc5500_spr.h,v $    COPYRIGHT (c) Freescale 2005 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contain all of the SPR register and bit field definitions    *
 * for MPC5500 products. There are no address assignments for the SPR     *
 * registers. Instead, the bit field structures are only assigned.        *
 * SPR register numbers are also assigned.                                *
 *========================================================================*
 * ORIGINAL AUTHOR: Glenn Jackson                                         *
 * $Log: mpc5500_spr.h,v $
 * Revision 1.5  2006/08/30 09:06:42  r54088
 * L1SCR0, Bit field changed from DPP to DPB to align with documentation.
 *
 * Revision 1.4  2006/08/10 16:08:04  r54088
 * L1CSR0[WAM] & L1CSR0[CORG] Added
 *
 * Revision 1.3  2005/02/22 13:49:32  r47354
 * Made the same as file currently on extranet.
 * Changed _MPC5554_SPR_ to _MPC5500_SPR_.
 * Changed to Freescale copyrigh/discimer.
 *
 * Revision 1.2  2005/02/22 13:06:16  r47354
 * Prepend all Special Purpose Register names with "SPR_" to prevent issues when using this file with mpc5553/4.h
 *
 * Revision 1.1  2005/02/22 10:55:20  r47354
 * Initial revision. Based on mpc5554_spr.h
 *........................................................................*
 * 0.01  G. Jackson   13/Nov/03    Initial version of file for SPR        *
 *                                 registers in the MPC5554.              *
 *                                  Based on SoC version 0.7.             *
 * 1.0   G. Jackson   23/Jan/04    Replaced MASnVAL with MASnCVAL to      *
 *                                  maintain unique function defintions.  *
 * 1.1   G. Jackson   19/Jul/04    #endif note placed after _MPC5554_SPR_ *
 *                                  #ifndef and #define.                  *
 *                                 Changed structures to typedefs that    *
 *                                  would be instantiated by customer     *
 *                                  definition later in code.             *
 * 1.2   G.Jackson    14/Sep/04    Added SPR_ prefixes for unique names.  *
 **************************************************************************/

/* >>>>NOTE! this file describes fixed special purpose registers.  */
/*           Please do not edit it directly!<<<<                   */

#ifndef _MPC5500_SPR_
#define _MPC5500_SPR_
/* This ifndef has a corresponding #endif at the bottom of this file  */
/*  so that it will only be included once.                            */

#include "typedefs.h"

#ifdef  __cplusplus
extern "C" {
#endif

/********************************************/
/* Example instantiation and use:           */
/*  union CRVAL my_cr;                      */
/*  my_cr.B.CR0 = 1;                        */
/*  my_cr.R = 0x10000000                    */
/********************************************/



/****************************************************************************/
/*                   CPU REGISTERS: General Registers                       */
/****************************************************************************/
        union SPR_CRVAL {
            vuint32_t R;
            struct {
                vuint32_t CR0:4;
                vuint32_t CR1:4;
                vuint32_t CR2:4;
                vuint32_t CR3:4;
                vuint32_t CR4:4;
                vuint32_t CR5:4;
                vuint32_t CR6:4;
                vuint32_t CR7:4;
            } B;
        };

        union SPR_LRVAL {
            vuint32_t R;
            struct {
                vuint32_t LINKADDRESS:32;
            } B;
        };

        union SPR_CTRVAL {
            vuint32_t R;
            struct {
                vuint32_t COUNTVALUE:32;
            } B;
        };

        union SPR_XERVAL {
            vuint32_t R;
            struct {
                vuint32_t SO:1;
                vuint32_t OV:1;
                vuint32_t CA:1;
                vuint32_t :22;
                vuint32_t BYTECNT:7;
            } B;
        };


/****************************************************************************/
/*                   CPU REGISTERS: Processor Control Registers             */
/****************************************************************************/
        union SPR_MSRVAL {
            vuint32_t R;
            struct {
                vuint32_t :5;
                vuint32_t UCLE:1;
                vuint32_t SPE:1;
                vuint32_t :6;
                vuint32_t WE:1;
                vuint32_t CE:1;
                vuint32_t :1;
                vuint32_t EE:1;
                vuint32_t PR:1;
                vuint32_t FP:1;
                vuint32_t ME:1;
                vuint32_t FE0:1;
                vuint32_t :1;
                vuint32_t DE:1;
                vuint32_t FE1:1;
                vuint32_t :2;
                vuint32_t IS:1;
                vuint32_t DS:1;
                vuint32_t :4;
            } B;
        };

        union SPR_PIRVAL {
            vuint32_t R;
            struct {
                vuint32_t :24;
                vuint32_t ID:8;
            } B;
        };

        union SPR_PVRVAL {
            vuint32_t R;
            struct {
                vuint32_t MFGID:4;
                vuint32_t :2;
                vuint32_t TYPE:6;
                vuint32_t VER:4;
                vuint32_t MGBUSE:8;
                vuint32_t MJRREV:4;
                vuint32_t MGBID:4;
            } B;
        };

         union SPR_SVRVAL {
            vuint32_t R;
            struct {
                vuint32_t SYSVER:32;
            } B;
        };

        union SPR_HID0VAL {
            vuint32_t R;
            struct {
                vuint32_t EMCP:1;
                vuint32_t :5;
                vuint32_t BPRED:2;
                vuint32_t DOZE:1;
                vuint32_t NAP:1;
                vuint32_t SLEEP:1;
                vuint32_t :3;
                vuint32_t ICR:1;
                vuint32_t NHR:1;
                vuint32_t :1;
                vuint32_t TBEN:1;
                vuint32_t SEL_TBCLK:1;
                vuint32_t DCLREE:1;
                vuint32_t DCLRCE:1;
                vuint32_t CICLRDE:1;
                vuint32_t MCCLRDE:1;
                vuint32_t DAPUEN:1;
                vuint32_t :8;
            } B;
        };

        union SPR_HID1VAL {
            vuint32_t R;
            struct {
                vuint32_t :24;
                vuint32_t ATS:1;
                vuint32_t :7;
            } B;
        };

/****************************************************************************/
/*                   CPU REGISTERS: TIMERS                                  */
/****************************************************************************/

        union SPR_TBLVAL {
            vuint32_t R;
            struct {
                vuint32_t TBLVALUE:32;
            } B;
        };

        union SPR_TBUVAL {
            vuint32_t R;
            struct {
                vuint32_t TBUVALUE:32;
            } B;
        };

        union SPR_TCRVAL {
            vuint32_t R;
            struct {
                vuint32_t WP:2;
                vuint32_t WRC:2;
                vuint32_t WIE:1;
                vuint32_t DIE:1;
                vuint32_t FP:2;
                vuint32_t FIE:1;
                vuint32_t ARE:1;
                vuint32_t :1;
                vuint32_t WPEXT:4;
                vuint32_t FPEXT:4;
                vuint32_t :13;
            } B;
        };

        union SPR_TSRVAL {
            vuint32_t R;
            struct {
                vuint32_t ENW:1;
                vuint32_t WIS:1;
                vuint32_t WRS:2;
                vuint32_t DIS:1;
                vuint32_t FIS:1;
                vuint32_t :26;
            } B;
        };


        union SPR_DECVAL {
            vuint32_t R;
            struct {
                vuint32_t DECVALUE:32;
            } B;
        };

        union SPR_DECARVAL {
            vuint32_t R;
            struct {
                vuint32_t DECARVALUE:32;
            } B;
        };



/****************************************************************************/
/*                   CPU REGISTERS: MMU                                     */
/****************************************************************************/
        union SPR_PID0VAL {
            vuint32_t R;
            struct {
                vuint32_t :24;
                vuint32_t PID:8;
            } B;
        };

        union SPR_MMUCSR0VAL {
            vuint32_t R;
            struct {
                vuint32_t :30;
                vuint32_t TLBCAM_FI:1;
                vuint32_t:1;
            } B;
        };

        union SPR_MMUCFGVAL {
            vuint32_t R;
            struct {
                vuint32_t:17;
                vuint32_t NPIDS:4;
                vuint32_t PIDSIZE:5;
                vuint32_t:2;
                vuint32_t NLTBS:2;
                vuint32_t MAVN:2;
            } B;
        };

        union SPR_TLB0CFGVAL {
            vuint32_t R;
            struct {
                vuint32_t ASSOC:8;
                vuint32_t MINSIZE:4;
                vuint32_t MAXSIZE:4;
                vuint32_t IPROT:1;
                vuint32_t AVAIL:1;
                vuint32_t :2;
                vuint32_t NENTRY:12;
            } B;
        };

        union SPR_TLB1CFGVAL {
            vuint32_t R;
            struct {
                vuint32_t ASSOC:8;
                vuint32_t MINSIZE:4;
                vuint32_t MAXSIZE:4;
                vuint32_t IPROT:1;
                vuint32_t AVAIL:1;
                vuint32_t :2;
                vuint32_t NENTRY:12;
            } B;
        };

        union SPR_MAS0CVAL {
            vuint32_t R;
            struct {
                vuint32_t:2;
                vuint32_t TLBSEL:2;
                vuint32_t:7;
                vuint32_t ESELCAM:5;
                vuint32_t:11;
                vuint32_t NVCAM:5;
            } B;
        };

        union SPR_MAS1CVAL {
            vuint32_t R;
            struct {
                vuint32_t VALID:1;
                vuint32_t IPROT:1;
                vuint32_t:6;
                vuint32_t TID:8;
                vuint32_t:3;
                vuint32_t TS:1;
                vuint32_t TSIZ:4;
                vuint32_t:8;
            } B;
        };

        union SPR_MAS2CVAL {
            vuint32_t R;
            struct {
                vuint32_t EPN:20;  /* Effective Page Number            */
                vuint32_t:7;
                vuint32_t W:1;     /* Write through required;          */
                                   /*  0=write back; 1=write through   */
                vuint32_t I:1;     /* Cache Inhibit; 0=not inhibited   */
                vuint32_t M:1;     /* Memory coherence; 0=not required */
                vuint32_t G:1;     /* Gaurded; 0=not gaurded           */
                vuint32_t E:1;     /* Endianess; 0=Big; 1=Little       */
            } B;
        };

        union SPR_MAS3CVAL {
            vuint32_t R;
            struct {
                vuint32_t RPN:20;   /* Real Page Number              */
                vuint32_t:2;
                vuint32_t U0:1;     /* User bits [0:3]               */
                vuint32_t U1:1;
                vuint32_t U2:1;
                vuint32_t U3:1;
                vuint32_t UX:1;     /* Permission bits               */
                vuint32_t SX:1;
                vuint32_t UW:1;
                vuint32_t SW:1;
                vuint32_t UR:1;
                vuint32_t SR:1;
            } B;
        };

        union SPR_MAS4CVAL {
            vuint32_t R;
            struct {
                vuint32_t:2;
                vuint32_t TLBSELD:2;
                vuint32_t:10;
                vuint32_t TIDSELD:2;
                vuint32_t:4;
                vuint32_t TSIZED:4;
                vuint32_t :3;
                vuint32_t WD:1;
                vuint32_t ID:1;
                vuint32_t MD:1;
                vuint32_t GD:1;
                vuint32_t ED:1;
            } B;
        };

        union SPR_MAS6CVAL {
            vuint32_t R;
            struct {
                vuint32_t:8;
                vuint32_t SPID:8;
                vuint32_t:15;
                vuint32_t SAS:1;
            } B;
        };

/****************************************************************************/
/*                   CPU REGISTERS: CACHE                                   */
/****************************************************************************/
        union SPR_L1CFG0VAL {   /* Read only register */
            vuint32_t R;
            struct {
                vuint32_t CARCH:2;  /* Cache Architecture; 01=Unified      */
                vuint32_t CWPA:1;   /* Cache way partitioning available =1 */
                vuint32_t CFAHA:1;  /* Cache Flush by all avail; 0=not     */
                vuint32_t CFISWA:1; /* Cache Flush Inv by set & way avail=1 */
                vuint32_t :2;
                vuint32_t CBSIZE:2; /* Block Size 00=32 bytes         */
                vuint32_t CREPL:2;  /* Replacement Policy 10=pseudo round robin */
                vuint32_t CLA:1;    /* Line locking APU; 1=avail      */
                vuint32_t CPA:1;    /* Parity available  1=avail      */
                vuint32_t CNWAY:8;  /* Num of ways; 0x03=4way, 0x07=8way */
                vuint32_t CSIZE:11; /* Size; 0x008=8KB, 0x010=16KB,0x020=32KB */
            } B;
        };              /* Read only register */


        union SPR_L1CSR0VAL {
            vuint32_t R;
            struct {
                vuint32_t WID:4;
                vuint32_t WDD:4;
                vuint32_t AWD:1;
                vuint32_t AWDD:1;
                vuint32_t WAM:1;
                vuint32_t CWM:1;
                vuint32_t DPB:1;
                vuint32_t DSB:1;
                vuint32_t DSTR:1;
                vuint32_t CPE:1;
                vuint32_t :5;
                vuint32_t CUL:1;
                vuint32_t CLO:1;
                vuint32_t CLFC:1;
                vuint32_t :3;	
                vuint32_t CORG:1;	
                vuint32_t :1;
                vuint32_t CABT:1;
                vuint32_t CINV:1;
                vuint32_t CE:1;
            } B;
        };

        union SPR_L1FINV0VAL {
            vuint32_t R;
            struct {
                vuint32_t :5;
                vuint32_t CWAY:3;
                vuint32_t :12;
                vuint32_t CSET:7;
                vuint32_t :3;
                vuint32_t CCMD:2;
            } B;
        };

/****************************************************************************/
/*                   CPU REGISTERS: APU                                     */
/****************************************************************************/
        union SPR_SPEFSCRVAL {  /* Status and Control of SPE instructions */
            vuint32_t R;
            struct {
                vuint32_t SOVH:1;
                vuint32_t OVH:1;
                vuint32_t FGH:1;
                vuint32_t FXH:1;
                vuint32_t FINVH:1;
                vuint32_t FDBZH:1;
                vuint32_t FUNFH:1;
                vuint32_t FOVFH:1;
                vuint32_t :2;
                vuint32_t FINXS:1;
                vuint32_t FINVS:1;
                vuint32_t FDBZS:1;
                vuint32_t FUNFS:1;
                vuint32_t FOVFS:1;
                vuint32_t MODE:1;
                vuint32_t SOV:1;
                vuint32_t OV:1;
                vuint32_t FG:1;
                vuint32_t FX:1;
                vuint32_t FINV:1;
                vuint32_t FDBZ:1;
                vuint32_t FUNF:1;
                vuint32_t FOVF:1;
                vuint32_t :1;
                vuint32_t FINXE:1;
                vuint32_t FINVE:1;
                vuint32_t FDBZE:1;
                vuint32_t FUNFE:1;
                vuint32_t FOVFE:1;
                vuint32_t FRMC:2;
            } B;
        };

/****************************************************************************/
/*                   CPU REGISTERS: Exception Handling/Control Registers    */
/****************************************************************************/
        union SPR_SPRGVAL { /* There are [8] entries for this tag */
            vuint32_t R;
            struct {
                vuint32_t SPRDATA:32;
            } B;
        };

        union SPR_USPRG0VAL {
            vuint32_t R;
            struct {
                vuint32_t USPR0DATA:32;
            } B;
        };

       union SPR_SRR0VAL {
            vuint32_t R;
            struct {
                vuint32_t NXTADDR:32;
            } B;
        };

       union SPR_SRR1VAL {
            vuint32_t R;
            struct {
                vuint32_t MSRSTATE:32;
            } B;
        };

        union SPR_CSRR0VAL {
            vuint32_t R;
            struct {
                vuint32_t CRITNXTADDR:32;
            } B;
        };

       union SPR_CSRR1VAL {
            vuint32_t R;
            struct {
                vuint32_t CRITMSRSTATE:32;
            } B;
        };

        union SPR_DSRR0VAL {
            vuint32_t R;
            struct {
                vuint32_t DBGNXTADDR:32;
            } B;
        };

       union SPR_DSRR1VAL {
            vuint32_t R;
            struct {
                vuint32_t DBGMSRSTATE:32;
            } B;
        };

       union SPR_DEARVAL {
            vuint32_t R;
            struct {
                vuint32_t DATEFADDR:16;
                vuint32_t :16;
            } B;
        };

        union SPR_ESRVAL {
            vuint32_t R;
            struct {
                vuint32_t :4;
                vuint32_t PIL:1;
                vuint32_t PPR:1;
                vuint32_t PTR:1;
                vuint32_t FP:1;
                vuint32_t ST:1;
                vuint32_t :1;
                vuint32_t DLK:1;
                vuint32_t ILK:1;
                vuint32_t AP:1;
                vuint32_t PUO:1;
                vuint32_t BO:1;
                vuint32_t PIE:1;
                vuint32_t :8;
                vuint32_t SPE:1;
                vuint32_t :6;
                vuint32_t XTE:1;
            } B;
        };


        union SPR_MCSRVAL {
            vuint32_t R;
            struct {
                vuint32_t MCP:1;
                vuint32_t :1;
                vuint32_t CP_PERR:1;
                vuint32_t CPERR:1;
                vuint32_t EXCP_ERR:1;
                vuint32_t :24;
                vuint32_t BUS_WRERR:1;
                vuint32_t :2;
            } B;
        };

       union SPR_IVPRVAL {
            vuint32_t R;
            struct {
                vuint32_t VECBASE:16;
                vuint32_t :16;
            } B;
        };

 /* Note: IVOR0 is not supported by the MPC5554 */
        union SPR_IVORVAL {  /* There are [16] entries for this tag */
            vuint32_t R;
            struct {
                vuint32_t :16;
                vuint32_t VECOFFSET:12;
                vuint32_t :4;
            } B;
        };

        union SPR_IVOR32VAL {
            vuint32_t R;
            struct {
                vuint32_t :16;
                vuint32_t VECOFFSET:12;
                vuint32_t :4;
            } B;
        };

        union SPR_IVOR33VAL {
            vuint32_t R;
            struct {
                vuint32_t :16;
                vuint32_t VECOFFSET:12;
                vuint32_t :4;
            } B;
        };

        union SPR_IVOR34VAL {
            vuint32_t R;
            struct {
                vuint32_t :16;
                vuint32_t VECOFFSET:12;
                vuint32_t :4;
            } B;
        };

/****************************************************************************/
/*                   CPU REGISTERS: DEBUG                                     */
/****************************************************************************/
        union {
            vuint32_t R;
            struct SPR_DBCR0VAL {
                vuint32_t EDM:1;
                vuint32_t IDM:1;
                vuint32_t RST:2;
                vuint32_t ICMP:1;
                vuint32_t BRT:1;
                vuint32_t IRPT:1;
                vuint32_t TRAP:1;
                vuint32_t IAC1:1;
                vuint32_t IAC2:1;
                vuint32_t IAC3:1;
                vuint32_t IAC4:1;
                vuint32_t DAC1:2;
                vuint32_t DAC2:2;
                vuint32_t RET:1;
                vuint32_t :4;
                vuint32_t DEVT1:1;
                vuint32_t DEVT2:1;
                vuint32_t DCNT1:1;
                vuint32_t DCNT2:1;
                vuint32_t CIRPT:1;
                vuint32_t CRET:1;
                vuint32_t :4;
                vuint32_t FT:1;
            } B;
        };

        union SPR_DBCR1VAL {
            vuint32_t R;
            struct {
                vuint32_t IAC1US:2;
                vuint32_t IAC1ER:2;
                vuint32_t IAC2US:2;
                vuint32_t IAC2ER:2;
                vuint32_t IAC12M:2;
                vuint32_t :6;
                vuint32_t IAC3US:2;
                vuint32_t IAC3ER:2;
                vuint32_t IAC4US:2;
                vuint32_t IAC4ER:2;
                vuint32_t IAC34M:2;
                vuint32_t :6;
            } B;
        };

        union SPR_DBCR2VAL {
            vuint32_t R;
            struct {
                vuint32_t DIAC1US:2;
                vuint32_t DIAC1ER:2;
                vuint32_t DIAC2US:2;
                vuint32_t DIAC2ER:2;
                vuint32_t DIAC12M:2;
                vuint32_t DAC1LNK:2;
                vuint32_t DAC2LNK:2;
                vuint32_t :20;
            } B;
        };

         union SPR_DBCR3VAL {
            vuint32_t R;
            struct {
                vuint32_t DEVT1C1:1;
                vuint32_t DEVT2C1:1;
                vuint32_t ICMPC1:1;
                vuint32_t IAC1C1:1;
                vuint32_t IAC2C1:1;
                vuint32_t IAC3C1:1;
                vuint32_t IAC4C1:1;
                vuint32_t DAC1RC1:1;
                vuint32_t DAC1WC1:1;
                vuint32_t DAC2RC1:1;
                vuint32_t DAC2WC1:1;
                vuint32_t IRPTC1:1;
                vuint32_t RETC1:1;
                vuint32_t DEVT1C2:1;
                vuint32_t DEVT2C2:1;
                vuint32_t ICMPC2:1;
                vuint32_t IAC1C2:1;
                vuint32_t IAC2C2:1;
                vuint32_t IAC3C2:1;
                vuint32_t IAC4C2:1;
                vuint32_t DAC1RC2:1;
                vuint32_t DAC1WC2:1;
                vuint32_t DAC2RC2:1;
                vuint32_t DAC2WC2:1;
                vuint32_t DEVT1T1:1;
                vuint32_t DEVT2T1:1;
                vuint32_t IAC1T1:1;
                vuint32_t IAC3T1:1;
                vuint32_t DAC1RT1:1;
                vuint32_t DAC1WT1:1;
                vuint32_t CNT2T1:1;
                vuint32_t CONFIG:1;
            } B;
        };

        union SPR_DBSRVAL {
            vuint32_t R;
            struct {
                vuint32_t IDE:1;
                vuint32_t UDE:1;
                vuint32_t MRR:2;
                vuint32_t ICMP:1;
                vuint32_t BRT:1;
                vuint32_t IRPT:1;
                vuint32_t TRAP:1;
                vuint32_t IAC1:1;
                vuint32_t IAC2:1;
                vuint32_t IAC3:1;
                vuint32_t IAC4:1;
                vuint32_t DAC1R:1;
                vuint32_t DAC1W:1;
                vuint32_t DAC2R:1;
                vuint32_t DAC2W:1;
                vuint32_t RET:1;
                vuint32_t :4;
                vuint32_t DEVT1:1;
                vuint32_t DEVT2:1;
                vuint32_t DCNT1:1;
                vuint32_t DCNT2:1;
                vuint32_t CIRPT:1;
                vuint32_t CRET:1;
                vuint32_t :4;
                vuint32_t CNT1RG:1;
            } B;
        };

        union SPR_DBCNTVAL {
            vuint32_t R;
            struct {
                vuint32_t CNT1:16;
                vuint32_t CNT2:16;
           } B;
        };

        union SPR_IAC1VAL {
            vuint32_t R;
            struct {
                vuint32_t INSTADDR:30;
                vuint32_t :2;
           } B;
        };

        union SPR_IAC2VAL {
            vuint32_t R;
            struct {
                vuint32_t INSTADDR:30;
                vuint32_t :2;
           } B;
        };

        union SPR_IAC3VAL {
            vuint32_t R;
            struct {
                vuint32_t INSTADDR:30;
                vuint32_t :2;
           } B;
        };

        union SPR_IAC4VAL {
            vuint32_t R;
            struct {
                vuint32_t INSTADDR:30;
                vuint32_t :2;
           } B;
        };


        union SPR_DAC1VAL {
            vuint32_t R;
            struct {
                vuint32_t DATTADDR:32;
           } B;
        };

        union SPR_DAC2VAL {
            vuint32_t R;
            struct {
                vuint32_t DATTADDR:32;
           } B;
        };



/*****************************************************/
/* Define instances of modules                       */
/*  with special register numbers                    */
/*****************************************************/

// The CR register does not have an SPR#
// The GPR registers do not have an SPR#
// The MSR register does not have an SPR#

#define SPR_LR          8
#define SPR_CTR         9
#define SPR_XER         1

#define SPR_PIR       286
#define SPR_PVR       287
#define SPR_SVR      1023
#define SPR_HID0     1008
#define SPR_HID1     1009

#define SPR_TBL       284
#define SPR_TBU       285
#define SPR_TCR       340
#define SPR_TSR       336
#define SPR_DEC        22
#define SPR_DECAR      54

#define SPR_PID0       48
#define SPR_MMUCSR0  1012
#define SPR_MMUCFG   1015
#define SPR_TLB0CFG   688
#define SPR_TLB1CFG   689
#define SPR_MAS0      624
#define SPR_MAS1      625
#define SPR_MAS2      626
#define SPR_MAS3      627
#define SPR_MAS4      628
#define SPR_MAS6      630

#define SPR_L1CFG0    515
#define SPR_L1CSR0   1010
#define SPR_L1FINV0  1016

#define SPR_SPEFSCR   512

#define SPR_SPRG0     272
#define SPR_SPRG1     273
#define SPR_SPRG2     274
#define SPR_SPRG3     275
#define SPR_SPRG4     276
#define SPR_SPRG5     277
#define SPR_SPRG6     278
#define SPR_SPRG7     279
#define SPR_USPRG0    256
#define SPR_SRR0       26
#define SPR_SRR1       27
#define SPR_CSRR0      58
#define SPR_CSRR1      59
#define SPR_DSRR0     574
#define SPR_DSRR1     575
#define SPR_ESR        62
#define SPR_MCSR      572
#define SPR_DEAR       61
#define SPR_IVPR       63
#define SPR_IVOR0     400
#define SPR_IVOR1     401
#define SPR_IVOR2     402
#define SPR_IVOR3     403
#define SPR_IVOR4     404
#define SPR_IVOR5     405
#define SPR_IVOR6     406
#define SPR_IVOR7     407
#define SPR_IVOR8     408
#define SPR_IVOR9     409
#define SPR_IVOR10    410
#define SPR_IVOR11    411
#define SPR_IVOR12    412
#define SPR_IVOR13    413
#define SPR_IVOR14    414
#define SPR_IVOR15    415
#define SPR_IVOR32    528
#define SPR_IVOR33    529
#define SPR_IVOR34    530

#define SPR_DBCR0     308
#define SPR_DBCR1     309
#define SPR_DBCR2     310
#define SPR_DBCR3     561
#define SPR_DBSR      304
#define SPR_DBCNT     562
#define SPR_IAC1      312
#define SPR_IAC2      313
#define SPR_IAC3      314
#define SPR_IAC4      315
#define SPR_DAC1      316
#define SPR_DAC2      317



#ifdef  __cplusplus
}
#endif

#endif  /* ends inclusion of #ifndef __MPC5500_SPR_ for one instantiation */

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


