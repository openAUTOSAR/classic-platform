/*
 * core_cr4.h
 *
 *  Created on: 12 okt 2010
 *      Author: maek
 */

#ifndef CORE_CR4_H_
#define CORE_CR4_H_

#define     __I     volatile const            /*!< defines 'read only' permissions      */
#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */


typedef volatile struct vimBase
{
    unsigned              : 24U;    /* 0x0000        */
    unsigned      IRQIVEC :  8U;    /* 0x0000        */
    unsigned              : 24U;    /* 0x0004        */
    unsigned      FIQIVEC :  8U;    /* 0x0004        */
    unsigned      : 32U;            /* 0x0008        */
    unsigned      : 32U;            /* 0x000C        */
    unsigned      FIRQPR0;          /* 0x0010        */
    unsigned      FIRQPR1;          /* 0x0014        */
    unsigned      FIRQPR2;          /* 0x0018        */
    unsigned      FIRQPR3;          /* 0x001C        */
    unsigned      INTREQ0;          /* 0x0020        */
    unsigned      INTREQ1;          /* 0x0024        */
    unsigned      INTREQ2;          /* 0x0028        */
    unsigned      INTREQ3;          /* 0x002C        */
    unsigned      REQMASKSET0;      /* 0x0030        */
    unsigned      REQMASKSET1;      /* 0x0034        */
    unsigned      REQMASKSET2;      /* 0x0038        */
    unsigned      REQMASKSET3;      /* 0x003C        */
    unsigned      REQMASKCLR0;      /* 0x0040        */
    unsigned      REQMASKCLR1;      /* 0x0044        */
    unsigned      REQMASKCLR2;      /* 0x0048        */
    unsigned      REQMASKCLR3;      /* 0x004C        */
    unsigned      WAKEMASKSET0;     /* 0x0050        */
    unsigned      WAKEMASKSET1;     /* 0x0054        */
    unsigned      WAKEMASKSET2;     /* 0x0058        */
    unsigned      WAKEMASKSET3;     /* 0x005C        */
    unsigned      WAKEMASKCLR0;     /* 0x0060        */
    unsigned      WAKEMASKCLR1;     /* 0x0064        */
    unsigned      WAKEMASKCLR2;     /* 0x0068        */
    unsigned      WAKEMASKCLR3;     /* 0x006C        */
    unsigned      IRQVECREG;        /* 0x0070        */
    unsigned      FIQVECREQ;        /* 0x0074        */
    unsigned                 :  9U; /* 0x0078        */
    unsigned      CAPEVTSRC1 :  7U; /* 0x0078        */
    unsigned                 :  9U; /* 0x0078        */
    unsigned      CAPEVTSRC0 :  7U; /* 0x0078        */
    unsigned      : 32U;            /* 0x007C        */
    unsigned char CHANMAP[64U];     /* 0x0080-0x017C */
} vimBASE_t;

#define vimREG ((vimBASE_t *)0xFFFFFE00U)



typedef volatile struct rtiBase
{
    unsigned GCTRL;          /**< 0x0000: Global Control Register   */
    unsigned TBCTRL;         /**< 0x0004: Timebase Control Register */
    unsigned CAPCTRL;        /**< 0x0008: Capture Control Register  */
    unsigned COMPCTRL;       /**< 0x000C: Compare Control Register  */
    struct
    {
        unsigned FRCx;       /**< 0x0010,0x0030: Free Running Counter x Register         */
        unsigned UCx;        /**< 0x0014,0x0034: Up Counter x Register                   */
        unsigned CPUCx;      /**< 0x0018,0x0038: Compare Up Counter x Register           */
        unsigned : 32;       /**< 0x001C,0x003C: Reserved                                */
        unsigned CAFRCx;     /**< 0x0020,0x0040: Capture Free Running Counter x Register */
        unsigned CAUCx;      /**< 0x0024,0x0044: Capture Up Counter x Register           */
        unsigned : 32;       /**< 0x0028,0x0048: Reserved                                */
        unsigned : 32;       /**< 0x002C,0x004C: Reserved                                */
    } CNT[2U];               /**< Counter x selection:
                                    - 0: Counter 0
                                    - 1: Counter 1                                       */
    struct
    {
        unsigned COMPx;      /**< 0x0050,0x0058,0x0060,0x0068: Compare x Register        */
        unsigned UDCPx;      /**< 0x0054,0x005C,0x0064,0x006C: Update Compare x Register */
    } CMP[4U];               /**< Compare x selection:
                                    - 0: Compare 0
                                    - 1: Compare 1
                                    - 2: Compare 2
                                    - 3: Compare 3                                       */
    unsigned TBLCOMP;        /**< 0x0070: External Clock Timebase Low Compare Register   */
    unsigned TBHCOMP;        /**< 0x0074: External Clock Timebase High Compare Register  */
    unsigned : 32;           /**< 0x0078: Reserved                                       */
    unsigned : 32;           /**< 0x007C: Reserved                                       */
    unsigned SETINT;         /**< 0x0080: Set/Status Interrupt Register                  */
    unsigned CLEARINT;       /**< 0x0084: Clear/Status Interrupt Register                */
    unsigned INTFLAG;        /**< 0x008C: Interrupt Flag Register                        */
} rtiBASE_t;

