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

#ifndef SOAD_CALLOUTS_H
#define SOAD_CALLOUTS_H

#include "Std_Types.h"


/**
 * Callback from SoAd/DoIp BSW to fetch Vehicle Identification Number (VIN).
 *
 * If no VIN is available, buffer need not be filled, but E_NOT_OK is to be
 * returned. If VIN is available, buffer needs to be filled and E_OK is to be
 * returned.
 *
 * @param buf Pointer to buffer where VIN shall be stored
 * @param len Size (in bytes) of the buffer where VIN is to be stored.
 * @return E_OK if buffer was filled with valid data, E_NOT_OK otherwise.
 */
Std_ReturnType SoAd_DoIp_Arc_GetVin(uint8* buf, uint8 len);


/**
 * Callback from SoAd/DoIp BSW to fetch EID.
 *
 * If no EID is available, buffer need not be filled, but E_NOT_OK is to be
 * returned. If EID is available, buffer needs to be filled and E_OK is to be
 * returned.
 *
 * @param buf Pointer to buffer where EID shall be stored
 * @param len Size (in bytes) of the buffer where EID is to be stored.
 * @return E_OK if buffer was filled with valid data, E_NOT_OK otherwise.
 */
Std_ReturnType SoAd_DoIp_Arc_GetEid(uint8* buf, uint8 len);


/**
 * Callback from SoAd/DoIp BSW to fetch GID.
 *
 * If no GID is available, buffer need not be filled, but E_NOT_OK is to be
 * returned. If GID is available, buffer needs to be filled and E_OK is to be
 * returned.
 *
 * @param buf Pointer to buffer where GID shall be stored
 * @param len Size (in bytes) of the buffer where GID is to be stored.
 * @return E_OK if buffer was filled with valid data, E_NOT_OK otherwise.
 */
Std_ReturnType SoAd_DoIp_Arc_GetGid(uint8* buf, uint8 len);

/**
 * Callback from SoAd/DoIp BSW to application in order to determine if the
 * vehicle is ready to initiate a full diagnostic session.
 *
 * If no GID is available, buffer need not be filled, but E_NOT_OK is to be
 * returned. If GID is available, buffer needs to be filled and E_OK is to be
 * returned.
 *
 * @param buf Pointer to buffer where GID shall be stored
 * @param len Size (in bytes) of the buffer where GID is to be stored.
 * @return E_OK if buffer was filled with valid data, E_NOT_OK otherwise.
 */
Std_ReturnType SoAd_DoIp_Arc_GetFurtherActionRequired(uint8* buf);

/**
 * Callback from SoAd/DoIp BSW to determine the desired hostname.
 * Application is responsible for supplying this buffer and may not alter the
 * buffer during the course of life. New value
 *
 * @param buf Pointer to string (char**) that contains the zero-terminated
 *            preferred hostname.
 * @param buflen Length in bytes of the preferred hostname.
 * @return E_OK if a preferred hostname is available. E_NOT_OK otherwise.
 */
Std_ReturnType SoAd_DoIp_Arc_GetHostname(uint8** buf, uint8* buflen);



#endif
