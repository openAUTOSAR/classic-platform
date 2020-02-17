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

#ifndef ICU_INTERNAL_H_
#define ICU_INTERNAL_H_

#define ICU_ISR_PRIORITY	4

#if defined(CFG_MPC5645S)

#define ICU_NUMBER_OF_EACH_EMIOS	24
#define ICU_MAX_CHANNEL				48

// EMIOS UC modes of operation (UC Channel CCR.B.MODE register setting)
#define EMIOS_UC_MODE_SAIC 			0x02 /* 0b0000010 */
#define EMIOS_UC_MODE_MCB_UP		0x50 /* 0b1010000 */
#define EMIOS_UC_MODE_MCB_UP_DOWN	0x54 /* 0b1010100 */

#elif defined(CFG_MPC5746C)

#define ICU_NUMBER_OF_EACH_EMIOS    32
#define ICU_MAX_CHANNEL             64

// EMIOS UC modes of operation (UC Channel CCR.B.MODE register setting)
// EMIOS UC modes of operation (UC Channel EMIOSC.B.MODE register setting)
#define EMIOS_UC_MODE_SAIC          0x02 /* 0b0000010 */
#define EMIOS_UC_MODE_IPWM          0x04 /* 0b0000100 */
#define EMIOS_UC_MODE_IPM           0x05 /* 0b0000101 */
#define EMIOS_UC_MODE_MCB_UP        0x50 /* 0b1010000 */
#define EMIOS_UC_MODE_MCB_UP_DOWN   0x54 /* 0b1010100 */

#elif defined(CFG_MPC5748G)
#define ICU_NUMBER_OF_EACH_EMIOS    32
#define ICU_MAX_CHANNEL             96

// EMIOS UC modes of operation (UC Channel CCR.B.MODE register setting)
// EMIOS UC modes of operation (UC Channel EMIOSC.B.MODE register setting)
#define EMIOS_UC_MODE_SAIC          0x02 /* 0b0000010 */
#define EMIOS_UC_MODE_IPWM          0x04 /* 0b0000100 */
#define EMIOS_UC_MODE_IPM           0x05 /* 0b0000101 */
#define EMIOS_UC_MODE_MCB_UP        0x50 /* 0b1010000 */
#define EMIOS_UC_MODE_MCB_UP_DOWN   0x54 /* 0b1010100 */

#elif defined(CFG_MPC5606B) || defined(CFG_SPC560B54) || defined(CFG_MPC5646B)

#define ICU_NUMBER_OF_EACH_EMIOS	32
#define ICU_MAX_CHANNEL				64

// EMIOS UC modes of operation (UC Channel EMIOSC.B.MODE register setting)
#define EMIOS_UC_MODE_SAIC 			0x02 /* 0b0000010 Single Action Input Capture */
#define EMIOS_UC_MODE_IPWM 			0x04 /* 0b0000100 Input Pulse Width Measurement */
#define EMIOS_UC_MODE_IPM 			0x05 /* 0b0000101 Input Period Measurement */
#define EMIOS_UC_MODE_MCB_UP		0x50 /* 0b1010000 Modulus Counter Buffered (Up counter) */
#define EMIOS_UC_MODE_MCB_UP_DOWN	0x54 /* 0b1010100 Modulus Counter Buffered (Up/Down counter) */

#elif defined(CFG_ZYNQ)

#define ICU_MAX_CHANNEL				118

#elif defined(CFG_MPC5606S) /*MPC5606S does not support IPM and IPWM mode*/
#define ICU_NUMBER_OF_EACH_EMIOS	32
#define ICU_MAX_CHANNEL				56

#define EMIOS_UC_MODE_SAIC 			0x02 /* 0b0000010 */
#endif

/*
 * Setting to ON freezes the current register state of a ICU channel when in
 * debug mode.
 */
#define ICU_FREEZE_ENABLE	STD_ON



#endif /* ICU_INTERNAL_H_ */
