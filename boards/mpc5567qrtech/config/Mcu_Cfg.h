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








#ifndef MCU_CFG_H_
#define MCU_CFG_H_

#include "mpc55xx.h"

#define MCU_DEV_ERROR_DETECT 	STD_ON
// Preprocessor switch to enable / disable the use of the function
// Mcu_PerformReset()
#define MCU_PERFORM_RESET_API 	STD_ON

#define MCU_VERSION_INFO_API 	STD_ON

#include "Std_Types.h"

/* FMPLL modes( atleast in 5553/5554 ) */

typedef enum {
	MCU_FMPLL_BYPASS = 0,
	MCU_FMPLL_EXTERNAL_REF,
	MCU_FMPLL_EXTERNAL_REF_NO_FM,
	MCU_FMPLL_DUAL_CONTROLLER_MODE,
} Mcu_FMPLLmode_t;

typedef enum {
  MCU_CLOCKTYPE_EXT_REF_80MHZ = 0,
  MCU_NBR_OF_CLOCKS,
	//MCU_CLOCKTYPE_EXTERNAL_REF,
	//MCU_CLOCKTYPE_EXTERNAL_REF_NO_FM,
	//MCU_CLOCKTYPE_DUAL_CONTROLLER_MODE,
} Mcu_ClockType;

