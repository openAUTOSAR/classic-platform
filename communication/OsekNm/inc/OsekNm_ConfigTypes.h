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
#ifndef OSEKNM_CONFIGTYPES_H_
#define OSEKNM_CONFIGTYPES_H_

#include "Std_Types.h"
#if defined(USE_COM)
#include "Com_Types.h"
#else
#error "OsekNm: Module Com not used but is required"
#endif
#include "Os.h"

#define OSEKNM_NODE_MAX 32   /* 32 * 8 = 256 Nodes */
#define OSEKNM_INVALID_NETID 0xffu
#define OSEKNM_INVALID_NODEID 0xffu

typedef uint8   NodeIdType;    /* references to several nodes */
typedef uint8   NetIdType;     /* references to several communication networks */

typedef uint8 ScalingParamType;

typedef void (*RoutineRefType)(NetIdType NetId);

typedef enum {
    OSEKNM_OFF = 0,
    OSEKNM_INIT,
    OSEKNM_INITRESET,
    OSEKNM_NORMAL,
    OSEKNM_NORMALPREPSLEEP,
    OSEKNM_TwbsNORMAL,
    OSEKNM_BUSSLEEP,
    OSEKNM_LIMPHOME,
    OSEKNM_LIMPHOMEPREPSLEEP,
    OSEKNM_TwbsLIMPHOME,
    OSEKNM_ON
}OsekNmDirectNmStateType;



/* @req OSEKNM066 */ /* @req OSEKNM064 */ /* @req OSEKNM065*/
/* @req OSEKNM067 */
/* Internal States of Indirect NM */
typedef enum
{
    OSEKINDNM_OFF = 0,
    OSEKINDNM_INIT,
    OSEKINDNM_AWAKE,
    OSEKINDNM_BUSSLEEP,
    OSEKINDNM_NORMAL,
    OSEKINDNM_WAITBUSSLEEP,
    OSEKINDNM_LIMPHOME,
    OSEKINDNM_ON
}OsekNmIndirectNmStateType;

/* Function pointer for User defined State change callback */
typedef void (*OsekNmDirectULStateChangeType)(NetIdType netId, OsekNmDirectNmStateType nodeStatus);
typedef void (*OsekNmIndirectULStateChangeType)(NetIdType netId, OsekNmIndirectNmStateType nodeStatus);


typedef enum{
    NM_DIRECT = 0,
    NM_INDIRECT
}NMType;

typedef enum{
    SIGNAL_INVALID = 0,
    SIGNAL_ACTIVATION,
    SIGNAL_EVENT
}SignallingMode;

typedef enum {
    MASTER=0,
    SLAVE
}IndirectNodeType;

typedef enum {
    GLOBAL_TIMEOUT =0,
    TIMEOUT_PER_MESSAGE
}OsekNmIndirectTimeOutType;

typedef enum{
    NM_BUSSLEEP = 1,
    NM_AWAKE
}NMModeName;

typedef enum{
    NM_CKNORMAL = 1,    /* supported by direct and indirect NM */
    NM_CKNORMALEXTENDED,/* only supported by indirect NM */
    NM_CKLIMPHOME       /* only supported by direct NM */
}ConfigKindName;



/*This data type represents the reference of a configuration*/
typedef uint8* ConfigRefType;

/*This data type represents a handle to reference values of the type ConfigRefType*/
typedef ConfigRefType ConfigHandleType;

/** @req OSEKNM003 */
/** @req OSEKNM054 */ /* network status is made common for both direct and indirect*/
typedef union{
    uint32 status;
    /*lint --e{46}*//*structure must remain the same, field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    struct{
        uint8 configurationstable:1;
        uint8 busoff:1;
        uint8 active:1;
        uint8 nmOff:1;
        uint8 limpHome:1;
        uint8 bussleep:1;
        uint8 twbsNormalLimp:1;
        uint8 ringdataNotallowed:1;
        uint8 gotoModeSleep:1;
        uint8 ExtendedNetworkstatus:2; /* @req OSEKNM052 */
    }nodeStatus;
}NetworkStatusType;


