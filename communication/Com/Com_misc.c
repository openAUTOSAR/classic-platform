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


//lint -esym(960,8.7)	PC-Lint misunderstanding of Misra 8.7 for Com_SystenEndianness and endianess_test

#include <string.h>
#include <assert.h>

#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include "debug.h"
#include "Cpu.h"

static void Com_ReadDataSegment(uint8 *dest, const uint8 *source, uint8 destByteLength,
		Com_BitPositionType segmentStartBitOffset, uint8 segmentBitLength, boolean signedOutput);

static void Com_WriteDataSegment(uint8 *pdu, uint8 *pduSignalMask, const uint8 *signalDataPtr, uint8 destByteLength,
		Com_BitPositionType segmentStartBitOffset, uint8 segmentBitLength);

void Com_CopySignalGroupDataFromShadowBuffer(const Com_SignalIdType signalGroupId) {

	// Get PDU
	const ComSignal_type * Signal = GET_Signal(signalGroupId);
	const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);
	uint8 pduSize = IPdu->ComIPduSize;

	const void* pduDataPtr = 0;
	if (IPdu->ComIPduSignalProcessing == DEFERRED && IPdu->ComIPduDirection == RECEIVE) {
		pduDataPtr = IPdu->ComIPduDeferredDataPtr;
	} else {
		pduDataPtr = IPdu->ComIPduDataPtr;
	}

	imask_t state;
	Irq_Save(state);

	// Get data, Aligned opaque data -> straight copy
	memcpy((void *)Signal->Com_Arc_ShadowBuffer, pduDataPtr, pduSize);

	Irq_Restore(state);
}

void Com_ReadSignalDataFromPduBuffer(
		const uint16 signalId,
		const boolean isGroupSignal,
		void *signalData,
		const void *pduBuffer,
		uint8 pduSize) {

	Com_SignalType signalType;
	ComSignalEndianess_type signalEndianess;
	uint8 signalLength;
	Com_BitPositionType bitPosition;
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
	const uint8 *pduBufferBytes = (const uint8 *)pduBuffer + (bitPosition/8);
	Com_BitPositionType startBitOffset = 0;
	imask_t state;
	Irq_Save(state);

	if (signalEndianess == COM_OPAQUE || signalType == UINT8_N) {
		// Aligned opaque data -> straight copy
		memcpy(signalDataBytes, pduBufferBytes, destSize);

	} else {
		// Unaligned data and/or endianness conversion

		if (signalEndianess == COM_LITTLE_ENDIAN) {
			// Swap source bytes before reading
			// TODO: Must adapt to larger PDUs!
			uint8 pduBufferBytes_swap[8];
			for (uint8 i = 0; i < 8; ++i) {
				pduBufferBytes_swap[i] = pduBufferBytes[7 - i];
			}
			startBitOffset = intelBitNrToPduOffset(bitPosition%8, bitSize, 64);
			//lint -save -esym(960,12.5) PC-Lint Exception: OK. PC-Lint Wrong interpretation of MISRA rule 12.5.
			Com_ReadDataSegment(
					signalDataBytesArray, pduBufferBytes_swap, destSize,
					startBitOffset, bitSize,
					SignalTypeSignedness(signalType));
			//lint -restore
		} else {
			startBitOffset = motorolaBitNrToPduOffset(bitPosition%8);
			Com_ReadDataSegment(
					signalDataBytesArray, pduBufferBytes, destSize,
					startBitOffset, bitSize,
					SignalTypeSignedness(signalType));
		}

		if (Com_SystemEndianness == COM_BIG_ENDIAN) {
			// Straight copy
			uint8 i;
			for (i = 0; i < destSize; i++) {
				signalDataBytes[i] = signalDataBytesArray[i];
			}

		} else if (Com_SystemEndianness == COM_LITTLE_ENDIAN) {
			// Data copy algorithm creates big-endian output data so we swap
			uint8 i;
			for (i = 0; i < destSize; i++) {
				signalDataBytes[(destSize - 1) - i] = signalDataBytesArray[i];
			}
		} else {
			//lint --e(506)	PC-Lint exception Misra 13.7, 14.1, Allow boolean to always be false.
			assert(0);
		}
	}
	Irq_Restore(state);
}


