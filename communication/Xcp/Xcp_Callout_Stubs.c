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

#include "Xcp_Callout_Stubs.h"
#include <string.h>

#if (XCP_FEATURE_PROTECTION == STD_ON)

uint8 Xcp_Arc_GetSeed(Xcp_ProtectType res, uint8* seed)
{
    (void)(res);
    strcpy((char*)seed, "HELLO_BELLO");
    return strlen((const char*)seed);
}

Std_ReturnType Xcp_Arc_Unlock(Xcp_ProtectType res, const uint8* seed, uint8 seed_len, const uint8* key, uint8 key_len)
{
    (void)(res);
    if(seed_len != key_len)
        return E_NOT_OK;
    if(memcmp(seed, key, seed_len))
        return E_NOT_OK;
    return E_OK;
}

#endif /* XCP_FEATURE_PROTECTION == STD_ON */


