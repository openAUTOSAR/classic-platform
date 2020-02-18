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

/* ----------------------------[information]----------------------------------
 * Author: mahi
 *
 * Description:
 *   Implements the glue between different clibs and ArcCore "system"
 *
 * Implementation Notes:
 *   Heavily inspired by http://neptune.billgatliff.com/newlib.html
 *
 *   If a module should be used is controlled by USE_TTY_T32, USE_TTY_WINIDEA, etc
 *   The console is controlled by SELECT_CONSOLE=TTY_T32|TTY_WINIDEA|RAMLOG|....
 */

/*lint -w1 Only errors in generic module used during development */

/* ----------------------------[includes]------------------------------------*/

#if defined(__GNUC__) && defined(__HIGHTEC__) && (SELECT_CLIB == CLIB_DNK )
#include <stat.h>
#include <sys/types.h>
#elif defined(__GNUC__)
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "stdio.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "Std_Types.h"
#if defined(USE_KERNEL)
#include "Os.h"
#endif

#include "fcntl.h"
#include "device.h"
#include "fs.h"
#include "device_serial.h"

#if defined(USE_TTY_UDE)
#include "serial_dbg_ude.h"
#endif
#if defined(USE_TTY_T32)
#include "serial_dbg_t32.h"
#endif
#if defined(USE_TTY_WINIDEA)
#include "serial_dbg_winidea.h"
#endif
#if defined(USE_TTY_CODE_COMPOSER)
#include "serial_dbg_code_composer.h"
#endif
#if defined(USE_TTY_SCI)
#include "serial_sci.h"
#endif

#include "Ramlog.h"

#if defined(__ARMCC_VERSION)
#define caddr_t char *
#endif

/* ----------------------------[private define]------------------------------*/

#if !((defined(CFG_ARM) || defined(CFG_RH850)) && defined(__GNUC__))
#define _open		open
#if !defined(__HIGHTEC__)
#define _exit		exit
#endif
#define _fstat		fstat
#define _getpid		getpid
#define _kill		kill
#define _close		close
#define _isatty		isatty
#define _sbrk		sbrk
#define _read		read
#define _write		write
#define _lseek		lseek
#endif

/* Unify over compilers */
#ifndef EBADF
#define EBADF       9
#endif
#ifndef ESPIPE
#define ESPIPE      29
#endif

#define MAX_OPEN_FILES 5

/* Console devices...usage:
 * Normally you have a console device when it's connected to a debugger. Some debuggers do not have
 * a proper terminal window, then the ramlog can be used.
 *
 * Use cases:
 * 1. SELECT_CONSOLE=TTY_T32
 * 2. SELECT_CONSOLE=RAMLOG
 * 3. SELECT_CONSOLE=TTY_NONE
 * 4. SELECT_CONSOLE=TTY_T32 and you want to write the the RAMLOG (since it's much faster)
 *
 *
 *
 */
#if defined(USE_TTY_T32)
#define CONSOLE_DEVICE		&T32_Device.device
#elif defined(USE_TTY_UDE)
#define CONSOLE_DEVICE		&UDE_Device.device
#elif defined(USE_TTY_WINIDEA)
#define CONSOLE_DEVICE		&WinIdea_Device.device
#elif defined(USE_TTY_CODE_COMPOSER)
#define CONSOLE_DEVICE		&CodeComposer_Device.device
#elif defined(USE_TTY_SCI)
#define CONSOLE_DEVICE      &SCI_Device.device
#endif

#if !defined(CONSOLE_DEVICE) && defined(USE_RAMLOG)
#define CONSOLE_DEVICE 		&Ramlog_Device.device
#endif

#if !defined(CONSOLE_DEVICE)
#define CONSOLE_DEVICE      &Dummy_Device.device
#endif

/* ----------------------------[private macro]-------------------------------*/

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#include <clib.h>

extern FSDriverType FS_RamDevice;

DeviceSerialType Dummy_Device = { .name = "serial_Dummy", .read = NULL, .write =
        NULL, .open = NULL, };

FILE openFiles[MAX_OPEN_FILES] =
{
    /* stdin */
    [0].devicePtr = CONSOLE_DEVICE, [0].type = DEVICE_TYPE_CONSOLE,
    [0].fileNo = 0,
    /* stdout */
    [1].devicePtr = CONSOLE_DEVICE, [1].type = DEVICE_TYPE_CONSOLE,
    [1].fileNo = 1,
    /* stderr */
    [2].devicePtr = CONSOLE_DEVICE, [2].type = DEVICE_TYPE_CONSOLE,
    [2].fileNo = 2,
};

_EnvType __Env = {
        ._stdin = &openFiles[0],
        ._stdout = &openFiles[1],
        ._stderr =
        &openFiles[2], };

