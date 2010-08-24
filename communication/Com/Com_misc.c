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








#include <stdlib.h>
#include <string.h>

#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"

extern Com_Arc_Config_type Com_Arc_Config;

void Com_ReadSignalDataFromPdu(
			const Com_SignalIdType signalId,
			void *signalData) {

	// Get PDU
	GET_Signal(signalId);
	GET_ArcSignal(Signal->ComHandleId);
	GET_ArcIPdu(Arc_Signal->ComIPduHandleId);

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
	GET_Signal(parentSignalId);
	GET_ArcSignal(Signal->ComHandleId);
	GET_ArcIPdu(Arc_Signal->ComIPduHandleId);

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
		GET_Signal(signalId);
		signalType = Signal->ComSignalType;
		signalEndianess = Signal->ComSignalEndianess;
		signalLength = Signal->ComBitSize / 8;
		bitPosition = Signal->ComBitPosition;
		bitSize = Signal->ComBitSize;
	} else {
		GET_GroupSignal(signalId);
		signalType = GroupSignal->ComSignalType;
		signalEndianess = GroupSignal->ComSignalEndianess;
		signalLength = GroupSignal->ComBitSize / 8;
		bitPosition = GroupSignal->ComBitPosition;
		bitSize = GroupSignal->ComBitSize;
	}

	uint8 destSize = SignalTypeToSize(signalType, signalLength);

	// Pointer to a byte of the source and dest respectively.
	uint8 *signalDataBytes = (uint8 *)signalData;
	uint8 *pduBufferBytes = (uint8 *)pduBuffer;
	uint8 startBitOffset = 0;

	if (signalEndianess != CPU_ENDIANESS) {
		// Swap source bytes before reading
		// TODO: Must adapt to larger PDUs!
		uint8 pduBufferBytes_swap[8];
		int i = 0;
		for (i = 0; i < 8; ++i) {
			pduBufferBytes_swap[i] = pduBufferBytes[7 - i];
		}
		startBitOffset = intelBitNrToPduOffset(bitPosition, bitSize, 64);
		Com_ReadDataSegment(
				signalDataBytes, pduBufferBytes_swap, destSize,
				startBitOffset, bitSize,
				SignalTypeSignedness(signalType));

	} else {
		startBitOffset = motorolaBitNrToPduOffset(bitPosition);
		Com_ReadDataSegment(
				signalDataBytes, pduBufferBytes, destSize,
				startBitOffset, bitSize,
				SignalTypeSignedness(signalType));
	}
}


void Com_WriteSignalDataToPdu(
			const Com_SignalIdType signalId,
			const void *signalData) {

	// Get PDU
	GET_Signal(signalId);
	GET_ArcSignal(Signal->ComHandleId);
	GET_ArcIPdu(Arc_Signal->ComIPduHandleId);

	// Get data
	Com_WriteSignalDataToPduBuffer(
			signalId,
			FALSE,
			signalData,
			Arc_IPdu->ComIPduDataPtr);
}

void Com_WriteGroupSignalDataToPdu(
		const Com_SignalIdType parentSignalId,
		const Com_SignalIdType groupSignalId,
		const void *signalData) {

	// Get PDU
	GET_Signal(parentSignalId);
	GET_ArcSignal(Signal->ComHandleId);
	GET_ArcIPdu(Arc_Signal->ComIPduHandleId);

	// Get data
	Com_WriteSignalDataToPduBuffer(
			groupSignalId,
			TRUE,
			signalData,
			Arc_IPdu->ComIPduDataPtr);
}


void Com_WriteSignalDataToPduBuffer(
			const uint16 signalId,
			const boolean isGroupSignal,
			const void *signalData,
			void *pduBuffer) {
	// TODO: Implement writing little-endian signals

	Com_SignalType signalType;
	uint8 signalLength;
	uint8 bitPosition;
	uint8 bitSize;

	if (!isGroupSignal) {
		GET_Signal(signalId);
		signalType = Signal->ComSignalType;
		signalLength = Signal->ComBitSize / 8;
		bitPosition = Signal->ComBitPosition;
		bitSize = Signal->ComBitSize;
	} else {
		GET_GroupSignal(signalId);
		signalType = GroupSignal->ComSignalType;
		signalLength = GroupSignal->ComBitSize / 8;
		bitPosition = GroupSignal->ComBitPosition;
		bitSize = GroupSignal->ComBitSize;
	}


	uint8 *signalDataBytes = (uint8 *)signalData;
	uint8 *pduBufferBytes = (uint8 *)pduBuffer;
	uint8 startBitOffset = motorolaBitNrToPduOffset(bitPosition);
	uint8 signalBufferSize = SignalTypeToSize(signalType, signalLength);

	Com_WriteDataSegment(pduBufferBytes, signalDataBytes, signalBufferSize, startBitOffset, bitSize);
}

