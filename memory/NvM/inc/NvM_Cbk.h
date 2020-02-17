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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.0.3 */


/** @req NVM551 *//* Include file structure should include: A callback interface NvM_Cbk.h that provides the callback function prototypes */
/** @req NVM384 *//* The NvM module shall provide a callback interface */
/** @req NVM438 *//* NvM shall provide callback functions */


#ifndef NVM_CBK_H_
#define NVM_CBK_H_

void NvM_JobEndNotification(void);		/** @req NVM462 */
void NvM_JobErrorNotification(void);	/** @req NVM463 */

#endif /*NVM_CBK_H_*/
