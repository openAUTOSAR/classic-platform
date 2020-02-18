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

/*lint -w2 Reduced static code analysis (only used during test) */

/*
 * DESCRIPTION
 *   RAM filesystem
 */

/* ----------------------------[includes]------------------------------------*/
#define DBG

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "fs.h"
#include "sys/queue.h"
#include "MemMap.h"
#include "debug.h"

/* ----------------------------[private define]------------------------------*/

#define CHUNK_SIZE          1000
/* ----------------------------[private macro]-------------------------------*/

#ifndef MAX
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif

#ifndef ENOSPC
#define  ENOSPC       28
#endif


/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


/**
 *
 * @param dev - File system device
 * @param f - File
 * @param pathname The filename
 * @return
 */
static int ramfs_open(FSDriverType *dev, FILE *f, const char *pathname) {
    return 0;
}


static int ramfs_close(FSDriverType *dev, FILE *f) {
    /*lint -e586 MISRA:CONFIGURATION:argument check:[MISRA 2012 Rule 21.6, required] */
    dbg("ramfs_close no=%d\n",f->fileNo,f->filepath);
    /* WINIDEA_BP */
    return 0;
}

static int ramfs_read(FSDriverType *dev, FILE *f, void *buf, size_t size) {

    /* If the starting position is at or after the end-of-file, 0 shall be returned */
    if( f->end == 0 ) {
        return 0;
    }

    dbg("ramfs_read no=%d,buf=%p,size=%d (%s)\n",f->fileNo,f->data,size,f->filepath);

    memcpy(buf,f->data + f->pos,size);

    return size;
}
static int ramfs_write(FSDriverType *dev, FILE *f, const void *buf, size_t nbyte) {
    int totSize;

    if ( (f->pos + nbyte) >  f->asize ) {
        totSize = MAX(CHUNK_SIZE,nbyte) + f->pos;
        dbg("ramfs_write no=%d,buf=%p,size=%d (%s)\n",f->fileNo,f->data,nbyte,f->filepath);
        f->data = realloc(f->data,totSize);
        f->asize = totSize;
        if( f->data == NULL ) {
            /* Can't allocate more */
            dbg("  Fail: ENOSPC\n");
            return -ENOSPC;
        }
    }

    memcpy((f->data + f->pos),buf,nbyte);

    return nbyte;
}

static int ramfs_lseek(FSDriverType *dev, FILE *f, int pos) {
    /* All things done in upper layer */
    return 0;
}

FSDriverType FS_RamDevice = {
    .device.type = DEVICE_TYPE_FS,
    .name = "ram",
    .open = ramfs_open,
    .close = ramfs_close,
    .read = ramfs_read,
    .write = ramfs_write,
    .lseek = ramfs_lseek,
};

