/** @file can.h
*   @brief CAN Driver Header File
*   @date 10.August.2009
*   @version 1.00.000
*   
*   This file contains:
*   - Definitions
*   - Types
*   - Interface Prototypes
*   .
*   which are relevant for the CAN driver.
*/

/* (c) Texas Instruments 2009, All rights reserved. */


#ifndef __CAN_H__
#define __CAN_H__

/* USER CODE BEGIN (0) */
/* USER CODE END */


/* CAN General Definitions */

/** @def canLEVEL_ACTIVE
*   @brief Alias name for CAN error operation level active (Error couter 0-95)  
*/
#define canLEVEL_ACTIVE 0x00U

/** @def canLEVEL_WARNING
*   @brief Alias name for CAN error operation level warning (Error couter 96-127)  
*/
#define canLEVEL_WARNING 0x40U

/** @def canLEVEL_PASSIVE
*   @brief Alias name for CAN error operation level passive (Error couter 128-255)  
*/
#define canLEVEL_PASSIVE 0x20U

/** @def canLEVEL_BUS_OFF
*   @brief Alias name for CAN error operation level bus off (Error couter 256)  
*/
#define canLEVEL_BUS_OFF 0x80U

/** @def canERROR_NO
*   @brief Alias name for no CAN error occured 
*/
#define canERROR_OK 0U

/** @def canERROR_STUFF
*   @brief Alias name for CAN stuff error an RX message 
*/
#define canERROR_STUFF 1U

/** @def canERROR_FORMAT
*   @brief Alias name for CAN form/format error an RX message 
*/
#define canERROR_FORMAT 2U

/** @def canERROR_ACKNOWLEDGE
*   @brief Alias name for CAN TX message wasn't acknowledged  
*/
#define canERROR_ACKNOWLEDGE 3U

/** @def canERROR_BIT1
*   @brief Alias name for CAN TX message sendig recessive level but monitoring dominant  
*/
#define canERROR_BIT1 4U

/** @def canERROR_BIT0
*   @brief Alias name for CAN TX message sendig dominant level but monitoring recessive  
*/
#define canERROR_BIT0 5U

/** @def canERROR_CRC
*   @brief Alias name for CAN RX message received wrong CRC  
*/
#define canERROR_CRC 6U

/** @def canERROR_NO
*   @brief Alias name for CAN no message has send or received sinced last call of canGetLastError  
*/
#define canERROR_NO 7U

