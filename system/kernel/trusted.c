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



#error Do not use. Its only supported in SC3 and SC4





#include "Os.h"
#include "Cpu.h"


StatusType CallTrustedFunction(	TrustedFunctionIndexType ix,
								TrustedFunctionParameterRefType param )
{

	/* INFO: Can't find anything in the autosar documentation about calling a
	 *       trusted function directly from a trusted application(not calling
	 *       CallTrustedFunction() at all). So this code checks if in user mode
	 *       or not.
	 *
	 *       In Linux you either call the function directly e.g "getpid()" when
	 *       in priv. mode and through "syscall(SYS_getpid)" when in user mode.
	 */

	/* According to OS097 we can call this function from both trusted and non-trusted application */

	if( ix > Os_CfgGetServiceCnt() ) {
		return E_OS_SERVICEID;
	}

	CallService(ix,param);

	return E_OK;
}
