/*
 * simple_stdio.c
 *
 *  Created on: 6 dec 2013
 *      Author: avenir
 */

#include "simple_stdio.h"

// KVM-includes
//#include "global.h"

/********************************************************************************
 * File-scope variables															*
 *******************************************************************************/
static FileSystem	fs;

static void setBit(BitTable *bitTable,
				   int bit);
static int findFirstFreeBit(BitTable *bitTable);
static int findIbit(const char * filename,
					int freeBit);

/********************************************************************************
 * Local methods																*
 *******************************************************************************/

static void setBit(BitTable *bitTable,
				   int bit) {
	*bitTable[bit/32] |= (1 << bit);
}

static int findFirstFreeBit(BitTable *bitTable) {
	int i;
	int j;

	for (i=0; i<NR_BITTABLES; i++) {
		for (j=0; j<32; j++) {
			if (!(*bitTable[i] & (1 << j))) {
				return i*32+j;
			}
		}
	}

	return -1;
}

static int findIbit(const char * filename,
					int freeBit) {
	int ibit;
	for (ibit=0; ibit<freeBit; ibit++) {
		if (strcmp(fs.inodes[ibit].fname, filename) == 0) {
			return ibit;
		}
	}

	return -1;
}

static uint16_t getFileSize(FILE *stream) {
	int ibit = -1;
	int i;
	for (i=0; i<findFirstFreeBit(&fs.ibits); i++) {
		if (&fs.inodes[i].file == stream) {
			ibit = i;
			break;
		}
	}

	if (ibit == -1)
		return 0;

	return fs.inodes[ibit].size;
}

/********************************************************************************
 * Helper methods																*
 *******************************************************************************/

FileSystem * getFS(void) {
	return &fs;
}

void setSize(const char *filename, size_t size) {
	int iBit = findIbit(filename, sizeof(BitTable));
	if (iBit < 0)
		return;

	fs.inodes[iBit].size = size;

	/* ARM_EABI-additions */
//	fs.inodes[iBit].file._r = size - fs.inodes[iBit].file._offset;
//	fs.inodes[iBit].file._w = size - fs.inodes[iBit].file._offset;
}

/********************************************************************************
 * Overloaded <stdio>-methods 													*
 *******************************************************************************/
