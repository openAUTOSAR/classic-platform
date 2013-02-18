/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


#ifndef MPC55XX_H_
#define MPC55XX_H_


#if defined(CFG_MPC5554)
#include "mpc5554.h"
#elif defined(CFG_MPC5516) || defined(MPC5517)
#include "mpc5516.h"
#elif defined(CFG_MPC5567)
#include "mpc5567.h"
#elif defined(CFG_MPC563XM)
#include "MPC5634M_MLQB80.h"
#elif defined(CFG_MPC5604B)
#include "MPC5604B_0M27V_0102.h"
#elif defined(CFG_MPC5606B)
#include "MPC5606B.h"
#elif defined(CFG_MPC5606S)
#include "mpc5606s.h"
#elif defined(CFG_MPC5668)
#include "mpc5668.h"
#elif defined(CFG_MPC5604P)
#include "560xP_HEADER_v1_10_SBCHM.h"
#else
#error NO MCU SELECTED!!!!
#endif

/* Harmonization */
typedef struct EDMA_TCD_STD_tag Dma_TcdType;

/*
 * ECC  module
 */
#define ECSM_BASE 	0xfff40000
#define ECSM_ECR	0x43		/* 8-bit */
#define ECSM_ESR    0x47		/* 8-bit */
#define ECSM_EEGR	0x4a		/* 16-bit */

#define ESR_R1BC	0x20
#define ESR_RNCE	0x02
#define ESR_F1BC	0x10
#define ESR_FNCE	0x01

// MPC5567
#if defined(CFG_MPC5567) || defined(CFG_MPC5516)
#define ESR_ERNCR 	0x02 	// Ram non-correctable
#define ESR_EFNCR 	0x01    // Flash non-correctable
#endif


#endif /* MPC55XX_H_ */
