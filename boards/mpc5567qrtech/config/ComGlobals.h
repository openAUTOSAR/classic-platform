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








#warning "This default file may only be used as an example!"

#ifndef COMGLOBALS_H_
#define COMGLOBALS_H_


// PDU definitions
enum {
  // Used for PCAN.
  PCAN_RX_FRAME_1 = 0,
  PCAN_RX_FRAME_2 = 2,
  PCAN_TX_FRAME_1 = 3,
  PCAN_TX_FRAME_2 = 4,

  // Used for hardware test.
  PDU_MSG_HARDWARE_TEST_ENGINE_STATUS = 0,
  PDU_MSG_HARDWARE_TEST_ENGINE_CONTROL = 1,
  PDU_MSG_HARDWARE_TEST_ROUTED_MSG = 2,

  // Used for testing CanIf
  PDU_MSG_RX234 = 8,
  PDU_MSG_RX123 = 10,
  PDU_MSG_TX789 = 9,
  PDU_MSG_TX987 = 11

};


#endif /* COMGLOBALS_H_ */
