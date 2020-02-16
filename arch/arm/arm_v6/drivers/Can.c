/*
 * Can.c
 *
 *  Created on: 29 sep 2014
 *      Author: zsz
 */
#include "Can.h"
#include "CanIf_Cbk.h"

#include "Spi.h"
#include "Spi_Cfg.h"

#include "Os.h"

#include <string.h>

#include "isr.h"
#include "irq.h"

#include "mcp2515.h"
#include "bcm2835.h"

#if defined(USE_DET)
#include "Det.h"
#endif

/****************************************************************************/
/*	 				 			Private definitions							*/
/****************************************************************************/

/* A flag for printing debug messages */
#define CAN_DEBUG         	 		0

/* Max baudrate (not actually supported, but only used for loop control) */
#define CAN_MAX_BAUDRATE 			2000

/* Max ECU-id in CAN messages (typically used as max value for the acceptance masks) */
#define CAN_MAX_ECU_CANID			7

/* Default mask for a data byte in a register */
#define CAN_NO_MASK					0xFF

/* Max number of bytes in a CAN frame */
#define CAN_MAX_CHAR_IN_MESSAGE 	8

/****************************************************************************/
/*	 				 			Private macros								*/
/****************************************************************************/

#define CTRL_TO_UNIT_PTR(_controller)   	&CanUnit[Can_Global.config->CanConfigSet->ArcCtrlToUnit[_controller]]
#define VALID_CONTROLLER(_ctrl)         	(Can_Global.configuredMask & (1<<(_ctrl)))
#define VALID_BAUDRATE(_baudrateId)			(((_baudrateId) < CAN_BAUDRATE_CNT) && ((_baudrateId) >= 0))
#define GET_CALLBACKS()                 	(Can_Global.config->CanConfigSet->CanCallbacks)

#define GET_CONTROLLER_CONFIG(_controller)	&Can_Global.config->CanConfigSet->CanController[(_controller)]
#define GET_CAN_UNIT_PTR(_controller) 		&CanUnit[_controller]
#define GET_CONTROLLER_CNT() 				(CAN_CONTROLLER_CNT)

//TODO: We should get CAN-communication mode (polling or interrupt) from a configuration file (instead of bcm2835.h)
///* Check if the first (and currently only) CAN controller is configured to be interrupt-driven */
//#define CAN_INTERRUPT						((GET_CONTROLLER_CONFIG(0))->CanRxProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT)

/* Print debug messages if CAN_DEBUG-flag is set */
#if CAN_DEBUG
#define CAN_DEBUG_PRINT(format, ...) \
	printf(format, ##__VA_ARGS__)
#else
#define CAN_DEBUG_PRINT(format, ...)
#endif

#if (CAN_DEV_ERROR_DETECT == STD_ON)
/** @req 4.0.3/CAN027 */
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_CAN,0,_api,_err); \
          return E_NOT_OK; \
        }
#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_CAN,0,_api,_err); \
          return; \
        }
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)
#else
/** @req 4.0.3/CAN424 */
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif


/****************************************************************************/
/*	 				 		Private typedefs 								*/
/****************************************************************************/

typedef enum {
    CAN_UNINIT = 0,
    CAN_READY
} Can_DriverStateType;

// Mapping between HRH and Controller//HOH
typedef struct Can_Arc_ObjectHOHMapStruct
{
  CanControllerIdType CanControllerRef;    // Reference to controller
  const Can_HardwareObjectType* CanHOHRef;       // Reference to HOH.
} Can_Arc_ObjectHOHMapType;

/* Type for holding global information used by the driver */
typedef struct {
    Can_DriverStateType     initRun;            				// True if Can_Init() have been run
    const Can_ConfigType *  config;             				// Pointer to CAN configuration
    uint32                  configuredMask;     				// Bitmask for configured CAN HW units
    uint8   				channelMap[CAN_CONTROLLER_CNT];		// Maps controller id to configuration id

    // This is a map that maps the HTH:s with the controller and Hoh. It is built
    // during Can_Init and is used to make things faster during a transmit.
    Can_Arc_ObjectHOHMapType CanHTHMap[NUM_OF_HTHS];
} Can_GlobalType;

// Global config
Can_GlobalType Can_Global =
{
    .initRun = CAN_UNINIT,
};

/* Type for holding information about each CAN controller */
typedef struct {
	CanIf_ControllerModeType 		state;					// Controller state (see R4.1.3 fig. 7-2)
	uint32							lock_cnt;				// Nr of interrupt locks
	const Can_ControllerConfigType* cfgCtrlPtr;     		// Pointer to controller configuration
	Can_Arc_StatisticsType 			stats;					// Statistics
	PduIdType 						swPduHandle; 			// PDU handle
} Can_UnitType;

Can_UnitType CanUnit[CAN_CONTROLLER_CNT] =
{
  {
    .state = CANIF_CS_UNINIT,
  },
};

/****************************************************************************/
/*	 				 			Private variables							*/
/****************************************************************************/

/* Last incoming data (temporary storage before transmitting upwards to the CanIf-layer) */
static uint8 incomingData[CAN_MAX_CHAR_IN_MESSAGE];

/****************************************************************************/
/*	 				 	  Private function prototypes						*/
/****************************************************************************/

static Spi_DataType Can_ReadController_Reg(const uint8 address,
										   const Spi_DataType mask);
static void Can_ReadController_Regs(const Spi_DataType address,
									Spi_DataType *values,
									const uint8 dlc);
static Spi_DataType Can_ReadController_Status(void);
static uint32 Can_ReadController_MsgID(const Spi_DataType mcp_addr);
static void Can_ReadController_Msg(const Spi_DataType mcp_addr,
								   Can_PduType* pduInfo);
static void Can_WriteController_Reg(const Spi_DataType address,
									const Spi_DataType value);
