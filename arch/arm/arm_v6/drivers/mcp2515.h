/*
 * mcp2515.h
 *  Contains register definitions of the CAN Controller board MCP2515
 *
 *  A more detailed description can be found in the mcp2515_can.pdf data sheet.
 *
 *  Created on: 2 okt 2014
 *      Author: zsz
 */

#ifndef MCP2515_H_
#define MCP2515_H_

#include "Platform_Types.h"

/* Possible baud rates */
typedef enum {
	MCP2515_25KBPS  = 25,
	MCP2515_125KBPS = 125,
	MCP2515_250KBPS = 250,
	MCP2515_500KBPS = 500
} Mcp2515_BaudrateTypes;

/* Counters */
#define MCP2515_NR_TXBUFFERS 		3
#define MCP2515_NR_RXBUFFERS 		2
#define MCP2515_NR_BUFFERS			(MCP2515_NR_TXBUFFERS + \
									 MCP2515_NR_RXBUFFERS)
#define MCP2515_NR_ACCMASKS			2
#define MCP2515_NR_ACCFILTERS		6

/* MCP2515 register addresses (see register map on p.61 in mcp2515_can.pdf) */
#define MCP2515_RXF0SIDH			0x00
#define MCP2515_RXF0SIDL			0x01
#define MCP2515_RXF0EID8			0x02
#define MCP2515_RXF0EID0			0x03
#define MCP2515_RXF1SIDH			0x04
#define MCP2515_RXF1SIDL			0x05
#define MCP2515_RXF1EID8			0x06
#define MCP2515_RXF1EID0			0x07
#define MCP2515_RXF2SIDH			0x08
#define MCP2515_RXF2SIDL			0x09
#define MCP2515_RXF2EID8			0x0A
#define MCP2515_RXF2EID0			0x0B
#define MCP2515_CANSTAT				0x0E
#define MCP2515_CANCTRL				0x0F				// CAN control register
#define MCP2515_RXF3SIDH			0x10
#define MCP2515_RXF3SIDL			0x11
#define MCP2515_RXF3EID8			0x12
#define MCP2515_RXF3EID0			0x13
#define MCP2515_RXF4SIDH			0x14
#define MCP2515_RXF4SIDL			0x15
#define MCP2515_RXF4EID8			0x16
#define MCP2515_RXF4EID0			0x17
#define MCP2515_RXF5SIDH			0x18
#define MCP2515_RXF5SIDL			0x19
#define MCP2515_RXF5EID8			0x1A
#define MCP2515_RXF5EID0			0x1B
#define MCP2515_TEC					0x1C
#define MCP2515_REC					0x1D
#define MCP2515_RXM0SIDH			0x20
#define MCP2515_RXM0SIDL			0x21
#define MCP2515_RXM0EID8			0x22
#define MCP2515_RXM0EID0			0x23
#define MCP2515_RXM1SIDH			0x24
#define MCP2515_RXM1SIDL			0x25
#define MCP2515_RXM1EID8			0x26
#define MCP2515_RXM1EID0			0x27
#define MCP2515_CNF3				0x28
#define MCP2515_CNF2				0x29
#define MCP2515_CNF1				0x2A
#define MCP2515_CANINTE				0x2B
#define MCP2515_CANINTF				0x2C
#define MCP2515_EFLG				0x2D
#define MCP2515_TXB0CTRL			0x30
#define MCP2515_TXB1CTRL			0x40
#define MCP2515_TXB2CTRL			0x50
#define MCP2515_RXB0CTRL			0x60
#define MCP2515_RXB1CTRL			0x70

/* Transmit (Tx) and recieve (Rx) buffer registers */
#define MCP2515_TXBUF_0 		 	0x31				// First Tx-buffer
#define MCP2515_TXBUF_1 	     	0x41				// Second Tx-buffer
#define MCP2515_TXBUF_2          	0x51				// Third Tx-buffer
#define MCP2515_RXBUF_0 		 	0x61				// First Rx-buffer
#define MCP2515_RXBUF_1 	   	 	0x71				// Second Rx-buffer

