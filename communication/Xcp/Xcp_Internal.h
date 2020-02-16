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


#ifndef XCP_INTERNAL_H_
#define XCP_INTERNAL_H_



#include "Xcp_ByteStream.h"
#include "Xcp_ConfigTypes.h"
#include <string.h>

#if(XCP_DEV_ERROR_DETECT)
#include "Det.h"
#endif

#include "Os.h"
#include "ComStack_Types.h"

#include "asdebug.h"

#if !defined(HIGH_BYTE_FIRST)
#	define HIGH_BYTE_FIRST     0U
#endif

#if !defined(LOW_BYTE_FIRST)
#	define LOW_BYTE_FIRST      1U
#endif

#if !defined(CPU_BYTE_ORDER)
#	error Unknown machine endian
#endif


typedef enum {
    XCP_PID_RES  = 0xFF,
    XCP_PID_ERR  = 0xFE,
    XCP_PID_EV   = 0xFD,
    XCP_PID_SERV = 0xFC,
} Xcp_ReplyType;


/*********************    COMMANDS    *****************/

/* STANDARD COMMANDS */
                                                    /* OPTIONAL */
#define XCP_PID_CMD_STD_CONNECT                 0xFF    // N
#define XCP_PID_CMD_STD_DISCONNECT              0xFE    // N
#define XCP_PID_CMD_STD_GET_STATUS              0xFD    // N
#define XCP_PID_CMD_STD_SYNCH                   0xFC    // N
#define XCP_PID_CMD_STD_GET_COMM_MODE_INFO      0xFB    // Y
#define XCP_PID_CMD_STD_GET_ID                  0xFA    // Y
#define XCP_PID_CMD_STD_SET_REQUEST             OxF9    // Y
#define XCP_PID_CMD_STD_GET_SEED                0xF8    // Y
#define XCP_PID_CMD_STD_UNLOCK                  0xF7    // Y
#define XCP_PID_CMD_STD_SET_MTA                 0xF6    // Y
#define XCP_PID_CMD_STD_UPLOAD                  0xF5    // Y
#define XCP_PID_CMD_STD_SHORT_UPLOAD            0xF4    // Y
#define XCP_PID_CMD_STD_BUILD_CHECKSUM          0xF3    // Y
#define XCP_PID_CMD_STD_TRANSPORT_LAYER_CMD     0xF2    // Y
#define XCP_PID_CMD_STD_USER_CMD                0xF1    // Y

/* CALIBRATION COMMANDS */
                                                    /* OPTIONAL */
#define XCP_PID_CMD_CAL_DOWNLOAD                0xF0    // N
#define XCP_PID_CMD_CAL_DOWNLOAD_NEXT           0xEF    // Y
#define XCP_PID_CMD_CAL_DOWNLOAD_MAX            0xEE    // Y
#define XCP_PID_CMD_CAL_SHORT_DOWNLOAD          0xED    // Y
#define XCP_PID_CMD_CAL_MODIFY_BITS             0xEC    // Y

/* PAGE SWITCHING COMMANDS */
                                                    /* OPTIONAL */
#define XCP_PID_CMD_PAG_SET_CAL_PAGE            0xEB    // N
#define XCP_PID_CMD_PAG_GET_CAL_PAGE            0xEA    // N
#define XCP_PID_CMD_PAG_GET_PAG_PROCESSOR_INFO  0xE9    // Y
#define XCP_PID_CMD_PAG_GET_SEGMENT_INFO        0xE8    // Y
#define XCP_PID_CMD_PAG_GET_PAGE_INFO           0xE7    // Y
#define XCP_PID_CMD_PAG_SET_SEGMENT_MODE        0xE6    // Y
#define XCP_PID_CMD_PAG_GET_SEGMENT_MODE        0xE5    // Y
#define XCP_PID_CMD_PAG_COPY_CAL_PAGE           0xE4    // Y

/* DATA SCQUISITION AND STIMULATION COMMANDS */
                                                    /* OPTIONAL */
