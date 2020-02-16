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

#ifndef STDIO_H_
#define STDIO_H_

#include <stdarg.h>
#include <stdlib.h>
#include <clib.h>

typedef _FileType FILE;

#if !defined(EOF)
#define EOF		(-1)
#endif

/* File handles */
#define stdin		(_EnvPtr->_stdin)
#define stdout		(_EnvPtr->_stdout)
#define stderr		(_EnvPtr->_stderr)

int printf(const char *format, ...);
int fprintf(FILE *file, const char *format, ...);
int sprintf(char *buffer, const char *format, ...);
int snprintf(char *buffer, size_t n, const char *format, ...);
int vprintf(const char *format, va_list ap);
int vsprintf(char *buffer, const char *format, va_list ap);
int vfprintf(FILE *file, const char *format, va_list ap);
int vsnprintf(char *buffer, size_t n, const char *format, va_list ap);

int fileno( FILE *);

#define putc(_x,_file)	fputc(_x,_file)
#define putchar(_x)		fputc(_x,stdout)

int	fputc(int, FILE *);
int	fputs(const char *, FILE *);
int puts(const char *s);

FILE *fopen(const char *filename, const char *mode);

/* Low level file I/O */
int open(const char *name, int flags, int mode);
int close( int fd );
int read( int fd, void *buf, size_t nbytes );
int write(  int fd, const void *buf, size_t nbytes);


#define puts(_x)	fputs(_x,stdout)

#define getc(_x)		fgetc(_x)
#define getchar()		getc(stdin)

int fgetc(FILE *file);



#endif /* STDIO_H_ */