//
//typedef enum {
//	CPU_Z1=0,
//	CPU_Z0,
//} Cpu_t;
//
//typedef enum
//{
//  /* Software interrupts. */
//  INTC_SSCIR0_CLR0,
//  INTC_SSCIR0_CLR1,
//  INTC_SSCIR0_CLR2,
//  INTC_SSCIR0_CLR3,
//  INTC_SSCIR0_CLR4,
//  INTC_SSCIR0_CLR5,
//  INTC_SSCIR0_CLR6,
//  INTC_SSCIR0_CLR7,
//  MCM_MSWTIR_SWTIC,
//  MCM_ESR_COMB,
//  /* eDMA */
//  EDMA_ERRL_ERR31_0,
//  EDMA_INTL_INT0,
//  EDMA_INTL_INT1,
//  EDMA_INTL_INT2,
//  EDMA_INTL_INT3,
//  EDMA_INTL_INT4,
//  EDMA_INTL_INT5,
//  EDMA_INTL_INT6,
//  EDMA_INTL_INT7,
//  EDMA_INTL_INT8,
//  EDMA_INTL_INT9,
//  EDMA_INTL_INT10,
//  EDMA_INTL_INT11,
//  EDMA_INTL_INT12,
//  EDMA_INTL_INT13,
//  EDMA_INTL_INT14,
//  EDMA_INTL_INT15,
//  RESERVED0,
//  RESERVED1,
//  RESERVED2,
//  RESERVED3,
//  RESERVED4,
//  RESERVED5,
//  RESERVED6,
//  RESERVED7,
//  RESERVED8,
//  RESERVED9,
//  RESERVED10,
//  RESERVED11,
//  RESERVED12,
//  RESERVED13,
//  RESERVED14,
//  RESERVED15,
//  /* Semahpore's */
//  SEMAPHORE_INT0,
//  SEMAPHORE_INT1,
//  RESERVED16,
//  CRP_INTERRUPT,
//  LVI_INTERRUPT,
//  IIC_A_IBSR_IBIF,
//  RESERVED17,
//  PLL_SYNSR_LOCF,
//  PLL_SYNSR_LOLF,
//  SIU_OSR_OVER,
//  /* External interrupts */
//  SIU_EISR_EIF0,
//  SIU_EISR_EIF1,
//  SIU_EISR_EIF2,
//  SIU_EISR_EIF3,
//  SIU_EISR_EIF15_4,
//  /* eMIOS */
//  EMISOS200_FLAG_F0,
//  EMISOS200_FLAG_F1,
//  EMISOS200_FLAG_F2,
//  EMISOS200_FLAG_F3,
//  EMISOS200_FLAG_F4,
//  EMISOS200_FLAG_F5,
//  EMISOS200_FLAG_F6,
//  EMISOS200_FLAG_F7,
//  EMISOS200_FLAG_F8,
//  EMISOS200_FLAG_F9,
//  EMISOS200_FLAG_F10,
//  EMISOS200_FLAG_F11,
//  EMISOS200_FLAG_F12,
//  EMISOS200_FLAG_F13,
//  EMISOS200_FLAG_F14,
//  EMISOS200_FLAG_F15,
//  EMISOS200_FLAG_F16,
//  EMISOS200_FLAG_F17,
//  EMISOS200_FLAG_F18,
//  EMISOS200_FLAG_F19,
//  EMISOS200_FLAG_F20,
//  EMISOS200_FLAG_F21,
//  EMISOS200_FLAG_F22,
//  EMISOS200_FLAG_F23,
//  /* eQADC */
//  EQADC_FISR_OVER,
//  EQADC_FISR0_NCF0,
//  EQADC_FISR0_PF0,
//  EQADC_FISR0_EOQF0,
//  EQADC_FISR0_CFFF0,
//  EQADC_FISR0_RFDF0,
//  EQADC_FISR1_NCF1,
//  EQADC_FISR1_PF1,
//  EQADC_FISR1_EOQF1,
//  EQADC_FISR1_CFFF1,
//  EQADC_FISR1_RFDF1,
//  EQADC_FISR2_NCF2,
//  EQADC_FISR2_PF2,
//  EQADC_FISR2_EOQF2,
//  EQADC_FISR2_CFFF2,
//  EQADC_FISR2_RFDF2,
//  EQADC_FISR3_NCF3,
//  EQADC_FISR3_PF3,
//  EQADC_FISR3_EOQF3,
//  EQADC_FISR3_CFFF3,
//  EQADC_FISR3_RFDF3,
//  EQADC_FISR4_NCF4,
//  EQADC_FISR4_PF4,
//  EQADC_FISR4_EOQF4,
//  EQADC_FISR4_CFFF4,
//  EQADC_FISR4_RFDF4,
//  EQADC_FISR5_NCF5,
//  EQADC_FISR5_PF5,
//  EQADC_FISR5_EOQF5,
//  EQADC_FISR5_CFFF5,
//  EQADC_FISR5_RFDF5,
//  /* SCI */
//  SCI_A_COMB,
//  SCI_B_COMB,
//  SCI_C_COMB,
//  SCI_D_COMB,
//  /* DSPI A,B */
//  DSPI_A_ISR_OVER,
//  DSPI_A_ISR_EOQF,
//  DSPI_A_ISR_TFFF,
//  DSPI_A_ISR_TCF,
//  DSPI_A_ISR_RFDF,
//  DSPI_B_ISR_OVER,
//  DSPI_B_ISR_EOQF,
//  DSPI_B_ISR_TFFF,
//  DSPI_B_ISR_TCF,
//  DSPI_B_ISR_RFDF,
//  /* FlexCAN A */
//  FLEXCAN_A_ESR_BOFF_INT,
//  FLEXCAN_A_ESR_ERR_INT,
//  RESERVED18,
//  FLEXCAN_A_IFLAG1_BUF0I,
//  FLEXCAN_A_IFLAG1_BUF1I,
//  FLEXCAN_A_IFLAG1_BUF2I,
//  FLEXCAN_A_IFLAG1_BUF3I,
//  FLEXCAN_A_IFLAG1_BUF4I,
//  FLEXCAN_A_IFLAG1_BUF5I,
//  FLEXCAN_A_IFLAG1_BUF6I,
//  FLEXCAN_A_IFLAG1_BUF7I,
//  FLEXCAN_A_IFLAG1_BUF8I,
//  FLEXCAN_A_IFLAG1_BUF9I,
//  FLEXCAN_A_IFLAG1_BUF10I,
//  FLEXCAN_A_IFLAG1_BUF11I,
//  FLEXCAN_A_IFLAG1_BUF12I,
//  FLEXCAN_A_IFLAG1_BUF13I,
//  FLEXCAN_A_IFLAG1_BUF14I,
//  FLEXCAN_A_IFLAG1_BUF15I,
//  FLEXCAN_A_IFLAG1_BUF31_16I,
//  FLEXCAN_A_IFLAG1_BUF63_32I,
//  /* Periodic interrupt timer */
//  PIT_PITFLG_RTIF,
//  PIT_PITFLG_PIT1,
//  PIT_PITFLG_PIT2,
//  PIT_PITFLG_PIT3,
//  PIT_PITFLG_PIT4,
//  PIT_PITFLG_PIT5,
//  PIT_PITFLG_PIT6,
//  PIT_PITFLG_PIT7,
//  PIT_PITFLG_PIT8,
//  /* FlexCAN B */
//  FLEXCAN_B_ESR_BOFF_INT,
//  FLEXCAN_B_ESR_ERR_INT,
//  RESERVED19,
//  FLEXCAN_B_IFLAG1_BUF0I,
//  FLEXCAN_B_IFLAG1_BUF1I,
//  FLEXCAN_B_IFLAG1_BUF2I,
//  FLEXCAN_B_IFLAG1_BUF3I,
//  FLEXCAN_B_IFLAG1_BUF4I,
//  FLEXCAN_B_IFLAG1_BUF5I,
//  FLEXCAN_B_IFLAG1_BUF6I,
//  FLEXCAN_B_IFLAG1_BUF7I,
//  FLEXCAN_B_IFLAG1_BUF8I,
//  FLEXCAN_B_IFLAG1_BUF9I,
//  FLEXCAN_B_IFLAG1_BUF10I,
//  FLEXCAN_B_IFLAG1_BUF11I,
//  FLEXCAN_B_IFLAG1_BUF12I,
//  FLEXCAN_B_IFLAG1_BUF13I,
//  FLEXCAN_B_IFLAG1_BUF14I,
//  FLEXCAN_B_IFLAG1_BUF15I,
//  FLEXCAN_B_IFLAG1_BUF31_16I,
//  FLEXCAN_B_IFLAG1_BUF63_32I,
//  /* FlexCAN C */
//  FLEXCAN_C_ESR_BOFF_INT,
//  FLEXCAN_C_ESR_ERR_INT,
//  RESERVED20,
//  FLEXCAN_C_IFLAG1_BUF0I,
//  FLEXCAN_C_IFLAG1_BUF1I,
//  FLEXCAN_C_IFLAG1_BUF2I,
//  FLEXCAN_C_IFLAG1_BUF3I,
//  FLEXCAN_C_IFLAG1_BUF4I,
//  FLEXCAN_C_IFLAG1_BUF5I,
//  FLEXCAN_C_IFLAG1_BUF6I,
//  FLEXCAN_C_IFLAG1_BUF7I,
//  FLEXCAN_C_IFLAG1_BUF8I,
//  FLEXCAN_C_IFLAG1_BUF9I,
//  FLEXCAN_C_IFLAG1_BUF10I,
//  FLEXCAN_C_IFLAG1_BUF11I,
//  FLEXCAN_C_IFLAG1_BUF12I,
//  FLEXCAN_C_IFLAG1_BUF13I,
//  FLEXCAN_C_IFLAG1_BUF14I,
//  FLEXCAN_C_IFLAG1_BUF15I,
//  FLEXCAN_C_IFLAG1_BUF31_16I,
//  FLEXCAN_C_IFLAG1_BUF63_32I,
//  /* FlexCAN D */
//  FLEXCAN_D_ESR_BOFF_INT,
//  FLEXCAN_D_ESR_ERR_INT,
//  RESERVED21,
//  FLEXCAN_D_IFLAG1_BUF0I,
//  FLEXCAN_D_IFLAG1_BUF1I,
//  FLEXCAN_D_IFLAG1_BUF2I,
//  FLEXCAN_D_IFLAG1_BUF3I,
//  FLEXCAN_D_IFLAG1_BUF4I,
//  FLEXCAN_D_IFLAG1_BUF5I,
//  FLEXCAN_D_IFLAG1_BUF6I,
//  FLEXCAN_D_IFLAG1_BUF7I,
//  FLEXCAN_D_IFLAG1_BUF8I,
//  FLEXCAN_D_IFLAG1_BUF9I,
//  FLEXCAN_D_IFLAG1_BUF10I,
//  FLEXCAN_D_IFLAG1_BUF11I,
//  FLEXCAN_D_IFLAG1_BUF12I,
//  FLEXCAN_D_IFLAG1_BUF13I,
//  FLEXCAN_D_IFLAG1_BUF14I,
//  FLEXCAN_D_IFLAG1_BUF15I,
//  FLEXCAN_D_IFLAG1_BUF31_16I,
//  FLEXCAN_D_IFLAG1_BUF63_32I,
//  /* FlexCAN E */
//  FLEXCAN_E_ESR_BOFF_INT,
//  FLEXCAN_E_ESR_ERR_INT,
//  RESERVED22,
//  FLEXCAN_E_IFLAG1_BUF0I,
//  FLEXCAN_E_IFLAG1_BUF1I,
//  FLEXCAN_E_IFLAG1_BUF2I,
//  FLEXCAN_E_IFLAG1_BUF3I,
//  FLEXCAN_E_IFLAG1_BUF4I,
//  FLEXCAN_E_IFLAG1_BUF5I,
//  FLEXCAN_E_IFLAG1_BUF6I,
//  FLEXCAN_E_IFLAG1_BUF7I,
//  FLEXCAN_E_IFLAG1_BUF8I,
//  FLEXCAN_E_IFLAG1_BUF9I,
//  FLEXCAN_E_IFLAG1_BUF10I,
//  FLEXCAN_E_IFLAG1_BUF11I,
//  FLEXCAN_E_IFLAG1_BUF12I,
//  FLEXCAN_E_IFLAG1_BUF13I,
//  FLEXCAN_E_IFLAG1_BUF14I,
//  FLEXCAN_E_IFLAG1_BUF15I,
//  FLEXCAN_E_IFLAG1_BUF31_16I,
//  FLEXCAN_E_IFLAG1_BUF63_32I,
//  /* FlexCAN F */
//  FLEXCAN_F_ESR_BOFF_INT,
//  FLEXCAN_F_ESR_ERR_INT,
//  RESERVED23,
//  FLEXCAN_F_IFLAG1_BUF0I,
//  FLEXCAN_F_IFLAG1_BUF1I,
//  FLEXCAN_F_IFLAG1_BUF2I,
//  FLEXCAN_F_IFLAG1_BUF3I,
//  FLEXCAN_F_IFLAG1_BUF4I,
//  FLEXCAN_F_IFLAG1_BUF5I,
//  FLEXCAN_F_IFLAG1_BUF6I,
//  FLEXCAN_F_IFLAG1_BUF7I,
//  FLEXCAN_F_IFLAG1_BUF8I,
//  FLEXCAN_F_IFLAG1_BUF9I,
//  FLEXCAN_F_IFLAG1_BUF10I,
//  FLEXCAN_F_IFLAG1_BUF11I,
//  FLEXCAN_F_IFLAG1_BUF12I,
//  FLEXCAN_F_IFLAG1_BUF13I,
//  FLEXCAN_F_IFLAG1_BUF14I,
//  FLEXCAN_F_IFLAG1_BUF15I,
//  FLEXCAN_F_IFLAG1_BUF31_16I,
//  FLEXCAN_F_IFLAG1_BUF63_32I,
//  RESERVED24,
//  RESERVED25,
//  RESERVED26,
//  RESERVED27,
//  RESERVED28,
//  RESERVED29,
//  RESERVED30,
//  RESERVED31,
//  /* SCI */
//  SCI_E_COMB,
//  SCI_F_COMB,
//  SCI_G_COMB,
//  SCI_H_COMB,
//  /* DSPI */
//  DSPI_C_ISR_OVER,
//  DSPI_C_ISR_EOQF,
//  DSPI_C_ISR_TFFF,
//  DSPI_C_ISR_TCF,
//  DSPI_C_ISR_RFDF,
//  DSPI_D_ISR_OVER,
//  DSPI_D_ISR_EOQF,
//  DSPI_D_ISR_TFFF,
//  DSPI_D_ISR_TCF,
//  DSPI_D_ISR_RFDF,
//  /* Flexray */
//  FLEXRAY_GLOB,
//  FLEXRAY_PRIF,
//  FLEXRAY_CHIF,
//  FLEXRAY_WUP_IF,
//  FLEXRAY_FBNE_F,
//  FLEXRAY_FANE_F,
//  FLEXRAY_RBIF,
//  FLEXRAY_TBIF,
//  RESERVED32,
//  MLB_SERVICE_REQUEST,
//  INTC_NUMBER_OF_INTERRUPTS,
//  /* End of INTC interrupts. The vectors below are used to handle exceptions. */
//  RESERVED_SPACE_BEFORE_EXCEPTIONS1,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS2,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS3,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS4,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS5,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS6,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS7,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS8,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS9,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS10,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS11,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS12,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS13,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS14,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS15,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS16,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS17,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS18,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS19,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS20,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS21,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS22,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS23,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS24,
//  RESERVED_SPACE_BEFORE_EXCEPTIONS25,
//  CRITICAL_INPUT_EXCEPTION,
//  MACHINE_CHECK_EXCEPTION,
//  DATA_STORAGE_EXCEPTION,
//  INSTRUCTION_STORAGE_EXCEPTION,
//  EXTERNAL_INTERRUPT,                    /* This is the place where the "normal" interrupts will hit the CPU... */
//  ALIGNMENT_EXCEPTION,
//  PROGRAM_EXCEPTION,
//  FLOATING_POINT_EXCEPTION,
//  SYSTEM_CALL_EXCEPTION,
//  AUX_EXCEPTION,
//  DECREMENTER_EXCEPTION,
//  FIXED_INTERVAL_TIMER_EXCEPTION,
//  WATCHDOG_TIMER_EXCEPTION,
//  DATA_TLB_EXCEPTION,
//  INSTRUCTION_TLB_EXCEPTION,
//  DEBUG_EXCEPTION,
//  NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS,
//}IrqType;
//
//typedef enum {
//  PERIPHERAL_CLOCK_FLEXCAN_A,
//  PERIPHERAL_CLOCK_FLEXCAN_B,
//  PERIPHERAL_CLOCK_FLEXCAN_C,
//  PERIPHERAL_CLOCK_FLEXCAN_D,
//  PERIPHERAL_CLOCK_FLEXCAN_E,
//  PERIPHERAL_CLOCK_FLEXCAN_F,
//  PERIPHERAL_CLOCK_PIT,
//  PERIPHERAL_CLOCK_DSPI_A,
//  PERIPHERAL_CLOCK_DSPI_B,
//  PERIPHERAL_CLOCK_DSPI_C,
//  PERIPHERAL_CLOCK_DSPI_D,
//  PERIPHERAL_CLOCK_EMIOS,
//  PERIPHERAL_CLOCK_ESCI_A,
//  PERIPHERAL_CLOCK_ESCI_B,
//  PERIPHERAL_CLOCK_ESCI_C,
//  PERIPHERAL_CLOCK_ESCI_D,
//  PERIPHERAL_CLOCK_ESCI_E,
//  PERIPHERAL_CLOCK_ESCI_F,
//  PERIPHERAL_CLOCK_ESCI_G,
//  PERIPHERAL_CLOCK_ESCI_H,
//  PERIPHERAL_CLOCK_IIC_A,
//  PERIPHERAL_CLOCK_MLB,
//} McuE_PeriperalClock_t;