void Com_WriteSignalDataToPdu(
			const Com_SignalIdType signalId,
			const void *signalData) {

	// Get PDU
	const ComSignal_type *Signal     = GET_Signal(signalId);
	const ComIPdu_type   *IPdu       = GET_IPdu(Signal->ComIPduHandleId);

	// Get data
	Com_WriteSignalDataToPduBuffer(
			signalId,
			FALSE,
			signalData,
			IPdu->ComIPduDataPtr,
			IPdu->ComIPduSize);
}

void Com_WriteGroupSignalDataToPdu(
		const Com_SignalIdType parentSignalId,
		const Com_SignalIdType groupSignalId,
		const void *signalData) {

	// Get PDU
	const ComSignal_type *Signal     = GET_Signal(parentSignalId);
	const ComIPdu_type   *IPdu       = GET_IPdu(Signal->ComIPduHandleId);

	// Get data
	Com_WriteSignalDataToPduBuffer(
			groupSignalId,
			TRUE,
			signalData + (GET_GroupSignal(groupSignalId)->ComBitPosition / 8), // TODO, can we make sure group signals is on byte boundary
			(void *)IPdu->ComIPduDataPtr,
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
	Com_BitPositionType bitPosition;
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
	imask_t irq_state;

	Irq_Save(irq_state);
	if (endian == COM_OPAQUE || signalType == UINT8_N) {
		//assert(bitPosition % 8 == 0);
		//assert(bitSize % 8 == 0);
		uint8 *pduBufferBytes = (uint8 *)pduBuffer;
		uint8 startFromPduByte = bitPosition / 8;
		memcpy(pduBufferBytes + startFromPduByte, signalDataBytes, signalLength);
	} else {
		if (Com_SystemEndianness == COM_BIG_ENDIAN) {
			// Straight copy
			uint8 i;
			for (i = 0; i < signalBufferSize; i++) {
				signalDataBytesArray[i] = signalDataBytes[i];
			}

		} else if (Com_SystemEndianness == COM_LITTLE_ENDIAN) {
			// Data copy algorithm assumes big-endian input data so we swap
			uint8 i;
			for (i = 0; i < signalBufferSize; i++) {
				signalDataBytesArray[(signalBufferSize - 1) - i] = signalDataBytes[i];
			}
		} else {
			//lint --e(506)	PC-Lint exception Misra 13.7, 14.1, Allow boolean to always be false.
			assert(0);
		}

		if (endian == COM_BIG_ENDIAN) {
			Com_BitPositionType startBitOffset = motorolaBitNrToPduOffset(bitPosition%8);
			uint8 pduBufferBytesStraight[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

			Com_WriteDataSegment(pduBufferBytesStraight, pduSignalMask,
					signalDataBytesArray, signalBufferSize, startBitOffset, bitSize);

			// Straight copy into real pdu buffer (with mutex)
			uint8 *pduBufferBytes = ((uint8 *)pduBuffer)+(bitPosition/8);
			uint8 i;
			for (i = 0; i < 8; i++) {
				pduBufferBytes[ i ]  &=        ~( pduSignalMask[ i ] );
				pduBufferBytes[ i ]  |=  pduBufferBytesStraight[ i ];
			}

		} else {
			uint8 startBitOffset = intelBitNrToPduOffset(bitPosition%8, bitSize, 64);
			uint8 pduBufferBytesSwapped[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

			Com_WriteDataSegment(pduBufferBytesSwapped, pduSignalMask,
					signalDataBytesArray, signalBufferSize, startBitOffset, bitSize);

			// Swapped copy into real pdu buffer (with mutex)
			uint8 *pduBufferBytes = ((uint8 *)pduBuffer)+(bitPosition/8);
			uint8 i;
			// actually it is only necessary to iterate through the bytes that are written.
			for (i = 0; i < 8; i++) {
				pduBufferBytes[ i ]  &=       ~( pduSignalMask[ (8 - 1) - i ] );
				pduBufferBytes[ i ]  |=  pduBufferBytesSwapped[ (8 - 1) - i ];
			}
		}
	}
	Irq_Restore(irq_state);
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
static void Com_ReadDataSegment(uint8 *dest, const uint8 *source, uint8 destByteLength,
		Com_BitPositionType segmentStartBitOffset, uint8 segmentBitLength, boolean signedOutput) {

	Com_BitPositionType sourceEndBitOffset = segmentStartBitOffset + segmentBitLength - 1;
	Com_BitPositionType sourceStartByte = segmentStartBitOffset / 8;
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
		Com_BitPositionType segmentStartBitOffset, uint8 segmentBitLength) {
	Com_BitPositionType pduEndBitOffset = segmentStartBitOffset + segmentBitLength - 1;
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
Com_BitPositionType motorolaBitNrToPduOffset (Com_BitPositionType motorolaBitNr) {
	uint8 byte = motorolaBitNr / 8;
	Com_BitPositionType offsetToByteStart = (Com_BitPositionType) (byte * 8u);
	Com_BitPositionType offsetInsideByte = motorolaBitNr % 8;
	return (Com_BitPositionType) (offsetToByteStart + (7u - offsetInsideByte));
}

/*
 * Converts from intel CAN bit nr to PDU bit offset
 *
 *                        pduNumBits: 40
 *  intelBitNr (after PDU byte-swap): 39 38 37 36 35 34 33 32 31 ...  3  2  1  0
 *             intelBitNrToPduOffset:  0  1  2  3  4  5  6  7  8 ... 36 37 38 39
 */
Com_BitPositionType intelBitNrToPduOffset (Com_BitPositionType intelBitNr, Com_BitPositionType segmentBitLength, Com_BitPositionType pduBitLength) {
	return pduBitLength - (intelBitNr + segmentBitLength);
}

void Com_RxProcessSignals(const ComIPdu_type *IPdu,Com_Arc_IPdu_type *Arc_IPdu) {
	const ComSignal_type *comSignal;
	for (uint8 i = 0; IPdu->ComIPduSignalRef[i] != NULL; i++) {
		comSignal = IPdu->ComIPduSignalRef[i];
		Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(comSignal->ComHandleId);

		// If this signal uses an update bit, then it is only considered if this bit is set.
		if ( (!comSignal->ComSignalArcUseUpdateBit) ||
			( (comSignal->ComSignalArcUseUpdateBit) && (TESTBIT(IPdu->ComIPduDataPtr, comSignal->ComUpdateBitPosition)) ) ) {

			if (comSignal->ComTimeoutFactor > 0) { // If reception deadline monitoring is used.
				// Reset the deadline monitoring timer.
				Arc_Signal->Com_Arc_DeadlineCounter = comSignal->ComTimeoutFactor;
			}

			// Check the signal processing mode.
			if (IPdu->ComIPduSignalProcessing == IMMEDIATE) {
				// If signal processing mode is IMMEDIATE, notify the signal callback.
				if (IPdu->ComIPduSignalRef[i]->ComNotification != NULL) {
					IPdu->ComIPduSignalRef[i]->ComNotification();
				}

			} else {
				// Signal processing mode is DEFERRED, mark the signal as updated.
				Arc_Signal->ComSignalUpdated = 1;
			}

		} else {
			DEBUG(DEBUG_LOW, "Com_RxIndication: Ignored signal %d of I-PD %d since its update bit was not set\n", comSignal->ComHandleId, ComRxPduId);
		}
	}
}
void UnlockTpBuffer(PduIdType PduId) {
	Com_BufferPduState[PduId].locked = false;
	Com_BufferPduState[PduId].currentPosition = 0;
}

boolean isPduBufferLocked(PduIdType id) {
	imask_t state;
	Irq_Save(state);
	boolean bufferLocked = Com_BufferPduState[id].locked;
	Irq_Restore(state);
	if (bufferLocked) {
		return true;
	} else {
		return false;
	}
}
PduIdType getPduId(const ComIPdu_type* IPdu) {
	return (PduIdType)(IPdu - (ComConfig->ComIPdu));
}
