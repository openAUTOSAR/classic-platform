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

#include "Xcp.h"
#include "Xcp_Internal.h"
#include <string.h>

#if(XCP_FEATURE_DIO == STD_ON)
#include "Dio.h"
#endif


/**
 * Read a character from MTA
 * @return
 */
static uint8 Xcp_MtaGetMemory(Xcp_MtaType* mta)
{
    return *(uint8*)(mta->address++);
}

/**
 * Read a character from memory
 * @return
 */
static void Xcp_MtaReadMemory(Xcp_MtaType* mta, uint8* data, int len)
{
    memcpy(data, (void*)mta->address, len);
    mta->address += len;
}

/**
 * Write a character to memory
 * @return
 */
static void Xcp_MtaWriteMemory(Xcp_MtaType* mta, uint8* data, int len)
{
    memcpy((void*)mta->address, data, len);
    mta->address += len;
}

#if 0
/**
 * Write a character to MTA
 * @param val
 */
static void Xcp_MtaPutMemory(Xcp_MtaType* mta, uint8 val)
{
    *(uint8*)(mta->address++) = val;
}
#endif

#if(XCP_FEATURE_DIO == STD_ON)
/**
 * Read a character from DIO
 * @return
 */
static uint8 Xcp_MtaGetDioPort(Xcp_MtaType* mta)
{
    unsigned int offset = mta->address % sizeof(Dio_PortLevelType);
    Dio_PortType port   = mta->address / sizeof(Dio_PortLevelType);

    if(offset == 0) {
        mta->buffer = Dio_ReadPort(port);
    }
    mta->address++;
    return (mta->buffer >> (offset * 8)) & 0xFF;
}


/**
 * Flush data in buffer to DIO
 * @return
 */
static void Xcp_MtaFlushDioPort(Xcp_MtaType* mta)
{
    Dio_PortType port = mta->address / sizeof(Dio_PortLevelType);
    Dio_WritePort(port, mta->buffer);
    mta->buffer = 0;
}

#if 0
/**
 * Write a character to DIO
 * @param val
 */
static void Xcp_MtaPutDioPort(Xcp_MtaType* mta, uint8 val)
{
    unsigned int offset = mta->address % sizeof(Dio_PortLevelType);
    mta->buffer = (mta->buffer & ~(0xFFu << offset)) | (val << offset);
    mta->address++;
    if(offset == 0) {
        Xcp_MtaFlushDioPort(mta);
    }
}
#endif

/**
 * Read a character from a DIO channel
 * @return
 */
static uint8 Xcp_MtaGetDioChan(Xcp_MtaType* mta)
{
    if(Dio_ReadChannel(mta->address++) == STD_HIGH)
        return 1;
    else
        return 0;
}

#if 0
/**
 * Write a character to DIO channel
 * @param val
 */
static void Xcp_MtaPutDioChan(Xcp_MtaType* mta, uint8 val)
{
    if(val == 1)
        Dio_WriteChannel(mta->address++, STD_HIGH);
    else
        Dio_WriteChannel(mta->address++, STD_LOW);
}
#endif


/**
 * Generic function that writes character to mta using put
 * @param val
 */
static void Xcp_MtaWriteGeneric(Xcp_MtaType* mta, uint8* data, int len)
{
    while(len-- > 0) {
        mta->put(mta, *(data++));    }
}


/**
 * Generic function that reads buffer from mta using get
 * @param val
 */
static void Xcp_MtaReadGeneric(Xcp_MtaType* mta, uint8* data, int len)
{
    while(len-- > 0) {
        *(data++) = mta->get(mta);
    }
}
#endif /* XCP_FEATURE_DIO == STD_ON */

/**
 * Set the MTA pointer to given address on given extension
 * @param address
 * @param extension
 */
void Xcp_MtaInit(Xcp_MtaType* mta, uint32 address, uint8 extension)
{
    mta->address   = address;
    mta->extension = extension;

    if(extension == XCP_MTA_EXTENSION_MEMORY) {
        mta->get   = Xcp_MtaGetMemory;
#if 0
        mta->put   = Xcp_MtaPutMemory;
#endif
        mta->read  = Xcp_MtaReadMemory;
        mta->write = Xcp_MtaWriteMemory;
        mta->flush = NULL;
    } else if(extension == XCP_MTA_EXTENSION_FLASH) {
        mta->get   = Xcp_MtaGetMemory;
#if 0
        mta->put   = NULL;
#endif
        mta->read  = Xcp_MtaReadMemory;
        mta->write = NULL;
        mta->flush = NULL;
#if(XCP_FEATURE_DIO == STD_ON)
    } else if(extension == XCP_MTA_EXTENSION_DIO_PORT) {
        mta->get   = Xcp_MtaGetDioPort;
#if 0
        mta->put   = Xcp_MtaPutDioPort;
#endif
        mta->read  = Xcp_MtaReadGeneric;
        mta->write = Xcp_MtaWriteGeneric;
        mta->flush = Xcp_MtaFlushDioPort;
        /* if not aligned to start of port, we must fill buffer */
        unsigned int offset = address % sizeof(Dio_PortLevelType);
        mta->address -= offset;
        while(offset--)
            (void)Xcp_MtaGetDioPort(mta);
    } else if(extension == XCP_MTA_EXTENSION_DIO_CHAN) {
        mta->get   = Xcp_MtaGetDioChan;
#if 0
        mta->put   = Xcp_MtaPutDioChan;
#endif
        mta->read  = Xcp_MtaReadGeneric;
        mta->write = Xcp_MtaWriteGeneric;
#endif /* XCP_FEATURE_DIO == STD_ON */
    } else {
        mta->get   = NULL;
#if 0
        mta->put   = NULL;
#endif
        mta->read  = NULL;
        mta->write = NULL;
        mta->flush = NULL;
    }
}