typedef struct {
	// This container defines a reference point in the Mcu Clock tree
	// It defines the frequency which then can be used by other modules
	// as an input value. Lower multiplicity is 1, as even in the
	// simplest case (only one frequency is used), there is one
	// frequency to be defined.
	uint32 McuClockReferencePoint;

  // Phase locked loop configuration parameters for MPC551x.
  uint8 PllEprediv;
  uint8 PllEmfd;
  uint8 PllErfd;
} Mcu_ClockSettingConfigType;

typedef struct {
	// The parameter represents the MCU Mode settings
	uint32 McuMode;
} Mcu_ModeSettingConfigType;

typedef struct {
	// This parameter shall represent the Data pre-setting to be initialized
	uint32 McuRamDefaultValue;

	// This parameter shall represent the MCU RAM section base address
	uint32 McuRamSectionBaseAddress;

	// This parameter shall represent the MCU RAM Section size
	uint32 McuRamSectionSize;

} Mcu_RamSectorSettingConfigType;


// This container defines a reference point in the Mcu Clock tree. It defines
// the frequency which then can be used by other modules as an input value.
// Lower multiplicity is 1, as even in the simplest case (only one frequency is
// used), there is one frequency to be defined.
typedef struct {

	//	This is the frequency for the specific instance of the McuClockReference-
	//	Point container. It shall be givn in Hz.
	uint32 McuClockReferencePointFrequency;

} Mcu_ClockReferencePointType;

