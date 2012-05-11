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

#if 0
#include "Rte_EcuM.h"
#else
#include "EcuM.h"
#include "EcuM_Generated_Types.h"
#include "EcuM_Internals.h"
#endif

Std_ReturnType EcuM_ShutdownTarget_GetLastShutdownTarget(EcuM_StateType* target, UInt8* mode) {
	return E_NOT_OK;
}

Std_ReturnType EcuM_ShutdownTarget_GetShutdownTarget(EcuM_StateType* target, UInt8* mode) {
	return EcuM_GetShutdownTarget(target, (uint8*)mode);
}

Std_ReturnType EcuM_ShutdownTarget_SelectShutdownTarget(EcuM_StateType target, UInt8 mode) {
	return EcuM_SelectShutdownTarget(target, (uint8)mode);
}

Std_ReturnType EcuM_StateRequest_RequestRUN(EcuM_UserType user) {
	return EcuM_RequestRUN(user);
}

Std_ReturnType EcuM_StateRequest_ReleaseRUN(EcuM_UserType user) {
	return EcuM_ReleaseRUN(user);
}

Std_ReturnType EcuM_StateRequest_RequestPOSTRUN(EcuM_UserType user) {
	return EcuM_RequestPOST_RUN(user);
}

Std_ReturnType EcuM_StateRequest_ReleasePOSTRUN(EcuM_UserType user) {
	return EcuM_ReleasePOST_RUN(user);
}

Std_ReturnType EcuM_BootTarget_GetBootTarget(EcuM_BootTargetType* target) {
	return EcuM_GetBootTarget(target);
}

Std_ReturnType EcuM_BootTarget_SelectBootTarget(EcuM_BootTargetType target) {
	return EcuM_SelectBootTarget(target);
}
