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


#ifndef SCHM_CFG_H_
#define SCHM_CFG_H_

#warning "This default file may only be used as an example!"

#define SCHM_MF_MEM_PERIOD 		20

/*
 * Scheduling BSW
 */
#define SCHM_CYCLE_MAIN				(5)

#define SCHM_MAINFUNCTION_CYCLE_ADC     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_CAN_WRITE     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_CAN_READ     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_CAN_BUSOFF     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_CAN_WAKEUP     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_CAN_ERROR     SCHM_CYCLE_MAIN

#define SCHM_MAINFUNCTION_CYCLE_CANNM   SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_CANSM   SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_CANTP   SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_CANTRCV SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_COMRX     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_COMTX     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_COMM    SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_DCM     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_DEM     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_ECUM    SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_EA      SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_EEP     SCHM_CYCLE_MAIN
//#define SCHM_MAINFUNCTION_CYCLE_FEE     SCHM_CYCLE_MAIN
//#define SCHM_MAINFUNCTION_CYCLE_FLS     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_IOHWAB  SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_NM      SCHM_CYCLE_MAIN
//#define SCHM_MAINFUNCTION_CYCLE_NvM     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_PDUR    SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_SPI     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_WDGM    SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_WDGM_TRIGGER     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_WDGM_ALIVESUPERVISION     SCHM_CYCLE_MAIN

/*
 * Schedule BSW memory
 */

#define SCHM_MAINFUNCTION_CYCLE_NVM     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_FEE     SCHM_CYCLE_MAIN
#define SCHM_MAINFUNCTION_CYCLE_FLS     SCHM_CYCLE_MAIN


#endif /*SCHM_CFG_H_*/