typedef NetworkStatusType* StatusRefType;

typedef StatusRefType StatusHandleType;


typedef uint8 RingDataType[6];
typedef RingDataType* RingDataRefType;

/** @req OSEKNM001 */
#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
typedef struct{
    uint8 source;
    uint8 destination;
    union{
        uint8 b;
        /*lint --e{46}*//*structure must remain the same, field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
        struct {
            uint8 reserved2 :2;
            uint8 sleepAck  :1;
            uint8 sleepInd  :1;
            uint8 reserved1 :1;
            uint8 limphome  :1;
            uint8 ring      :1;
            uint8 alive     :1;
        }B;
    }OpCode;
    RingDataType ringData ;
}OsekNm_PduType;
#else
typedef struct{
    RingDataType ringData ;
    union{
            uint8 b;
            /*lint --e{46}*//*structure must remain the same, field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
            struct {
                uint8 alive     :1;
                uint8 ring      :1;
                uint8 limphome  :1;
                uint8 reserved1 :1;
                uint8 sleepInd  :1;
                uint8 sleepAck  :1;
                uint8 reserved2 :2;
            }B;
        }OpCode;
    uint8 destination;
    uint8 source;
}OsekNm_PduType;
#endif

typedef struct{
    uint8 normal[OSEKNM_NODE_MAX];
    uint8 normalExtended[OSEKNM_NODE_MAX];
    uint8 limphome[OSEKNM_NODE_MAX];
}OsekNm_ConfigParamsType;

typedef struct{
    uint8 normal[OSEKNM_NODE_MAX];
    uint8 normalExtended[OSEKNM_NODE_MAX];
    uint8 limphome[OSEKNM_NODE_MAX];
}OsekNm_CmaskParamsType;

/** @req OSEKNM002 */
typedef struct{
    NodeIdType nodeId;
    TickType   osekdirectNmTTx;
    TickType   osekdirectNmTTyp;
    TickType   osekdirectNmTMax;
    TickType   osekdirectNmTError;
    TickType   osekdirectNmTWbs;
}Oseknm_DirectNMParamsType;

typedef struct{
    Com_SignalIdType comSignalRef;
    NodeIdType osekNmIndirectNodeId;
    uint8     osekNmIndirectDeltaInc;
    uint8     osekNmIndirectDeltaDec;
    uint8     osekNmIndirectNodeHandle;

}OsekNmIndirectNodePrams;

typedef struct{

    Com_SignalIdType comTxSignalRef;
    const OsekNmIndirectNodePrams* osekNmIndirectNodevalues;
    uint8 osekNmIndirectNodeCount;
    IndirectNodeType osekNmIndirectNodeType;
    TickType   osekNmIndirectTOB;
    TickType   osekNmIndirectTError;
    TickType   osekNmIndirectTWBS;
    uint8      osekNmIndirectDeltaInc;
    uint8      osekNmIndirectDeltaDec;
    uint8      osekNmIndirectThreshold;
    NodeIdType osekNmIndirectSourceNodeId;
    ConfigKindName   configName;
    OsekNmIndirectTimeOutType osekNmIndirectTimeoutConf;

}OsekIndNMNetPrams;

typedef struct {
    const Oseknm_DirectNMParamsType* osekDirectNmNodeconfParams;
    const OsekIndNMNetPrams* OsekNmIndirectNodeIdPrams;
    NetIdType        netId;
    PduIdType canIfTxLPduRef;
    uint8            networkHandle;
    NMType           osekNmType;
    NetworkHandleType osekNmNetworkHandle;
}OsekNm_NetworkType;

typedef struct {
    uint8  osekNmNetworkCount;
    const OsekNm_NetworkType* osekNmNetwork;
    const OsekNmDirectULStateChangeType osekNmDirectCallBack;
    const OsekNmIndirectULStateChangeType osekNmIndirectCallBack;
} OsekNm_ConfigType;

#endif /* OSEKNM_CONFIGTYPES_H_ */
