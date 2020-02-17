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


#ifndef FEC_5XXX_H_
#define FEC_5XXX_H_

#include "Std_Types.h"

/* Interrupt Event register */
#define	HEARTBEAT_ERROR    0x80000000U
#define	BABBLINGRECEIVER   0x40000000U
#define	BABBLINGTRANSMIT   0x20000000U
#define	GRACEFULSTOPACK    0x10000000U
#define	TRANSMITFRAMEINT   0x08000000U
#define	TRANSMITBUFINT     0x04000000U
#define	RECEIVEFRAMEINT    0x02000000U
#define	RECEIVEBUFINT	   0x01000000U
#define	MIIIINT 	   	   0x00800000U
#define	ETHBUS_ERROR  	   0x00400000U
#define	LATECOLLISION  	   0x00200000U
#define	COLLISIONRETRYLIMIT 0x00100000U
#define	TRANSMITFIFOUNDERRUN 0x00080000U

sint8 fec_mii_read(uint8 phyAddr, uint8 regAddr, uint16 * retVal);
sint8 fec_mii_write(uint8 phyAddr, uint8 regAddr, uint32 data);
sint8 fec_init(const uint8 *macAddr);
uint8 fec_find_phy(uint8 startPhyAddr, uint8 endPhyAddr);
sint8 fec_init_phy(void);
sint8 fec_send(uint8 *buf, uint16 len);
sint8 fec_recv(void);
void fec_set_macaddr(const uint8 *macAddr);
uint8 * fec_get_buffer_to_send(void);
boolean fec_is_rx_data_available(void);
void fec_enable_reception(void);
void fec_linkUp(void);
void fec_stop(void);

#endif	/* MPC5XXX_FEC_H_ */
