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
#include "Com_misc.h"


void Com_CopyFromSignal(const ComSignal_type *signal, void *Destination) {
	// Reset destination (for easier sign extension)
	memset(Destination, 0, SignalTypeToSize(signal->ComSignalType, signal->ComSignalLength));

	// Variables to store the source and destination bytes we are currently looking at.
	uint8 sourceByte;
	uint8 destByte;

	// Pointer to a byte of the source and dest respectively.
	ComGetArcSignal(signal->ComHandleId);
	ComGetArcIPdu(Arc_Signal->ComIPduHandleId);
	uint8 *source = (uint8 *)Arc_IPdu->ComIPduDataPtr;
	uint8 *dest = (uint8 *)Destination;

	uint8 signBit = 0;


	if (signal->ComSignalEndianess != CPU_ENDIANESS && 0) {

		uint8 numBytes = ((signal->ComBitPosition + signal->ComBitSize) / 8) + 1;
		uint8 *sourceEndianConverted = malloc(numBytes) ; // NOT OK TO USE MALLOC! BUT IS THERE ANOTHER WAY?
		//memcpy(sourceEndianConverted, source, numBytes);

		// Reverse all bits in this temporary IPdu.
		for(int i = 0; i < numBytes * 8; i++) {
			sourceByte = i / 8; // Find out what byte this bit belongs to in the source.
			destByte   = i / 8; // Find the target byte.
			if ( *(source + sourceByte) & (1 << (i % 8))) { // Is the bit set?
				// Then set the target bit.
				*(sourceEndianConverted + destByte) |= (1 << (7 - (i % 8)));
			} else {
				// Otherwise clear the target bit.
				*(sourceEndianConverted + destByte) &= ~(1 << (7 - (i % 8)));
			}
		}

		source = sourceEndianConverted;
	}

	signBit = Com_CopyData(dest, source, signal->ComBitSize, 0, signal->ComBitPosition);

	// Sign extend!
	// ############### THIS is NOT WORKING!
	// Are there any unfilled bits in the destination?
	if (signal->ComBitSize < SignalTypeToSize(signal->ComSignalType, signal->ComSignalLength) * 8) {
		// These bits needs to be sign extended.
		if ((signal->ComSignalType == SINT8
			|| signal->ComSignalType == SINT16
			|| signal->ComSignalType == SINT32)
			&& signBit) {

			for (int i = signal->ComBitSize; i < SignalTypeToSize(signal->ComSignalType, signal->ComSignalLength) * 8; i++) {
				destByte = i / 8;
				*(dest + destByte) |= (1 << i % 8);
			}
		}
	}

	if (signal->ComSignalEndianess != CPU_ENDIANESS) {
		free(source);
	}
}


void Com_CopyToSignal(ComSignal_type *signal, const void *Source) {
	ComGetArcSignal(signal->ComHandleId);
	ComGetArcIPdu(Arc_Signal->ComIPduHandleId);
	Com_CopyData(Arc_IPdu->ComIPduDataPtr, Source, signal->ComBitSize, signal->ComBitPosition, 0);
}

uint8 Com_CopyData(void *Destination, const void *Source, uint8 numBits, uint8 destOffset, uint8 sourceOffset) {

	uint8 signBit;
	// Variables to store the source and destination bytes we are currently looking at.
	uint8 sourceByte;
	uint8 destByte;

	// Pointer to the first byte of the source and dest respectively.
	uint8 *source = (uint8 *)Source;
	uint8 *dest = (uint8 *)Destination;


	// For every bit of the signal
	for(int i = 0; i < numBits; i++) {
		sourceByte = (i + sourceOffset) / 8; // Find out what byte this bit belongs to in the source.
		destByte   = (i + destOffset) / 8; // Find the target byte.
		if ( testBit(source, (i + sourceOffset))) { // Is the bit set?
			// Then set the target bit.
			setBit(dest, (i + destOffset)); //*(dest + destByte) |= (1 << ((i + signal->ComBitPosition) % 8));
			signBit = 1;
		} else {
			// Otherwise clear the target bit.
			clearBit(dest, (i + destOffset)); //*(dest + destByte) &= ~(1 << ((i + signal->ComBitPosition) % 8));
			signBit = 0;
		}
	}
	return signBit;
}


uint8 Com_Filter(ComSignal_type *signal) {
	ComGetArcSignal(signal->ComHandleId);
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