typedef struct {
	//	Enables/Disables clock failure notification. In case this feature is not supported
	//	by HW the setting should be disabled.
	uint8	McuClockSrcFailureNotification;

	//	This parameter shall represent the number of Modes available for the
	//	MCU. calculationFormula = Number of configured McuModeSettingConf
	uint8 McuNumberOfMcuModes;

  //  This parameter shall represent the number of RAM sectors available for
  //  the MCU. calculationFormula = Number of configured McuRamSectorSet-
  //  tingConf
  uint8 McuRamSectors;

  //  This parameter shall represent the number of clock setting available for
  //  the MCU.
  uint8 McuClockSettings;

  // This parameter defines the default clock settings that should be used
  // It is an index into the McuClockSettingsConfig
  Mcu_ClockType McuDefaultClockSettings;

	//	This parameter relates to the MCU specific reset configuration. This ap-
	//	plies to the function Mcu_PerformReset, which performs a microcontroller
	//	reset using the hardware feature of the microcontroller.
	uint32 McuResetSetting;

	//	This container contains the configuration (parameters) for the
	//	Clock settings of the MCU. Please see MCU031 for more in-
	//	formation on the MCU clock settings.
	Mcu_ClockSettingConfigType * McuClockSettingConfig;

	//	This container contains the configuration (parameters) for the
	//	Mode setting of the MCU. Please see MCU035 for more information
	//  on the MCU mode settings.
	Mcu_ModeSettingConfigType  *McuModeSettingConfig;

	//	This container contains the configuration (parameters) for the
	//	RAM Sector setting. Please see MCU030 for more information
	//	on RAM sec-tor settings.
	Mcu_RamSectorSettingConfigType *McuRamSectorSettingConfig;

} Mcu_ConfigType;

extern const Mcu_ConfigType McuConfigData[];

#define MCU_DEFAULT_CONFIG McuConfigData[0]

#endif /*MCU_CFG_H_*/
