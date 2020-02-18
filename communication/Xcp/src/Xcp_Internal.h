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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.3 */

#ifndef XCP_INTERNAL_H_
#define XCP_INTERNAL_H_


#include "Xcp.h"
#include "SchM_Xcp.h"
#include <string.h>
#include <stdlib.h>

#if(XCP_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#include "Os.h"
#include "ComStack_Types.h"

#include "debug.h"

/*lint -emacro(904,DET_VALIDATE,DET_VALIDATE_NRV,DET_VALIDATE_RV,RETURN_ERROR,RETURN_SUCCESS) 904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/
/* HELPER DEFINES */
#define RETURN_ERROR(code, ...) do {      \
        DEBUG(DEBUG_HIGH,## __VA_ARGS__ );\
        Xcp_TxError(code);                \
        return E_NOT_OK;                  \
    } while(0)

#define RETURN_SUCCESS() do { \
        Xcp_TxSuccess();      \
        return E_OK;          \
    } while(0)

#if(XCP_DEV_ERROR_DETECT == STD_ON)
#   define DET_VALIDATE(_exp, ApiId, ErrorId) \
        if( !(_exp) ) { \
          (void)Det_ReportError(XCP_MODULE_ID, 0, ApiId, ErrorId); \
        }
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
#   define DET_REPORTERROR(ApiId,ErrorId) (void)Det_ReportError(XCP_MODULE_ID, 0, ApiId, ErrorId)
#else
#   define DET_VALIDATE(_exp, ApiId, ErrorId)
#   define DET_VALIDATE_NRV(_exp, ApiId, ErrorId )
#   define DET_VALIDATE_RV(_exp, ApiId, ErrorId, Return)
#   define DET_REPORTERROR(ApiId,ErrorId)
#endif

/*********************************************
 *          PROTOCOL SETTINGS                *
 *********************************************/

/*Only element size 1 is supported*/
#define XCP_ELEMENT_SIZE 1
#define XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ  1
#define XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM 1

#if(XCP_IDENTIFICATION == XCP_IDENTIFICATION_ABSOLUTE)
#   define XCP_MAX_ODT_SIZE (XCP_MAX_DTO - 1) /**< defines the maximum number of bytes that can fit in a dto packages data area*/
#elif(XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_BYTE)
#   define XCP_MAX_ODT_SIZE (XCP_MAX_DTO - 2) /**< defines the maximum number of bytes that can fit in a dto packages data area*/
#elif(XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_WORD)
#   define XCP_MAX_ODT_SIZE (XCP_MAX_DTO - 3) /**< defines the maximum number of bytes that can fit in a dto packages data area*/
#elif(XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_WORD_ALIGNED)
#   define XCP_MAX_ODT_SIZE (XCP_MAX_DTO - 4) /**< defines the maximum number of bytes that can fit in a dto packages data area*/
#endif

#ifndef    XCP_MAX_ODT_ENTRY_SIZE_DAQ
#   define XCP_MAX_ODT_ENTRY_SIZE_DAQ     XCP_MAX_ODT_SIZE
#endif

#ifndef    XCP_MAX_ODT_ENTRY_SIZE_STIM
#   define XCP_MAX_ODT_ENTRY_SIZE_STIM    XCP_MAX_ODT_SIZE
#endif

#if(XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM > XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ)
#   define XCP_GRANULARITY_ODT_ENTRY_SIZE_MIN XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ
#   define XCP_GRANULARITY_ODT_ENTRY_SIZE_MAX XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM
#else
#   define XCP_GRANULARITY_ODT_ENTRY_SIZE_MIN XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM
#   define XCP_GRANULARITY_ODT_ENTRY_SIZE_MAX XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ
#endif

#ifndef XCP_MAX_ODT_ENTRIES
#   define XCP_MAX_ODT_ENTRIES (XCP_MAX_ODT_SIZE / XCP_GRANULARITY_ODT_ENTRY_SIZE_MIN)
#endif

#define XCP_ELEMENT_OFFSET(base) (XCP_ELEMENT_SIZE - 1 - ( (base)+XCP_ELEMENT_SIZE-1 ) % XCP_ELEMENT_SIZE )