/* Bits in Tx control registers (TXBnCTRL), see p.18 in mcp2515_can.pdf */
#define MCP2515_TXBnCTRL_ABTF      	BIT(6)				// Message aborted flag
#define MCP2515_TXBnCTRL_MLOA     	BIT(5)				// Message lost arbitration
#define MCP2515_TXBnCTRL_TXERR     	BIT(4)				// Transmission error detected
#define MCP2515_TXBnCTRL_TXREQ     	BIT(3)				// Message transmit request (1: pending)
#define MCP2515_TXBnCTRL_TXP10     	(BIT(1) | BIT(0))	// Transmit buffer priority (11: highest)

/*
 * Bits in Rx control registers - RXBnCTRL (pp. 27-28 in mcp2515_can.pdf)
 *
 * RXB0CTRL: | - | RXM1 | RXM0 | - | RXRTR | BUKT | BUKT1 | FILHIT0 |
 * bit 6-5: Rx buffer operating mode:
 * 				11 = receive any messages, i.e. don't care about the filters;
 *          	10 = receive valid messages with extended ids
 *          	01 = receive valid messages with standard ids
 *          	00 = receive all valid messages, i.e. following the filter rules
 * bit 3:	Remote Transmit Request (RTR)
 * bit 2:   Rollover Enable bit (BUKT)
 * 				1 = message will be written to RXB1 if RXB0 is full
 * bit 1: 	internal
 * bit 0: 	Filter Hit bit
 * 				1 = Acceptance filter 1 (RXF1)
 * 				0 = Acceptance filter 0 (RXF0)
 *
 * RXB1CTRL: | - | RXM1 | RXM0 | - | RXRTR | FIHIT2 | FIHIT1 | FIHIT0 |
 * bit 7-3: as above
 * bit 2-0: Filter Hit bit - acceptance filter nr (e.g. 101 corresponds to RXF5)
 */
#define MCP2515_RXBnCTRL_RXM_ANY      	(BIT(6) | BIT(5))
#define MCP2515_RXBnCTRL_RXM_EXT      	BIT(6)
#define MCP2515_RXBnCTRL_RXM_STD      	BIT(5)
#define MCP2515_RXBnCTRL_RXM_STDEXT   	0x00
#define MCP2515_RXBnCTLR_RXM_MASK     	(BIT(6) | BIT(5))
#define MCP2515_RXBnCTRL_BUKT_MASK   	BIT(2)

/* Data Length Code (DLC) mask in RXBnDLC/TXBnDLC-registers */
#define MCP2515_DLC_MASK        		0x0F    	// 4 LSBits

/* Offsets with respect to SIDH-registers (which coincide with Tx/Rx-buffer bases) */
#define MCP2515_CTRL_OFFSET 			(-1)		// CTRL (control reg) offset from RX-register (RXBUF_n)
#define MCP2515_SIDH_OFFSET 			0			// SIDH (standard id, high bits) offset from RX-register (RXBUF_n)
#define MCP2515_SIDL_OFFSET 			1			// SIDL (stardard id, low bits) offset from RX-register (RXBUF_n)
#define MCP2515_EID8_OFFSET 			2			// EID8 (extended id, high bits) offset from RX-register (RXBUF_n)
#define MCP2515_EID0_OFFSET 			3			// EID0 (extended id, low bits) offset from RX-register (RXBUF_n)
#define MCP2515_DLC_OFFSET				4			// DLC (data length in bytes) offset from RX-register (RXBUF_n)
#define MCP2515_D0_OFFSET				5			// DO (first data byte) offset from RX-register (RXBUF_n)

/* CANINTE/CANINTF Register Bits, see pp.50-51 in mcp2515_can.pdf */
#define MCP2515_CANINT_RX0I				BIT(0)		// RX0 full interrupt enable / flag
#define MCP2515_CANINT_RX1I				BIT(1)		// RX1 full interrupt enable / flag
#define MCP2515_CANINT_TX0I				BIT(2)		// TX0 empty interrupt enable / flag
#define MCP2515_CANINT_TX1I				BIT(3)		// TX1 empty interrupt enable / flag
#define MCP2515_CANINT_TX2I				BIT(4)		// TX2 empty interrupt enable / flag
#define MCP2515_CANINT_ERRI				BIT(5)		// Error interrupt enable / flag
#define MCP2515_CANINT_WAKI				BIT(6)		// Wakeup interrupt enable / flag
#define MCP2515_CANINT_MERR				BIT(7)		// Message error interrupt enable / flag