/** @def canMESSAGE_BOX1
*   @brief Alias name for CAN message box 1
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX1 1U

/** @def canMESSAGE_BOX2
*   @brief Alias name for CAN message box 2
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX2 2U

/** @def canMESSAGE_BOX3
*   @brief Alias name for CAN message box 3
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX3 3U

/** @def canMESSAGE_BOX4
*   @brief Alias name for CAN message box 4
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX4 4U

/** @def canMESSAGE_BOX5
*   @brief Alias name for CAN message box 5
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX5 5U

/** @def canMESSAGE_BOX6
*   @brief Alias name for CAN message box 6
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX6 6U

/** @def canMESSAGE_BOX7
*   @brief Alias name for CAN message box 7
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX7 7U

/** @def canMESSAGE_BOX8
*   @brief Alias name for CAN message box 8
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX8 8U

/** @def canMESSAGE_BOX9
*   @brief Alias name for CAN message box 9
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX9 9U

/** @def canMESSAGE_BOX10
*   @brief Alias name for CAN message box 10
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX10 10U

/** @def canMESSAGE_BOX11
*   @brief Alias name for CAN message box 11
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX11 11U

/** @def canMESSAGE_BOX12
*   @brief Alias name for CAN message box 12
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX12 12U

/** @def canMESSAGE_BOX13
*   @brief Alias name for CAN message box 13
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX13 13U

/** @def canMESSAGE_BOX14
*   @brief Alias name for CAN message box 14
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX14 14U

/** @def canMESSAGE_BOX15
*   @brief Alias name for CAN message box 15
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX15 15U

/** @def canMESSAGE_BOX16
*   @brief Alias name for CAN message box 16
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX16 16U

/** @def canMESSAGE_BOX17
*   @brief Alias name for CAN message box 17
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX17 17U

/** @def canMESSAGE_BOX18
*   @brief Alias name for CAN message box 18
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX18 18U

/** @def canMESSAGE_BOX19
*   @brief Alias name for CAN message box 19
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX19 19U

/** @def canMESSAGE_BOX20
*   @brief Alias name for CAN message box 20
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX20 20U

/** @def canMESSAGE_BOX21
*   @brief Alias name for CAN message box 21
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX21 21U

/** @def canMESSAGE_BOX22
*   @brief Alias name for CAN message box 22
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX22 22U

/** @def canMESSAGE_BOX23
*   @brief Alias name for CAN message box 23
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX23 23U

/** @def canMESSAGE_BOX24
*   @brief Alias name for CAN message box 24
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX24 24U

/** @def canMESSAGE_BOX25
*   @brief Alias name for CAN message box 25
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX25 25U

/** @def canMESSAGE_BOX26
*   @brief Alias name for CAN message box 26
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX26 26U

/** @def canMESSAGE_BOX27
*   @brief Alias name for CAN message box 27
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX27 27U

/** @def canMESSAGE_BOX28
*   @brief Alias name for CAN message box 28
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX28 28U

/** @def canMESSAGE_BOX29
*   @brief Alias name for CAN message box 29
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX29 29U

/** @def canMESSAGE_BOX30
*   @brief Alias name for CAN message box 30
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX30 30U

/** @def canMESSAGE_BOX31
*   @brief Alias name for CAN message box 31
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX31 31U

/** @def canMESSAGE_BOX32
*   @brief Alias name for CAN message box 32
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX32 32U

/** @def canMESSAGE_BOX33
*   @brief Alias name for CAN message box 33
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX33 33U

/** @def canMESSAGE_BOX34
*   @brief Alias name for CAN message box 34
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX34 34U

/** @def canMESSAGE_BOX35
*   @brief Alias name for CAN message box 35
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX35 35U

/** @def canMESSAGE_BOX36
*   @brief Alias name for CAN message box 36
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX36 36U

/** @def canMESSAGE_BOX37
*   @brief Alias name for CAN message box 37
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX37 37U

/** @def canMESSAGE_BOX38
*   @brief Alias name for CAN message box 38
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX38 38U

/** @def canMESSAGE_BOX39
*   @brief Alias name for CAN message box 39
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX39 39U

/** @def canMESSAGE_BOX40
*   @brief Alias name for CAN message box 40
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX40 40U

/** @def canMESSAGE_BOX41
*   @brief Alias name for CAN message box 41
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX41 41U

/** @def canMESSAGE_BOX42
*   @brief Alias name for CAN message box 42
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX42 42U

/** @def canMESSAGE_BOX43
*   @brief Alias name for CAN message box 43
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX43 43U

/** @def canMESSAGE_BOX44
*   @brief Alias name for CAN message box 44
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX44 44U

/** @def canMESSAGE_BOX45
*   @brief Alias name for CAN message box 45
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX45 45U

/** @def canMESSAGE_BOX46
*   @brief Alias name for CAN message box 46
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX46 46U

/** @def canMESSAGE_BOX47
*   @brief Alias name for CAN message box 47
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX47 47U

/** @def canMESSAGE_BOX48
*   @brief Alias name for CAN message box 48
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX48 48U

/** @def canMESSAGE_BOX49
*   @brief Alias name for CAN message box 49
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX49 49U

/** @def canMESSAGE_BOX50
*   @brief Alias name for CAN message box 50
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX50 50U

/** @def canMESSAGE_BOX51
*   @brief Alias name for CAN message box 51
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX51 51U

/** @def canMESSAGE_BOX52
*   @brief Alias name for CAN message box 52
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX52 52U

/** @def canMESSAGE_BOX53
*   @brief Alias name for CAN message box 53
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX53 53U

/** @def canMESSAGE_BOX54
*   @brief Alias name for CAN message box 54
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX54 54U

/** @def canMESSAGE_BOX55
*   @brief Alias name for CAN message box 55
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX55 55U

/** @def canMESSAGE_BOX56
*   @brief Alias name for CAN message box 56
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX56 56U

/** @def canMESSAGE_BOX57
*   @brief Alias name for CAN message box 57
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX57 57U

/** @def canMESSAGE_BOX58
*   @brief Alias name for CAN message box 58
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX58 58U

/** @def canMESSAGE_BOX59
*   @brief Alias name for CAN message box 59
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX59 59U

/** @def canMESSAGE_BOX60
*   @brief Alias name for CAN message box 60
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX60 60U

/** @def canMESSAGE_BOX61
*   @brief Alias name for CAN message box 61
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX61 61U

/** @def canMESSAGE_BOX62
*   @brief Alias name for CAN message box 62
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX62 62U

/** @def canMESSAGE_BOX63
*   @brief Alias name for CAN message box 63
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX63 63U

/** @def canMESSAGE_BOX64
*   @brief Alias name for CAN message box 64
*
*   @note This value should be used for API argument @a messageBox
*/
#define canMESSAGE_BOX64 64U