_EnvType *_EnvPtr = &__Env;

DeviceType *deviceList[] = {
        CONSOLE_DEVICE,
#if defined(USE_RAMLOG)
        &Ramlog_Device.device,
#endif
#if defined(CFG_FS_RAM)
        &FS_RamDevice.device,
#endif
        &Dummy_Device.device };

/* Errno is made reentrant by using malloc and we don't want this. This is usually fixed
 * by undef errno and declaring it as extern int. It does not work when using GCC for
 * HC1X and this ifdef takes care of that.
 */
//#undef errno
#if  defined(__GNUC__) && defined(CFG_HC1X)
int errno;
#elif defined(__GNUC__) && defined(__HIGHTEC__)
extern int _Errno;
#elif defined(__ARMCC_VERSION)
extern volatile int errno;
#else
extern int errno;
#endif

/* ----------------------------[private functions]---------------------------*/

static int newFileNo(void) {
    int i;
    /* search, but skip standard handles */
    for (i = 2; i < MAX_OPEN_FILES; i++) {
        if (openFiles[i].fileNo == 0) {
            break;
        }
    }

    if (i == MAX_OPEN_FILES) {
        return (-1);
    }

    return i;
}

static int validFileNo(int fileno) {

    return (fileno < MAX_OPEN_FILES) && (openFiles[fileno].fileNo != 0);
}

#if defined(CFG_FS_RAM)
int fcntl(int fd, int cmd, int arg) {
    int rv = -1;
    if (cmd == F_SETFL) {
        /* get arg */
        openFiles[fd].o_flags = arg;
        rv = 0;
    } else if (cmd == F_GETFL) {
        rv = openFiles[fd].o_flags;
    }
    return rv;
}
#endif

DeviceType * findDeviceByName(const char *filepath, uint32_t filter) {
    uint32_t i;
    for (i = 0UL; i < sizeof(deviceList) / sizeof(deviceList[0UL]); i++) {
        if (deviceList[i]->type & filter) {
            if (strcmp(filepath, deviceList[i]->name) == 0) {
                return deviceList[i];
            }
        }
    }
    return NULL;
}

/* ----------------------------[public functions]----------------------------*/

#if defined(CFG_FS_RAM)
FILE *fopen(const char *filename, const char * mode) {
    int fileNo;
    fileNo = open(filename, 0, 0);
    return &openFiles[fileNo];
}

int fclose(FILE *f) {
    int fd;
    fd = fileno(f);
    return close(fd);
}

size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *f) {
    int fd;
    fd = fileno(f);
    return write(fd, ptr, size * nitems) / size;
}

size_t fread(void *ptr, size_t size, size_t nitems, FILE *f) {

    int fd;
    fd = fileno(f);
    return read(fd, ptr, size * nitems);
}


int setvbuf(FILE *f, char *buf, int mode, size_t size) {

    if (buf == NULL) {
        if (size == 0) {
            size = BUFSIZ;
        }
        f->data = malloc(size);
        f->end = size;
        f->asize = size;
    }
    return 0;
}