/* Error flag bits, see p.47 in mcp2515_can.pdf */
#define MCP2515_EFLG_RX1OVR 			BIT(7)
#define MCP2515_EFLG_RX0OVR 			BIT(6)
#define MCP2515_EFLG_TXBO   			BIT(5)
#define MCP2515_EFLG_TXEP   			BIT(4)
#define MCP2515_EFLG_RXEP   			BIT(3)
#define MCP2515_EFLG_TXWAR  			BIT(2)
#define MCP2515_EFLG_RXWAR  			BIT(1)
#define MCP2515_EFLG_EWARN  			BIT(0)

/**
 * CANCTRL register values (used for selecting operation mode), see ch.10 in mcp2515_can.pdf
 * Mode selection is decided by the highest three bits (7-5) of CANCTRL, REQOP2 - REQOP0, see p.58
 */
#define MCP2515_MODE_NORMAL     		0x00		// Normal operation mode, CANCTRL-reg: 000xxxxx
#define MCP2515_MODE_SLEEP      		0x20		// Sleep operation mode, CANCTRL-reg: 001xxxxx
#define MCP2515_MODE_LOOPBACK   		0x40		// Loopback operation mode, CANCTRL-reg: 010xxxxx
#define MCP2515_MODE_LISTENONLY 		0x60		// Listen-only operation mode, CANCTRL-reg: 011xxxxx
#define MCP2515_MODE_CONFIG     		0x80		// Configuration operation mode, CANCTRL-reg: 100xxxxx
#define MCP2515_MODE_POWERUP			0xE0		// Power up, CANCTRL-reg: 111xxxxx
#define MCP2515_MODE_MASK				0xE0		// Used to mask out all mode related (REQOP) bits

/* SPI instruction set (see Table 12-1 on p.64 in mcp2515_can.pdf) */
#define MCP2515_WRITE					0x02		// Write to any register
#define MCP2515_READ					0x03		// Read any register
#define MCP2515_BITMOD					0x05		// Modify a bit in any register
#define MCP2515_READ_STATUS				0xA0		// Quick read of common status bits
#define MCP2515_RESET					0xC0		// Reset internal registers
#define MCP2515_LOAD_TX0				0x40		// Shortcut for loading TX0-buffer
#define MCP2515_LOAD_TX1				0x42		// Shortcut for loading TX1-buffer
#define MCP2515_LOAD_TX2				0x44		// Shortcut for loading TX2-buffer
#define MCP2515_RTS_TX0					0x81		// Request-to-Send on TX0
#define MCP2515_RTS_TX1					0x82		// Request-to-Send on TX1
#define MCP2515_RTS_TX2					0x84		// Request-to-Send on TX2
#define MCP2515_RTS_ALL					0x87		// Request-to-Send on all Tx-buffers
#define MCP2515_READ_RX0				0x90		// Shortcut for reading RX0-buffer
#define MCP2515_READ_RX1				0x94		// Shortcut for reading RX1-buffer

/**
 * Baud rate configuration parameters.
 *
 * In short, each nominal time bit is composed of four segments, synchronization,
 * propagation, and two phase segments.
 *
 * We need to assign time quanta (Tq) to the segments and scale Tq in terms of
 * crystal oscillations (BRP) so that the total number of bits per second is correct:
 * 		Nominal Bit Time (NBT) ==  2 * BRP * T_osc * (SyncSeg + PhaseSeg1 + PropSeg + PhaseSeg2)
 *
 * Example:
 * 		F_osc = 20MHz
 * 		baudrate = 250 kbps
 *
 * 		max_nr_Tq_per_NBT = oscillations_per_NBT / 2 =
 * 						  = (F_osc / baudrate) / 2 =
 * 						  = 20e6 / (2 * 2.5e5) = 40 =
 * 						  = 4 * (1 + 3 + 3 + 3)			// for example
 *
 * The details are more thoroughly explained in mcp2515_can.pdf, pp. 37-43.
 */