static void Can_WriteController_Regs(const Spi_DataType address,
									 const Spi_DataType *values,
									 const uint8 dlc);
static void Can_WriteController_BitModify(const Spi_DataType address,
										  const Spi_DataType mask,
										  const Spi_DataType data);
static void Can_WriteController_MsgID(const Spi_DataType mcp_addr,
									  const uint32 can_id);
static void Can_ResetController(void);
static Can_ReturnType Can_SetController_Mode(const uint8 newmode);
static Can_ReturnType Can_GetController_TxBuf(Spi_DataType *txbuf_addr);
static Std_ReturnType Can_SendMessage(Can_PduType *pduInfo);

/* This function is currently not used */
static Std_ReturnType Can_FindHoh(Can_HwHandleType hth,
		  						  uint32* controller,
								  const Can_HardwareObjectType **hohObjRef);
/* This function should definitely be rewritten, when we understand CanIf better */
static void Can_TriggerCanIf(Can_PduType *pduInfo, uint8 ctrlId);


/****************************************************************************/
/*	 				 		Private functions							   */
/****************************************************************************/

/**
 * Read a byte from one of the Rx-registers of the CAN controllers (MCP2515)
 *
 * The Read instruction is sent to to the MCP2515 followed by the
 * 8-bit address of the Rx-register. Then, the data stored in that register
 * will be shifted out on the SO pin and arrives through SPI to MCU's rx-buffer.
 *
 * @param address			----- CAN boards Rx-register address
 * @param mask				----- mask describing interesting bits in this register
 * 								  (MCP2515_NO_MASK preserves all bits)
 * @return rxbuf			----- pointer to MCU-local rx-buffer
 */
static Spi_DataType Can_ReadController_Reg(const Spi_DataType address,
										   const Spi_DataType mask)
{
    Spi_DataType rxbuf;

    /* Read only one register */
    Can_ReadController_Regs(address, &rxbuf, 1);

    /* Mask out non-interesting bits and return */
	return rxbuf & mask;
}

/**
 * Read several consecutive CAN-board registers
 *
 * The registers on MCP2515 have been arranged to optimize sequential reading
 * and writing of data. This is used in this function, where a number of consecutive
 * registers are looped through (e.g. RXB0SIDH, RXB0SIDL, RXB0EID8, RXB0EID0)
 *
 * @param address			----- the first CAN-board register to read
 * @param values			----- array of rx-buffers to store the received values in
 * @param dlc				----- number of consecutive registers to read
 */
static void Can_ReadController_Regs(const Spi_DataType address,
								    Spi_DataType *values,
									const uint8 dlc)
{
	/* Read instruction in MCP2515-language (00000011) */
	Spi_DataType cmdbuf = MCP2515_READ;

	/* Check that the data length does not exceed what is allowed in one CAN frame */
	VALIDATE_NO_RV(dlc <= CAN_MAX_CHAR_IN_MESSAGE,
			CAN_MAINFUNCTION_READ_SERVICE_ID, CAN_E_PARAM_DLC);

	/* Setup external buffers for SPI read operation (see p.65 in mcp2515_can.pdf for details) */
	Spi_SetupEB(SPI_CH_CMD,  &cmdbuf,  NULL,   sizeof(cmdbuf));			// Tell the CAN controller that this is a read operation
	Spi_SetupEB(SPI_CH_ADDR, &address, NULL,   sizeof(address));		// Read-from register address (A7-A0)
	Spi_SetupEB(SPI_CH_DATA, NULL, 	   values, dlc);					// Place the response in an array of values

	/* Transmit the SPI sequence to read data (up to 8 bytes) from the CAN controller
	 * (which increments its register address by itself) */
	Spi_SyncTransmit(SPI_SEQ_READ);
}

/**
 * Send the read-status instruction
 *
 * Read status instruction allows single instruction access to some
 * of the often used status bits for message reception and transmission.
 *
 * Once the command byte is sent, the MCP2515 will return eight bits
 * of data that contain the status (see p.67 in mcp2515_can.pdf), which are:
 * 		CANINTF.TX2IF | TXB2CTRL.TXREQ | CANINTF.TX1IF | TXB1CTRL.TXREQ |
 * 		CANINTF.TX0IF | TXB0CTRL.TXREQ | CANINTF.RX1IF | CANINTF.RX0IF
 *
 * @return rxbuf			----- rx-buffer containing the common status bits
 */
static Spi_DataType Can_ReadController_Status(void)
{
	Spi_DataType cmdbuf = MCP2515_READ_STATUS;
	Spi_DataType rxbuf;

	/* Setup SPI external buffers for this sequence */
	Spi_SetupEB(SPI_CH_CMD,  &cmdbuf, NULL,   sizeof(cmdbuf));			// Read status instruction (1010000)
    Spi_SetupEB(SPI_CH_DATA, NULL,    &rxbuf, sizeof(rxbuf));			// Listen for returning data (8 bits)

    /* Transmit the sequence */
	Spi_SyncTransmit(SPI_SEQ_CMD2);

	return rxbuf;
}

/**
 * Get the standard identifier for the received message
 *
 * Currently, only standard id types are supported (not extended or remote frames)
 *
 * @param mcp_addr			----- CAN boards Rx-register address (0x61 or 0x71)
 * @return can_id			----- standard identifier
 */
static uint32 Can_ReadController_MsgID(const Spi_DataType mcp_addr)
{
	Spi_DataType sidData[2];
	uint32 can_id;

	/* Read SIDH and SIDL registers, which contain the message id */
    Can_ReadController_Regs(mcp_addr, sidData, sizeof(sidData));

    /* Arrange the bits into an integer and return */
	can_id = (sidData[MCP2515_SIDH_OFFSET]<<3) + (sidData[MCP2515_SIDL_OFFSET]>>5);

	return can_id;
}

