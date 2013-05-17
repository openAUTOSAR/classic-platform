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








//
// Module Id:s from document "List of Basic Sofware Modules" Rev 1.2.1  Part of release 3.0
//

#ifndef MODULES_H
#define MODULES_H

#include "Std_Types.h"

#define VENDOR_ID_ARCCORE   (60)

#define MODULE_ID_OS       (1) // Os

#define MODULE_ID_ECUM    (10) // ECU state manager
#define MODULE_ID_FIM     (11) // Function Inhibition Manager
#define MODULE_ID_COMM    (12) // Communication manager
#define MODULE_ID_WDGM    (13) // Watchdog manager

#define MODULE_ID_DET     (15) // Development Error Tracer

#define MODULE_ID_NVM     (20) // NVRAM Manager
#define MODULE_ID_FEE     (21) // Flash EEPROM Emulation
#define MODULE_ID_MEMIF   (22) // Memory Abstraction Interface
#define MODULE_ID_NM      (29) // Generic networw management

#define MODULE_ID_CANNM   (31) // Can network management
#define MODULE_ID_FRNM    (32) // Flexray network management
#define MODULE_ID_CANTP   (35) // Can transport layer
#define MODULE_ID_FRTP    (36) // Flexray transport protocol
#define MODULE_ID_J1939TP (37) // J1939 Transport protocol (Autosar 4)
#define MODULE_ID_EA      (40) // EEPROM Abstraction

#define MODULE_ID_WDGIF   (43) // Watchdog interface

#define MODULE_ID_COM     (50) // Autosar COM
#define MODULE_ID_PDUR    (51) // PDU router
#define MODULE_ID_IPDUM   (52) // IPDU Multiplexer
#define MODULE_ID_DCM     (53) // Diagnostic Communication Manager
#define MODULE_ID_DEM     (54) // Diagnostic Event Manager
#define MODULE_ID_SOAD    (56) // Socket Adaptor

#define MODULE_ID_CANIF   (60) // CAN Interface
#define MODULE_ID_FRIF    (61) // Flexray interface
#define MODULE_ID_LINIF   (62) // LIN interface
#define MODULE_ID_LINNM   (63) // LIN network management

#define MODULE_ID_CANTRCV (70) // Can tranceiver driver
#define MODULE_ID_FRTRCV  (71) // Flexray tranceiver driver

#define MODULE_ID_CAN     (80) // Can Driver
#define MODULE_ID_FR      (81) // Flexray driver
#define MODULE_ID_LIN     (82) // LIN Driver
#define MODULE_ID_SPI     (83) // SPI Handler Driver

#define MODULE_ID_EEP     (90) // EEPROM Driver
#define MODULE_ID_FLS     (92) // Flash driver
#define MODULE_ID_RAMTST  (93) // RAM test

#define MODULE_ID_GPT    (100) // GPT driver
#define MODULE_ID_MCU    (101) // MCU driver
#define MODULE_ID_WDG    (102) // Watchdog driver

#define MODULE_ID_DIO    (120) // Dio driver
#define MODULE_ID_PWM    (121) // PWM driver
#define MODULE_ID_ICU    (122) // ICU Driver
#define MODULE_ID_ADC    (123) // ADC driver
#define MODULE_ID_PORT   (124) // Port driver

#define MODULE_ID_SCHM   (130) // BSW Scheduler Module

#define MODULE_ID_CANSM  (140) // Can state manager
#define MODULE_ID_LINSM  (141) // LIN state manager
#define MODULE_ID_FRSM   (142) // Flexray state manager

#define MODULE_ID_CRC    (201) // CRC Routines

#define MODULE_ID_C2CAN  (220) // CAN
#define MODULE_ID_C2COM  (221) // COM Services
#define MODULE_ID_C2DIAG (222) // Diagnostic
#define MODULE_ID_C2FW   (223) // ECU Firmware
#define MODULE_ID_C2FR   (224) // FlexRay
#define MODULE_ID_C2LIN  (225) // LIN
#define MODULE_ID_C2MMGT (226) // Mode Management

#define MODULE_ID_IO     (254) // IO Hardware Abstraction
#define MODULE_ID_CPLX   (255) // Complex drivers

#endif