#define XCP_PID_CMD_DAQ_CLEAR_DAQ_LIST          0xE3    // N
#define XCP_PID_CMD_DAQ_SET_DAQ_PTR             0xE2    // N
#define XCP_PID_CMD_DAQ_WRITE_DAQ               0xE1    // N
#define XCP_PID_CMD_DAQ_SET_DAQ_LIST_MODE       0xE0    // N
#define XCP_PID_CMD_DAQ_GET_DAQ_LIST_MODE       0xDF    // N
#define XCP_PID_CMD_DAQ_START_STOP_DAQ_LIST     0xDE    // N
#define XCP_PID_CMD_DAQ_START_STOP_SYNCH        0xDD    // N
#define XCP_PID_CMD_DAQ_GET_DAQ_CLOCK           0xDC    // Y
#define XCP_PID_CMD_DAQ_READ_DAQ                0xDB    // Y
#define XCP_PID_CMD_DAQ_GET_DAQ_PROCESSOR_INFO  0xDA    // Y
#define XCP_PID_CMD_DAQ_GET_DAQ_RESOLUTION_INFO 0xD9    // Y
#define XCP_PID_CMD_DAQ_GET_DAQ_LIST_INFO       0xD8    // Y
#define XCP_PID_CMD_DAQ_GET_DAQ_EVENT_INFO      0xD7    // Y
#define XCP_PID_CMD_DAQ_FREE_DAQ                0xD6    // Y
#define XCP_PID_CMD_DAQ_ALLOC_DAQ               0xD5    // Y
#define XCP_PID_CMD_DAQ_ALLOC_ODT               0xD4    // Y
#define XCP_PID_CMD_DAQ_ALLOC_ODT_ENTRY         0xD3    // Y

/* NON-VOLATILE MEMORY PROGRAMMING COMMANDS */
                                                    /* OPTIONAL */
#define XCP_PID_CMD_PGM_PROGRAM_START           0xD2    // N
#define XCP_PID_CMD_PGM_PROGRAM_CLEAR           0xD1    // N
#define XCP_PID_CMD_PGM_PROGRAM                 0xD0    // N
#define XCP_PID_CMD_PGM_PROGRAM_RESET           0xCF    // N
#define XCP_PID_CMD_PGM_GET_PGM_PROCESSOR_INFO  0xCE    // Y
#define XCP_PID_CMD_PGM_GET_SECTOR_INFO         0xCD    // Y
#define XCP_PID_CMD_PGM_PROGRAM_PREPARE         0xCC    // Y
#define XCP_PID_CMD_PGM_PROGRAM_FORMAT          0xCB    // Y
#define XCP_PID_CMD_PGM_PROGRAM_NEXT            0xCA    // Y
#define XCP_PID_CMD_PGM_PROGRAM_MAX             0xC9    // Y
#define XCP_PID_CMD_PGM_PROGRAM_VERIFY          0xC8    // Y

/* STIM LISTS */
#define XCP_PID_CMD_STIM_LAST                   0xBF    // Y

/* ERROR CODES */
typedef enum {
    XCP_ERR_CMD_SYNCH         = 0x00,
    XCP_ERR_CMD_BUSY          = 0x10,
    XCP_ERR_DAQ_ACTIVE        = 0x11,
    XCP_ERR_PGM_ACTIVE        = 0x12,

    XCP_ERR_CMD_UNKNOWN       = 0x20,
    XCP_ERR_CMD_SYNTAX        = 0x21,
    XCP_ERR_OUT_OF_RANGE      = 0x22,
    XCP_ERR_WRITE_PROTECTED   = 0x23,
    XCP_ERR_ACCESS_DENIED     = 0x24,
    XCP_ERR_ACCESS_LOCKED     = 0x25,
    XCP_ERR_PAGE_NOT_VALID    = 0x26,
    XCP_ERR_MODE_NOT_VALID    = 0x27,
    XCP_ERR_SEGMENT_NOT_VALID = 0x28,
    XCP_ERR_SEQUENCE          = 0x29,
    XCP_ERR_DAQ_CONFIG        = 0x2A,

    XCP_ERR_MEMORY_OVERFLOW   = 0x30,
    XCP_ERR_GENERIC           = 0x31,
    XCP_ERR_VERIFY            = 0x32,
} Xcp_ErrorType;

