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

#ifndef RTE_NVM_TYPE_H_
#define RTE_NVM_TYPE_H_

#define NVM_NOT_SERVICE_COMPONENT

#ifdef USE_RTE
#warning This file should only be used when not using an RTE with NvM Service Component.
#include "Rte_Type.h"
#define NVM_REQ_OK                  0x00
#define NVM_REQ_NOT_OK              0x01
#define NVM_REQ_PENDING             0x02
#define NVM_REQ_INTEGRITY_FAILED    0x03
#define NVM_REQ_BLOCK_SKIPPED       0x04
#define NVM_REQ_NV_INVALIDATED      0x05
#define NVM_REQ_CANCELLED           0x06
#define NVM_MULTI_BLOCK_REQUEST_ID          0
#define NVM_REDUNDANT_BLOCK_FOR_CONFIG_ID   1

#else
typedef uint8 NvM_RequestResultType;    /** @req NVM470 */
#define NVM_REQ_OK                  0x00
#define NVM_REQ_NOT_OK              0x01
#define NVM_REQ_PENDING             0x02
#define NVM_REQ_INTEGRITY_FAILED    0x03
#define NVM_REQ_BLOCK_SKIPPED       0x04
#define NVM_REQ_NV_INVALIDATED      0x05
#define NVM_REQ_CANCELLED           0x06

/** @req NVM471 */
/* 0 and 1 is reserved, sequential order */
typedef uint16 NvM_BlockIdType;

#define NVM_MULTI_BLOCK_REQUEST_ID          0
#define NVM_REDUNDANT_BLOCK_FOR_CONFIG_ID   1
#endif

#endif /* RTE_ECUM_TYPE_H_ */