/* The frequency of the crystal on MCP2515 (in MHz) */
#define MCP2515_FREQUENCY_MHz			20

/* The maximum BRP (at 25 kbps) also sets the upper limit of baudrates
 * that can be treated in this way (currently 1000 kbps) */
#define MCP2515_MAX_BRP					40

/* Calc BRP for any given baudrate between 25 and 1000 kbps */
#define MCP2515_BRP(x)					(MCP2515_MAX_BRP / ((x) / 25))

/* Variable segment lengths (in Tq), depending on the oscillation frequency.
 * (Synchronization segment is always equal to 1 Tq). */
#define MCP2515_16MHz_PHSEG1			2					// First phase segment (16 MHz)
#define MCP2515_16MHz_PRSEG				2					// Propagation segment (16 MHz)
#define MCP2515_16MHz_PHSEG2			3					// Second phase segment (16 MHz)

#define MCP2515_20MHz_PHSEG1			3					// First phase segment (20 MHz)
#define MCP2515_20MHz_PRSEG				3					// Propagation segment (20 MHz)
#define MCP2515_20MHz_PHSEG2			3					// Second phase segment (20 MHz)

#if (MCP2515_FREQUENCY_MHz == 16)							// Older boards (stand-alone) support frequencies up to 16 MHz
#define MCP2515_PHSEG1					MCP2515_16MHz_PHSEG1
#define MCP2515_PRSEG					MCP2515_16MHz_PRSEG
#define MCP2515_PHSEG2					MCP2515_16MHz_PHSEG2
#elif (MCP2515_FREQUENCY_MHz == 20) 						// Newer MCP2515-boards (soldered) support up to 20 MHz (our default oscillation frequency)
#define MCP2515_PHSEG1					MCP2515_20MHz_PHSEG1
#define MCP2515_PRSEG					MCP2515_20MHz_PRSEG
#define MCP2515_PHSEG2					MCP2515_20MHz_PHSEG2
#else 														// Default values (frequency undefined)
#define MCP2515_PHSEG1					0x00
#define MCP2515_PRSEG					0x00
#define MCP2515_PHSEG2					0x00
#endif

/* Other bit timing configuration flags */
#define MCP2515_SJW   					1					// Synchronization Jump Width, programmable to 1-4 Tq (typically, 1 is enough)
#define MCP2515_BTLMODE					1 					// Defines where to find the length of PhaseSeg2 (::1 => in PHSEG2)
#define MCP2515_SAM     				0 					// 1 or 3 sample points (0/1)
#define MCP2515_SOF 					0					// Start-of-Frame, CLKOUT disabled/enabled (0/1)
#define MCP2515_WAKFIL					0					// Wake-up filter disabled/enabled (0/1)

/**
 * Bit timing configuration registers, used e.g. to set the baud rate (see pp. 41-43 in mcp2515_can.pdf)
 *
 * 		CNF1 = | SJW1 | SJW0 | BRP5 | BRP4 | BRP3 | BRP2 | BRP1 | BRP0 |
 * 		CNF2 = | BTLMODE | SAM | PHSEG12 | PHSEG11 | PHSEG10 | PRSEG2 | PRSEG1 | PHSEG0 |
 * 		CNF3 = | SOF | WAKFIL | - | - | - | PHSEG22 | PHSEG21 | PHSEG20 |
 *
 * Typically, most values in the configuration registers are decreased by one
 * compared to their real meaning, in order to fully use all available bits.
 */
#define MCP2515_CNF1_BITS(x)			(((MCP2515_SJW - 1) << 6) | \
										 (MCP2515_BRP(x) - 1))
#define MCP2515_CNF2_BITS 				((MCP2515_BTLMODE << 7) | \
										 (MCP2515_SAM << 6) | \
										 ((MCP2515_PHSEG1 - 1) << 3) | \
										 (MCP2515_PRSEG - 1))
#define MCP2515_CNF3_BITS 				((MCP2515_SOF << 7) | \
										 (MCP2515_WAKFIL << 6) | \
										 (MCP2515_PHSEG2 - 1))

#endif /* MCP2515_H_ */
