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


#if !(((EA_SW_MAJOR_VERSION == 1) && (EA_SW_MINOR_VERSION == 0)) )
#error Ea: Configuration file expected BSW module version to be 1.0.*
#endif



#ifndef EA_CFG_H
#define EA_CFG_H


#include "MemIf_Types.h"

#define EA_NUMBER_OF_BLOCKS		2

/*The size in bytes to which logical blocks shall be aligned.*/
#define EA_VIRTUAL_PAGE_SIZE	8

/*The largest block size used.*/
#define EA_MAX_BLOCK_SIZE		516

/*
    ITEM NAME:		<EA_DEV_ERROR_DETECT>
    SCOPE:			<EA Module>
    DESCRIPTION:
        Pre-processor switch to enable and disable development error detection. 
        true: Development error detection enabled. 
        false: Development error detection disabled. 
*/
#define EA_DEV_ERROR_DETECT		STD_ON


/*
    ITEM NAME:		<EA_SET_MODE_SUPPORTED>
    SCOPE:			<EA Module>
    DESCRIPTION:
        Compile switch to enable / disable the function Ea_SetMode
*/
#define EA_SET_MODE_SUPPORTED		STD_ON

/*
    ITEM NAME:		<EA_VERSION_INFO_API>
    SCOPE:			<EA Module>
    DESCRIPTION:
        Pre-processor switch to enable / disable the API to read out the modules version information. 
        true: Version info API enabled. 
        false: Version info API disabled.
*/
#define EA_VERSION_INFO_API		STD_OFF

/*
    ITEM NAME:		<Ea_BlockConfigType>
    SCOPE:			<Ea Module>
    DESCRIPTION:
        Configuration of block specific parameters for the EEPROM abstraction module.
*/

typedef struct {
    /*Block identifier (handle). 0x0000 and 0xFFFF shall not be used for block numbers (see EA006).
    Range:	min = 2^NVM_DATA_SELECTION_BITS
            max = 0xFFFF -2^NVM_DATA_SELECTION_BITS
    Note: 	Depending on the number of bits set aside for dataset selection several other block numbers shall
            also be left out to ease implementation.
    NVM_DATA_SELECTION_BITS = 4		So range: 0x10 ~ 0xFFEF*/
    uint16	EaBlockNumber;

    /*Size of a logical block in bytes.*/
    /*@req EA117
    */
    uint16	EaBlockSize;

    /*Marker for high priority data.
    true: Block contains immediate data.
    false: Block does not contain immediate data.*/
    boolean	EaImmediateData;

    /*Device index (handle). Range: 0 .. 254 (0xFF reserved for broadcast call to GetStatus function).
    Type:	Reference to EepGeneral item 'EepDriverIndex'
    dependency: This information is needed by the NVRAM manager respectively the Memory Abstraction
    Interface to address a certain logical block. It is listed in this specification to give a complete overview
    over all block related configuration parameters.*/
    uint8	EaDeviceIndex;

    /*indicates the end of the Ea Block List*/
    boolean EaBlockEOL;
}Ea_BlockConfigType;

/*
    ITEM NAME:		<Ea_GeneralType>
    SCOPE:			<Ea Module>
    DESCRIPTION:
        General configuration of the EEPROM abstraction module. This container lists block independent
        configuration parameters.
*/
typedef struct {
    /*Specifies the InstanceId of this module instance. If only one instance is present it shall have the Id 0.*/
    uint8	EaIndex;

    /*Mapped to the job end notification routine provided by the upper layer module (NvM_JobEndNotification).*/
    void (*EaNvmJobEndNotification)(void);

    /*Mapped to the job error notification routine provided by the upper layer module (NvM_JobErrorNotification). */
    void (*EaNvmJobErrorNotification)(void);
}Ea_GeneralType;

extern const Ea_GeneralType Ea_GeneralData;
extern const Ea_BlockConfigType Ea_BlockConfigData[];

#endif /*EA_CFG_H*/