/**
 * Read data in a CAN frame and store it in a CAN message structure
 *
 * This includes collecting CAN id, data length (in bytes), and the data bytes themselves.
 *
 * @param mcp_addr 			----- Rx-register (RXBUF_0: 0x61 or RBBUF_1: 0x71) address on the CAN board
 * @param msg				----- pointer to a local CAN message structure to be filled in with data
 */
static void Can_ReadController_Msg(const Spi_DataType mcp_addr,
								   Can_PduType* pduInfo)
{
	/* Read message id */
	pduInfo->id = Can_ReadController_MsgID(mcp_addr);

    /* Read the data length (in bytes) */
	pduInfo->length = Can_ReadController_Reg(mcp_addr + MCP2515_DLC_OFFSET, MCP2515_DLC_MASK);

	/* Read bytes that contain new data and store them */
	Can_ReadController_Regs(mcp_addr + MCP2515_D0_OFFSET, incomingData, pduInfo->length);

	/* Assign pointer to the stored data (for transmission upwards to the CanIf-layer) */
	pduInfo->sdu = incomingData;
}

/**
 * Set up SPI and write one byte of data to a register on the CAN board
 *
 * @param address			----- register address on the CAN board
 * @param value				----- data
 */
static void Can_WriteController_Reg(const Spi_DataType address,
									const Spi_DataType value)
{
	Can_WriteController_Regs(address, &value, 1);
}

/**
 * Set up SPI buffers and write several consecutive registers on the CAN board
 *
 * The write operation on MCP2515 works in such a way that sequential
 * registers are being written to on each rising edge of SCK, as long
 * as CS is held low (see p. 63 in mcp2515_can.pdf)
 *
 * This is typically used for transmitting several data bytes (up to 8) in
 * one CAN frame. Then the first register is given by TXBnD0.
 *
 * @param address			----- register address on the CAN board
 * @param values			----- data bytes
 * @param dlc				----- number of data bytes
 */
static void Can_WriteController_Regs(const Spi_DataType address,
									 const Spi_DataType *values,
									 const uint8 dlc)
{
	/* Write instruction (00000010), see p.64 in mcp2515_can.pdf */
	Spi_DataType cmdbuf = MCP2515_WRITE;

	/* Set up external buffers for each SPI channel in a write sequence */
	Spi_SetupEB(SPI_CH_CMD,  &cmdbuf,  NULL, sizeof(cmdbuf));		// Write instruction
	Spi_SetupEB(SPI_CH_ADDR, &address, NULL, sizeof(address));		// Tx-register address
    Spi_SetupEB(SPI_CH_DATA, values,   NULL, dlc);					// Data that will be written

    /* Transmit the SPI sequence */
    Spi_SyncTransmit(SPI_SEQ_WRITE);
}

/**
 * Set or clear individual bits in specific status and control registers
 *
 * Example of how mask and data bytes are used:
 *          Mask type:    00101000
 *          Data type:    00001000
 *          Pre-Reg data: xxxxxxxx
 *          New-Reg data: xx0x1xxx
 *
 * @param address			----- register address
 * @param mask 				----- mask byte determines which bits in the register that will change
 * @param data				----- data byte contains new bit values
 */
static void Can_WriteController_BitModify(const Spi_DataType address,
										  const Spi_DataType mask,
										  const Spi_DataType data)
{
	Spi_DataType cmdbuf = MCP2515_BITMOD;
	Spi_DataType valbuf[2] = {mask, data};

	/* Set up external buffers for SPI for status bit modification */
	Spi_SetupEB( SPI_CH_CMD,  &cmdbuf,  NULL, sizeof(cmdbuf));			// Bit modify instruction
	Spi_SetupEB( SPI_CH_ADDR, &address, NULL, sizeof(address));			// Register address
	Spi_SetupEB( SPI_CH_DATA, valbuf,   NULL, sizeof(valbuf));			// Mask and new bit values

	/* Initiate SPI transmission */
    Spi_SyncTransmit(SPI_SEQ_WRITE);
}

/**
 * Write message standard identifier to the CAN board
 *
 * Typically a message id is written to 4 registers, SIDH, SIDL, EID8 and EID0.
 * Unless extended ids are used, only the first two registers are set.
 *
 * Currently only standard ids are used (not extended or remote frames).
 *
 * @param mcp_addr			----- SIDH register address (the other registers are written consecutively)
 * @param can_id			----- message id
 */
static void Can_WriteController_MsgID(const Spi_DataType mcp_addr, const uint32 can_id)
{
	/* Standard id, divided into SIDH/SIDL/EID8/EID0-registers */
	Spi_DataType sidData[4];

	/* Set up values for the different id-registers */
	sidData[0] = (Spi_DataType)(can_id >> 3 ); 		// standard id, high bits (SIDH)
	sidData[1] = (Spi_DataType)(can_id << 5 ); 		// standard id, low bits (SIDL)
	sidData[2] = (Spi_DataType)0;					// default values in EID8
	sidData[3] = (Spi_DataType)0;					// default values in EID0

	/* Write id to the CAN board */
	Can_WriteController_Regs(mcp_addr, sidData, sizeof(sidData));
}

/**
 * Send the reset command, causing re-initialization of MCP2515-internal registers
 */
static void Can_ResetController(void)
{
	Spi_DataType cmdbuf = MCP2515_RESET;		// Reset instruction (see p.64 in mcp2515_can.pdf)
	Std_ReturnType rv;

	/* Set up external buffer for SPI and transmit the sequence */
    Spi_SetupEB(SPI_CH_CMD, &cmdbuf, NULL, sizeof(cmdbuf));
    rv = Spi_SyncTransmit(SPI_SEQ_CMD);

    /* Print out the result */
	if (rv != E_OK) {
		CAN_DEBUG_PRINT("infor: fail to reset mcp2515\r\n");
	} else {
		CAN_DEBUG_PRINT("infor: success to reset mcp2515\r\n");
		bcm2835_Sleep(85);
	}
}