/** @def rtiREG1
*   @brief RTI1 Register Frame Pointer
*
*   This pointer is used by the RTI driver to access the RTI1 registers.
*/
#define rtiREG1 ((rtiBASE_t *)0xFFFFFC00)


enum systemClockSource
{
    SYS_OSC      = 0, /**< Alias for oscillator clock Source                */
    SYS_PLL      = 1, /**< Alias for Pll clock Source                       */
    SYS_O32      = 2, /**< Alias for 32 kHz oscillator clock Source         */
    SYS_EXTERNAL = 3, /**< Alias for external clock Source                  */
    SYS_LPO_LOW  = 4, /**< Alias for low power oscillator low clock Source  */
    SYS_LPO_HIGH = 5, /**< Alias for low power oscillator high clock Source */
    SYS_FR_PLL   = 6, /**< Alias for flexray pll clock Source               */
    SYS_VCLK     = 9  /**< Alias for synchronous VCLK1 clock Source         */
};

typedef volatile struct systemBase1
{
    unsigned SYSPC1;                 /* 0x0000 */
    unsigned SYSPC2;                 /* 0x0004 */
    unsigned SYSPC3;                 /* 0x0008 */
    unsigned SYSPC4;                 /* 0x000C */
    unsigned SYSPC5;                 /* 0x0010 */
    unsigned SYSPC6;                 /* 0x0014 */
    unsigned SYSPC7;                 /* 0x0018 */
    unsigned SYSPC8;                 /* 0x001C */
    unsigned SYSPC9;                 /* 0x0020 */
    unsigned SSWPLL1;                /* 0x0024 */
    unsigned SSWPLL2;                /* 0x0028 */
    unsigned SSWPLL3;                /* 0x002C */
    unsigned CSDIS;                  /* 0x0030 */
    unsigned CSDISSET;               /* 0x0034 */
    unsigned CSDISCLR;               /* 0x0038 */
    unsigned CSDDIS;                 /* 0x003C */
    unsigned CSDDISSET;              /* 0x0040 */
    unsigned CSDDISCLR;              /* 0x0044 */
    unsigned GHVSRC;                 /* 0x0048 */
    unsigned VCLKASRC;               /* 0x004C */
    unsigned RCLKSRC;                /* 0x0050 */
    unsigned CSVSTAT;                /* 0x0054 */
    unsigned MSTGCR;                 /* 0x0058 */
    unsigned MINITGCR;               /* 0x005C */
    unsigned MSINENA;                /* 0x0060 */
    unsigned MSTFAIL;                /* 0x0064 */
    unsigned MSTCGSTAT;              /* 0x0068 */
    unsigned MINISTAT;               /* 0x006C */
    unsigned PLLCTL1;                /* 0x0070 */
    unsigned PLLCTL2;                /* 0x0074 */
    unsigned UERFLAG;                /* 0x0078 */
    unsigned DIEIDL;                 /* 0x007C */
    unsigned DIEIDH;                 /* 0x0080 */
    unsigned VRCTL;                  /* 0x0084 */
    unsigned LPOMONCTL;              /* 0x0088 */
    unsigned CLKTEST;                /* 0x008C */
    unsigned DFTCTRLREG1;            /* 0x0090 */
    unsigned DFTCTRLREG2;            /* 0x0094 */
    unsigned : 32U;                  /* 0x0098 */
    unsigned : 32U;                  /* 0x009C */
    unsigned GPREG1;                 /* 0x00A0 */
    unsigned BTRMSEL;                /* 0x00A4 */
    unsigned IMPFASTS;               /* 0x00A8 */
    unsigned IMPFTADD;               /* 0x00AC */
    unsigned SSISR1;                 /* 0x00B0 */
    unsigned SSISR2;                 /* 0x00B4 */
    unsigned SSISR3;                 /* 0x00B8 */
    unsigned SSISR4;                 /* 0x00BC */
    unsigned RAMGCR;                 /* 0x00C0 */
    unsigned BMMCR1;                 /* 0x00C4 */
    unsigned BMMCR2;                 /* 0x00C8 */
    unsigned MMUGCR;                 /* 0x00CC */
#ifdef _little_endian__
    unsigned        : 8U;            /* 0x00D0 */
    unsigned PENA   : 1U;            /* 0x00D0 */
    unsigned        : 7U;            /* 0x00D0 */
    unsigned VCLKR  : 4U;            /* 0x00D0 */
    unsigned        : 4U;            /* 0x00D0 */
    unsigned VCLK2R : 4U;            /* 0x00D0 */
    unsigned        : 4U;            /* 0x00D0 */
#else
    unsigned        : 4U;            /* 0x00D0 */
    unsigned VCLK2R : 4U;            /* 0x00D0 */
    unsigned        : 4U;            /* 0x00D0 */
    unsigned VCLKR  : 4U;            /* 0x00D0 */
    unsigned        : 7U;            /* 0x00D0 */
    unsigned PENA   : 1U;            /* 0x00D0 */
    unsigned        : 8U;            /* 0x00D0 */
#endif
    unsigned : 32U;                  /* 0x00D4 */
    unsigned DSPGCR;                 /* 0x00D8 */
    unsigned DEVCR1;                 /* 0x00DC */
    unsigned SYSECR;                 /* 0x00E0 */
    unsigned SYSESR;                 /* 0x00E4 */
    unsigned ITIFLAG;                /* 0x00E8 */
    unsigned GBLSTAT;                /* 0x00EC */
    unsigned DEV;                    /* 0x00F0 */
    unsigned SSIVEC;                 /* 0x00F4 */
    unsigned SSIF;                   /* 0x00F8 */
} systemBASE1_t;