#if (XCP_MAX_DTO > XCP_MAX_CTO)
#	define XCP_MAX_BUFFER XCP_MAX_DTO
#else
#   define XCP_MAX_BUFFER XCP_MAX_CTO
#endif

/*********************    COMMANDS    *****************/

/* STANDARD COMMANDS */
                                                    /* OPTIONAL */
#define XCP_PID_CMD_STD_CONNECT                 0xFF    // N
#define XCP_PID_CMD_STD_DISCONNECT              0xFE    // N
#define XCP_PID_CMD_STD_GET_STATUS              0xFD    // N
#define XCP_PID_CMD_STD_SYNCH                   0xFC    // N
#define XCP_PID_CMD_STD_GET_COMM_MODE_INFO      0xFB    // Y
#define XCP_PID_CMD_STD_GET_ID                  0xFA    // Y
#define XCP_PID_CMD_STD_SET_REQUEST             0xF9    // Y
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
#define XCP_PID_CMD_PAG_SET_CAL_PAGE            0xEB    // Y
#define XCP_PID_CMD_PAG_GET_CAL_PAGE            0xEA    // Y
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


/* Exclusive areas */
#define SchM_Enter_Xcp(_area) SchM_Enter_Xcp_##_area()
#define SchM_Exit_Xcp(_area) SchM_Exit_Xcp_##_area()

#define XCP_UNUSED(x) (void)(x)

// SET ENDIANNESS
// BIG ENDIAN
#if defined(CFG_PPC) || defined(CFG_TMS570)
#define XPC_C32_0 0
#define XPC_C32_1 1
#define XPC_C32_2 2
#define XPC_C32_3 3
#define XPC_C16_0 0
#define XPC_C16_1 1
#else
// LITTLE ENDIAN
#define XPC_C32_0 3
#define XPC_C32_1 2
#define XPC_C32_2 1
#define XPC_C32_3 0
#define XPC_C16_0 1
#define XPC_C16_1 0
#endif

static inline uint8 GET_UINT8(const uint8* data, uint8 offset) {
    return *(uint8*)(data + offset);
}

static inline uint16 GET_UINT16(const uint8* data, uint8 offset) {
	uint8* location = (uint8*)(data + offset);
	uint16 result  = location[XPC_C16_0] << 8u;
		   result |= location[XPC_C16_1];
	return result;
}

static inline uint32 GET_UINT32(const uint8* data, uint8 offset) {
	uint8* location = (uint8*)(data + offset);
	uint32 result  = location[XPC_C32_0] << 24u;
		   result |= location[XPC_C32_1] << 16u;
		   result |= location[XPC_C32_2] << 8u;
		   result |= location[XPC_C32_3];
    return result;
}

static inline void SET_UINT8(uint8* data, uint8 offset, uint8 value) {
	*(uint8*)(data + offset) = value;
}

static inline void SET_UINT16(uint8* data, uint8 offset, uint16 value) {
	uint8* location = (uint8*)(data + offset);
    location[XPC_C16_0] = (uint8)((value & 0xFF00) >> 8u);
	location[XPC_C16_1] = (uint8)(value & 0x00FF);
}

static inline void SET_UINT32(uint8* data, uint8 offset, uint32 value) {
	uint8* location = (uint8*)(data + offset);
    location[XPC_C32_0] = (uint8)((value & 0xFF000000) >> 24u);
    location[XPC_C32_1] = (uint8)((value & 0x00FF0000) >> 16u);
    location[XPC_C32_2] = (uint8)((value & 0x0000FF00) >> 8u);
	location[XPC_C32_3] = (uint8)(value & 0x000000FF);
}

#define FIFO_GET_WRITE(fifo, it) \
    for(Xcp_BufferType* it = Xcp_Fifo_Get(fifo.free); it; Xcp_Fifo_Put(&fifo, it), it = NULL)

#define FIFO_FOR_READ(fifo, it) \
    for(Xcp_BufferType* it = Xcp_Fifo_Get(&fifo); it; Xcp_Fifo_Free(&fifo, it), it = Xcp_Fifo_Get(&fifo))

#define FIFO_ADD_U8(buffer, value) \
    do { SET_UINT8((buffer)->data, (buffer)->len, value); (buffer)->len+=1; } while(0)

#define FIFO_ADD_U16(buffer, value) \
    do { SET_UINT16((buffer)->data, (buffer)->len, value); (buffer)->len+=2; } while(0)

