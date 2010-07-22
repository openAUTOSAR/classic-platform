/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/









#ifndef COM_MISC_H_
#define COM_MISC_H_

/*
 * The following function are exported only for testing purposes.
 */
uint8 Com_Filter(ComSignal_type *signal);

// Read data from PDU
void Com_ReadSignalDataFromPdu(
		const Com_SignalIdType signalId,
		void *signalData);

void Com_ReadGroupSignalDataFromPdu(
		const Com_SignalIdType parentSignalId,
		const Com_SignalIdType groupSignalId,
		void *signalData);

void Com_ReadSignalDataFromPduBuffer(
		const uint16 signalId,
		const boolean isGroupSignal,
		void *signalData,
		const void *pduBuffer);

// write data to PDU
void Com_WriteSignalDataToPdu(
		const Com_SignalIdType signalId,
		const void *signalData);

void Com_WriteGroupSignalDataToPdu(
		const Com_SignalIdType parentSignalId,
		const Com_SignalIdType groupSignalId,
		const void *signalData);

void Com_WriteSignalDataToPduBuffer(
		const uint16 signalId,
		const boolean isGroupSignal,
		const void *signalData,
		void *pduBuffer);

/*
 * This function copies numBits bits of data from Source to Destination with the possibility to offset
 * both the source and destination.
 *
 * Return value: the last bit it copies (sign bit).
 */
//uint8 Com_CopyData(void *Destination, const void *Source, uint8 numBits, uint8 destOffset, uint8 sourceOffset);

//void Com_CopyData2(char *dest, const char *source, uint8 destByteLength, uint8 segmentStartBitOffset, uint8 segmentBitLength);

void Com_ReadDataSegment(uint8 *dest, const uint8 *source, uint8 destByteLength,
		uint8 segmentStartBitOffset, uint8 segmentBitLength, boolean signedOutput);

void Com_WriteDataSegment(uint8 *pdu, const uint8 *signal, uint8 destByteLength,
		uint8 segmentStartBitOffset, uint8 segmentBitLength);

uint8 motorolaBitNrToPduOffset (uint8 motorolaBitNr);
uint8 intelBitNrToPduOffset (uint8 intelBitNr, uint8 segmentBitLength, uint8 pduBitLenght);


#endif /* COM_MISC_H_ */
