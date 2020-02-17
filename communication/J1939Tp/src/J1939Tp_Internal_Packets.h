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

/**
 * @file  J1939Tp_Internal_Packets.h
 * @brief Structure of the J1939 messages used to manage the transmission of
 *        segmented data.
 *
 * The J1939 communication protocol transfers parameter groups with data
 * (payload) larger than 8 bytes by segmenting the data into packets. The
 * segmented data is sent by two means: either as broadcasted messages with
 * no connection management (BAM transmission), or by means of peer-to-peer
 * managed connections (CMDT transmission).
 * Both trasmission methods use only two parameter groups (transport frames):
 *     - TP.CM (Transport Protocol - Connection Management) with PGN 0x00EC00,
 *       used to announce, initiate, manage and close connections
 *     - TP.DT (Transport Protocol - Data Transfer) with PGN 0x00EB00, used to
 *       transfer the actual data of the large parameter groups. The payload of
 *       this transport frame contains a sequence number in the first byte and
 *       the actual data in the remaining 7 bytes.
 *
 * The connection management is done by filling the 8 bytes data (payload) of
 * the TP.CM parameter group with a special control code (command) in the first
 * byte and the arguments to that command in the remaining 7 bytes. These codes
 * correspond to different messages:
 *     - BAM:    announce the transmission of a broadcasted message
 *               (after sending this message, data is sent in defined 
 *               intervals of time without further announcement and without
 *               to wait for receive confirmations)
 *     - RTS:    request to send
 *               (sender initiates the peer-to-peer connection)
 *     - CTS:    clear to send
 *               (receiver is ready for reception)
 *     - EOMAck: Acknowledge the End of Message
 *               (receiver confirms the reception of the entire message)
 *     - Abort:  Abort Connection.
 *
 *
 * This file defines the following items:
 *     - Parameter Group Numbers (PGN) of the TP.CM and TP.DT parameter groups
 *     - Control codes identifying the management messages
 *     - Structure of the control messages. This structure is given by defining
 *       the indexes of the control byte and the indexes of the arguments to
 *       that command inside the 8 byte data of the J1939 TP.CM frame.
 *
 * Example of using the definitions of this file:
 * @code
 *     uint8 connAbortData[CONNABORT_SIZE];
 *     connAbortData[CM_BYTE_CONTROL]             = CONNABORT_CONTROL_VALUE;
 *     connAbortData[CONNABORT_BYTE_REASON]       = CONNABBORT_REASON_NO_RESSOURCES;
 *     connAbortData[CONNABORT_BYTE_SAE_ASSIGN_2] = RESERVED;
 *     connAbortData[CONNABORT_BYTE_SAE_ASSIGN_3] = RESERVED;
 *     connAbortData[CONNABORT_BYTE_SAE_ASSIGN_4] = RESERVED;
 *     connAbortData[CM_PGN_BYTE_1]               = 0xEE;
 *     connAbortData[CM_PGN_BYTE_2]               = 0xFE
 *     connAbortData[CM_PGN_BYTE_3]               = 0x00;
 * @endcode
 */
#ifndef J1939TP_PACKETS_INTERNAL_H_
#define J1939TP_PACKETS_INTERNAL_H_

/** @brief Data size of mesages transmitted directly. */
#define DIRECT_TRANSMIT_SIZE                8

/** @brief   Maximum data size of a message in bytes.
 *  @details This number equals 7 * 255, where 255 is the maximal sequence
 *           number of a data frame and 7 the number of bytes in a single frame.
 *  @note    Sequence numbers range from 1 to 255. */
#define J1939TP_MAX_DATA_LENGTH             1785u

/*==================[Connection Management Frame]=============================*/

/** The Parameter Group Number (PGN) of a Connection Management message.
 *  This number is 0x00EC00 and takes 18 bits inside the CAN ID. */
#define CM_PGN_VALUE_1                      0x00
#define CM_PGN_VALUE_2                      0xEC
#define CM_PGN_VALUE_3                      0x00