/* EVENT CODES */
typedef enum {
    XCP_EV_RESUME_MODE        = 0x00,
    XCP_EV_CLEAR_DAQ          = 0x01,
    XCP_EV_STORE_DAQ          = 0x02,
    XCP_EV_STORE_CAL          = 0x03,
    XCP_EV_CMD_PENDING        = 0x05,
    XCP_EV_DAQ_OVERLOAD       = 0x06,
    XCP_EV_SESSION_TERMINATED = 0x07,
    XCP_EV_USER               = 0xFE,
    XCP_EV_TRANSPORT          = 0xFF,
} Xcp_EventType;

typedef enum  {
    XCP_CHECKSUM_ADD_11      = 0x01,
    XCP_CHECKSUM_ADD_12      = 0x02,
    XCP_CHECKSUM_ADD_14      = 0x03,
    XCP_CHECKSUM_ADD_22      = 0x04,
    XCP_CHECKSUM_ADD_24      = 0x05,
    XCP_CHECKSUM_ADD_44      = 0x06,
    XCP_CHECKSUM_CRC_16      = 0x07,
    XCP_CHECKSUM_CRC_16_CITT = 0x08,
    XCP_CHECKSUM_CRC_32      = 0x09,
    XCP_CHECKSUM_USERDEFINE  = 0xFF,
} Xcp_ChecksumType;


/* COMMAND LIST FUNCTION CALLBACK */

typedef Std_ReturnType (*Xcp_CmdFuncType)(uint8, void*, int);
typedef void           (*Xcp_CmdWorkType)(void);

typedef struct {
    Xcp_CmdFuncType fun;  /**< pointer to function to use */
    uint8           len;  /**< minimum length of command  */
    uint8           lock; /**< locked by following types  (Xcp_ProtectType) */
} Xcp_CmdListType;



/* INTERNAL STATE STRUCTURES */

typedef enum {
    XCP_DYNAMIC_STATE_UNDEFINED = 0,
    XCP_DYNAMIC_STATE_FREE_DAQ,
    XCP_DYNAMIC_STATE_ALLOC_DAQ,
    XCP_DYNAMIC_STATE_ALLOC_ODT,
    XCP_DYNAMIC_STATE_ALLOC_ODT_ENTRY,
} Xcp_DaqListConfigStateEnum;

typedef struct {
    Xcp_OdtEntryType*   ptr;
    Xcp_OdtType*        odt;
    const Xcp_DaqListType*    daq;
    Xcp_DaqListConfigStateEnum dyn;
} Xcp_DaqPtrStateType;

typedef struct {
    int         len; /**< Original upload size */
    int         rem; /**< Remaining upload size */
} Xcp_TransferType;

#ifndef XCP_SEEDKEY_LENGTH
#define XCP_SEEDKEY_LENGTH (XCP_MAX_CTO-2)
#endif

typedef struct {
    Xcp_ProtectType res;

    uint8           seed[XCP_SEEDKEY_LENGTH];
    uint8           seed_len;
    uint8           seed_rem;

    uint8           key[XCP_SEEDKEY_LENGTH];
    uint8           key_len;
    uint8           key_rem;
} Xcp_UnlockType;


/* MEMORY READING/WRITING HELPERS */

typedef enum {
    XCP_MTA_EXTENSION_MEMORY   = 0,
    XCP_MTA_EXTENSION_FLASH    = 1,
    XCP_MTA_EXTENSION_DIO_PORT = 2,
    XCP_MTA_EXTENSION_DIO_CHAN = 3,
    XCP_MTA_EXTENSION_DEBUG    = 0xFF,
} Xcp_MtaExtentionType;

typedef struct Xcp_MtaType {
	uint32 address;
    uint8    extension;
    uint32   buffer;

    unsigned char (*get)  (struct Xcp_MtaType* mta);
#if 0
    void          (*put)  (struct Xcp_MtaType* mta, unsigned char val);
#endif
    void          (*write)(struct Xcp_MtaType* mta, uint8* data, int len);
    void          (*read) (struct Xcp_MtaType* mta, uint8* data, int len);
    void          (*flush)(struct Xcp_MtaType* mta);
} Xcp_MtaType;

