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





