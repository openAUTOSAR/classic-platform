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

#ifndef RTE_ECUM_TYPE_H_
#define RTE_ECUM_TYPE_H_

#define ECUM_NOT_SERVICE_COMPONENT
#ifdef USE_RTE
#warning This file should only be used when NOT using an EcuM Service Component.
#include "Rte_Type.h"
#else
/* @req SWS_EcuMf_00104 */
/* @req SWS_EcuM_02664 */
/* @req SWS_EcuM_00507 */
/* @req SWS_EcuM_04039 */ /*@req SWS_EcuMf_00105*/
typedef uint8 EcuM_StateType;
/* @req SWS_EcuM_04067 */ /*@req SWS_EcuMf_00048*/
typedef uint8 EcuM_UserType;
/* @req SWS_EcuM_04042 */ /*@req SWS_EcuMf_00036*/
typedef uint8 EcuM_BootTargetType;

#endif
#endif /* RTE_ECUM_TYPE_H_ */
