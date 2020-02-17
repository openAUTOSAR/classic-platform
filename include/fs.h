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
#ifndef FS_H_
#define FS_H_

#include <stdlib.h>
#include "device.h"

#define MAX_FILENAME_LEN   128


#define FLAGS_SETBUF_ALLOC  1

typedef struct _FileS {
#if defined(CFG_FS_RAM)
    char filepath[MAX_FILENAME_LEN];
#endif
    int fileNo;
    int b;
    uint32_t type;
    uint32_t o_flags;
    uint32_t flags;
    DeviceType *devicePtr;
    /* Ptr to file data */
    uint8_t *data;
    /* position with a file */
    uint32_t pos;
    /* total size of allocated data */
    uint32_t asize;
    /* end of written data, note that asize !=end normally */
    uint32_t end;
} _FileType;

typedef struct _EnvS {

    /* Each "Task" should at least have a set of std file handles */
    _FileType *_stdin;
    _FileType *_stdout;
    _FileType *_stderr;
} _EnvType;

extern _EnvType *_EnvPtr;

typedef _FileType FILE;

struct FSDriver;

typedef struct FSDriver {
    DeviceType device;
    uint32_t data;
    const char *name;
    /**
     *
     * @param dev
     * @param f
     * @param pathname
     * @return  0       - on success.
     *          ENOENT  - If we can't open it.
     */
    int (*open)(struct FSDriver *dev, FILE *f, const char *pathname);

    /**
     * Close a file
     * @param dev  - The device
     * @param f    - The file to close
     * @return
     */
    int (*close)(struct FSDriver *dev, FILE *f);

    /**
     *
     * @param dev   - The device
     * @param f     - The file to read
     * @param buf   - Buffer to read to
     * @param size  - Number of bytes to read
     * @return
     */
    int (*read)(struct FSDriver *dev, FILE *f, void *buf, size_t size);
    /**
     * Write nbyte to file f from buffer buf.
     *
     * @param dev   - Device pointer
     * @param f     - File
     * @param buf   - Buffer to write
     * @param nbyte - Bytes to write
     * @return
     */
    int (*write)(struct FSDriver *dev, FILE *f, const void *buf, size_t nbyte);
    int (*lseek)(struct FSDriver *dev, FILE *f, int pos);

//	TAILQ_ENTRY(DeviceSerial) nextDevice;
} FSDriverType;


#endif /* FS_H_ */
