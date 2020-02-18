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

/*lint -w1 Warning level one in tests */


#if defined(CFG_BULLSEYE)

int printf(const char *format, ...);

#if _BullseyeCoverage
	#pragma BullseyeCoverage off
#endif

#define NULL 0

#define O_CREAT 0
#define O_TRUNC 0
#define O_WRONLY 0
#define S_IRUSR 0
#define S_IWUSR 0

#define DBG


static int getpid(void)
{
	return 1;
}

#if defined(CFG_BRD_LINUX)
/* Host */
#include <stdio.h>
#include <stdlib.h>

static int open(const char* path, int oflag, int mode)
{
    int fd = -1;
    FILE *fh;
    fh = fopen((char*)path, "w");
    if (fh != NULL) {
        fd = (int)fh;
    }

    return fd;
}

static int close(int fildes)
{
    return fclose((FILE *)fildes);
}

static int write(int fildes, const void* buf, unsigned nbyte)
{
    int n;
    if( fildes == 2 ) {
        /* stderr */
        n = printf("%s", (const char*)buf);
    } else {
        n = (int)fwrite((void*)buf, 1, nbyte, (FILE *)fildes);
        if (n == 0 && nbyte != 0) {
            n = -1;
        }
    }

    return n;
}

#else

static int open(const char* path, int oflag, int mode)
{
    printf("--- BullseyeCoverage begin file '%s', data begins next line ---\n", path);
    return 3;
}

static int close(int fildes)
{
    printf("--- BullseyeCoverage end file ---\n");
    return 0;
}

static int write(int fildes, const void* buf, unsigned nbyte)
{
    printf("%s", (const char*)buf);
    return (int)nbyte;
}

#endif


#include "run/atomic.h"
#include "run/libcov.h"
#include "run/libcov-version.h"
#include "run/libcov-core-small.h"


#endif
