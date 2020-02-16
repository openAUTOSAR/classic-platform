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

#include "Crc.h"

#define CRC_SW_MAJOR_VERSION_INT        1
#define CRC_SW_MINOR_VERSION_INT        0


/* @req 4.0.3/CRC005 */
#if (CRC_SW_MAJOR_VERSION != CRC_SW_MAJOR_VERSION_INT) || (CRC_SW_MINOR_VERSION != CRC_SW_MINOR_VERSION_INT)
#error "Version mismatch between Crc.c and Crc.h"
#endif

