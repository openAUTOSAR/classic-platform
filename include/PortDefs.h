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

#ifndef PORTDEFS_H_
#define PORTDEFS_H_

#if defined(CFG_MPC5516)
#include "mpc5516_portdefs.h"
#elif defined(CFG_MPC5604B)
#include "mpc5604b_portdefs.h"
#elif defined(CFG_MPC5567)
#include "mpc5567_portdefs.h"
#elif defined(CFG_MPC5744P)
#include "mpc5744p_portdefs.h"
#else
#error "No PortDefinition file defined for the CPU."
#endif

#endif /*PORTDEFS_H_*/