/** @def systemREG1
*   @brief System Register Frame 1 Pointer
*
*   This pointer is used by the system driver to access the system frame 1 registers.
*/
#define systemREG1 ((systemBASE1_t *)0xFFFFFF00U)


/** @def PRE1
*   @brief Alias name for RTI1CLK PRE clock source
*
*   This is an alias name for the RTI1CLK pre clock source.
*   This can be either:
*     - Oscillator
*     - Pll
*     - 32 kHz Oscillator
*     - External
*     - Low Power Oscillator Low
*     - Low Power Oscillator High
*     - Flexray Pll
*/
#define PRE1 SYS_PLL

/** @def PRE2
*   @brief Alias name for RTI2CLK pre clock source
*
*   This is an alias name for the RTI2CLK pre clock source.
*   This can be either:
*     - Oscillator
*     - Pll
*     - 32 kHz Oscillator
*     - External
*     - Low Power Oscillator Low
*     - Low Power Oscillator High
*     - Flexray Pll
*/
#define PRE2 SYS_PLL

typedef volatile struct systemBase2
{
    unsigned PLLCTL3;        /* 0x0000 */
    unsigned : 32U;          /* 0x0004 */
    unsigned STCCLKDIV;      /* 0x0008 */
    unsigned CLKHB_GLBREG;   /* 0x000C */
    unsigned CLKHB_RTIDREG;  /* 0x0010 */
    unsigned HBCD_STAT;      /* 0x0014 */
    unsigned : 32U;          /* 0x0018 */
    unsigned : 32U;          /* 0x001C */
    unsigned CLKTRMI1;       /* 0x0020 */
    unsigned ECPCNTRL0;      /* 0x0024 */
    unsigned ECPCNTRL1;      /* 0x0028 */
    unsigned ECPCNTRL2;      /* 0x002C */
    unsigned ECPCNTRL3;      /* 0x0030 */
} systemBASE2_t;