#define FIFO_ADD_U32(buffer, value) \
    do { SET_UINT32((buffer)->data, (buffer)->len, value); (buffer)->len+=4; } while(0)

/* REPLY TYPES */
#define    XCP_PID_RES   0xFFu
#define    XCP_PID_ERR   0xFEu
#define    XCP_PID_EV    0xFDu
#define    XCP_PID_SERV  0xFCu


/* EVENT CODES */

#define    XCP_EV_RESUME_MODE           0x00u
#define    XCP_EV_CLEAR_DAQ             0x01u
#define    XCP_EV_STORE_DAQ             0x02u
#define    XCP_EV_STORE_CAL             0x03u
#define    XCP_EV_CMD_PENDING           0x05u
#define    XCP_EV_DAQ_OVERLOAD          0x06u
#define    XCP_EV_SESSION_TERMINATED    0x07u
#define    XCP_EV_USER                  0xFEu
#define    XCP_EV_TRANSPORT             0xFFu



/* COMMAND LIST FUNCTION CALLBACK */

typedef Std_ReturnType (*Xcp_CmdFuncType)(uint8, uint8*, PduLengthType);
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
    Xcp_DaqListType*    daq;
    Xcp_DaqListConfigStateEnum dyn;
} Xcp_DaqPtrStateType;

typedef struct {
    PduLengthType  len; /**< Original upload size */
    PduLengthType  rem; /**< Remaining upload size */
} Xcp_TransferType;


typedef struct {
    uint8           seed[255];
    uint8           seed_len;
    uint8           seed_rem;
    uint8           key[255];
    uint8           key_len;
    uint8           key_rem;
    Xcp_ProtectType res;
} Xcp_UnlockType;


/* RX/TX FIFO */

typedef struct Xcp_BufferType {
    struct Xcp_BufferType* next;
    PduLengthType          len;
    uint8                  data[XCP_MAX_BUFFER];
} Xcp_BufferType;

typedef struct Xcp_FifoType {
    Xcp_BufferType*        front;
    Xcp_BufferType*        back;
    struct Xcp_FifoType*   free;
    void*                  lock;
} Xcp_FifoType;

/* INTERNAL GLOBAL VARIABLES */
extern       Xcp_FifoType    Xcp_FifoTxCto;
extern       uint8           Xcp_Inited;
extern const Xcp_ConfigType* xcpPtr;
extern       Xcp_MtaType    Xcp_Mta;
extern       uint8          Xcp_Connected;

/* This function is used to enter exclusive area to avoid data correction.*/
static inline void Xcp_Fifo_Lock(void)
{
    SchM_Enter_Xcp_EA_0();
}

/* This function is used to exit from exclusive area.*/
static inline void Xcp_Fifo_Unlock(void)
{
    SchM_Exit_Xcp_EA_0();
}

/* This function retrieves the fist element from the FIFO q.*/
static inline Xcp_BufferType* Xcp_Fifo_Get(Xcp_FifoType* q)
{
    DET_VALIDATE_RV(q, XCP_API_ID_XCP_FIFO_GET, XCP_E_NULL_POINTER,NULL);

    Xcp_Fifo_Lock();
    Xcp_BufferType* b = q->front;
    if(b == NULL) {
        /*FIFO is empty*/
        Xcp_Fifo_Unlock();
        return NULL; /*lint !e904 allow multiple exit */
    }

    /*retrieve b from the FIFO*/
    q->front = b->next;
    if(q->front == NULL) {
        q->back = NULL;
    }
    b->next = NULL;
    Xcp_Fifo_Unlock();
    return b;
}

/* This function add the new element b to the FIFO q.*/
static inline void Xcp_Fifo_Put(Xcp_FifoType* q, Xcp_BufferType* b)
{
    DET_VALIDATE_NRV(q, XCP_API_ID_XCP_FIFO_PUT, XCP_E_NULL_POINTER);
    DET_VALIDATE_NRV(b, XCP_API_ID_XCP_FIFO_PUT, XCP_E_NULL_POINTER);

    Xcp_Fifo_Lock();
    b->next = NULL;
    if(q->back) {
        q->back->next = b; /*add pointer to b from the previous element*/
    } else {
        q->front = b; /*if there is no previous element add as first element*/
    }
    q->back = b;
    Xcp_Fifo_Unlock();
}