#if (COM_ARC_FILTER_ENABLED == STD_ON)
uint8 Com_Filter(ComSignal_type *signal) {
	GET_ArcSignal(signal->ComHandleId);
	const ComFilter_type * filter = &signal->ComFilter;
	uint8 success = 0;
	if (filter->ComFilterAlgorithm == ALWAYS) {
		success = 1;

	} else if (filter->ComFilterAlgorithm == NEVER) {
		success = 0;

	} else if (filter->ComFilterAlgorithm == ONE_EVERY_N) {
		// Treat the special cases that should not exists.
		if (filter->ComFilterPeriodFactor < 2) {
			// If PeriodFactor is 0 then every package is discarded.
			// If PeriodFactor is 1 then every package is passed through.
			success = filter->ComFilterPeriodFactor;

		} else {
			if (filter->ComFilterArcN == 0) {
				success = 1;
			} else {
				success = 0;
			}
			Arc_Signal->ComFilter.ComFilterArcN++;
			if (filter->ComFilterArcN >= filter->ComFilterPeriodFactor) {
				Arc_Signal->ComFilter.ComFilterArcN = 0;
			}
		}

	} else if (filter->ComFilterAlgorithm == NEW_IS_OUTSIDE) {
		if ((filter->ComFilterMin > filter->ComFilterArcNewValue)
				|| (filter->ComFilterArcNewValue > filter->ComFilterMax)) {
			success = 1;
		} else {
			success = 0;
		}


	} else if (filter->ComFilterAlgorithm == NEW_IS_WITHIN) {
		if (filter->ComFilterMin <= filter->ComFilterArcNewValue
		 && filter->ComFilterArcNewValue <= filter->ComFilterMax) {
			success = 1;
		} else {
			success = 0;
		}


	} else if (filter->ComFilterAlgorithm == MASKED_NEW_DIFFERS_MASKED_OLD) {
		if ((filter->ComFilterArcNewValue & filter->ComFilterMask)
				!= (filter->ComFilterArcNewValue & filter->ComFilterMask)) {
			success = 1;
		} else {
			success = 0;
		}

	} else if (filter->ComFilterAlgorithm == MASKED_NEW_DIFFERS_X) {
		if ((filter->ComFilterArcNewValue & filter->ComFilterMask) != filter->ComFilterX) {
			success = 1;
		} else {
			success = 0;
		}

	} else if (filter->ComFilterAlgorithm == MASKED_NEW_EQUALS_X) {
		if ((filter->ComFilterArcNewValue & filter->ComFilterMask) == filter->ComFilterX) {
			success = 1;
		} else {
			success = 0;
		}
	}

	if (success) {
		Arc_Signal->ComFilter.ComFilterArcOldValue = filter->ComFilterArcNewValue;
		return 1;
	} else return 0;
}
#endif

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

	if ( signedOutput && (*(source + sourceStartByte) & (0x80 >> segmentStartBitOffsetInsideByte)) ) {
		negative = TRUE;
		sourceStartByteMask = (0xFF00 >> segmentStartBitOffsetInsideByte);
		memset(dest, 0xFF, destByteLength);
	} else {
		negative = FALSE;
		sourceStartByteMask = (0x00FF >> segmentStartBitOffsetInsideByte);
		memset(dest, 0x00, destByteLength);
	}

	// setup to point to end (LSB) of buffers
	source += sourceEndByte;
	dest += destByteLength - 1;

	if (negative) {
	// compiles and writes one destination byte on each iteration
		do {
			shiftReg = *(source - sourceByteNr) | 0xFF00;		// read source byte (already matching "byte space")
			if (sourceByteNr == sourceByteLength) {				// if we are on the last source byte..
				shiftReg |= sourceStartByteMask;			// ..we need to mask out stuff we don't want
			}
			shiftReg >>= sourceAlignmentShift;					// shift down to align
			*(dest - destByteNr) &= shiftReg | 0xFF00;		// write into destination byte

			sourceByteNr++;										// move to next source byte
			if (sourceAlignmentShift != 0						// do we have more bits for current dest. byte in this source byte?
					&& sourceByteNr <= sourceByteLength) {
				shiftReg = *(source - sourceByteNr) | 0xFF00;	// read next source byte
				if (sourceByteNr == sourceByteLength) {			// if we are on the last source byte..
					shiftReg |= sourceStartByteMask;		// ..we need to mask out stuff we don't want
				}
				shiftReg = ~(shiftReg);						// shifting inverted to shift in 1:s
				shiftReg <<= 8;								// shift up (to match destination "byte space")
				shiftReg = ~(shiftReg);
				shiftReg >>= sourceAlignmentShift;				// shift down to align
				*(dest - destByteNr) &= shiftReg | 0xFF00;	// write into destination byte
			}
			destByteNr++;
		} while (destByteNr < segmentByteLength);
	} else { // positive
		do {
			shiftReg = *(source - sourceByteNr) & 0x00FF;		// read source byte (already matching "byte space")
			if (sourceByteNr == sourceByteLength) {				// if we are on the last source byte..
				shiftReg &= sourceStartByteMask;			// ..we need to mask out stuff we don't want
			}
			shiftReg >>= sourceAlignmentShift;					// shift down to align
			*(dest - destByteNr) |= shiftReg & 0x00FF;		// write into destination byte

			sourceByteNr++;										// move to next source byte
			if (sourceAlignmentShift != 0						// do we have more bits for current dest. byte in this source byte?
					&& sourceByteNr <= sourceByteLength) {
				shiftReg = *(source - sourceByteNr) & 0x00FF;	// read next source byte
				if (sourceByteNr == sourceByteLength) {			// if we are on the last source byte..
					shiftReg &= sourceStartByteMask;		// ..we need to mask out stuff we don't want
				}
				shiftReg <<= 8;								// shift up (to match destination "byte space")
				shiftReg >>= sourceAlignmentShift;				// shift down to align
				*(dest - destByteNr) |= shiftReg & 0x00FF;	// write into destination byte
			}
			destByteNr++;
		} while (destByteNr < segmentByteLength);
	}
}