/** Indexes inside the data of a Connection Management frame (TP.CM) where the
 *  Parameter Group Number (PGN) of the transferred message is stored.
 *  This number takes three bytes in little endian order. In order to get the
 *  numerical value of PGN, the byte values must be inverted.
 */
#define CM_PGN_BYTE_1                       5 /* least significant byte */
#define CM_PGN_BYTE_2                       6
#define CM_PGN_BYTE_3                       7 /* most significant byte of the PGN*/

/**
 * @brief   Index of the control byte (command) inside the TP.CM frame's data.
 * @details This control byte distinguishes between RTS, CTS, EndOfMsgAck, Abort
 *          and BAM messages.
 */
#define CM_BYTE_CONTROL                     0

/**
 * @brief Size of a connection management message.
 *
 * @details All connection management messages have the same size. They are
 * identified by consulting the first byte.
 */
#define CM_SIZE                             8

/*--------[Connection Management: Clear To Send (CTS) message]----------------*/

/* The Clear To Send command is sent by the receiver node as response to a RTS
 * request when the receiver node is ready to accept incoming data. The content
 * of the CTS message contains information about how many packets, the next
 * packet number and the PGN of the transferred PG. */

/** @brief Data size in bytes of the CTS message. */
#define CTS_SIZE                            CM_SIZE

/** @brief Index of the CTS-command. */
#define CTS_BYTE_CONTROL                    CM_BYTE_CONTROL

/** @brief Total number of packets ready to receive (block size)
 *         before sending another CTS message.
 *  @note  The value stored at this index should not be larger
 *         than the one stored at index 4 in RTS (max blocksize).
 *  @see   SWS Item ECUC_J1939Tp_00061 */
#define CTS_BYTE_NUM_PACKETS                1

/** @brief Next expected packet number. */
#define CTS_BYTE_NEXT_PACKET                2

/** @brief Reserved byte index. Must be filled with 0xFF. */
#define CTS_BYTE_SAE_ASSIGN_1               3

/** @brief Reserved byte index. Must be filled with 0xFF. */
#define CTS_BYTE_SAE_ASSIGN_2               4

/** @brief Indexes of the Parameter Group Number of the transferred message.
 *  This number takes three bytes in little endian order. */
#define CTS_BYTE_PGN_1                      CM_PGN_BYTE_1
#define CTS_BYTE_PGN_2                      CM_PGN_BYTE_2
#define CTS_BYTE_PGN_3                      CM_PGN_BYTE_3

/** @brief First expected packet number. */
#define CTS_START_SEQ_NUM                   1

/** @brief Command code of the CTS message. */
#define CTS_CONTROL_VALUE                   17


/*--------[Connection Management: Broadcast Announce Message (BAM)]-----------*/
/* Following definitions define the indexes used in a BAM announce message.
 * The message is stored in the data segment of a TP.CM frame.
 * BAM is used to announce the transmission of non-managed multicast PG.
 */

/** @brief Size in bytes of entire BAM message data. */
#define BAM_SIZE                            CM_SIZE

/** @brief Index of the control byte. */
#define BAM_BYTE_CONTROL                    CM_BYTE_CONTROL

/** @brief Total message size which will be trasmitted. */
#define BAM_BYTE_LENGTH_1                   1
#define BAM_BYTE_LENGTH_2                   2

/** @brief Number of packets the sender wants to transmit.
 *         This is the number of TP.DT frames needed to send the entire PG data.
 */
#define BAM_BYTE_NUM_PACKETS                3

/** @brief Reserved index. Should be filled with 0xFF. */
#define BAM_BYTE_SAE_ASSIGN                 4

/** Indexes of the PGN */
#define BAM_BYTE_PGN_1                      CM_PGN_BYTE_1
#define BAM_BYTE_PGN_2                      CM_PGN_BYTE_2
#define BAM_BYTE_PGN_3                      CM_PGN_BYTE_3

/** @brief   Command code of the BAM mesage.
 *  @details This is the value to be set in the first data byte. */
#define BAM_CONTROL_VALUE                   32

