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








/*
 * config.h
 *
 *  Created on: 2009-jul-08
 *      Author: mahi
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/* For makefile's only (used to get the directory search paths right) */
#define ARCH        mpc55xx
#define ARCH_FAM    ppc
#define ARCH_MCU    mpc5567
/*
 * CPU and board
 */
#define CFG_PPC                   1
#define CFG_BOOKE                 1
#define CFG_E200Z6                1
#define CFG_MPC55XX               1
#define CFG_MPC5567               1

#define USE_KERNEL                1
#define USE_MCU                   1


/*
 * Misc
 */
//#define USE_T32_TERM              1
//#define CFG_CONSOLE_T32           1
#define USE_WINIDEA_TERM	1
#define CFG_CONSOLE_WINIDEA       1

#define USE_PROTECTIONHOOK        1
#define USE_STARTUPHOOK           1
#define USE_SHUTDOWNHOOK          1
#define USE_ERRORHOOK             1
#define USE_PRETASKHOOK           1
#define USE_POSTTASKHOOK          1

#endif /* CONFIG_H_ */