typedef struct
{
	const Xcp_ConfigType* config;
	const Xcp_DaqListType *XcpDaqList;

	Xcp_ProtectType XcpProtect; /**< Bitfield with currently locked features (Xcp_ProtectType) */

	uint16 XcpMaxDaq; /* 0 .. 65535, XCP_MAX_DAQ */

	/** Daq      counter which shows the dynamically allocated number of Daqs */
	uint16            cntrDynamicDaq;

	/** Odt      counter which shows the dynamically allocated number of Odts */
	uint8             cntrDynamicOdt;

	/** OdtEntry counter which shows the dynamically allocated number of OdtEntries */
	uint8             cntrDynamicOdtEntry;
} Xcp_ContextType;

/* INTERNAL GLOBAL VARIABLES */
extern       Xcp_ContextType    Xcp_Context;
extern       Xcp_FifoType    Xcp_FifoRx;
extern       Xcp_FifoType    Xcp_FifoTx;
extern       Xcp_MtaType       Xcp_Mta;
extern       int             Xcp_Inited;
extern       int             Xcp_Connected;

/* MTA HELPER FUNCTIONS */
void                Xcp_MtaInit (Xcp_MtaType* mta, uint32 address, uint8 extension);                       /**< Open a new mta reader/writer */
static inline void  Xcp_MtaFlush(Xcp_MtaType* mta)                       { if(mta->flush) mta->flush(mta); } /**< Will flush any remaining data to write */
static inline void  Xcp_MtaWrite(Xcp_MtaType* mta, uint8* data, int len) { mta->write(mta, data, len); }
static inline void  Xcp_MtaRead (Xcp_MtaType* mta, uint8* data, int len) { mta->read(mta, data, len);}
static inline uint8 Xcp_MtaGet  (Xcp_MtaType* mta)                       { return mta->get(mta); }
#if 0
static inline void  Xcp_MtaPut  (Xcp_MtaType* mta, uint8 val)            { mta->put(mta, val);}
#endif

/* PROGRAMMING COMMANDS */
Std_ReturnType Xcp_CmdProgramStart(uint8 pid, void* data, int len);
Std_ReturnType Xcp_CmdProgramClear(uint8 pid, void* data, int len);
Std_ReturnType Xcp_CmdProgram(uint8 pid, void* data, int len);
Std_ReturnType Xcp_CmdProgramReset(uint8 pid, void* data, int len);
Std_ReturnType Xcp_CmdProgramInfo(uint8 pid, void* data, int len);


/* CALLBACK FUNCTIONS */

extern void           Xcp_RxIndication(const void* data, int len);
extern void           Xcp_TxConfirmation(void);
extern Std_ReturnType Xcp_Transmit    (const void* data, int len);
extern Std_ReturnType Xcp_CmdTransportLayer(uint8 pid, void* data, int len);

extern void Xcp_TxError(Xcp_ErrorType code);
#if 0
extern void Xcp_TxEvent(Xcp_EventType code);
#endif
extern void Xcp_TxSuccess();

/* HELPER DEFINES */
#define RETURN_ERROR(code, msg) do {      \
        ASLOG(XCP, msg);\
        Xcp_TxError(code);                \
        return E_NOT_OK;                  \
    } while(0)

#define RETURN_SUCCESS() do { \
        Xcp_TxSuccess();      \
        return E_OK;          \
    } while(0)



#if(XCP_DEV_ERROR_DETECT == STD_ON)
#   define DET_VALIDATE_NRV(_exp, ApiId, ErrorId) \
        if( !(_exp) ) { \
          (void)Det_ReportError(XCP_MODULE_ID, 0, ApiId, ErrorId); \
          return; \
        }
#   define DET_VALIDATE_RV(_exp, ApiId, ErrorId, Return) \
        if( !(_exp) ) { \
          (void)Det_ReportError(XCP_MODULE_ID, 0, ApiId, ErrorId); \
          return Return; \
        }

#   define DET_REPORTERROR(ApiId,ErrorId) Det_ReportError(XCP_MODULE_ID, 0, ApiId, ErrorId)
#else
#   define DET_VALIDATE_NRV(_exp, ApiId, ErrorId )
#   define DET_VALIDATE_RV(_exp, ApiId, ErrorId, Return)
#   define DET_REPORTERROR(ApiId,ErrorId)
#endif


#define XCP_ELEMENT_OFFSET(base) (XCP_ELEMENT_SIZE - 1 - ( (base)+XCP_ELEMENT_SIZE-1 ) % XCP_ELEMENT_SIZE )


#endif /* XCP_INTERNAL_H_ */
