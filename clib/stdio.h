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
