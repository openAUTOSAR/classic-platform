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
#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>

#define DEVICE_TYPE_CONSOLE		1
#define DEVICE_TYPE_FS			2


#define GetRootObject(RootType, member, objPointer)		((RootType *)((char *)objPointer - (char *)(&((RootType*)0)->member)))

typedef struct {
    const char *name;
    uint32_t type;
} DeviceType;


#endif /* DEVICE_H_ */
