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








#ifndef HOOKS_H_
#define HOOKS_H_

#include "sys.h"
#include "pcb.h"
#include "kernel.h"

/* Called for sequence of error hook calls in case a service
 * does not return with E_OK. Note that in this case the general error hook and the OS-
 * Application specific error hook are called.
 */

#define ERRORHOOK(x) \
	if( os_sys.hooks->ErrorHook != NULL  ) { \
		os_sys.hooks->ErrorHook(x); \
	}


#define PRETASKHOOK() \
	if( os_sys.hooks->PreTaskHook != NULL ) { \
		os_sys.hooks->PreTaskHook(); \
	}

#define POSTTASKHOOK() \
	if( os_sys.hooks->PostTaskHook != NULL ) { 	\
		os_sys.hooks->PostTaskHook();			\
	}



#endif /*HOOKS_H_*/
