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


#ifndef KERNEL_H_
#define KERNEL_H_

#include "Os.h"
#include "application.h"
#include "task_i.h"
#include "resource_i.h"
#include "counter_i.h"
#include "alarm_i.h"
#include "sched_table_i.h"
#if (OS_NUM_CORES > 1)
#include "spinlock_i.h"
#endif
#include "isr.h"
#include "Mcu.h"

#endif /* KERNEL_H_ */
