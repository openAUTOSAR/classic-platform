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


#include <string.h>
#include <assert.h>

#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"



void Com_ReadSignalDataFromPdu(
			const Com_SignalIdType signalId,
			void *signalData) {

	// Get PDU
	const ComSignal_type * Signal = GET_Signal(signalId);
	Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(Signal->ComHandleId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Arc_Signal->ComIPduHandleId);

	// Get data
	Com_ReadSignalDataFromPduBuffer(
			signalId,
			FALSE,
			signalData,
			Arc_IPdu->ComIPduDataPtr);
}

void Com_ReadGroupSignalDataFromPdu(
		const Com_SignalIdType parentSignalId,
		const Com_SignalIdType groupSignalId,
		void *signalData) {

	// Get PDU
	const ComSignal_type * Signal = GET_Signal(parentSignalId);
	Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(Signal->ComHandleId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Arc_Signal->ComIPduHandleId);

	// Get data
	Com_ReadSignalDataFromPduBuffer(
			groupSignalId,
			TRUE,
			signalData,
			Arc_IPdu->ComIPduDataPtr);
}

void Com_ReadSignalDataFromPduBuffer(
		const uint16 signalId,
		const boolean isGroupSignal,
		void *signalData,
		const void *pduBuffer) {

	Com_SignalType signalType;
	ComSignalEndianess_type signalEndianess;
	uint8 signalLength;
	uint8 bitPosition;
	uint8 bitSize;

	if (!isGroupSignal) {
		const ComSignal_type * Signal =  GET_Signal(signalId);
		signalType = Signal->ComSignalType;
		signalEndianess = Signal->ComSignalEndianess;
		signalLength = Signal->ComBitSize / 8;
		bitPosition = Signal->ComBitPosition;
		bitSize = Signal->ComBitSize;
	} else {
		const ComGroupSignal_type *GroupSignal = GET_GroupSignal(signalId);
		signalType = GroupSignal->ComSignalType;
		signalEndianess = GroupSignal->ComSignalEndianess;
		signalLength = GroupSignal->ComBitSize / 8;
		bitPosition = GroupSignal->ComBitPosition;
		bitSize = GroupSignal->ComBitSize;
	}

	uint8 destSize = SignalTypeToSize(signalType, signalLength);

	// Pointer to a byte of the source and dest respectively.
	uint8 *signalDataBytes = (uint8 *)signalData;
	uint8 signalDataBytesArray[8];
	const uint8 *pduBufferBytes = (const uint8 *)pduBuffer;
	uint8 startBitOffset = 0;

	if (signalEndianess == COM_LITTLE_ENDIAN) {
		// Swap source bytes before reading
		// TODO: Must adapt to larger PDUs!
		uint8 pduBufferBytes_swap[8];
		for (uint8 i = 0; i < 8; ++i) {
			pduBufferBytes_swap[i] = pduBufferBytes[7 - i];
		}
		startBitOffset = intelBitNrToPduOffset(bitPosition, bitSize, 64);
		//lint -save -esym(960,12.5) PC-Lint Exception: OK. PC-Lint Wrong interpretation of MISRA rule 12.5.
		Com_ReadDataSegment(
				signalDataBytesArray, pduBufferBytes_swap, destSize,
				startBitOffset, bitSize,
				SignalTypeSignedness(signalType));
		//lint -restore
	} else {
		startBitOffset = motorolaBitNrToPduOffset(bitPosition);
		Com_ReadDataSegment(
				signalDataBytesArray, pduBufferBytes, destSize,
				startBitOffset, bitSize,
				SignalTypeSignedness(signalType));
	}

	if (Com_SystemEndianness == COM_BIG_ENDIAN) {
		// Straight copy
		int i;
		for (i = 0; i < destSize; i++) {
			signalDataBytes[i] = signalDataBytesArray[i];
		}

	} else if (Com_SystemEndianness == COM_LITTLE_ENDIAN) {
		// Data copy algorithm creates big-endian output data so we swap
		int i;
		for (i = 0; i < destSize; i++) {
			signalDataBytes[destSize - 1 - i] = signalDataBytesArray[i];
		}
	}

}


void Com_WriteSignalDataToPdu(
			const Com_SignalIdType signalId,
			const void *signalData) {

	// Get PDU
	const ComSignal_type *Signal     = GET_Signal(signalId);
	Com_Arc_Signal_type  *Arc_Signal = GET_ArcSignal(Signal->ComHandleId);
	Com_Arc_IPdu_type    *Arc_IPdu   = GET_ArcIPdu(Arc_Signal->ComIPduHandleId);
	const ComIPdu_type   *IPdu       = GET_IPdu(Arc_Signal->ComIPduHandleId);

	// Get data
	Com_WriteSignalDataToPduBuffer(
			signalId,
			FALSE,
			signalData,
			Arc_IPdu->ComIPduDataPtr,
			IPdu->ComIPduSize);
}

