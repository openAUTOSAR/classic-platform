/*
 * Calculator.c
 *
 *  Created on: 16 jun 2011
 *      Author: maek
 */

#include "Rte_Calculator.h"

Std_ReturnType Multiply(const UInt8 arg1, const UInt8 arg2, UInt16* result) {
	*result = arg1 * arg2;
	return RTE_E_OK;
}
