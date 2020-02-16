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


#ifndef SCHM_CFG_H_
#define SCHM_CFG_H_


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