/** @def systemREG2
*   @brief System Register Frame 2 Pointer
*
*   This pointer is used by the system driver to access the system frame 2 registers.
*/
#define systemREG2 ((systemBASE2_t *)0xFFFFE100U)

typedef volatile struct pcrBase
{
    unsigned PMPROTSET0;    /* 0x0000 */
    unsigned PMPROTSET1;    /* 0x0004 */
    unsigned : 32U;         /* 0x0008 */
    unsigned : 32U;         /* 0x000C */
    unsigned PMPROTCLR0;    /* 0x0010 */
    unsigned PMPROTCLR1;    /* 0x0014 */
    unsigned : 32U;         /* 0x0018 */
    unsigned : 32U;         /* 0x001C */
    unsigned PPROTSET0;     /* 0x0020 */
    unsigned PPROTSET1;     /* 0x0024 */
    unsigned PPROTSET2;     /* 0x0028 */
    unsigned PPROTSET3;     /* 0x002C */
    unsigned : 32U;         /* 0x0030 */
    unsigned : 32U;         /* 0x0034 */
    unsigned : 32U;         /* 0x0038 */
    unsigned : 32U;         /* 0x003C */
    unsigned PPROTCLR0;     /* 0x0040 */
    unsigned PPROTCLR1;     /* 0x0044 */
    unsigned PPROTCLR2;     /* 0x0048 */
    unsigned PPROTCLR3;     /* 0x004C */
    unsigned : 32U;         /* 0x0050 */
    unsigned : 32U;         /* 0x0054 */
    unsigned : 32U;         /* 0x0058 */
    unsigned : 32U;         /* 0x005C */
    unsigned PCSPWRDWNSET0; /* 0x0060 */
    unsigned PCSPWRDWNSET1; /* 0x0064 */
    unsigned : 32U;         /* 0x0068 */
    unsigned : 32U;         /* 0x006C */
    unsigned PCSPWRDWNCLR0; /* 0x0070 */
    unsigned PCSPWRDWNCLR1; /* 0x0074 */
    unsigned : 32U;         /* 0x0078 */
    unsigned : 32U;         /* 0x007C */
    unsigned PSPWRDWNSET0;  /* 0x0080 */
    unsigned PSPWRDWNSET1;  /* 0x0084 */
    unsigned PSPWRDWNSET2;  /* 0x0088 */
    unsigned PSPWRDWNSET3;  /* 0x008C */
    unsigned : 32U;         /* 0x0090 */
    unsigned : 32U;         /* 0x0094 */
    unsigned : 32U;         /* 0x0098 */
    unsigned : 32U;         /* 0x009C */
    unsigned PSPWRDWNCLR0;  /* 0x00A0 */
    unsigned PSPWRDWNCLR1;  /* 0x00A4 */
    unsigned PSPWRDWNCLR2;  /* 0x00A8 */
    unsigned PSPWRDWNCLR3;  /* 0x00AC */
} pcrBASE_t;

/** @def pcrREG
*   @brief Pcr Register Frame Pointer
*
*   This pointer is used by the system driver to access the Pcr registers.
*/
#define pcrREG ((pcrBASE_t *)0xFFFFE000U)


typedef struct
{
  __IO uint32_t CTRL;                         /*!< SysTick Control and Status Register */
  __IO uint32_t LOAD;                         /*!< SysTick Reload Value Register       */
  __IO uint32_t VAL;                          /*!< SysTick Current Value Register      */
  __I  uint32_t CALIB;                        /*!< SysTick Calibration Register        */
} SysTick_Type;

static inline void __disable_irq() {
  __asm volatile("CPSID if");
}
static inline void __enable_irq() {
	__asm volatile("CPSIE if");

	/*
	__asm volatile("MRS R1, CPSR");
	__asm volatile("BIC R1, R1, #0x80");
	__asm volatile("MSR CPSR, R1");

	__asm volatile("MRC p15 ,#0 ,R1 ,c1 ,c0 ,#0");
	__asm volatile("ORR R1 ,R1 ,#0x01000000"); //Mask 0-31 bits except bit 24 in Sys ; Ctrl Reg of CORTEX-R4
	__asm volatile("MCR p15 ,#0 ,R1 ,c1 ,c0 ,#0"); //; Enable bit 24
	//__asm volatile("SVC #1");
	*/

}




#endif /* CORE_CR4_H_ */