/* This function release one element b by adding it to q->free FIFO.*/
static inline void Xcp_Fifo_Free(Xcp_FifoType* q, Xcp_BufferType* b)
{
    DET_VALIDATE_NRV(q, XCP_API_ID_XCP_FIFO_FREE, XCP_E_NULL_POINTER);
    DET_VALIDATE_NRV(b, XCP_API_ID_XCP_FIFO_FREE, XCP_E_NULL_POINTER);

    if(b) {
        b->len = 0;
        Xcp_Fifo_Put(q->free, b);
    }
}

/* This function initialize a FIFO q by the content defined by b and e pointer as start
 * and end pointer respectively.*/
static inline void Xcp_Fifo_Init(Xcp_FifoType* q, Xcp_BufferType* b, Xcp_BufferType* e)
{
    DET_VALIDATE_NRV(q, XCP_API_ID_XCP_FIFO_INIT, XCP_E_NULL_POINTER);
    DET_VALIDATE_NRV(b, XCP_API_ID_XCP_FIFO_INIT, XCP_E_NULL_POINTER);
    DET_VALIDATE_NRV(e, XCP_API_ID_XCP_FIFO_INIT, XCP_E_NULL_POINTER);

    q->front = NULL;
    q->back  = NULL;
    q->lock  = NULL;
    for(;b != e; b++) {
        Xcp_Fifo_Put(q, b);
    }
}

/* This kind of memory extension is used by XCP.c for reading or writing some general information
 * from XCP reserved memory (Ram), or reading from XCP Config (Flash) */
#define XCP_MTA_EXTENSION_MEMORY_INTERNAL_FIXED 0xFFu

/* MTA HELPER FUNCTIONS */
static inline void  Xcp_MtaFlush(Xcp_MtaType* mta)                       { if(mta->flush) mta->flush(mta); } /**< Will flush any remaining data to write */
static inline void  Xcp_MtaWrite(Xcp_MtaType* mta, uint8* data, uint32 len) { mta->write(mta, data, len); }
static inline void  Xcp_MtaRead (Xcp_MtaType* mta, uint8* data, uint32 len) { mta->read(mta, data, len);}
static inline uint8 Xcp_MtaGet  (Xcp_MtaType* mta)                       { return mta->get(mta); }


/* PROGRAMMING COMMANDS */
extern Std_ReturnType Xcp_CmdProgramStart(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdProgramClear(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdProgram(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdProgramReset(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdProgramInfo(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdGetPgmProcessorInfo(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdGetSectorInfo(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdProgramPrepare(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdProgramFormat(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdProgramVerify(uint8 pid, uint8* data, PduLengthType len);

/* ONLINE CALIBRATION COMMANDS*/
extern Std_ReturnType Xcp_CmdSetSegmentMode(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdGetSegmentMode(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdSetCalPage(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdGetCalPage(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdGetPageInfo(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdCopyCalPage(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdGetPagProcessorInfo(uint8 pid, uint8* data, PduLengthType len);
extern Std_ReturnType Xcp_CmdGetSegmentInfo(uint8 pid, uint8* data, PduLengthType len);

/* CALLBACK FUNCTIONS */
extern void           Xcp_RxIndication(const uint8* data, PduLengthType len);
extern void           Xcp_TxConfirmation(void);
extern Std_ReturnType Xcp_Transmit(const uint8* data, PduLengthType len);
extern void 		  Xcp_CancelTxRequests(void);
extern Std_ReturnType Xcp_CmdTransportLayer(uint8 pid, uint8* data, PduLengthType len);
extern void 		  Xcp_TxError(Xcp_ErrorType code);
extern void 	      Xcp_TxSuccess(void);

#if (XCP_PROTOCOL == XCP_PROTOCOL_CAN)
extern void           Xcp_CanLayerInit(void);
#elif (XCP_PROTOCOL == XCP_PROTOCOL_ETHERNET)
extern void           Xcp_SoAdLayerInit(void);
#endif

/* GENERIC HELPER FUNCTIONS */
extern void           Xcp_MemCpy(void * dst, void const * src, size_t len);

#endif /* XCP_INTERNAL_H_ */