/**
 * Set operating mode of the CAN controller
 *
 * Five modes can be set(see p.57 in mcp2515_can.pdf):
 * 		configuration, sleep, listen-only, loopback, and normal.
 *
 * @param new_mode			----- new operating mode
 * @return rv				----- result status
 */
static Can_ReturnType Can_SetController_Mode(const Spi_DataType new_mode)
{
	Spi_DataType buf;
	Std_ReturnType rv;

	/* Set new mode by modifying the three highest bits in the CAN control register */
	Can_WriteController_BitModify(MCP2515_CANCTRL, MCP2515_MODE_MASK, new_mode);

	/* Verify that the new mode has been set, as advised in the datasheet */
	buf = Can_ReadController_Reg(MCP2515_CANCTRL, MCP2515_MODE_MASK);

	/* Print out the result */
	if (buf == new_mode) {
		CAN_DEBUG_PRINT("infor: success to set CAN control mode\r\n");
		rv = CAN_OK;
	}
	else {
		CAN_DEBUG_PRINT("error: fail to set CAN control mode\r\n");
		rv = CAN_NOT_OK;
	}

	return rv;
}

/**
 * Find the first non-busy Tx-buffer on the CAN board and return its address
 *
 * MCP2515 has 3 transfer buffers, controlled by TXBnCTRL-registers,
 * located at 0x30H, 0x40H and 0x50H (see p.18 in mcp2515_can.pdf)
 *
 * TXBnCTRL: - | ABTF | MLOA | TXERR | TXREQ | - | TXP1 | TXP0
 * 		bit 6   (ABTF) 	 	Message aborted flag bit
 * 		bit 5   (MLOA) 	 	Message lost arbitration bit
 * 		bit 4   (TXERR)  	Transmission error detected bit
 * 		bit 3   (TXREQ)  	Message Transmit request bit, pending (1) or not (0)
 * 		bit 1-0 (TXPn)  	Transmit buffer priority bits (11 - highest priority)
 *
 * @param[in]
 */
static Can_ReturnType Can_GetController_TxBuf(Spi_DataType *txbuf_addr)
{
	/* Locals */
    uint8 i;
    Spi_DataType ctrl_val;
    const Spi_DataType ctrl_regs[MCP2515_NR_TXBUFFERS] = {MCP2515_TXB0CTRL,			// Available Tx-buffers
											 	    	  MCP2515_TXB1CTRL,
														  MCP2515_TXB2CTRL};

	/* Find the first non-busy TX-Buffer */
	for (i=0; i < MCP2515_NR_TXBUFFERS; i++) {
		/* Get TXBn control status bits */
		ctrl_val = Can_ReadController_Reg(ctrl_regs[i], MCP2515_TXBnCTRL_TXREQ);

		/* If no transmission is currently pending from this buffer, return it */
		if (ctrl_val == 0) {
			*txbuf_addr = ctrl_regs[i] + 1;			// Tx-buffer follows its control register

			return CAN_OK;
		}
	}

	return CAN_BUSY;
}

/**
 * Send a message to the CAN controller
 *
 * Design choice: if all 3 Tx-buffers are full, the message is dropped
 * 				  (to avoid building up buffer queues)
 *
 * @param pduInfo			----- PDU (protocol data unit) for this CAN message, incl. data, data length, and id
 * @return rv				----- return status
 */
static Std_ReturnType Can_SendMessage(Can_PduType *pduInfo) {
	Can_ReturnType rv;
	Spi_DataType txbuf;

	/* Find a free tx-buffer, returning an error status if no one was found */
	rv = Can_GetController_TxBuf(&txbuf);
	if (rv == CAN_BUSY) {
		return E_NOT_OK;
	}

	/* Write data bytes to the data register(s) */
	Can_WriteController_Regs(txbuf + MCP2515_D0_OFFSET, pduInfo->sdu, pduInfo->length);

	/* Write the message id */
	Can_WriteController_MsgID(txbuf + MCP2515_SIDH_OFFSET, pduInfo->id);

	/* Write the data length (in bytes) */
	Can_WriteController_Reg(txbuf + MCP2515_DLC_OFFSET, pduInfo->length);

	/* Set control status to transmission pending */
	Can_WriteController_BitModify(txbuf + MCP2515_CTRL_OFFSET,
			MCP2515_TXBnCTRL_TXREQ, MCP2515_TXBnCTRL_TXREQ);

	return E_OK;
}

/**
 * Find CAN controller id and HOH (hardware object handle) for a given HTH (hardware transmit handle)
 *
 * This function is currently not used in our implementation (should it?) (TODO)
 *
 * @param hth 				----- hardware transmit handle (HTH)
 * @param controller		----- controller id for this HTH
 * @param hohObjRef			----- reference to the HOH-object that holds this HTH
 * @return  				----- result status
 */
static Std_ReturnType Can_FindHoh(Can_HwHandleType hth,
								  uint32* controller,
								  const Can_HardwareObjectType **hohObjRef)
{
  const Can_Arc_ObjectHOHMapType *map = &Can_Global.CanHTHMap[hth];

  /* Verify that this is the correct map */
  VALIDATE(map->CanHOHRef->CanObjectId == hth,
		  CAN_WRITE_SERVICE_ID, CAN_E_PARAM_HANDLE);

  /* Verify that this is the correct HOH type */
  VALIDATE(map->CanHOHRef->CanObjectType == CAN_OBJECT_TYPE_TRANSMIT,
		  CAN_WRITE_SERVICE_ID, CAN_E_PARAM_HANDLE);

  /* If everything was fine, return */
  *controller = map->CanControllerRef;
  *hohObjRef = map->CanHOHRef;

  return E_OK;
}

