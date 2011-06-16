/*
 * Logger.c
 *
 *  Created on: 16 jun 2011
 *      Author: maek
 */

#include "Rte_Logger.h"

#define LOG_SIZE 10

static UInt16 log[LOG_SIZE];

void LoggerRunnable( void ) {
	static uint8 index = 0;
	log[index++] = Rte_IRead_LoggerRunnable_Result_result();
	if (index >= LOG_SIZE) index = 0;
}
