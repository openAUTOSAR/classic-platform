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


#ifndef MPC55XX_H_
#define MPC55XX_H_

#include "Arc_Types.h"

#if defined(CFG_MPC5554)
#include "mpc5554.h"
#elif defined(CFG_MPC5777C)
#include "MPC5777C.h"
#elif defined(CFG_MPC5516) || defined(CFG_MPC5517)
#include "mpc5516.h"
#elif defined(CFG_MPC5567)
#include "mpc5567.h"
#elif defined(CFG_MPC563XM)
#include "MPC5634M_MLQB80.h"
#elif defined(CFG_MPC5604B)
#include "MPC5604B_0M27V_0102.h"
#elif defined(CFG_SPC560B54)
#include "MPC5607B.h"
#elif defined(CFG_MPC5607B)
#include "MPC5607B.h"
#elif defined(CFG_MPC5606B)
#include "MPC5606B.h"
#elif defined(CFG_MPC5606S)
#include "mpc5606s.h"
#elif defined(CFG_MPC5668)
#include "mpc5668.h"
#elif defined(CFG_MPC5604P)
#include "560xP_HEADER_v1_10_SBCHM.h"
#elif defined(CFG_MPC5744P)
#include "mpc5744P.h"
#elif defined(CFG_MPC5746C)
#include "MPC5746C.h"
#elif defined(CFG_MPC5748G) || defined(CFG_MPC5747C)
#include "mpc5748g.h"
#elif defined(CFG_MPC5777M)
#include "MPC5777M.h"
#elif defined(CFG_MPC5643L)
#include "MPC5643L.h"
#elif defined(CFG_SPC56XL70)
#include "SPC56XL70.h"
#elif defined(CFG_MPC5645S)
#include "MPC5645S.h"
#elif defined(CFG_MPC5646B)
#include "MPC564xBC.h"
#elif defined(CFG_MPC5644A)
#include "MPC5644A.h"
#else
#error NO MCU SELECTED!!!!
#endif

/* Harmonization */
typedef struct EDMA_TCD_STD_tag Dma_TcdType;

/* Exception flags  */
#define EXC_NOT_HANDLED	        1UL
#define EXC_HANDLED		        2UL
#define EXC_ADJUST_ADDR	        4UL


/* CRP */
#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
#define CRP_BASE 			(0xFFFEC000ul)

#define CRP_CLKSRC			(CRP_BASE+0x0)
#define CRP_RTCSC			(CRP_BASE+0x10)
#define CRP_RTCCNT			(CRP_BASE+0x14)
/* 40--4F differs ALOT */
#if (CFG_MPC5516)
#define CRP_WKSE			(CRP_BASE+0x44)
#endif
#define CRP_Z1VEC			(CRP_BASE+0x50)
#define CRP_Z6VEC			(CRP_BASE+0x50)
#define CRP_Z0VEC			(CRP_BASE+0x54)
#define CRP_RECPTR			(CRP_BASE+0x58)
#define CRP_PSCR			(CRP_BASE+0x60)
#endif


#define WKSE_WKCLKSEL_16MHZ_IRC		(1<<0)

#define xVEC_xVEC(_x)
#define PSCR_SLEEP			0x00008000ul
#define PSCR_SLP12EN 		0x00000800ul
#define PCSR_RAMSEL(_x)		((_x)<<8)
#define xVEC_VLE			0x00000001ul
#define xVEC_xRST			0x00000002ul

#define RECPTR_FASTREC		0x00000002ul

#if defined(CFG_MPC5516)
#define PFCR_LBCFG(x)   (x<<(31-3))
#define PFCR_ARB        (1<<(31-4))
#define PFCR_PRI        (1<<(31-5))
#define PFCR_M0PFE      (1<<31-15))
#define PFCR_M1PFE      (1<<31-14))
#define PFCR_M2PFE      (1<<31-13))
#define PFCR_M3PFE      (1<<31-12))
#define PFCR_M4PFE      (1<<31-11))
#define PFCR_APC(x)	    (x<<(31-18))
#define PFCR_WWSC(x)    (x<<(31-20))
#define PFCR_RWSC(x)    (x<<(31-23))
#define PFCR_DPFEN      (1<<(31-25))
#define PFCR_IPFEN      (1<<(31-27))
#define PFCR_PFLIM(x)   (1<<(31-30))
#define PFCR_BFEN       (1<<(31-31))
#endif


// MPC5567
#if defined(CFG_MPC5567) || defined(CFG_MPC5516)
#define ESR_ERNCR 	0x02 	// Ram non-correctable
#define ESR_EFNCR 	0x01    // Flash non-correctable
#endif

/* Modes */
#define MODE_DRUN       3u
#define MODE_RUN0       4u
#define MODE_STANDBY    13u

#endif /* MPC55XX_H_ */