/**
 * Trigger RxIndication-callback function in the CanIf layer
 *
 * Find a HOH object for message reception. Then find and set
 * an event notifying upper layers that a certain type of message
 * has been received. Finally, trigger message handling in CanIf.

 * @param pduInfo			----- PDU info, incl. data, data length and id
 * @param ctrlId			----- CAN controller id
 */
static void Can_TriggerCanIf(Can_PduType *pduInfo, uint8 ctrlId) {
	/* Get HOH pointer and message id table for the CAN controller */
	const Can_ControllerConfigType *cfgCtrlPtr = GET_CONTROLLER_CONFIG(Can_Global.channelMap[ctrlId]);
	const Can_HardwareObjectType *hohObj = cfgCtrlPtr->Can_Arc_Hoh;
	const Can_IdTableType *msgIdTable = cfgCtrlPtr->CanMsgIdTable;
	uint32 i;

	--hohObj;
	do {
		++hohObj;
		if (hohObj->CanObjectType == CAN_OBJECT_TYPE_RECEIVE) {
			if (GET_CALLBACKS()->RxIndication != NULL ) {
				/* Find and set an appropriate event to notify upper layers that a CAN-message has arrived */
				for (i=0; i<CAN_MESSAGE_TYPE_CNT; i++) {
					if (msgIdTable[i].msgId == pduInfo->id) {
						SetEvent(msgIdTable[i].taskId, msgIdTable[i].eventMask);
						break;
					}
				}

				/* Trigger a method in CanIf to forward the fetched data */
				GET_CALLBACKS()->RxIndication(hohObj->CanObjectId, pduInfo->id, pduInfo->length, pduInfo->sdu);
			}
		}
	} while (!hohObj->Can_Arc_EOL);
}


/****************************************************************************/
/*	 				 			Public functions 							*/
/****************************************************************************/

/**
 * Initialize software and hardware configurations for all attached CAN controllers
 *
 * @param Config			----- CAN driver configuration
 */
void Can_Init(const Can_ConfigType *Config) {
	/* Locals */
	Can_UnitType *unitPtr;
	uint8 ctrlId, configId;
	const Can_ControllerConfigType *cfgCtrlPtr;

	/** @req 4.1.3/SWS_Can_00259: Check that the CAN module has not been initialized yet */
	VALIDATE_NO_RV((Can_Global.initRun == CAN_UNINIT),
			CAN_INIT_SERVICE_ID, CAN_E_TRANSITION);
	/** @req 4.1.3/SWS_Can_00175: Check that the configuration parameter is non-null */
	VALIDATE_NO_RV((Config != NULL ),
			CAN_INIT_SERVICE_ID, CAN_E_PARAM_POINTER);

	/* Save configuration */
	Can_Global.config = Config;
	Can_Global.initRun = CAN_READY;

	for (configId = 0; configId < CAN_CTRL_CONFIG_CNT; configId++) {
		cfgCtrlPtr = GET_CONTROLLER_CONFIG(configId);
		ctrlId = cfgCtrlPtr->CanControllerId;

		/* Initialize CAN unit structure to default values */
		unitPtr = GET_CAN_UNIT_PTR(ctrlId);
		unitPtr->state = CANIF_CS_STOPPED;
		unitPtr->lock_cnt = 0;
		unitPtr->cfgCtrlPtr = cfgCtrlPtr;
		memset(&unitPtr->stats, 0, sizeof(Can_Arc_StatisticsType));
		unitPtr->swPduHandle = 0;

		/* Map controller id to configuration id */
		Can_Global.channelMap[ctrlId] = configId;

		/* Record that this controller has been configured */
		Can_Global.configuredMask |= (1 << ctrlId);

		/* Loop through all HOH:s (HW Object Handles, which are
         * either HTH or HRH (for transmission and reception))
		 * and map them into the HTHMap. These handles are used
		 * as an interface between CanInterface and CanDriver. */
		const Can_HardwareObjectType* hoh;
		hoh = cfgCtrlPtr->Can_Arc_Hoh;
		hoh--;
		do {
			hoh++;
			if (hoh->CanObjectType == CAN_OBJECT_TYPE_TRANSMIT) {
				Can_Global.CanHTHMap[hoh->CanObjectId].CanControllerRef = cfgCtrlPtr->CanControllerId;
				Can_Global.CanHTHMap[hoh->CanObjectId].CanHOHRef = hoh;
			}
		} while (!hoh->Can_Arc_EOL);

		/* Initialize the CAN controller HW */
		Can_InitController(ctrlId, cfgCtrlPtr);
	}
}

/**
 * Initialize the CAN board by configuring its registers
 *
 * This includes:
 *  	setting the baud rate
 *  	setting up acceptance filters
 *  	enabling rollover in rx-buffers
 *  	enabling CAN messages with standard ids
 *  	clearing tx/rx-buffers
 *  	resetting the controller and switching to normal operation mode
 *
 * @param ctrlId			----- CAN controller id
 * @param cfgCtrlPtr		----- CAN controller configuration
 */