/** @brief Destination address used for the BAM protocol */
#define BAM_DESTINATION_ADDRESS             0xFFu



/*--------[Connection Management: Request To Send (RTS)]----------------------*/
/* Following definitions define the indexes used in a RTS message.
 * The RTS message is stored in the data segment of a TP.CM frame.
 * RTS is used to initiate (request) a peer-to-peer connection with a specific
 * node.
 */

/** @brief Size in bytes of entire RTS message. */
#define RTS_SIZE                            CM_SIZE

/** @brief Index of the control byte. */
#define RTS_BYTE_CONTROL                    CM_BYTE_CONTROL

/** @brief Total message size. */
#define RTS_BYTE_LENGTH_1                   1
#define RTS_BYTE_LENGTH_2                   2

/** @brief Number of packets (data frames) the sender wants to transmit.*/
#define RTS_BYTE_NUM_PACKETS                3

/** @brief Maximum number of packets (DT frames) the transmitter node is ready
 *         to send before waiting for another CTS message (block size).
 *  @see   CTS_BYTE_NUM_PACKETS
 *  @see   SWS Item ECUC_J1939Tp_00122. */
#define RTS_BYTE_MAX_NUM_PACKETS            4

/** Indexes of the PGN */
#define RTS_BYTE_PGN_1                      CM_PGN_BYTE_1
#define RTS_BYTE_PGN_2                      CM_PGN_BYTE_2
#define RTS_BYTE_PGN_3                      CM_PGN_BYTE_3


/** @brief Command code of the RTS mesage.
 *  This is the value to be set in the first data byte. */
#define RTS_CONTROL_VALUE                   16

/** @brief This special value tells the receiver of the RTS message that the
 *         number of packets it should declare within a CTS message has no upper
 *         limit i.e. the transmitting node can send any number of packets
 *         specified in the CTS message.
 *         This values is stored at index RTS_BYTE_MAX_NUM_PACKETS in the RTS
 *         message. */
#define RTS_MAX_NUM_PACKETS_VALUE           0xFFu


/*--------[Connection Management: End of Message Acknowledgment]--------------*/

/* This message is send by the receiver of a CMDT message to acknowledge the
 * reception of the last data paket. Sending this message closes the connection
 * with the sending node after successfully receiving all data packets.
 */

/** @brief Size of the ACK message. */
#define ENDOFMSGACK_SIZE                    CM_SIZE

/** @brief Index of the ACK control byte inside the TP.CM data. */
#define ENDOFMSGACK_BYTE_CONTROL            CM_BYTE_CONTROL

/* The size of the aknowledged message takes the next two bytes after the
 * control byte. This size is stored in little endian byte order. */

/** @brief LSB of the size of the acknowledged message. */
#define ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_1   1

/** @brief MSB of the size of the aknowledged message. */
#define ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_2   2

/** @brief Number of acknowledged packets. */
#define ENDOFMSGACK_BYTE_NUM_PACKETS        3

/** @brief Reserved byte. Should be filled with 0xFF. */
#define ENDOFMSGACK_BYTE_SAE_ASSIGN         4

/* The PGN of the acknowledged PG takes the next three byte and is stored in
 * little endian byte order. */

/** @brief LSB of the acknowledged PGN. */
#define ENDOFMSGACK_BYTE_PGN_1              CM_PGN_BYTE_1
/** @brief Second byte of the acknowledged PGN. */
#define ENDOFMSGACK_BYTE_PGN_2              CM_PGN_BYTE_2
/** @brief MSB of the acknowledged PGN. */
#define ENDOFMSGACK_BYTE_PGN_3              CM_PGN_BYTE_3

/** @brief Value of the control byte of the End of Message Acknowledge message.
 */
#define ENDOFMSGACK_CONTROL_VALUE           19


/*--------[Connection Management: Connection Abort]---------------------------*/
/* The Connection Abort message is send by the receiver node to close or refuse
 * a connection. */

/** @brief Size of the Connection Abort message. */
#define CONNABORT_SIZE                      CM_SIZE

/** @brief Index of the control byte of the Conn Abort message. */
#define CONNABORT_BYTE_CONTROL              0

