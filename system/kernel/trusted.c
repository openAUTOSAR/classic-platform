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
