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


#ifndef CANSM_BSWM_H_
#define CANSM_BSWM_H_

/* @req CANSM347 */
/* "The header file CanSM_BswM.h shall export the interfaces, which
 * are dedicated to the BswM module..". But there are no interfaces dedicated to
 * BswM module..
 */

/** This type shall define the CAN specific communication modes/states notified
 * to the BswM module. */
typedef enum
{
	CANSM_BSWM_NO_COMMUNICATION = 0,
	CANSM_BSWM_SILENT_COMMUNICATION,
	CANSM_BSWM_FULL_COMMUNICATION,
	CANSM_BSWM_BUS_OFF,
	CANSM_BSWM_CHANGE_BAUDRATE,
}CanSM_BswMCurrentStateType;

#endif /* CANSM_BSWM_H_ */
