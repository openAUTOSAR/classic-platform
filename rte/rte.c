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









/* This file contains all runnable and instansiation */

//-------------------------------------------------------------------
// 
// Runnable entity
//
//<void|Std_ReturnType> Rte_<name>( [IN Rte_Instance <instance>], [role parameters])

// Triggered by RTEEvent(always) 
void Rte_Runnable_10ms( Rte_Instance ) {
	
} 


//-------------------------------------------------------------------
// Applicatio headers

typedef struct {
	uint8 value;
} Rte_DE_uint8;

typedef struct {
	uint8 value;
	Std_ReturnType status;
} Rte_DES_uint8;

// re - runnable entity
Rte_DE_uint8* re1_doors_get_status; 

#define Rte_IRead_re1_doors_get_status(inst, data) ((inst)->doors_get_status

//-------------------------------------------------------------------
// Application code

void Doors( void ) {
			
}

// Call in runnable
void Doors_periodic( void ) {
	// read the door status from I/O HW
	int door_status;
	Rte_Read_io_
	Rte_IRead_Runnable_10ms_io_ 
	
}


