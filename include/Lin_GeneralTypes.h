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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=PPC|TMS570|MPC5645S|MPC5607B|ZYNQ|JACINTO6|MPC5748G */

/** @req 4.1.2|4.3.0/SWS_Lin_00245 The content of Lin_GeneralTypes.h shall be protected by a LIN_GENERAL_TYPES define. */
#ifndef LIN_GENERAL_TYPES
#define LIN_GENERAL_TYPES

/** @req 4.1.2|4.3.0/SWS_Lin_00242 The types Lin_PduType and Lin_StatusType used by LIN driver shall be declared in Lin_GeneralTypes.h . */

/** @req 4.1.2|4.3.0/SWS_Lin_00228 Lin_FramePidType*/
/** Represents all valid protected Identifier used by Lin_SendFrame(). */
typedef uint8 Lin_FramePidType;

/** @req 4.1.2|4.3.0/SWS_Lin_00229 Lin_FrameCsModelType */
/** This type is used to specify the Checksum model to be used for the LIN Frame. */
typedef enum {
    LIN_ENHANCED_CS,
    LIN_CLASSIC_CS,
} Lin_FrameCsModelType;

/** @req 4.1.2|4.3.0/SWS_Lin_00230 Lin_FrameResponseType */
/** This type is used to specify whether the frame processor is required to transmit the
 *  response part of the LIN frame. */
typedef enum {
    /** Response is generated from this (master) node */
    LIN_MASTER_RESPONSE=0,
    /** Response is generated from a remote slave node */
    LIN_SLAVE_RESPONSE,
    /** Response is generated from one slave to another slave,
     *  for the master the response will be anonymous, it does not
     *  have to receive the response. */
    LIN_SLAVE_TO_SLAVE,

} Lin_FrameResponseType;

/** This type is used to specify the number of SDU data bytes to copy. */
/** @req 4.1.2|4.3.0/SWS_Lin_00231 Lin_FrameDlType */
typedef uint8 Lin_FrameDIType;

/** @req 4.1.2|4.3.0/SWS_Lin_00232 Lin_PduType */
/** This Type is used to provide PID, checksum model, data length and SDU pointer
 *  from the LIN Interface to the LIN driver. */
typedef struct {
    Lin_FrameCsModelType Cs;
    Lin_FramePidType  Pid;
    uint8* SduPtr;
    Lin_FrameDIType Dl;
    Lin_FrameResponseType Drc;
} Lin_PduType;

/** @req 4.1.2|4.3.0/SWS_Lin_00233 Lin_StatusType */
typedef enum {
    /** LIN frame operation return value.
     *  Development or production error occurred */
    LIN_NOT_OK,

    /** LIN frame operation return value.
     *  Successful transmission. */
    LIN_TX_OK,

    /** LIN frame operation return value.
     *  Ongoing transmission (Header or Response). */
    LIN_TX_BUSY,

    /** LIN frame operation return value.
     *  Erroneous header transmission such as:
     *  - Mismatch between sent and read back data
     *  - Identifier parity error or
     *  - Physical bus error */
    LIN_TX_HEADER_ERROR,

    /** LIN frame operation return value.
     *  Erroneous response transmission such as:
     *  - Mismatch between sent and read back data
     *  - Physical bus error */
    LIN_TX_ERROR,

    /** LIN frame operation return value.
     *  Reception of correct response. */
    LIN_RX_OK,

    /** LIN frame operation return value. Ongoing reception: at
     *  least one response byte has been received, but the
     *  checksum byte has not been received. */
    LIN_RX_BUSY,

    /** LIN frame operation return value.
     *  Erroneous response reception such as:
     *  - Framing error
     *  - Overrun error
     *  - Checksum error or
     *  - Short response */
    LIN_RX_ERROR,


    /** LIN frame operation return value.
     *  No response byte has been received so far. */
    LIN_RX_NO_RESPONSE,

    /** LIN channel state return value.
     *  LIN channel not initialized. */
    LIN_CH_UNINIT,

    /** LIN channel state return value.
     *  Normal operation; the related LIN channel is ready to
     *  transmit next header. No data from previous frame
     *  available (e.g. after initialization) */
    LIN_OPERATIONAL,

    /** LIN channel state return value.
     *  Sleep mode operation; in this mode wake-up detection
     *  from slave nodes is enabled. */
    LIN_CH_SLEEP,

    /** LIN channel state when LinGoToSleep is requested */
    LIN_CH_SLEEP_PENDING

} Lin_StatusType;

typedef enum {
    LINTRCV_TRCV_MODE_NORMAL,
    LINTRCV_TRCV_MODE_STANDBY,
    LINTRCV_TRCV_MODE_SLEEP
}LinTrcv_TrcvModeType;

#endif /* LIN_GENERAL_TYPES */