void Can_InitController(uint8 ctrlId, const Can_ControllerConfigType *cfgCtrlPtr)
{
	/* Locals */
	uint8 i, j;
	Std_ReturnType rv;
	Can_ReturnType crv;
	const Spi_DataType ctrlRegs[MCP2515_NR_BUFFERS] = {MCP2515_TXB0CTRL,				// Tx/Rx-buffer registers
											  	  	   MCP2515_TXB1CTRL,
													   MCP2515_TXB2CTRL,
													   MCP2515_RXB0CTRL,
													   MCP2515_RXB1CTRL};
	const Spi_DataType maskRegs[MCP2515_NR_ACCMASKS] = {MCP2515_RXM0SIDH,					// Acceptance mask registers
													 	MCP2515_RXM1SIDH};
	const Spi_DataType filterRegs[MCP2515_NR_ACCFILTERS] = {MCP2515_RXF0SIDH,			// Acceptance filter registers
															MCP2515_RXF1SIDH,
															MCP2515_RXF2SIDH,
															MCP2515_RXF3SIDH,
															MCP2515_RXF4SIDH,
															MCP2515_RXF5SIDH};

	/* Reset the MCP2515 CAN controller */
	Can_ResetController();

	/* Prepare the CAN controller for configuration */
	crv = Can_SetController_Mode(MCP2515_MODE_CONFIG);
	if (crv != CAN_OK) {
		CAN_DEBUG_PRINT("ERROR: Controller mode was not set successfully\r\n");
		return;
	}

	/* Set baud rate to a pre-defined value */
	rv = Can_SetBaudrate(ctrlId, cfgCtrlPtr->CanControllerBaudRateId);
	if (rv != E_OK) {
		CAN_DEBUG_PRINT("ERROR: Baudrate was not set successfully\r\n");
		return;
	}

	/* Set up acceptance masks to react on all CAN controller ids lower that the maximally defined */
	for (i=0; i < MCP2515_NR_ACCMASKS; i++) {
		Can_WriteController_MsgID(maskRegs[i], CAN_MAX_ECU_CANID);
	}

	/* Set up acceptance filters to only accept messages with this CAN controller's id */
	for (i=0; i < MCP2515_NR_ACCFILTERS; i++) {
		Can_WriteController_MsgID(filterRegs[i], cfgCtrlPtr->CanECUId);
	}

	/* Clear the buffers (three Tx and two Rx),
	 * controlled by 14 registers each, from XBnCTRL to XBnD7) */
	for (i=0; i < 14; i++) {
		for (j=0; j < MCP2515_NR_BUFFERS; j++) {
			Can_WriteController_Reg(ctrlRegs[j] + i, 0);
		}
	}

	/* Enable both rx-buffers to receive messages with standard ids (pp. 23-28 in mcp2515_can.pdf)
	 * and allow RXB0 to rollover (i.e. write to RXB1 if a message arrives when RXB0 is full). */
	Can_WriteController_BitModify(MCP2515_RXB0CTRL,
								  (MCP2515_RXBnCTLR_RXM_MASK | MCP2515_RXBnCTRL_BUKT_MASK),
								  (MCP2515_RXBnCTRL_RXM_STD | MCP2515_RXBnCTRL_BUKT_MASK));
	Can_WriteController_BitModify(MCP2515_RXB1CTRL,
								  MCP2515_RXBnCTLR_RXM_MASK,
								  MCP2515_RXBnCTRL_RXM_STD);

	if (cfgCtrlPtr->CanRxProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT) {
		/* Initialize the pin (on MCU) that is connected to MCP2515's interrupt (INT) pin.
		 * This pin should detect falling edge signals, since INT-pin is driven low by MCP2515
		 * when an interrupt occurs. */
		bcm2835_GpioFnSel(GPIO_CAN_IRQ, GPFN_IN);						// Define this pin to listen for inputs
		bcm2835_SetReadWriteGpioReg(&GPFEN0, GPIO_CAN_IRQ);				// Make the pin react on falling edges

		/* Install the interrupt handler */
		ISR_INSTALL_ISR2("GPIO0", bcm2835_GpioIsr, BCM2835_IRQ_ID_GPIO_0, 2, 0);

		/* Enable interrupts for both Rx-buffers */
		Can_WriteController_Reg(MCP2515_CANINTE,
								(MCP2515_CANINT_RX0I |
								 MCP2515_CANINT_RX1I));

		/* Enable this interrupt pin */
		bcm2835_SetReadWriteGpioReg(&IRQ_ENABLE1, BCM2835_IRQ_ID_GPIO_0);
	}

	/* Now that everything is configured, set the CAN controller to either
	 * normal or loopback (debug) operation mode, depending on the active configuration */
	if (cfgCtrlPtr->Can_Arc_Loopback) {
		Can_SetController_Mode(MCP2515_MODE_LOOPBACK);
	} else {
		Can_SetController_Mode(MCP2515_MODE_NORMAL);
	}
}

/**
 * Can controller state machine (see Autosar_SWS_CANDriver-specification)
 *
 * Currently, this is a shell function that implements specified state changes,
 * but not much more. The actual effects of state changes (for example that the
 * stop state means that the controller stops participating on the network) are
 * not implemented. (TODO)
 *
 * @param ctrlId			----- CAN controller id
 * @param transition		----- pending state transition
 * @return					----- result status
 */
Can_ReturnType Can_SetControllerMode(uint8 ctrlId, Can_StateTransitionType transition) {
	Can_UnitType *canUnit = GET_CAN_UNIT_PTR(ctrlId);

	/* Check that this is a valid controller */
	VALIDATE((ctrlId < GET_CONTROLLER_CNT()),
			CAN_SETCONTROLLERMODE_SERVICE_ID, CAN_E_PARAM_CONTROLLER);
	/* Check that the CAN module has been initialized */
	VALIDATE((canUnit->state != CANIF_CS_UNINIT),
			CAN_SETCONTROLLERMODE_SERVICE_ID, CAN_E_UNINIT);

	/* Change the controller state in response to transition
	 * If a non-valid transition is pending, report an error */
	switch (transition) {
		case CAN_T_START:
			VALIDATE((canUnit->state == CANIF_CS_STARTED) || (canUnit->state == CANIF_CS_STOPPED),
					CAN_SETCONTROLLERMODE_SERVICE_ID, CAN_E_TRANSITION);
			canUnit->state = CANIF_CS_STARTED;
			break;
		case CAN_T_WAKEUP:
			VALIDATE((canUnit->state == CANIF_CS_SLEEP) || (canUnit->state == CANIF_CS_STOPPED),
					CAN_SETCONTROLLERMODE_SERVICE_ID, CAN_E_TRANSITION);
			canUnit->state = CANIF_CS_STOPPED;
			break;
		case CAN_T_SLEEP:
			VALIDATE((canUnit->state == CANIF_CS_SLEEP) || (canUnit->state == CANIF_CS_STOPPED),
					CAN_SETCONTROLLERMODE_SERVICE_ID, CAN_E_TRANSITION);
			canUnit->state = CANIF_CS_SLEEP;
			break;
		case CAN_T_STOP:
			VALIDATE((canUnit->state == CANIF_CS_STOPPED) || (canUnit->state == CANIF_CS_STARTED),
					CAN_SETCONTROLLERMODE_SERVICE_ID, CAN_E_TRANSITION);
			canUnit->state = CANIF_CS_STOPPED;
			break;
		default:
			VALIDATE(canUnit->state == CANIF_CS_STOPPED,
					CAN_SETCONTROLLERMODE_SERVICE_ID, CAN_E_TRANSITION);
			break;
	}

	return CAN_OK;
}

