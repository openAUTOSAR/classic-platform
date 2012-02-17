/*
 * Eep_ConfigTypes.h
 *
 *  Created on: 1 jun 2011
 *      Author: mahi
 */

#ifndef EEP_CONFIGTYPES_H_
#define EEP_CONFIGTYPES_H_

/* STD container : EepExternalDriver
 * EepSpiReference:				1..*  Ref to SPI sequence
 */

#if defined(EEP_USES_EXTERNAL_DRIVER)
typedef struct {
  /* Reference to SPI sequence (required for external EEPROM drivers).
   *
   * The 3.0 and 4.0 does things a bit different here
   *
   * 3.0
   * =======
   *   const Eep_ConfigType EepConfigData =
   *   {
   *    …
   *    EepCmdChannel    = EEP_SPI_CH_COMMAND,
   *    EepAdrChannel    = EEP_SPI_CH_ADDRESS,
   *    …
   *    EepWriteSequence = EEP_SPI_SEQ_WRITE,
   *    …
   *  };
   *
   *
   * 4.0
   * =======
   *  Wants the defines generated to Spi_Cfg.h to be used directly as:
   *  #define Spi_EepReadSequence 10
   *  #define Spi_EepReadJob	  20
   *  #define Spi_.....
   *  #define Spi_EepChCommand 	  30
   *  #define Spi_EepChAddress 	  31
   *  #define Spi_EepChReadData	  32
   *
   */

//  uint32  SpiReference;

  /* EEP094 */
  Spi_SequenceType EepCmdSequence;
  Spi_SequenceType EepCmd2Sequence;
  Spi_SequenceType EepReadSequence;
  Spi_SequenceType EepWriteSequence;

  Spi_ChannelType EepAddrChannel;
  Spi_ChannelType EepCmdChannel;
  Spi_ChannelType EepDataChannel;
  Spi_ChannelType EepWrenChannel;
} Eep_ExternalDriverType;
#endif


/* STD container : EepInitConfiguration
 * EepBaseAddress:				1    int
 * EepDefaultMode:				1	 enum MEMIF_MODE_FAST, MEMIF_MODE_SLOW
 * EepFastReadBlockSize:		1
 * EepFastWriteBlockSize:		1
 * EepJobCallCycle:				1	 float (not used by config?)
 * Eep_JobEndNotification	    0..1 Function
 * Eep_JobErrorNotification	    0..1 Function
 * EepNormalReadBlockSize		1	 Int
 * EepNormalWriteBlockSize		1	 Int
 * EepSize						1	 Int
 *
 * Defined by Arc:
 * EepPageSize					1	 Int
 */
typedef struct {

  // This parameter is the EEPROM device base address.
  Eep_AddressType EepBaseAddress;

  // This parameter is the default EEPROM device mode after initialization.
  MemIf_ModeType EepDefaultMode;

  // This parameter is the number of bytes read within one job processing cycle in fast mode
  Eep_LengthType EepFastReadBlockSize;

  // This parameter is the number of bytes written within one job processing cycle in fast mode
  Eep_LengthType EepFastWriteBlockSize;

  // call cycle of the job processing function during write/erase operations. Unit: [s]
  //float	 EepJobCallCycle;

  // This parameter is a reference to a callback function for positive job result
  void (*Eep_JobEndNotification)();

  // This parameter is a reference to a callback function for negative job result
  void (*Eep_JobErrorNotification)();

  // number of bytes read within one job processing cycle in normal mode.
  Eep_LengthType EepNormalReadBlockSize;

  // Number of bytes written within one job processing cycle in normal mode.
  Eep_LengthType EepNormalWriteBlockSize;

  // This parameter is the used size of EEPROM device in bytes.
  Eep_LengthType	EepSize;

  // ---- ARC ADDITIONS ----
  // This parameter is the EEPROM page size, i.e. number of bytes.
  Eep_LengthType EepPageSize;

#if defined(EEP_USES_EXTERNAL_DRIVER)
  const Eep_ExternalDriverType *externalDriver;
#endif
} Eep_ConfigType;




#endif /* EEP_CONFIGTYPES_H_ */