/** @brief Index of the abort reason. */
#define CONNABORT_BYTE_REASON               1

/** @brief Reserved index. */
#define CONNABORT_BYTE_SAE_ASSIGN_2         2
/** @brief Reserved index. */
#define CONNABORT_BYTE_SAE_ASSIGN_3         3
/** @brief Reserved index. */
#define CONNABORT_BYTE_SAE_ASSIGN_4         4

/* The PGN of the PG whose transfer is being aborted takes the last three bytes
 * of the abort message. The PGN is stored in little endian order. */

/** @brief LSM or the aborted PGN. */
#define CONNABORT_BYTE_PGN_1                CM_PGN_BYTE_1
/** @brief Second byte of the aborted PGN. */
#define CONNABORT_BYTE_PGN_2                CM_PGN_BYTE_2
/** @brief MSM or the aborted PGN. */
#define CONNABORT_BYTE_PGN_3                CM_PGN_BYTE_3

/** @brief Value of the control byte in a Connection Abort message. */
#define CONNABORT_CONTROL_VALUE             255

/*--------[Abort reasons]-----------------------------------------------------*/
/* Connection abort reasons used in the connection abort message. These values
 * are used in the Connection Abort message. */

/** @brief Already engaged in one or more CMDT sessions with the requesting node
 *         and cannot open another session. */
#define CONNABORT_REASON_BUSY               1u

/** @brief Lacking system resources*/
#define CONNABORT_REASON_NO_RESOURCES       2u

/** @brief Timeout occurred. */
#define CONNABORT_REASON_TIMEOUT            3u

/** @brief CTS message received when data transfer is in progress. */
#define CONNABORT_REASON_CTS                4u

/** @brief Maximum retransmit request limit reached.
 *  @see   item SWS_J1939Tp_00194 of the Autosar J1939 specification v4.1 */
#define CONNABORT_REASON_MAX_RETRY_LIMIT    5u

/** @brief Sequence number error occured during reception and retry support is
 *         disabled.
 *
 *  This error happens when the sequence number of a received data packet does
 *  not match the expected sequence number.
 *  @note The sequence number is the first byte of a TP.DT message. */
#define CONNABORT_REASON_SEQUENCE_ERR       0xFFu


/*--------[Timeouts for a connection]-----------------------------------------*/
/* Connections can be aborted not only when a node sends the Connection Abort
 * message, but also when a timeout occurs, either on the sender side or on the
 * receiver side. The J1939 standard defines a few timouts which should be used
 * in certain situations.
 * All values are given in milliseconds */


/** @brief Minimum packet frequency when sending data packets with BAM. */
#define J1939TP_DT_BROADCAST_MIN_INTERVAL   50

/** @brief   Response time.
 *  @details Maximal time needed by a node to process some data or control flow
 *           message. This is not the time a node waits for its communication
 *           partner, but the internal time needed by a node to process a
 *           request or indication.
 *  @see     Figure 7-3 in Specification of a Transport Layer for SAE J1939,
 *           V1.3.0, R4.1 Rev 1.
 */
#define J1939TP_TR_TIMEOUT_MS               200

/** @brief   Holding time.
 *  @details When the receiving node delays reception by sending an empty CTS
 *           message (wait frame, with number of packages set to 0) it must
 *           resend a new CTS message withing 500 ms in order to keep the
 *           connection alive. Relevant only for CMDT transmission.
 */
#define J1939TP_TH_TIMEOUT_MS               500

/** @brief Maximal elapsed time between reception of two data frames. */
#define J1939TP_T1_TIMEOUT_MS               750

/** @brief Maximal elapsed time after having sent the CTS message without
 *         receiving data.
 */
#define J1939TP_T2_TIMEOUT_MS               1250

/** @brief Maximal elapsed time after having sent the last data packet without
 *         receiving a new CTS or EndOfMessageACK message.
 */
#define J1939TP_T3_TIMEOUT_MS               1250