void setbuf(FILE *f, char *buf) {
    setvbuf(f, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

int fflush(FILE *f) {
    (void) f;
    return 0;
}

void rewind(FILE *f) {
    f->pos = 0;
}

long ftell(FILE *f) {
    return (long) f->pos;
}

int fseek(FILE * f, long int offset, int origin) {
    int fd;
    fd = fileno(f);
    return lseek(fd, offset, origin);
}
#endif

/**
 * Get a file number from a file
 * @param file
 * @return
 */
int fileno(FILE *file) {
    return file->fileNo;
}

/**
 * POSIX open function
 *
 * Should probably support some O and S flags here
 *   See http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html
 *
 * @param name  Name of the file to open
 * @param flags O_xx flags
 * @param mode  S_xx modes.
 * @return	The file descriptor or -1 if failed to open.
 */
int _open(const char *filepath, int flags, int mode) {
    int fileNo = -1;
    DeviceType *devH;

    devH = findDeviceByName(filepath, DEVICE_TYPE_CONSOLE);
    if (devH != NULL) {

        fileNo = newFileNo();

        if (fileNo == (-1)) {
            return (-1);
        }

        /* console device */
        DeviceSerialType *consoleH = GetRootObject(DeviceSerialType, device,
                devH);
        consoleH->open(filepath, flags, mode);

        openFiles[fileNo].fileNo = fileNo;
        openFiles[fileNo].devicePtr = devH;

    } else {
#if defined(CFG_FS_RAM)
        int i;
        FILE *f;
        /* the fileNo must be global so allocate here */
        fileNo = newFileNo();

        if (fileNo == (-1)) {
            return (-1);
        }

        f = &openFiles[fileNo];

        /* Support for one filesystem only, so grab the first one we get */

        for( i=0; i<sizeof(deviceList)/sizeof(deviceList[0]); i++ ) {
            if (deviceList[i]->type & DEVICE_TYPE_FS ) {
                FSDriverType *devPtr = GetRootObject(FSDriverType,device,deviceList[i]);

                devPtr->open(devPtr,f,filepath);
                /* Save global information */
                f->fileNo = fileNo;
                f->devicePtr = deviceList[i];
                strncpy(f->filepath,filepath,MAX_FILENAME_LEN);

                break;
            }
        }

#endif

    }
    return fileNo;
}

#if defined(__GNUC__)
int fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

#if !((defined(CFG_ARM) && defined(__GNUC__)) || defined(CFG_RH850))
#define _getpid getpid
#endif

pid_t _getpid() {
    return 1;
}
#endif

#if !(defined(CFG_ARM) && defined(__GNUC__) || defined(CFG_RH850) )
#define _kill kill
#endif

int _kill(int pid, int sig) {
    (void) pid;
    (void) sig;
    errno = EINVAL;
    return (-1);
}

/* Do nothing */
int _close(int fd) {
    int rv = -1;
    DeviceType * dPtr;

    if (!validFileNo(fd)) {
        return EBADF;
    }

    dPtr = openFiles[fd].devicePtr;

    if (dPtr->type == DEVICE_TYPE_FS) {
        FILE *f;
        FSDriverType *fs = GetRootObject(FSDriverType, device, dPtr);

        f = &openFiles[fd];
        fs->close(fs, f);

        /* Cleanup */
        if ((f->flags & FLAGS_SETBUF_ALLOC) == 0) {
            free(f->data);
        }

        f->data = NULL;
        f->asize = 0;
        f->end = 0;
        f->pos = 0;
        f->fileNo = 0;
        rv = 0;
    }
    return rv;
}

/**
 *
 * @param fd
 * @return 1 if connected to a terminal
 */
int _isatty(int fd) {
    (void) fd;
    return 1;
}

/* If we use malloc and it runs out of memory it calls sbrk()
 */

#if defined(CFG_PPC) || (defined(CFG_ARMV7_AR) && defined(__GNUC__)) || defined(CFG_RH850)

#if defined(CFG_PPC)
#define _sbrk   sbrk
#endif

/* linker symbols */
extern char _heap_start[]; // incomplete array to ensure not placed in small-data
extern char _heap_end[];

void * _sbrk( ptrdiff_t incr )
{
    char *prevEnd;
    static char *nextAvailMemPtr = _heap_start;

    if( nextAvailMemPtr + incr > _heap_end) {
        write( 2, "!!!! Heap overflow  !!!!\n", 25 );
        errno = EAGAIN;
        abort();
    }
    prevEnd = nextAvailMemPtr;
    nextAvailMemPtr += incr;
    return prevEnd;
}

#elif defined(__ARMCC_VERSION)
/* Do not have sbrk() but instead something else */
#else
extern char _end[];

//static char *curbrk = _end;

#ifndef HEAPSIZE
#define HEAPSIZE 16000
#endif

/*
 * The heap sadly have alignment that depends on the pagesize that
 * you compile malloc newlib with. From what I can tell from the
 * code that is a pagesize of 4096.
 */

unsigned char _heap[HEAPSIZE] __attribute__((aligned (4)));
//__attribute__((section(".heap")));

void * _sbrk(ptrdiff_t incr) {
    static unsigned char *heap_end;
    unsigned char *prev_heap_end;

    /* initialize */
    if (heap_end == 0) {
        heap_end = _heap;
    }
    prev_heap_end = heap_end;

    if (heap_end + incr - _heap > HEAPSIZE) {
        /* heap overflow - announce on stderr */
        write(2, "!!!! Heap overflow!!!!!\n", 25);
        abort();
    }

    heap_end += incr;

    return (void *) prev_heap_end;
}
#endif

int _read(int fd, void *buf, size_t nbytes) {
    int rv = 0;
    int32_t cpBytes;

//    if (!validFileNo(fd)) {
//        return EBADF;
//    }

    DeviceType * dPtr = openFiles[fd].devicePtr;

    if (dPtr == NULL) {
        return EBADF;
    }

    if (dPtr->type == DEVICE_TYPE_CONSOLE) {
        DeviceSerialType *devSerPtr = GetRootObject(DeviceSerialType, device,
                dPtr);

        /* non-blocking read */
        if (openFiles[fd].o_flags & O_NONBLOCK) {
            rv = devSerPtr->read(buf, nbytes);
            /* Nothing to read return -1 */
            if (rv == 0) {
                rv = -1;
            }
        } else {
            /* Blocking read */
            cpBytes = nbytes;
            while (cpBytes > 0) {
                rv = devSerPtr->read(buf, cpBytes);
                cpBytes -= rv;
            }
            rv = nbytes;
        }
    } else if (dPtr->type == DEVICE_TYPE_FS) {
        FILE *f;
        FSDriverType *fs = GetRootObject(FSDriverType, device, dPtr);
        f = &openFiles[fd];
        rv = fs->read(fs, f, (void *) buf, nbytes);
        if (rv < 0) {
            errno = -rv;
        } else {
            f->pos += rv;
        }

    }

    return rv;
}

/**
 * Write data to
 *
 * @param fd      The file descriptor
 * @param _buf
 * @param nbytes
 * @return
 */
int _write(int fd, const void *buf, size_t nbytes) {

    int rv = 0;
    DeviceType * dPtr;

    if (!validFileNo(fd)) {
        return EBADF;
    }

    dPtr = openFiles[fd].devicePtr;

    if (dPtr->type == DEVICE_TYPE_CONSOLE) {
        DeviceSerialType *devSerPtr = GetRootObject(DeviceSerialType, device,
                dPtr);

        rv = devSerPtr->write((void *) buf, nbytes);
    } else if (dPtr->type == DEVICE_TYPE_FS) {
        FILE *f;
        FSDriverType *fs = GetRootObject(FSDriverType, device, dPtr);
        f = &openFiles[fd];
        rv = fs->write(fs, f, (void *) buf, nbytes);
        if (rv < 0) {
            errno = -rv;
        } else {
            f->pos += nbytes;
            f->end = MAX(f->pos, f->end);
        }
    }

    return rv;
}

#if 1 // defined(__GNUC__)

/**
 *
 * @param fd
 * @param offset
 * @param origin    SEEK_SET - Beginning of the file
 *                  SEEK_CUR - Current file position
 *                  SEEK_END - End of the file
 * @return
 */
int _lseek(int fd, int offset, int origin) {
    int rv;
    DeviceType * dPtr;

    if (!validFileNo(fd)) {
        return EBADF;
    }

    dPtr = openFiles[fd].devicePtr;

    if (dPtr->type == DEVICE_TYPE_FS) {
        int tot = offset;
        FILE *f;
        FSDriverType *fs = GetRootObject(FSDriverType, device, dPtr);
        f = &openFiles[fd];
        if (origin == SEEK_CUR) {
            tot += f->pos;
        } else if (origin == SEEK_END) {
            tot += f->end;
        }

        rv = fs->lseek(fs, f, tot);

        if (rv < 0) {
            errno = -rv;
        } else {
            f->pos = tot;
        }
        rv = 0;
    } else {
        errno = ESPIPE;
        rv = -1;
    }

    return rv;
}
#endif

#if defined(__GNUC__) && !defined(CFG_CXX)
void __init( void )
{
}
#endif

#if defined(__ARMCC_VERSION) || (defined(__GNUC__) && ( defined(CFG_ZYNQ) || defined(CFG_TRAVEO)))

void * memcpy( void * dest, void const * src, size_t len)
{
    uint8_t* dst8 = (uint8_t*)dest;
    uint8_t* src8 = (uint8_t*)src;

    while (len--) {
        *dst8++ = *src8++;
    }
    return dest;
}

#endif

#if defined(CFG_BRD_TMS570_CC)
void _exit( int status ) {
    while(1);
}
#elif defined(CFG_ARM)
void _exit( int status ) {
#ifdef USE_TTY_CODE_COMPOSER
    __asm("        .global C$$EXIT");
    __asm("C$$EXIT: nop");
#endif

    ShutdownOS( E_OS_EXIT_ABORT );

    while(1);
}
#endif

#if defined(__GNUC__) && defined(CFG_RH850)
void _exit( int a ) {
    ShutdownOS( E_OS_EXIT_ABORT );

    while(1);
}
#endif

#if defined(__DCC__)
/* Set by atexit() in init() */
void _dcc_exit( void ) {
    ShutdownOS( E_OS_EXIT_ABORT );

    while(1);
}
#endif

#if defined(__CWCC__)
void exit(int a ) {
    (void)a;
    ShutdownOS( E_OS_EXIT_ABORT );

    while(1);
}
#endif

#if defined(__HIGHTEC__)
void _exit(int a ) {
    (void)a;
    ShutdownOS( E_OS_EXIT_ABORT );

    while(1);
}
#endif

#if defined(CFG_NEWLIB_MALLOC_LOCK)
void __malloc_lock ( struct _reent *_r )
{
    DisableAllInterrupts();
    return;
}

void __malloc_unlock ( struct _reent *_r )
{
    EnableAllInterrupts();
    return;
}
#endif


