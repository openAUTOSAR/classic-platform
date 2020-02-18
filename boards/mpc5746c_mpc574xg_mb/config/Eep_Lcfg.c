/* -------------------------------- Arctic Core ------------------------------
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


/* Configured for:
 *   Microchip 25LC160B (32 bytes pages)
 */


#include "Eep.h"
#include "Spi.h"
#include "debug.h"

#define E2_M9525		1
#define E2_25LC160B		2

#define E2_CHIP  E2_M9525


#if defined(USE_EA)
extern void Ea_JobErrorNotification(void);
extern void Ea_JobEndNotification(void);
#endif


static void _JobEndNotify(void){
    DEBUG(DEBUG_LOW,"EEP JOB END NOTIFICATION\n");
#if defined(USE_EA)
    Ea_JobEndNotification();
#endif
}
static void _JobErrorNotify(void){
    DEBUG(DEBUG_LOW,"EEP JOB ERROR NOTIFICATION\n");
#if defined(USE_EA)
    Ea_JobErrorNotification();
#endif
}

#define SPI_SEQ_EEP_CMD		SpiConf_SpiSequence_SEQ_CMD
#define SPI_SEQ_EEP_CMD2	SpiConf_SpiSequence_SEQ_CMD2
#define SPI_SEQ_EEP_READ	SpiConf_SpiSequence_SEQ_READ
#define SPI_SEQ_EEP_WRITE	SpiConf_SpiSequence_SEQ_WRITE

#define SPI_CH_EEP_CMD		SpiConf_SpiChannel_CH_CMD
#define SPI_CH_EEP_ADDR		SpiConf_SpiChannel_CH_ADDR
#define SPI_CH_EEP_WREN		SpiConf_SpiChannel_CH_WREN
#define SPI_CH_EEP_DATA		SpiConf_SpiChannel_CH_DATA


const Eep_ExternalDriverType EepExternalDriver = {
	// READ and WRITE sequences and ID's defined in Spi_Cfg.h
	.EepCmdSequence = 	SPI_SEQ_EEP_CMD,
	.EepCmd2Sequence = 	SPI_SEQ_EEP_CMD2,
	.EepReadSequence = 	SPI_SEQ_EEP_READ,
	.EepWriteSequence = SPI_SEQ_EEP_WRITE,

	// Jobs may be left out..

	// Channels used
	.EepCmdChannel	= SPI_CH_EEP_CMD,
	.EepAddrChannel	= SPI_CH_EEP_ADDR,
	.EepWrenChannel	= SPI_CH_EEP_WREN,
	.EepDataChannel	= SPI_CH_EEP_DATA,
};



const Eep_ConfigType EepConfigData[] = {
    {
    // call cycle of the job processing function during write/erase operations. Unit: [s]
//    .EepJobCallCycle = 0.2,
    // This parameter is the EEPROM device base address.
    .EepBaseAddress =  0,

    // This parameter is the default EEPROM device mode after initialization.
    .EepDefaultMode = MEMIF_MODE_FAST,

#if (E2_CHIP  == E2_25LC160B)
    // This parameter is the number of bytes read within one job processing cycle in fast mode
    .EepFastReadBlockSize = 32,
    // This parameter is the number of bytes written within one job processing cycle in fast mode
    .EepFastWriteBlockSize = 32,
#elif (E2_CHIP  == E2_M9525)
    .EepFastReadBlockSize = 64,
    .EepFastWriteBlockSize = 64,
#endif
    // This parameter is a reference to a callback function for positive job result
    .Eep_JobEndNotification = _JobEndNotify,

    // This parameter is a reference to a callback function for negative job result
    .Eep_JobErrorNotification = _JobErrorNotify,


    .EepNormalReadBlockSize = 4,

    // Number of bytes written within one job processing cycle in normal mode.
    .EepNormalWriteBlockSize = 1,

    // This parameter is the used size of EEPROM device in bytes.
#if (E2_CHIP  == E2_25LC160B)
    .EepSize = 0x800,  /* 16Kb for 25LC160B */
#elif (E2_CHIP  == E2_M9525)
    .EepSize = 0x8000,  /* 256Kb for M9525 */
#endif
#if (E2_CHIP  == E2_25LC160B)
    .EepPageSize = 32,	/* 64 for M9525, 32 for 25LC160B, 16 for 25LC160A */
#elif (E2_CHIP  == E2_M9525)
    .EepPageSize = 64,	/* 64 for M9525, 32 for 25LC160B, 16 for 25LC160A */
#endif

    .externalDriver =  &EepExternalDriver,
    }
};