/** @brief Elapsed time after receiving a delaying CTS message (number of
 *         packets set to zero) without receiving a new CTS message.
 *  @note  The receiver node can delay transmission of new data by sending a
 *         CTS message with the number of packets set to zero.
 */
#define J1939TP_T4_TIMEOUT_MS               1050



/*==================[Data Transfer Frame]=====================================*/
/* The Data Transfer Parameter Group with PGN 0x00EB00 is used to transfer the
 * actual segmented data. The 8 byte data content of this PG contains the
 * sequence number of the current data in the first byte and the data in the
 * remaining 7 bytes. */

/* Parameter Group Number (PGN) of the Data Transfer PG. */

#define DT_PGN_VALUE_1                      0x00
#define DT_PGN_VALUE_2                      0xEB
#define DT_PGN_VALUE_3                      0x00

/** @brief Data size of the TP.DT PG. */
#define DT_SIZE                             CM_SIZE

/** @brief Actual data in the TP.DT.
 *  First byte out of 8 is used for the sequence number. */
#define DT_DATA_SIZE                        7

/** @brief Index of the sequence number in TP.DT data. */
#define DT_BYTE_SEQ_NUM                     0

/* Symbolic values for the indexes of the 7 bytes of data in a TP.DT. */

#define DT_BYTE_DATA_1                      1
#define DT_BYTE_DATA_2                      2
#define DT_BYTE_DATA_3                      3
#define DT_BYTE_DATA_4                      4
#define DT_BYTE_DATA_5                      5
#define DT_BYTE_DATA_6                      6
#define DT_BYTE_DATA_7                      7

/*==================[Metadata]================================================*/
/* Indexes of different components of a CAN Id inside the metadata buffer.
 * The medatadata, which is appended to the regular Sdu data, contains 0 to 4
 * bytes of the 29 bit CAN Id in little endian order, that is the last byte of
 * the CAN Id is stored at index 0 inside the metadata buffer. How many bytes
 * are stored is specified by the configuration parameter MetaDataLength.
 * Here is the layout of a CAN Id for reference:
 +---------------------------------------------------------------------------+
 |  3 bit   |  1 bit   |   1 bit   |    8 bit    |    8 bit    |    8 bit    |
 +---------------------------------------------------------------------------+
 | priority | reserved | data page | PDU format  | destination |   source    |
 |          |          |           |             |  address    |   address   |
 +---------------------------------------------------------------------------+
 |            metadata[3]          | metadata[2] | metadata[1] | metadata[0] |
 +---------------------------------------------------------------------------+
 |                              CAN Id                                       |
 +---------------------------------------------------------------------------+
 * See also SWS_J1939Tp_00198, Specification of a Transport Layer for SAE J1939,
 * part of Autosar R4.1 and "A Comprehensible Guide to J1939" by Wilfried Voss.
 */

/** @brief Index of Source Address inside the metadata buffer. */
#define METADATA_SA                         0
/** @brief Index of Destination Address inside the metadata buffer. */
#define METADATA_DA                         1
/**
 * @brief Index of priority inside the metadata buffer.
 * @note  The priority is stored in the 4th byte of the metadata (little endian
 *        order) and is left shifted by two bits. So when reading the priority
 *        one has to right shift the value 2 bits.
 *        See figure above.
 */
#define METADATA_PRIORITY                   3

/** @brief Maximum value of the MetaDataLength configuration parameter of a Pdu.
 *  This is also the maximal size of the MetaData array which contains part of
 *  or Can Id or the whole Can Id. */
#define METADATALENGTH_MAX                  4

/**
 * @brief Default priority to use for metadata.
 *
 * The priority is an element of the Can Id stored in the metadata.
 * According to the J1939 specification, the default priority of TP.CM and
 * TP.DT parameter groups is 7. This number must be shifted 2 bits to left in
 * order to match the CAN Id layout. See figure above. */
#define J1939TP_DEFAULT_PRIORITY            (7 << 2)

/*==================[Other symbolic values]===================================*/

/** @brief Value of a reserved byte. */
#define J1939TP_RESERVED                    0xFFu

#endif
