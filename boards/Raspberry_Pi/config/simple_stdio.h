/*
 * simple_stdio.h
 *
 *  Created on: 6 dec 2013
 *      Author: avenir
 */

#ifndef SIMPLE_STDIO_H_
#define SIMPLE_STDIO_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

/********************************************************************************
 * Defines																		*
 *******************************************************************************/
#define DBLOCK_SIZE 	4096	// Each data block is 4kB
#define NR_DBLOCKS		30		// Max nr of data blocks can be allocated => 32*4kB = 128kB for this file system
#define NR_BITTABLES	64		// Max nr of uint32_t-bit-tables (currently they are too many, and used for padding)
//#define IBIT_OFFSET		0
//#define INODE_OFFSET	1
#define WRITE_FLAG		4		// This nr is chosen to mimic the _IO_NO_READS-define in libio.h in linux
#define READ_FLAG		8		// This nr is chosen to mimic the _IO_NO_WRITES-define in libio.h in linux

#define _stat 			stat

typedef char 		DataBlock[DBLOCK_SIZE];
typedef uint32_t 	BitTable[NR_BITTABLES];

typedef struct {
	FILE 	 file;
	uint16_t dIndex;
	uint16_t size;
	char	 fname[34];
	char	 fversion[2];
} Inode;

typedef struct {
	BitTable	ibits;
	BitTable	dbits;
	Inode		inodes[NR_DBLOCKS];
	DataBlock 	data[NR_DBLOCKS];
} FileSystem;

/********************************************************************************
 * Overloaded <stdio>-methods													*
 *******************************************************************************/
FILE * fopen(const char * filename, const char * mode );
int fclose(FILE *stream);
int fgetc(FILE *stream);
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
int fileno (FILE *__stream);
long int ftell (FILE *stream);
int fseek(FILE *stream, long int offset, int origin);
int autosar_fputc(int character, FILE *stream);

#ifdef getc
#undef getc
#endif
#define getc(fp) fgetc(fp)
#ifdef putc
#undef putc
#endif
#define putc(c, fp) fputc(c, fp)
#ifdef putchar
#undef putchar
#endif
#define putchar(c) fputc(c, 0)

/********************************************************************************
 * Other overloaded methods														*
 *******************************************************************************/
int fstat(int ibit, struct stat *buf);
int _stat(const char *filename, struct stat *buf);

/********************************************************************************
 * Helper-methods																*
 *******************************************************************************/
FileSystem * getFS(void);
void setSize(const char *filename, size_t size);
//void runKVM(void);

#endif /* SIMPLE_STDIO_H_ */