/**
 * Set the nominal bit rate to a pre-defined configuration (see mcp2515_can.pdf, pp. 37-43)
 *
 * @param ctrlId			----- CAN controller id
 * @param baudRateConfigID	----- baudrate id
 * @return 					----- result status
 */
Std_ReturnType Can_SetBaudrate(uint8 ctrlId, const uint16 baudRateConfigID) {
	uint32 baudrate;
	const Can_UnitType *canUnit;
	const Can_ControllerConfigType *cfgCtrlPtr;

	canUnit = GET_CAN_UNIT_PTR(ctrlId);
	cfgCtrlPtr = canUnit->cfgCtrlPtr;

	/** @req 4.1.3/SWC_Can_450 Check that CAN driver has been initialized */
	VALIDATE((Can_Global.initRun == CAN_READY), CAN_CHANGE_BAUD_RATE_SERVICE_ID, CAN_E_UNINIT);
	/** @req 4.1.3/SWC_Can_452 Check that controller id is valid (not out of range) */
	VALIDATE(VALID_CONTROLLER(ctrlId), CAN_CHANGE_BAUD_RATE_SERVICE_ID, CAN_E_PARAM_CONTROLLER);
	/* Check that the baudrate id is valid (not out of range) */
	VALIDATE(VALID_BAUDRATE(baudRateConfigID), CAN_CHECK_BAUD_RATE_SERVICE_ID, CAN_E_PARAM_BAUDRATE);

	/* Get the actual baudrate, corresponding to this id (in kbps) */
	baudrate = cfgCtrlPtr->CanControllerSupportedBaudRates[baudRateConfigID];

	/* Write segment configurations (baudrate prescaler, phase and propagation segments) to the CAN controller */
	Can_WriteController_Reg(MCP2515_CNF1, MCP2515_CNF1_BITS(baudrate));
	Can_WriteController_Reg(MCP2515_CNF2, MCP2515_CNF2_BITS);
	Can_WriteController_Reg(MCP2515_CNF3, MCP2515_CNF3_BITS);

	CAN_DEBUG_PRINT("Baudrate set successfully to %d\r\n", baudrate);

	return E_OK;
}

/**
 * Write data stored in a PDU-structure on the CAN bus
 *
 * Currently, HTH (hardware transmit handle) object is not used in this
 * implementation (TODO)
 *
 * @param hth 				----- hardware transmit handle (HTH)
 * @param pduInfo  			----- PDU (protocol data unit) for this CAN message, incl. data, data length, and id
 * @return 					----- result status
 */
Can_ReturnType Can_Write(Can_HwHandleType hth, Can_PduType *pduInfo) {
	/* Locals */
	Std_ReturnType rv;
	uint8 i;

	/* Check that CAN driver has been initialized */
	VALIDATE((Can_Global.initRun == CAN_READY), CAN_WRITE_SERVICE_ID, CAN_E_UNINIT);
	/* Check that there is a valid PDU object */
	VALIDATE((pduInfo != NULL), CAN_WRITE_SERVICE_ID, CAN_E_PARAM_POINTER);
	/* Check that data length does not exceed max data length in one frame */
	VALIDATE((pduInfo->length <= 8), CAN_WRITE_SERVICE_ID, CAN_E_PARAM_DLC);

	/* Send the data in PDU */
	rv = Can_SendMessage(pduInfo);

	/* Print out the result */
	if (rv == E_OK) {
		CAN_DEBUG_PRINT("infor: success to send id %d, data ", pduInfo->id);
	}
	else {
		CAN_DEBUG_PRINT("error: fail to send id %d, data ", pduInfo->id);
	}
	for (i=0; i < pduInfo->length; i++) {
		CAN_DEBUG_PRINT("%d ", *(pduInfo->sdu+i));
	}
	CAN_DEBUG_PRINT("\r\n");

	/* Return the result */
	return (rv == E_OK) ? CAN_OK : CAN_NOT_OK;
}

/**
 * Read data in Rx-buffers, when CAN processing is driven by polling
 */
