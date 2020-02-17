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


#ifndef MYCDDPDUR_H_
#define MYCDDPDUR_H_

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "CddPduR_Cfg.h"


/**
 * Send a buffer
 *
 * @param id		The PDU id
 * @param dataPtr	Pointer to the buffer to send.
 * @param length    The length of the buffer.
 * @return	E_OK :     The send buffer was accepted by lower layers
 * 			E_NOT_OK : The channel is busy
 */
Std_ReturnType CddPduR_Send(PduIdType id, const void* dataPtr, uint16 length );


/**
 * Provide a Rx buffer to receive data in.
 *
 * @param id		Pdu ID
 * @param dataPtr	Pointer to the buffer
 * @param length	THe length of buffer
 * @return
 */
Std_ReturnType CddPduR_ProvideBuffer( PduIdType id, void* dataPtr, uint16 length );

/**
 * Receive the
 *
 * @param 		id			Pdu ID
 * @param[out] 	dataPtrPtr	Pointer to where the data is.
 *                          This is a memory area once allocated by CddPduR_ProvideBuffer()
 * @param[out]	length      The length of the received message.
 * @return		E_OK
 *              E_NOT_OK
 */
Std_ReturnType CddPduR_Receive( PduIdType id, void** dataPtrPtr, uint16* length );

/**
 * Unlocks a buffer received by CddPduR_Receive()
 *
 * @param id
 * @return
 */
Std_ReturnType CddPduR_UnlockBuffer( PduIdType id );

#endif /* MYCDDPDUR_H_ */