//
//int fgetc(FILE *stream) {
//	char *cptr = stream->_IO_read_ptr++;
//	return (int)*cptr;
//}
//
//FILE * fopen(const char *filename, const char *mode) {
//	/* Local variables */
//	int freeIBit;
//	int storedIBit;
//	FILE * fp = 0;
//
//	// Get the first free i-bit
//	freeIBit = findFirstFreeBit(&fs.ibits);
//	// If out of range, return null
//	if (freeIBit >= NR_DBLOCKS) {
//		return 0;
//	}
//
//	// Check if there is a file with this filename stored (search up to the free i-bit)
//	storedIBit = findIbit(filename, freeIBit);
//	// If no file stored with this filename, store a new file
//	if (storedIBit < 0) {
//		setBit(&fs.ibits, freeIBit);
//		strcpy(fs.inodes[freeIBit].fname, filename);
//		strcpy(fs.inodes[freeIBit].fversion, "1.0"); //temp solution
//
//		storedIBit = freeIBit;
//	}
//
//	// Set file pointer
//	fp = &fs.inodes[storedIBit].file;
//
//	// Set read/writer-pointers
//	fp->_IO_read_ptr = fs.data[storedIBit];
//	fp->_IO_read_base = fs.data[storedIBit];
//	fp->_IO_write_ptr = fs.data[storedIBit];
//	fp->_IO_write_base = fs.data[storedIBit];
////	fp->_p = fs.data[storedIBit];
//
//	// Assign a file number (greater than 2, since 0-2 are reserved for standrad streams)
//	// Aehh, skip that, let's give the ibit-nr
//	fp->_fileno = storedIBit;
////	fp->_file = storedIBit;
//
//	if (*mode == 'r') {
//		fp->_flags = READ_FLAG;
//	}
//	else if (*mode == 'w') {
//		fp->_flags = WRITE_FLAG;
//	}
//	else {}
//
//	// Return the file pointer
//	return fp;
//}
//
//int fclose(FILE *stream) {
//	return 0;
//}
//
////int getc(FILE *stream) {
////	return (int)*(stream->_IO_read_ptr++);
////}
//
//size_t fread(void * ptr, size_t size, size_t count, FILE * stream) {
//	int i;
//	for (i=0; i<count*size; i++) {
//		*((char*)ptr + i) = *(stream->_IO_read_ptr++);
////		*((char*)ptr + i) = *(stream->_p++);
////		*(stream->_offset++);
////		*(stream->_r--);
//	}
//
//	return i;
//}
//
//size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
//	int i;
//	for (i=0; i<count*size; i++) {
//		*(stream->_IO_write_ptr++) = *((char*)ptr++);
////		*(stream->_p++) = *((char*)ptr++);
////		*(stream->_offset++);
////		*(stream->_w--);
//	}
//
//	return i;
//}
//
//int fileno (FILE *stream) {
//	return stream->_fileno;
////	return stream->_file;
//}
//
//long int ftell (FILE *stream) {
//	if (READ_FLAG == stream->_flags) {
//		return (stream->_IO_read_ptr - stream->_IO_read_base);
//	}
//	else if (WRITE_FLAG == stream->_flags) {
//		return (stream->_IO_write_ptr - stream->_IO_write_base);
//	}
//	else {
//		return 0;
//	}
//
////	return *(stream->_offset);
//}
//
//int fseek(FILE *stream, long int offset, int origin) {
//	char * basePosition;
//	uint16_t filesize = getFileSize(stream);
//
//	/* Calc base position using the supplied origin flag */
//	switch (origin) {
//	case 0: /* (origin = SEEK_SET) => base = read/write_base */
//		basePosition = stream->_IO_read_base;
//		if (WRITE_FLAG == stream->_flags) {
//			basePosition = stream->_IO_write_base;
//		}
//		break;
//
//	case 1: /* (origin = SEEK_CUR) => base = read/write_ptr */
//		basePosition = stream->_IO_read_ptr;
//		if (WRITE_FLAG == stream->_flags) {
//			basePosition = stream->_IO_write_ptr;
//		}
//		break;
//
//	case 2: /* (origin = SEEK_END) => base = eof */
//		basePosition = stream->_IO_read_base + filesize - 1;
//		if (WRITE_FLAG == stream->_flags) {
//			basePosition = stream->_IO_write_base + filesize - 1;
//		}
//		break;
//
//	default:
//		break;
//	}
//
//	/* Check that the new position is within the file area */
//	if ((basePosition + offset) > (basePosition + filesize)) {
//		return -1;
//	}
//
//	/* Calc the new position using the supplied offset and the base position */
//	if (WRITE_FLAG == stream->_flags) {
//		stream->_IO_write_ptr = basePosition + offset;
//	}
//	else {
//		stream->_IO_read_ptr = basePosition + offset;
//	}
//
//	return 0;
//}
//
//int autosar_fputc(int character, FILE *stream) {
////	mini_uart_send(character);
//	return 0;
//}
//
////putchar
//
///********************************************************************************
// * Other overloaded methods														*
// *******************************************************************************/
//
///* Maybe these should be moved to ArcCore/doip-1.3.2/common/newlib_port.c, where
// * these methods are uncommented today.
// */
//int fstat(int ibit, struct stat *buf) {
//	uint16_t size = fs.inodes[ibit].size;
//	if (size <= 0)
//		return -1;
//
//	buf->st_size = size;
//
//	return 0;
//}
//
int _stat(const char *filename, struct stat *buf) {
	if (strcmp(filename, ".") == 0) {
		buf->st_mode = S_IFDIR;
	}

	return 0;
}

/* Should not really be here, but .. */
void _fini( void ) {}