void Com_WriteGroupSignalDataToPdu(
		const Com_SignalIdType parentSignalId,
		const Com_SignalIdType groupSignalId,
		const void *signalData) {

	// Get PDU
	const ComSignal_type *Signal     = GET_Signal(parentSignalId);
	Com_Arc_Signal_type  *Arc_Signal = GET_ArcSignal(Signal->ComHandleId);
	Com_Arc_IPdu_type    *Arc_IPdu   = GET_ArcIPdu(Arc_Signal->ComIPduHandleId);
	const ComIPdu_type   *IPdu       = GET_IPdu(Arc_Signal->ComIPduHandleId);

	// Get data
	Com_WriteSignalDataToPduBuffer(
			groupSignalId,
			TRUE,
			signalData,
			Arc_IPdu->ComIPduDataPtr,
			IPdu->ComIPduSize);
}


void Com_WriteSignalDataToPduBuffer(
			const uint16 signalId,
			const boolean isGroupSignal,
			const void *signalData,
			void *pduBuffer,
			const uint8 pduSize) {
	// TODO: Implement writing little-endian signals

	Com_SignalType signalType;
	uint8 signalLength;
	uint8 bitPosition;
	uint8 bitSize;
	ComSignalEndianess_type endian;

	if (!isGroupSignal) {
		const ComSignal_type * Signal =  GET_Signal(signalId);
		signalType = Signal->ComSignalType;
		signalLength = Signal->ComBitSize / 8;
		bitPosition = Signal->ComBitPosition;
		bitSize = Signal->ComBitSize;
		endian = Signal->ComSignalEndianess;
	} else {
		const ComGroupSignal_type *GroupSignal = GET_GroupSignal(signalId);
		signalType = GroupSignal->ComSignalType;
		signalLength = GroupSignal->ComBitSize / 8;
		bitPosition = GroupSignal->ComBitPosition;
		bitSize = GroupSignal->ComBitSize;
		endian = GroupSignal->ComSignalEndianess;
	}

	uint8 signalBufferSize = SignalTypeToSize(signalType, signalLength);
	uint8 pduSignalMask[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	uint8 signalDataBytesArray[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	const uint8 *signalDataBytes = (const uint8 *)signalData;
	if (Com_SystemEndianness == COM_BIG_ENDIAN) {
		// Straight copy
		int i;
		for (i = 0; i < signalBufferSize; i++) {
			signalDataBytesArray[i] = signalDataBytes[i];
		}

	} else if (Com_SystemEndianness == COM_LITTLE_ENDIAN) {
		// Data copy algorithm assumes big-endian input data so we swap
		int i;
		for (i = 0; i < signalBufferSize; i++) {
			signalDataBytesArray[signalBufferSize - 1 - i] = signalDataBytes[i];
		}
	}

	if (endian == COM_BIG_ENDIAN) {
		uint8 *pduBufferBytes = (uint8 *)pduBuffer;
		uint8 startBitOffset = motorolaBitNrToPduOffset(bitPosition);

		Com_WriteDataSegment(pduBufferBytes, pduSignalMask,
				signalDataBytesArray, signalBufferSize, startBitOffset, bitSize);

	} else {
		uint8 startBitOffset = intelBitNrToPduOffset(bitPosition, bitSize, pduSize * 8);
		uint8 pduBufferBytesSwapped[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

		Com_WriteDataSegment(pduBufferBytesSwapped, pduSignalMask,
				signalDataBytesArray, signalBufferSize, startBitOffset, bitSize);

		uint8 *pduBufferBytes = (uint8 *)pduBuffer;
		uint8 i;
		for (i = 0; i < pduSize; i++) {
			pduBufferBytes[ i ]  &=       ~( pduSignalMask[ (pduSize - 1) - i ] );
			pduBufferBytes[ i ]  |=  pduBufferBytesSwapped[ (pduSize - 1) - i ];
		}
	}
}


/*
 * Read an arbitrary signal segment from buffer.
 * dest:                   pointer to start of destination buffer
 * source:                 pointer to start of source buffer
 * destByteLength:         size of destination buffer in bytes
 * segmentStartBitOffset:  bit offset to signal segment (from first bit in *source data)
 * segmentBitLength:       length in bits of the signal segment to be read
 *
 * Example:
 *    Source data: (ABC... = signal segment)
 *    | -----ABC | DEFGHIJK | LMNOPQ-- |
 *         B0         B1         B2
 *  Parameters:
 *    dest:                  pointer to 32 bit space (needs to be at least 3 bytes to keep the 17 signal bits)
 *    destByteLength:          4
 *    source:                *B0
 *    segmentStartBitOffset:   5
 *    segmentBitLength:       17
 *
 *  Result:
 *    Destination:
 *    | -------- | -------A | BCDEFGHI | JKLMNOPQ |
 *
 */
void Com_ReadDataSegment(uint8 *dest, const uint8 *source, uint8 destByteLength,
		uint8 segmentStartBitOffset, uint8 segmentBitLength, boolean signedOutput) {

	uint8 sourceEndBitOffset = segmentStartBitOffset + segmentBitLength - 1;
	uint8 sourceStartByte = segmentStartBitOffset / 8;
	uint8 sourceEndByte = (sourceEndBitOffset) / 8;
	uint8 sourceByteLength = sourceEndByte - sourceStartByte;

	uint8 segmentStartBitOffsetInsideByte = segmentStartBitOffset % 8;
	uint16 sourceStartByteMask;

	uint8 sourceAlignmentShift = 7 - (sourceEndBitOffset % 8);
	uint8 segmentByteLength = 1 + (segmentBitLength - 1) / 8;
	uint8 sourceByteNr = 0;
	uint8 destByteNr = 0;

	uint16 shiftReg = 0;

	boolean negative;

	if ( signedOutput && (*(source + sourceStartByte) & (0x80u >> segmentStartBitOffsetInsideByte)) ) {
		negative = TRUE;
		sourceStartByteMask = (0xFF00u >> segmentStartBitOffsetInsideByte);
		memset(dest, 0xFF, destByteLength);
	} else {
		negative = FALSE;
		sourceStartByteMask = (0x00FFu >> segmentStartBitOffsetInsideByte);
		memset(dest, 0x00u, destByteLength);
	}

	// setup to point to end (LSB) of buffers
	source += sourceEndByte;
	dest += destByteLength - 1;

	if (negative) {
	// compiles and writes one destination byte on each iteration
		do {
			shiftReg = *(source - sourceByteNr) | 0xFF00u;		// read source byte (already matching "byte space")
			if (sourceByteNr == sourceByteLength) {				// if we are on the last source byte..
				shiftReg |= sourceStartByteMask;			// ..we need to mask out stuff we don't want
			}
			shiftReg >>= sourceAlignmentShift;					// shift down to align
			*(dest - destByteNr) &= shiftReg | 0xFF00u;		// write into destination byte

			sourceByteNr++;										// move to next source byte
			if ( (sourceAlignmentShift != 0)						// do we have more bits for current dest. byte in this source byte?
					&& (sourceByteNr <= sourceByteLength) ) {
				shiftReg = *(source - sourceByteNr) | 0xFF00u;	// read next source byte
				if (sourceByteNr == sourceByteLength) {			// if we are on the last source byte..
					shiftReg |= sourceStartByteMask;		// ..we need to mask out stuff we don't want
				}
				shiftReg = (uint16)~(shiftReg);						// shifting inverted to shift in 1:s
				shiftReg <<= 8;								// shift up (to match destination "byte space")
				shiftReg = (uint16)~shiftReg;
				shiftReg >>= sourceAlignmentShift;				// shift down to align
				*(dest - destByteNr) &= shiftReg | 0xFF00u;	// write into destination byte
			}
			destByteNr++;
		} while (destByteNr < segmentByteLength);
	} else { // positive
		do {
			shiftReg = *(source - sourceByteNr) & 0x00FFu;		// read source byte (already matching "byte space")
			if (sourceByteNr == sourceByteLength) {				// if we are on the last source byte..
				shiftReg &= sourceStartByteMask;			// ..we need to mask out stuff we don't want
			}
			shiftReg >>= sourceAlignmentShift;					// shift down to align
			*(dest - destByteNr) |= shiftReg & 0x00FFu;		// write into destination byte

			sourceByteNr++;										// move to next source byte
			if (sourceAlignmentShift != 0						// do we have more bits for current dest. byte in this source byte?
					&& sourceByteNr <= sourceByteLength) {
				shiftReg = *(source - sourceByteNr) & 0x00FFu;	// read next source byte
				if (sourceByteNr == sourceByteLength) {			// if we are on the last source byte..
					shiftReg &= sourceStartByteMask;		// ..we need to mask out stuff we don't want
				}
				shiftReg <<= 8;								// shift up (to match destination "byte space")
				shiftReg >>= sourceAlignmentShift;				// shift down to align
				*(dest - destByteNr) |= shiftReg & 0x00FFu;	// write into destination byte
			}
			destByteNr++;
		} while (destByteNr < segmentByteLength);
	}
}

/*
 * Copies the <segmentBitLength> least significant bits from <signal> into <pdu>.
 * The bit segment is placed in <pdu> so that the most significant bit ends up
 * at <segmentStartBitOffset> from the msb of <pdu>.
 * <pduSignalMask> is cleared and written to contain a mask with 1´s where the
 * signal is located in the <pdu>.
 */
void Com_WriteDataSegment(uint8 *pdu, uint8 *pduSignalMask, const uint8 *signalDataPtr, uint8 destByteLength,
		uint8 segmentStartBitOffset, uint8 segmentBitLength) {
	uint8 pduEndBitOffset = segmentStartBitOffset + segmentBitLength - 1;
	uint8 pduStartByte = segmentStartBitOffset / 8;
	uint8 pduEndByte = (pduEndBitOffset) / 8;
	uint8 pduByteLength = pduEndByte - pduStartByte;

	uint8 segmentStartBitOffsetInsideByte = segmentStartBitOffset % 8;
	uint8 pduStartByteMask = (0xFFu >> segmentStartBitOffsetInsideByte);

	uint8 pduAlignmentShift = 7 - (pduEndBitOffset % 8);
	uint8 segmentByteLength = 1 + (segmentBitLength - 1) / 8;
	uint8 pduByteNr = 0;
	uint8 signalByteNr = 0;

	uint16 shiftReg = 0;
	uint16 clearReg = 0;

	// clear pduSignalMask all the way from 0
	memset(pduSignalMask, 0x00, pduEndByte);

	// setup to point to end (LSB) of buffers
	pdu           += pduEndByte;
	pduSignalMask += pduEndByte;

	signalDataPtr += destByteLength - 1;

	// splits and writes one source byte on each iteration
	do {
		shiftReg = *(signalDataPtr - signalByteNr) & 0x00FFu;
		clearReg = 0x00FF;
		//lint -save -e701 -e734 //PC-Lint Wrong interpretation of MISRA rule 10.5.
		shiftReg <<= pduAlignmentShift;
		clearReg <<= pduAlignmentShift;
		//lint -restore
		if (pduByteNr == pduByteLength) {
			shiftReg &= pduStartByteMask;
			clearReg &= pduStartByteMask;
		}
		*(pdu           - pduByteNr) &= (uint16)~(clearReg & 0x00FFu);
		*(pduSignalMask - pduByteNr) |= (uint16) (clearReg & 0x00FFu);
		*(pdu           - pduByteNr) |= shiftReg & 0x00FFu;

		pduByteNr++;
		if ( (pduAlignmentShift != 0)
				&& (pduByteNr <= pduByteLength) ) {
			shiftReg = *(signalDataPtr - signalByteNr) & 0x00FFu;
			clearReg = 0x00FF;
			//lint -save -e701 -e734 //PC-Lint Wrong interpretation of MISRA rule 10.5.
			shiftReg <<= pduAlignmentShift;
			clearReg <<= pduAlignmentShift;
			shiftReg >>= 8;
			clearReg >>= 8;
			//lint -restore
			if (pduByteNr == pduByteLength) {
				shiftReg &= pduStartByteMask;
				clearReg &= pduStartByteMask;
			}
			*(pdu           - pduByteNr) &= (uint16)~(clearReg & 0x00FFu);
			*(pduSignalMask - pduByteNr) |= (uint16) (clearReg & 0x00FFu);
			*(pdu           - pduByteNr) |= shiftReg & 0x00FFu;
		}
		signalByteNr++;
	} while (signalByteNr < segmentByteLength);
}

/*
 * Converts from motorola CAN bit nr to PDU bit offset
 *
 *             motorolaBitNr:  7  6  5  4  3  2  1  0 15 14 13 12 ...
 *  motorolaBitNrToPduOffset:  0  1  2  3  4  5  6  7  8  9 10 11 ...
 */
uint8 motorolaBitNrToPduOffset (uint8 motorolaBitNr) {
	uint8 byte = motorolaBitNr / 8;
	uint8 offsetToByteStart = (uint8) (byte * 8u);
	uint8 offsetInsideByte = motorolaBitNr % 8;
	return (uint8) (offsetToByteStart + (7u - offsetInsideByte));
}

/*
 * Converts from intel CAN bit nr to PDU bit offset
 *
 *                        pduNumBits: 40
 *  intelBitNr (after PDU byte-swap): 39 38 37 36 35 34 33 32 31 ...  3  2  1  0
 *             intelBitNrToPduOffset:  0  1  2  3  4  5  6  7  8 ... 36 37 38 39
 */
uint8 intelBitNrToPduOffset (uint8 intelBitNr, uint8 segmentBitLength, uint8 pduBitLength) {
	return pduBitLength - (intelBitNr + segmentBitLength);
}
