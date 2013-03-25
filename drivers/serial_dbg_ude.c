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

/* ----------------------------[information]----------------------------------*/
/*
 * COPYRIGHT   :  pls Programmierbare Logik & Systeme GmbH  1999,2011
 *
 * Author: mahi and parts taken from
 *
 * Description:
 *   Implements terminal for PLS/UDE debugger.
 *   Assumes JTAG access port is in non-cached area.
 */

/* ----------------------------[includes]------------------------------------*/
#include <stdint.h>
#include <stdlib.h>

/* ----------------------------[private define]------------------------------*/

#define MPC55XX_SIMIO_HOSTAVAIL   		(0xAA000000)
#define MPC55XX_SIMIO_TARGETAVAIL 		(0x0000AA00)
#define MPC55XX_SIMIO_HOSTAVAILMASK   	(0xFF000000)
#define MPC55XX_SIMIO_TARGETAVAILMASK 	(0x0000FF00)
#define MPC55XX_SIMIO_THAVAIL     		(0x00000080)
#define MPC55XX_SIMIO_THLEN(dw)   		( (dw) & 0x0000007F)
#define MPC55XX_SIMIO_HTNEED      		(0x00800000)
#define MPC55XX_SIMIO_HTLEN(dw)   		( ( (dw) & 0x007F0000 ) >> 16 )

/* ----------------------------[private macro]-------------------------------*/

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif

/* ----------------------------[private typedef]-----------------------------*/

typedef struct tagSimioAccess
{
	volatile uint32_t dwCtrl;
	volatile uint32_t adwData[16];
} TJtagSimioAccess;

/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
TJtagSimioAccess g_JtagSimioAccess = { .dwCtrl = MPC55XX_SIMIO_HOSTAVAIL };

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

static void sendBuffer( int len ) {
	uint32_t dwCtrlReg;

	dwCtrlReg = g_JtagSimioAccess.dwCtrl;
	if( MPC55XX_SIMIO_HOSTAVAIL == ( dwCtrlReg & MPC55XX_SIMIO_HOSTAVAILMASK ) ) {
		dwCtrlReg &= 0xFFFF0000;
		dwCtrlReg |= ( MPC55XX_SIMIO_TARGETAVAIL | MPC55XX_SIMIO_THAVAIL );
		dwCtrlReg |= MPC55XX_SIMIO_THLEN(len);

		g_JtagSimioAccess.dwCtrl = dwCtrlReg;

		 while (dwCtrlReg & MPC55XX_SIMIO_THAVAIL) {
			dwCtrlReg = g_JtagSimioAccess.dwCtrl;
		}
	}
}

/**
 *
 * @param fd
 * @param buffer
 * @param count
 * @return
 */
size_t UDE_write(int fd, char *buffer, size_t nbytes) {
	char *ePtr = buffer + nbytes;
	char *tPtr;
	int left;
	int i;

	tPtr = (char*)&g_JtagSimioAccess.adwData[0];

	while( buffer < ePtr ) {
		left = MIN(sizeof(g_JtagSimioAccess.adwData),nbytes);

		for (i = 0; i < left; i++) {
			if( *buffer == '\r' ) {
				*tPtr++ = '\n';
			}
			*tPtr++ = *buffer++;
		}
		sendBuffer(i);
	}
	return nbytes;
}