/* USER CODE BEGIN (1) */
/* USER CODE END */


/** @struct canBase
*   @brief CAN Register Frame Definition
*
*   This type is used to access the CAN Registers.
*/
/** @typedef canBASE_t
*   @brief CAN Register Frame Type Definition
*
*   This type is used to access the CAN Registers.
*/
typedef volatile struct canBase
{
    unsigned      CTL;          /**< 0x0000: Control Register                       */
    unsigned      ES;           /**< 0x0004: Error and Status Register              */
    unsigned      EERC;         /**< 0x0008: Error Counter Register                 */
    unsigned      BTR;          /**< 0x000C: Bit Timing Register                    */
    unsigned      INT;          /**< 0x0010: Interrupt Register                     */
    unsigned      TEST;         /**< 0x0014: Test Register                          */
    unsigned      : 32U;        /**< 0x0018: Reserved                               */
    unsigned      PERR;         /**< 0x001C: Parity/SECDED Error Code Register      */
    unsigned      REL;          /**< 0x0020: Core Release Register                  */
    unsigned      ECCDIAG;      /**< 0x0024: ECC Diagnostic Register                */
    unsigned      ECCDIADSTAT;  /**< 0x0028: ECC Diagnostic Status Register         */
    unsigned      : 32U;        /**< 0x002C: Reserved                               */
    unsigned      : 32U;        /**< 0x0030: Reserved                               */
    unsigned      : 32U;        /**< 0x0034: Reserved                               */
    unsigned      : 32U;        /**< 0x0038: Reserved                               */
    unsigned      : 32U;        /**< 0x003C: Reserved                               */
    unsigned      : 32U;        /**< 0x0040: Reserved                               */
    unsigned      : 32U;        /**< 0x0044: Reserved                               */
    unsigned      : 32U;        /**< 0x0048: Reserved                               */
    unsigned      : 32U;        /**< 0x004C: Reserved                               */
    unsigned      : 32U;        /**< 0x0050: Reserved                               */
    unsigned      : 32U;        /**< 0x0054: Reserved                               */
    unsigned      : 32U;        /**< 0x0058: Reserved                               */
    unsigned      : 32U;        /**< 0x005C: Reserved                               */
    unsigned      : 32U;        /**< 0x0060: Reserved                               */
    unsigned      : 32U;        /**< 0x0064: Reserved                               */
    unsigned      : 32U;        /**< 0x0068: Reserved                               */
    unsigned      : 32U;        /**< 0x006C: Reserved                               */
    unsigned      : 32U;        /**< 0x0070: Reserved                               */
    unsigned      : 32U;        /**< 0x0074: Reserved                               */
    unsigned      : 32U;        /**< 0x0078: Reserved                               */
    unsigned      : 32U;        /**< 0x007C: Reserved                               */
    unsigned      ABOTR;        /**< 0x0080: Auto Bus On Time Register              */
    unsigned      TXRQX;        /**< 0x0084: Transmission Request X Register        */
    unsigned      TXRQx[4U];    /**< 0x0088-0x0094: Transmission Request Registers  */
    unsigned      NWDATX;       /**< 0x0098: New Data X Register                    */
    unsigned      NWDATx[4U];   /**< 0x009C-0x00A8: New Data Registers              */
    unsigned      INTPNDX;      /**< 0x00AC: Interrupt Pending X Register           */
    unsigned      INTPNDx[4U];  /**< 0x00B0-0x00BC: Interrupt Pending Registers     */
    unsigned      MSGVALX;      /**< 0x00C0: Message Valid X Register               */
    unsigned      MSGVALx[4U];  /**< 0x00C4-0x00D0: Message Valid Registers         */
    unsigned      : 32U;        /**< 0x00D4: Reserved                               */
    unsigned      INTMUXx[4U];  /**< 0x00D8-0x00E4: Interrupt Multiplexer Registers */
    unsigned      : 32U;        /**< 0x00E8: Reserved                               */
    unsigned      : 32U;        /**< 0x00EC: Reserved                               */
    unsigned      : 32U;        /**< 0x00F0: Reserved                               */
    unsigned      : 32U;        /**< 0x00F4: Reserved                               */
    unsigned      : 32U;        /**< 0x00F8: Reserved                               */
    unsigned      : 32U;        /**< 0x00FC: Reserved                               */
#ifdef _little_endian__
    unsigned char IF1NO;        /**< 0x0100: IF1 Command Register, Msg Number       */
    unsigned char IF1STAT;      /**< 0x0100: IF1 Command Register, Status           */
    unsigned char IF1CMD;       /**< 0x0100: IF1 Command Register, Command          */
    unsigned      : 8U;         /**< 0x0100: IF1 Command Register, Reserved         */
#else
    unsigned      : 8U;         /**< 0x0100: IF1 Command Register, Reserved         */
    unsigned char IF1CMD;       /**< 0x0100: IF1 Command Register, Command          */
    unsigned char IF1STAT;      /**< 0x0100: IF1 Command Register, Status           */
    unsigned char IF1NO;        /**< 0x0100: IF1 Command Register, Msg Number       */
#endif
    unsigned      IF1MSK;       /**< 0x0104: IF1 Mask Register                      */
    unsigned      IF1ARB;       /**< 0x0108: IF1 Arbitration Register               */
    unsigned      IF1MCTL;      /**< 0x010C: IF1 Message Control Register           */
    unsigned char IF1DATx[8U];  /**< 0x0110-0x0114: IF1 Data A and B Registers      */
    unsigned      : 32U;        /**< 0x0118: Reserved                               */
    unsigned      : 32U;        /**< 0x011C: Reserved                               */
#ifdef _little_endian__
    unsigned char IF2NO;        /**< 0x0100: IF2 Command Register, Msg No           */
    unsigned char IF2STAT;      /**< 0x0100: IF2 Command Register, Status           */
    unsigned char IF2CMD;       /**< 0x0100: IF2 Command Register, Command          */
    unsigned      : 8U;         /**< 0x0100: IF2 Command Register, Reserved         */
#else
    unsigned      : 8U;         /**< 0x0100: IF2 Command Register, Reserved         */
    unsigned char IF2CMD;       /**< 0x0100: IF2 Command Register, Command          */
    unsigned char IF2STAT;      /**< 0x0100: IF2 Command Register, Status           */
    unsigned char IF2NO;        /**< 0x0100: IF2 Command Register, Msg Number       */
#endif
    unsigned      IF2MSK;       /**< 0x0124: IF2 Mask Register                      */
    unsigned      IF2ARB;       /**< 0x0128: IF2 Arbitration Register               */
    unsigned      IF2MCTL;      /**< 0x012C: IF2 Message Control Register           */
    unsigned char IF2DATx[8U];  /**< 0x0130-0x0134: IF2 Data A and B Registers      */
    unsigned      : 32U;        /**< 0x0138: Reserved                               */
    unsigned      : 32U;        /**< 0x013C: Reserved                               */
    unsigned      IF3OBS;       /**< 0x0140: IF3 Observation Register               */
    unsigned      IF3MSK;       /**< 0x0144: IF3 Mask Register                      */
    unsigned      IF3ARB;       /**< 0x0148: IF3 Arbitration Register               */
    unsigned      IF3MCTL;      /**< 0x014C: IF3 Message Control Register           */
    unsigned char IF3DATx[8U];  /**< 0x0150-0x0154: IF3 Data A and B Registers      */
    unsigned      : 32U;        /**< 0x0158: Reserved                               */
    unsigned      : 32U;        /**< 0x015C: Reserved                               */
    unsigned      IF3UEy[4U];   /**< 0x0160-0x016C: IF3 Update Enable Registers     */
    unsigned      : 32U;        /**< 0x0170: Reserved                               */
    unsigned      : 32U;        /**< 0x0174: Reserved                               */
    unsigned      : 32U;        /**< 0x0178: Reserved                               */
    unsigned      : 32U;        /**< 0x017C: Reserved                               */
    unsigned      : 32U;        /**< 0x0180: Reserved                               */
    unsigned      : 32U;        /**< 0x0184: Reserved                               */
    unsigned      : 32U;        /**< 0x0188: Reserved                               */
    unsigned      : 32U;        /**< 0x018C: Reserved                               */
    unsigned      : 32U;        /**< 0x0190: Reserved                               */
    unsigned      : 32U;        /**< 0x0194: Reserved                               */
    unsigned      : 32U;        /**< 0x0198: Reserved                               */
    unsigned      : 32U;        /**< 0x019C: Reserved                               */
    unsigned      : 32U;        /**< 0x01A0: Reserved                               */
    unsigned      : 32U;        /**< 0x01A4: Reserved                               */
    unsigned      : 32U;        /**< 0x01A8: Reserved                               */
    unsigned      : 32U;        /**< 0x01AC: Reserved                               */
    unsigned      : 32U;        /**< 0x01B0: Reserved                               */
    unsigned      : 32U;        /**< 0x01B4: Reserved                               */
    unsigned      : 32U;        /**< 0x01B8: Reserved                               */
    unsigned      : 32U;        /**< 0x01BC: Reserved                               */
    unsigned      : 32U;        /**< 0x01C0: Reserved                               */
    unsigned      : 32U;        /**< 0x01C4: Reserved                               */
    unsigned      : 32U;        /**< 0x01C8: Reserved                               */
    unsigned      : 32U;        /**< 0x01CC: Reserved                               */
    unsigned      : 32U;        /**< 0x01D0: Reserved                               */
    unsigned      : 32U;        /**< 0x01D4: Reserved                               */
    unsigned      : 32U;        /**< 0x01D8: Reserved                               */
    unsigned      : 32U;        /**< 0x01DC: Reserved                               */
    unsigned      TIOC;         /**< 0x01E0: TX IO Control Register                 */
    unsigned      RIOC;         /**< 0x01E4: RX IO Control Register                 */
} canBASE_t;


