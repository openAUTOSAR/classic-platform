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










#ifndef RTE_H_
#define RTE_H_



/* Contract 
 * rte_sws_1143
 * <name>_<c>_<p>_<o>
 * <p> - port name, e.g 
 * <c> - component name, e.g. doors, lights
 * <o> - operation name or element name
 *
 * and more 
 * Rte_Pim?
 * Rte_CData?
 */

/* 
 * Generated, rte_sws_3730(p.154)
 */
#define	Rte_Send_p_d
#define Rte_Switch_p_m
#define Rte_Write_p_d
#define Rte_Invalidate_p_d
#define Rte_Feedback_p_d
#define Rte_Read_p_d
#define Rte_Receive_p_d
#define Rte_Call_p_o
#define Rte_Result_p_o
#define Rte_Mode_p_o


// Errors
typedef uint8 Std_ReturnType;
#define RTE_E_OK				((Std_ReturnType) 0)
#define RTE_E_INVALID			((Std_ReturnType) 1)
#define RTE_E_COMMS_ERROR		((Std_ReturnType) 128)
#define RTE_E_TIMEOUT			((Std_ReturnType) 129)
#define RTE_E_LIMIT				((Std_ReturnType) 130)
#define RTE_E_NO_DATA			((Std_ReturnType) 131)
#define RTE_E_TRANSMIT_ACK		((Std_ReturnType) 132)
// Overlayd errors
#define RTE_E_LOST_DATA			((Std_ReturnType) 64)
#define RTE_E_MAX_AGE_EXCEEDED	((Std_ReturnType) 64)


// RTE Mode, TODO

// Rte_Ports
#define Rte_PortHandle_i_RP
#define Rte_Ports_i_RP

// Rte_NPorts
#define Rte_NPorts_i_RP

// Rte_Port
//#define Rte_PortHandle_i_RP
#define Rte_Port_RP

// Rte_Send/Rte_Write/Rte_Switch
#define Rte_Write_p_o
#define Rte_Send_p_o
#define Rte_Swich_p_o

// Rte_Invalidate
#define Rte_Invalidate_p_o

// Rte_Feedback
#define Rte_Feedback_p_o

// Rte_Read
#define Rte_Read_p_o

// Rte_Receive
#define Rte_Receive_p_o

// Rte_Call
#define Rte_Call_p_o

// Rte_Result
#define Rte_Result_p_o

//Rte_Pim
#define Rte_Pim_name

// Rte_CData
#define Rte_CData_name

// Rte_IRead
#define Rte_IRead_re_p_d

// Rte_IWrite
#define Rte_IWrite_re_p_d

// Rte_IInvalidate
#define Rte_IInvalidate_re_p_d

// Rte_IStatus
#define Rte_IStatus_re_p_d

// Rte_IrvIRead
#define Rte_IrvIRead_re_name

// TODO: bla bla bla .. more methods
//


// RTE Lifecycle API reference.( see chap 5.8 )
Std_ReturnType Rte_Start( void );
Std_ReturnType Rte_Stop( void );

#endif /*RTE_H_*/






