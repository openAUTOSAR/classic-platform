
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

/* Environment "design"
 *
 * Use case: We need only one "enviroment"
 *   This should be the normal
 *
 * One environment
 * - All is defined by _Env
 *
 */

#include <clib.h>

_FileType stdFiles[3] = {
	{
		.fileNo = 0,
	},
	{
		.fileNo = 1,
	},
	{
		.fileNo = 2,
	}
};

_EnvType __Env = {
	._stdin = &stdFiles[0],
	._stdout = &stdFiles[1],
	._stderr = &stdFiles[2],
};

_EnvType *_EnvPtr = &__Env;





