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
#elif defined(CFG_MPC5633)
#include "mpc563m.h"
#elif defined(CFG_MPC5604B)
#include "MPC5604B_0M27V_0102.h"
#elif defined(CFG_MPC5606B)
#include "MPC5606B.h"
#elif defined(CFG_MPC5606S)
#include "mpc5606s.h"
#elif defined(CFG_MPC5668)
#include "mpc5668.h"
#else
#error NO MCU SELECTED!!!!
#endif

/* Harmonization */
typedef struct EDMA_TCD_STD_tag Dma_TcdType;

#endif /* MPC55XX_H_ */