/*
 * Copies the <segmentBitLength> least significant bits from <signal> into <pdu>.
 * The bit segment is placed in <pdu> som that the most significant bit ends up
 * at <segmentStartBitOffset> from the msb of <pdu>.
 */
void Com_WriteDataSegment(uint8 *pdu, const uint8 *signal, uint8 destByteLength,
		uint8 segmentStartBitOffset, uint8 segmentBitLength) {
	uint8 pduEndBitOffset = segmentStartBitOffset + segmentBitLength - 1;
	uint8 pduStartByte = segmentStartBitOffset / 8;
	uint8 pduEndByte = (pduEndBitOffset) / 8;
	uint8 pduByteLength = pduEndByte - pduStartByte;

	uint8 segmentStartBitOffsetInsideByte = segmentStartBitOffset % 8;
	uint8 pduStartByteMask = (0xFF >> segmentStartBitOffsetInsideByte);

	uint8 pduAlignmentShift = 7 - (pduEndBitOffset % 8);
	uint8 segmentByteLength = 1 + (segmentBitLength - 1) / 8;
	uint8 pduByteNr = 0;
	uint8 signalByteNr = 0;

	uint16 shiftReg = 0;
	uint16 clearReg = 0;

	// setup to point to end (LSB) of buffers
	pdu += pduEndByte;
	signal += destByteLength - 1;

	// splits and writes one source byte on each iteration
	do {
		shiftReg = *(signal - signalByteNr) & 0x00FF;
		clearReg = 0x00FF;
		shiftReg <<= pduAlignmentShift;
		clearReg <<= pduAlignmentShift;
		if (pduByteNr == pduByteLength) {
			shiftReg &= pduStartByteMask;
			clearReg &= pduStartByteMask;
		}
		*(pdu - pduByteNr) &= ~(clearReg & 0x00FF);
		*(pdu - pduByteNr) |= shiftReg & 0x00FF;

		pduByteNr++;
		if (pduAlignmentShift != 0
				&& pduByteNr <= pduByteLength) {
			shiftReg = *(signal - signalByteNr) & 0x00FF;
			clearReg = 0x00FF;
			shiftReg <<= pduAlignmentShift;
			clearReg <<= pduAlignmentShift;
			shiftReg >>= 8;
			clearReg >>= 8;
			if (pduByteNr == pduByteLength) {
				shiftReg &= pduStartByteMask;
				clearReg &= pduStartByteMask;
			}
			*(pdu - pduByteNr) &= ~(clearReg & 0x00FF);
			*(pdu - pduByteNr) |= shiftReg & 0x00FF;
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
	uint8 offsetInsideByte = motorolaBitNr % 8;
	return byte * 8 + (7 - offsetInsideByte);
}

/*
 * Converts from intel CAN bit nr to PDU bit offset
 *
 *                        pduNumBits: 40
 *  intelBitNr (after PDU byte-swap): 39 38 37 36 35 34 33 32 31 ...  3  2  1  0
 *             intelBitNrToPduOffset:  0  1  2  3  4  5  6  7  8 ... 36 37 38 39
 */
uint8 intelBitNrToPduOffset (uint8 intelBitNr, uint8 segmentBitLength, uint8 pduBitLenght) {
	return pduBitLenght - (intelBitNr + segmentBitLength);
}
