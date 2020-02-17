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


#warning "This default file may only be used as an example!"

#include "Ea.h"
#include "NvM_Cbk.h"

/*
    ITEM NAME:		<Ea_GeneralData>
    SCOPE:			<Ea Module>
    DESCRIPTION:
        configuration for Ea_General.  
*/
const Ea_GeneralType Ea_GeneralData = {
    0,						/*EaIndex*/
    NvM_JobEndNotification,				/*EaNvmJobEndNotification*/
    NvM_JobErrorNotification,				/*EaNvmJobEndNotification*/
};

/*
    ITEM NAME:		<Ea_BlockConfigData>
    SCOPE:			<Ea Module>
    DESCRIPTION:
        configuration for Ea_BlockConfig.
        This container mapped EEPROM with block of EA Module
*/

const Ea_BlockConfigType Ea_BlockConfigData[] = 
/*EaBlockNumber*/ /*EaBlockSize*/ /*EaImmediateData*/ /*EaDeviceIndex*/ /*EaBlockEOL*/
{ 
    {1,   260,   false,   2,   false}, 
    {2,   516,   false,   2,   true},     
};

