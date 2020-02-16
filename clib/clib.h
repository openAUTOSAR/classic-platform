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

#ifndef CLIB_H_
#define CLIB_H_


typedef struct _FileS {
	int fileNo;
	int b;
} _FileType;


typedef struct _EnvS {
	/* Each "Task" should at least have a set of std file handles */
	_FileType *_stdin;
	_FileType *_stdout;
	_FileType *_stderr;
} _EnvType;

extern _EnvType *_EnvPtr;


#endif /* CLIB_H_ */
