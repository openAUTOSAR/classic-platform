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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.3 */

#include "Xcp_Internal.h"

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
static void Xcp_MtaReadMemory(Xcp_MtaType* mta, uint8* data, uint32 len)
{
    Xcp_MemCpy(data, (void*)mta->address, len);
    mta->address += len;
}

/**
 * Write a character to memory
 * @return
 */
static void Xcp_MtaWriteMemory(Xcp_MtaType* mta, uint8* data, uint32 len)
{
    Xcp_MemCpy((void*)mta->address, data, len);
    mta->address += len;
}

/**
 * Set the MTA pointer to given address on given extension
 * @param address
 * @param extension
 */
void Xcp_MtaInit(Xcp_MtaType* mta, uint32 address, uint8 extension)
{
    mta->address   = address;
    mta->extension = extension;

    if(extension == XCP_MTA_EXTENSION_MEMORY)
    {
        mta->get   = Xcp_MtaGetMemory;
        mta->read  = Xcp_MtaReadMemory;
        mta->write = Xcp_MtaWriteMemory;
        mta->flush = NULL;
    }
    else if(extension == XCP_MTA_EXTENSION_FLASH)
    {
        mta->get   = Xcp_MtaGetMemory;
        mta->read  = Xcp_MtaReadMemory;
        mta->write = NULL;
        mta->flush = NULL;
    }
    else
    {
        mta->get   = NULL;
        mta->read  = NULL;
        mta->write = NULL;
        mta->flush = NULL;
    }
}