/** @def canREG1
*   @brief DCAN1 Register Frame Pointer
*
*   This pointer is used by the CAN driver to access the DCAN1 registers.
*/
#define canREG1 ((canBASE_t *)0xFFF7DC00U)

/** @def canREG2
*   @brief DCAN2 Register Frame Pointer
*
*   This pointer is used by the CAN driver to access the DCAN2 registers.
*/
#define canREG2 ((canBASE_t *)0xFFF7DE00U)

/** @def canREG3
*   @brief DCAN3 Register Frame Pointer
*
*   This pointer is used by the CAN driver to access the DCAN3 registers.
*/
#define canREG3 ((canBASE_t *)0xFFF7E000U)

/* USER CODE BEGIN (2) */
/* USER CODE END */


/* CAN Interface Functions */

void     canInit(void);
unsigned canTransmit(canBASE_t *node, unsigned messageBox, const unsigned char *data);
unsigned canGetData(canBASE_t *node, unsigned messageBox, unsigned char * const data);
unsigned canIsTxMessagePending(canBASE_t *node, unsigned messageBox);
unsigned canIsRxMessageArrived(canBASE_t *node, unsigned messageBox);
unsigned canIsMessageBoxValid(canBASE_t *node, unsigned messageBox);
unsigned canGetLastError(canBASE_t *node);
unsigned canGetErrorLevel(canBASE_t *node);
void     canEnableErrorNotification(canBASE_t *node);
void     canDisableErrorNotification(canBASE_t *node);
unsigned canEnableMessageNotification(canBASE_t *node, unsigned messageBox);
unsigned canDisableMessageNotification(canBASE_t *node, unsigned messageBox);

/** @fn void canErrorNotification(canBASE_t *node, unsigned notification)
*   @brief Error notification
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] notification Error notification code:
*           - canLEVEL_WARNING (0x40): When RX- or TX error counter are between 96 and 127     
*           - canLEVEL_BUS_OFF (0x80): When RX- or TX error counter are above 255     
*
*   @note This function has to be provide by the user.
*/
void canErrorNotification(canBASE_t *node, unsigned notification);

/** @fn void canMessageNotification(canBASE_t *node, unsigned messageBox)
*   @brief Message notification
*   @param[in] node Pointer to CAN node:
*              - canREG1: DCAN1 node pointer
*              - canREG2: DCAN2 node pointer
*              - canREG3: DCAN3 node pointer
*   @param[in] messageBox Message box number of CAN node:
*              - canMESSAGE_BOX1: CAN message box 1
*              - canMESSAGE_BOXn: CAN message box n [n: 1-64]
*              - canMESSAGE_BOX64: CAN message box 64
*
*   @note This function has to be provide by the user.
*/
void canMessageNotification(canBASE_t *node, unsigned messageBox);

/* USER CODE BEGIN (3) */
/* USER CODE END */


#endif