void Can_MainFunction_Read(void) {
	/* Locals */
	Spi_DataType stat;
	Can_PduType pduInfo;
	uint8 i;

	const Spi_DataType rxBufRegs[MCP2515_NR_BUFFERS] = {MCP2515_RXBUF_0,					// Rx-buffer registers
													    MCP2515_RXBUF_1};
	const Spi_DataType rxStatBits[MCP2515_NR_BUFFERS] = {MCP2515_CANINT_RX0I,				// Rx-buffer interrupt status bits
													     MCP2515_CANINT_RX1I};

	/** @req 4.0.3/CAN181 Check that the module has been initialized  */
	VALIDATE_NO_RV((Can_Global.initRun == CAN_READY), CAN_MAINFUNCTION_READ_SERVICE_ID, CAN_E_UNINIT);

	/* If CAN communication is defined to be interrupt-driven and
	 * no event has been detected on the CAN interrupt pin, exit */
	if (CAN_INTERRUPT && (bcm2835_ReadGpioPin(&GPEDS0, GPIO_CAN_IRQ) == 0x0))
		return;

	/* Read interrupt status bits to see if any Rx-buffer is non-empty */
	stat = Can_ReadController_Reg(MCP2515_CANINTF, CAN_NO_MASK);

	/* If there is data in any register, read it and clear the interrupt status bit,
	 * so that new data can be written into the Rx-buffer */
	for (i=0; i < MCP2515_NR_BUFFERS; i++) {
		if (stat & rxStatBits[i]) {													// RXBUF_i contains data
			Can_ReadController_Msg(rxBufRegs[i], &pduInfo);							// Read data
			Can_WriteController_BitModify(MCP2515_CANINTF, rxStatBits[i], 0);		// Clear interrupt status

			for (int i = 0; i < pduInfo.length; i++) {								// Debug print out
				CAN_DEBUG_PRINT("%d ", *(pduInfo.sdu+i));
			}
			CAN_DEBUG_PRINT("\r\n");

			/* Set event reception events and forward the CAN message upwards */
			Can_TriggerCanIf(&pduInfo, 0);
		}
	}

	/* In the interrupt mode, don't forget to clear the interrupt pin, when done reading */
	if (CAN_INTERRUPT) {
		bcm2835_ClearEventDetectPin(GPIO_CAN_IRQ);
	}
}

///**
// * Handle interrupts on CAN if the CAN controller is configured to be interrupt-driven
// */
//void Can_RxIsr(void) {
//	if (CAN_INTERRUPT) {//TODO: Hur ska vi göra med denna flagga???
//		/* If an event has been detected on the CAN interrupt pin,
//		 * read data from Rx-buffers */
//		if (bcm2835_ReadGpioPin(&GPEDS0, CAN_INT_PIN) == 0x1) {
//			Can_MainFunction_Read();
//
//			/* Don't forget to clear the interrupt pin, when done reading */
//			bcm2835_ClearEventDetectPin(CAN_INT_PIN);
//		}
//	}
//}

/**
 * @deprecated Check that the target baudrate can be set for this controller
 *
 * Since rel.4.2, use SetBaudrate(ctrlId, baudrateId) directly instead
 *
 *
 * @param ctrlId			----- CAN controller id
 * @param baudrate			----- target baudrate
 * @return rv				----- CAN_OK if the target baudrate can be set
 */
Std_ReturnType Can_CheckBaudrate(uint8 ctrlId, const uint16 baudrate)
{
	/* Locals */
    const Can_UnitType 				*canUnit;
    const Can_ControllerConfigType 	*cfgCtrlPtr;
    const uint32 					*supportedBaudrate;

    /* Check that the CAN module has been initialized, that we have a valid
     * controller and that the baud rate value is not extremely wrong */
    /** @req 4.0.3/CAN456 UNINIT */
    VALIDATE( (Can_Global.initRun == CAN_READY),
    		CAN_CHECK_BAUD_RATE_SERVICE_ID, CAN_E_UNINIT );
    /** @req 4.0.3/CAN457 Invalid controller */
    VALIDATE( VALID_CONTROLLER(ctrlId),
    		CAN_CHECK_BAUD_RATE_SERVICE_ID, CAN_E_PARAM_CONTROLLER );
    /** @req 4.0.3/CAN458 Invalid baudrate value */
    VALIDATE( baudrate < CAN_MAX_BAUDRATE,
    		CAN_CHECK_BAUD_RATE_SERVICE_ID, CAN_E_PARAM_BAUDRATE );

    /* Read controller configuration */
    canUnit = GET_CAN_UNIT_PTR(ctrlId);
    cfgCtrlPtr = canUnit->cfgCtrlPtr;

    /* Loop through supported baud rates and return OK if this particular baud rate was found */
    supportedBaudrate = cfgCtrlPtr->CanControllerSupportedBaudRates;
    while (*supportedBaudrate < CAN_MAX_BAUDRATE) {
    	if (baudrate == *supportedBaudrate) {
    		return CAN_OK;
    	}

    	supportedBaudrate++;
    }

    /* If we reached here, then the baudrate is not supported */
    return CAN_NOT_OK;
}

/**
 * @deprecated Change baudrate on the CAN controller
 *
 * Since rel.4.2, use SetBaudrate(ctrlId, baudrateId) directly instead
 *
 * @param ctrlId			----- CAN controller id
 * @param baudrate			----- target baudrate
 * @return rv				----- E_OK if the target baudrate was set
 */
Std_ReturnType Can_ChangeBaudrate(uint8 ctrlId, const uint16 baudrate) {
	/* Locals */
	Std_ReturnType rv = Can_CheckBaudrate(ctrlId, baudrate);

	if (rv == CAN_OK) {
		/* Write segment configurations (baudrate prescaler, phase and propagation segments) to the CAN controller */
		Can_WriteController_Reg(MCP2515_CNF1, MCP2515_CNF1_BITS(baudrate));
		Can_WriteController_Reg(MCP2515_CNF2, MCP2515_CNF2_BITS);
		Can_WriteController_Reg(MCP2515_CNF3, MCP2515_CNF3_BITS);

		CAN_DEBUG_PRINT("Baudrate set successfully to %d\r\n", baudrate);

		return E_OK;
	}

	return E_NOT_OK;
}

// Non-implemented functions, declared in Can.h
//void Can_DeInit(void);
//void Can_DisableControllerInterrupts( uint8 ctrlId );
//void Can_EnableControllerInterrupts( uint8 ctrlId );
//Can_ReturnType Can_CheckWakeup( uint8 ctrlId );
//void Can_MainFunction_Write( void ){}
//void Can_MainFunction_BusOff( void );
//void Can_MainFunction_Error( void );
//void Can_MainFunction_Wakeup( void );
//void Can_MainFunction_Mode( void );
