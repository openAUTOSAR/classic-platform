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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef RTE_OS_TYPE_H_
#define RTE_OS_TYPE_H_


#define OS_NOT_SERVICE_COMPONENT
#ifdef USE_RTE
#include "Rte_Type.h"
#else
typedef uint32 CounterType; /* @req SWS_Os_00786 */
typedef uint32 TickType; /*@req OSEK_SWS_AL_00024 */
typedef uint32* TickRefType; /*@req OSEK_SWS_AL_00024 */
#endif


#endif /* RTE_OS_TYPE_H_ */

/** @req SWS_Rte_07126 */
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
