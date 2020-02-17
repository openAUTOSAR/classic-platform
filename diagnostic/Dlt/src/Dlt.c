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

/*
 * Diagnostic Log & Trace module
 *
 */

/* Requirements implemented */
/* @req SWS_Dlt_00464 */
/* @req SWS_Dlt_00466 */
/* @req SWS_Dlt_00482 */
/* @req SWS_Dlt_00005 */
/* @req SWS_Dlt_00007 */
/* @req SWS_Dlt_00009 */
/* @req SWS_Dlt_00010 */
/* @req SWS_Dlt_00011 */
/* @req SWS_Dlt_00014 */
/* @req SWS_Dlt_00474 */
/* @req SWS_Dlt_00475 */
/* @req SWS_Dlt_00377 */
/* @req SWS_Dlt_00477 */
/* @req SWS_Dlt_00478 */
/* @req SWS_Dlt_00479 */
/* @req SWS_Dlt_00430 */
/* @req SWS_Dlt_00431 */
/* @req SWS_Dlt_00376 */
/* @req SWS_Dlt_00480 */
/* @req SWS_Dlt_00031 */
/* @req SWS_Dlt_00040 */
/* @req SWS_Dlt_00042 */
/* @req SWS_Dlt_00043 */
/* @req SWS_Dlt_00044 */
/* @req SWS_Dlt_00052 */
/* @req SWS_Dlt_00341 */
/* @req SWS_Dlt_00342 */
/* @req SWS_Dlt_00053 */
/* @req SWS_Dlt_00461 */
/* @req SWS_Dlt_00462 */
/* @req SWS_Dlt_00463 */
/* @req SWS_Dlt_00450 */
/* 7.6.7 Processing of control messages */
/* @req SWS_Dlt_00079 */
/* @req SWS_Dlt_00351 */
/* @req SWS_Dlt_00456 */

/* 7.6.8 Message Handling */
/* @req SWS_Dlt_00080 */
/* @req SWS_Dlt_00298 */
/* @req SWS_Dlt_00081 */
/* 7.6.8.1  Filling the Header */
/* @req SWS_Dlt_00082 */
/* @req SWS_Dlt_00083 */
/* @req SWS_Dlt_00084 */
/* @req SWS_Dlt_00085 */
/* 7.6.8.2  Filling the extended Header */
/* @req SWS_Dlt_00086 */
/* @req SWS_Dlt_00087 */
/* @req SWS_Dlt_00088 */
/* 7.6.8.3  Switch between Verbose and Non Verbose Mode */
/* @req SWS_Dlt_00089 */
/* @req SWS_Dlt_00090 */
/* @req SWS_Dlt_00300 */

/* 7.7.1&2 Message format */
/* @req SWS_Dlt_00301 */
/* @req SWS_Dlt_00467 */
/* @req SWS_Dlt_00091 */
/* 7.7.3 Standard header */
/* @req SWS_Dlt_00302 */
/* @req SWS_Dlt_00458 */
/* @req SWS_Dlt_00094 */
/* @req SWS_Dlt_00406 */
/* @req SWS_Dlt_00095 */
/* @req SWS_Dlt_00303 */
/* @req SWS_Dlt_00096 */
/* @req SWS_Dlt_00098 */
/* @req SWS_Dlt_00305 */
/* @req SWS_Dlt_00407 */
/* @req SWS_Dlt_00101 */
/* @req SWS_Dlt_00306 */
/* @req SWS_Dlt_00408 */
/* @req SWS_Dlt_00102 */
/* @req SWS_Dlt_00307 */
/* @req SWS_Dlt_00318 */
/* @req SWS_Dlt_00103 */
/* @req SWS_Dlt_00104 */
/* @req SWS_Dlt_00299 */
/* @req SWS_Dlt_00319 */
/* @req SWS_Dlt_00105 */
/* @req SWS_Dlt_00106 */
/* @req SWS_Dlt_00320 */
/* @req SWS_Dlt_00107 */
/* @req SWS_Dlt_00321 */
/* @req SWS_Dlt_00108 */
/* @req SWS_Dlt_00308 */
/* @req SWS_Dlt_00322 */
/* @req SWS_Dlt_00323 */
/* @req SWS_Dlt_00112 */
/* @req SWS_Dlt_00309 */
/* @req SWS_Dlt_00113 */

/* 7.7.4  Dlt Extended Header */
/* @req SWS_Dlt_00116 */
/* @req SWS_Dlt_00117 */
/* @req SWS_Dlt_00457 */
/* @req SWS_Dlt_00118 */
/* @req SWS_Dlt_00119 */
/* @req SWS_Dlt_00310 */
/* @req SWS_Dlt_00324 */
/* @req SWS_Dlt_00120 */
/* @req SWS_Dlt_00325 */
/* @req SWS_Dlt_00121 */
/* @req SWS_Dlt_00122 */
/* @req SWS_Dlt_00123 */
/* @req SWS_Dlt_00124 */
/* @req SWS_Dlt_00125 */
/* @req SWS_Dlt_00326 */
/* @req SWS_Dlt_00126 */
/* @req SWS_Dlt_00127 */
/* @req SWS_Dlt_00312 */
/* @req SWS_Dlt_00128 */
/* @req SWS_Dlt_00313 */
/* @req SWS_Dlt_00314 */
/* @req SWS_Dlt_00315 */

/* 7.7 Protocol Specification (for transmitting to a external client and
 saving on the client) */

/* @req SWS_Dlt_00460 */
/* @req SWS_Dlt_00129 */
/* @req SWS_Dlt_00329 */
/* @req SWS_Dlt_00352 */
/* @req SWS_Dlt_00353 */
/* @req SWS_Dlt_00134 */
/* @req SWS_Dlt_00459 */
/* @req SWS_Dlt_00409 */

/* @req SWS_Dlt_00378 */
/* @req SWS_Dlt_00186 */
/* @req SWS_Dlt_00187 */
/* @req SWS_Dlt_00188 */
/* @req SWS_Dlt_00189 */
/* @req SWS_Dlt_00190 */
/* @req SWS_Dlt_00191 */
/* @req SWS_Dlt_00417 */
/* @req SWS_Dlt_00416 */
/* @req SWS_Dlt_00192 */
/* @req SWS_Dlt_00193 */

/* 7.7.7.1  Control messages */
/* @req SWS_Dlt_00380 */
/* @req SWS_Dlt_00205 */
/* @req SWS_Dlt_00206 */
/* @req SWS_Dlt_00393 */

/* @req SWS_Dlt_00447 */
/* @req SWS_Dlt_00468 */

/* 8.2 Service interfaces */
/* @req SWS_Dlt_00495 */
/* @req SWS_Dlt_00499 */

/* 8.3 Type definitions */
/* @req SWS_Dlt_00437 */
/* @req SWS_Dlt_00224 */
/* @req SWS_Dlt_00225 */
/* @req SWS_Dlt_00226 */
/* @req SWS_Dlt_00227 */
/* @req SWS_Dlt_00228 */
/* @req SWS_Dlt_00229 */
/* @req SWS_Dlt_00230 */
/* @req SWS_Dlt_00231 */
/* @req SWS_Dlt_00232 */
/* @req SWS_Dlt_00235 */
/* @req SWS_Dlt_00236 */
/* @req SWS_Dlt_00237 */
/* @req SWS_Dlt_00238 */

/* 8.4 Functions */
/* @req SWS_Dlt_00239 */
/* @req SWS_Dlt_00271 */
/* @req SWS_Dlt_00241 */
/* @req SWS_Dlt_00242 */
/* @req SWS_Dlt_00470 */
/* @req SWS_Dlt_00432 */
/* @req SWS_Dlt_00272 */
/* @req SWS_Dlt_00273 */
/* @req SWS_Dlt_00262 */
/* @req SWS_Dlt_00515 */
/* @req SWS_Dlt_00516 */
/* @req SWS_Dlt_00517 */
/* @req SWS_Dlt_00263 */

/*
 * Requirements NOT implemented
 */
/* !req SWS_Dlt_00490 */
/* !req SWS_Dlt_00003 */
/* !req SWS_Dlt_00004 */
/* !req SWS_Dlt_00333 */
/* !req SWS_Dlt_00295 */
/* !req SWS_Dlt_00434 */
/* !req SWS_Dlt_00039 */
/* !req SWS_Dlt_00483 */
/* !req SWS_Dlt_00296 */
/* !req SWS_Dlt_00012 */
/* !req SWS_Dlt_00330 */
/* !req SWS_Dlt_00331 */
/* !req SWS_Dlt_00015 */
/* !req SWS_Dlt_00016 */
/* !req SWS_Dlt_00017 */
/* !req SWS_Dlt_00018 */
/* !req SWS_Dlt_00019 */
/* !req SWS_Dlt_00021 */
/* !req SWS_Dlt_00022 */
/* !req SWS_Dlt_00023 */
/* !req SWS_Dlt_00024 */
/* !req SWS_Dlt_00332 */
/* !req SWS_Dlt_00334 */
/* !req SWS_Dlt_00025 */
/* !req SWS_Dlt_00284 */
/* !req SWS_Dlt_00276 */
/* !req SWS_Dlt_00026 */
/* !req SWS_Dlt_00027 */
/* !req SWS_Dlt_00335 */
/* !req SWS_Dlt_00285 */
/* !req SWS_Dlt_00277 */
/* !req SWS_Dlt_00278 */
/* !req SWS_Dlt_00336 */
/* !req SWS_Dlt_00279 */
/* !req SWS_Dlt_00337 */
/* !req SWS_Dlt_00484 */
/* !req SWS_Dlt_00280 */
/* !req SWS_Dlt_00281 */
/* !req SWS_Dlt_00283 */
/* !req SWS_Dlt_00339 */
/* !req SWS_Dlt_00435 */
/* !req SWS_Dlt_00469 */
/* !req SWS_Dlt_00037 */
/* !req SWS_Dlt_00340 */
/* !req SWS_Dlt_00434 */
/* !req SWS_Dlt_00039 */
/* !req SWS_Dlt_00465 */
/* !req SWS_Dlt_00290 */
/* !req SWS_Dlt_00046 */
/* !req SWS_Dlt_00048 */
/* !req SWS_Dlt_00049 */
/* !req SWS_Dlt_00050 */
/* !req SWS_Dlt_00051 */
/* !req SWS_Dlt_00054 */
/* !req SWS_Dlt_00055 */
/* !req SWS_Dlt_00056 */
/* !req SWS_Dlt_00344 */
/* !req SWS_Dlt_00057 */
/* !req SWS_Dlt_00058 */
/* !req SWS_Dlt_00059 */
/* !req SWS_Dlt_00289 */
/* !req SWS_Dlt_00060 */
/* !req SWS_Dlt_00345 */
/* !req SWS_Dlt_00426 */
/* !req SWS_Dlt_00471 */
/* !req SWS_Dlt_00472 */
/* !req SWS_Dlt_00061 */
/* !req SWS_Dlt_00064 */
/* !req SWS_Dlt_00065 */
/* !req SWS_Dlt_00066 */
/* !req SWS_Dlt_00067 */
/* !req SWS_Dlt_00068 */
/* !req SWS_Dlt_00347 */
/* !req SWS_Dlt_00069 */
/* !req SWS_Dlt_00070 */
/* !req SWS_Dlt_00071 */
/* !req SWS_Dlt_00072 */
/* !req SWS_Dlt_00493 */
/* 7.6.6 Storing Configuration in NVRAM */
/* !req SWS_Dlt_00287 */
/* !req SWS_Dlt_00073 */
/* !req SWS_Dlt_00074 */
/* !req SWS_Dlt_00076 */
/* !req SWS_Dlt_00077 */
/* !req SWS_Dlt_00451 */
/* !req SWS_Dlt_00078 */
/* !req SWS_Dlt_00452 */
/* !req SWS_Dlt_00453 */
/* !req SWS_Dlt_00491 */
/* !req SWS_Dlt_00288 */
/* !req SWS_Dlt_00348 */
/* 7.7.3.1.2 Most Significant Byte First (MSBF) */
/* !req SWS_Dlt_00097 */
/* !req SWS_Dlt_00304 */

/* gpt timer */
/* !req SWS_Dlt_00481 */

/* 7.7.5.1.2 Assembly of Non-Static Data(xml and fibex related non driver specific) */
/* !req SWS_Dlt_00338 */
/* !req SWS_Dlt_00402 */
/* !req SWS_Dlt_00403 */
/* !req SWS_Dlt_00418 */
/* !req SWS_Dlt_00396 */
/* !req SWS_Dlt_00397 */
/* !req SWS_Dlt_00398 */
/* !req SWS_Dlt_00399 */
/* !req SWS_Dlt_00400 */
/* !req SWS_Dlt_00401 */

/* 7.7.5.2.2 Type info(VERBOSE MODE not supported */
/* !req SWS_Dlt_00421 */
/* !req SWS_Dlt_00135 */
/* !req SWS_Dlt_00354 */
/* !req SWS_Dlt_00410 */
/* !req SWS_Dlt_00411 */
/* !req SWS_Dlt_00389 */
/* !req SWS_Dlt_00169 */
/* !req SWS_Dlt_00182 */
/* !req SWS_Dlt_00366 */
/* !req SWS_Dlt_00182 */
/* !req SWS_Dlt_00366 */
/* !req SWS_Dlt_00183 */
/* !req SWS_Dlt_00367 */
/* !req SWS_Dlt_00422 */
/* !req SWS_Dlt_00423 */
/* !req SWS_Dlt_00139 */
/* !req SWS_Dlt_00355 */
/* !req SWS_Dlt_00369 */
/* !req SWS_Dlt_00385 */
/* !req SWS_Dlt_00386 */
/* !req SWS_Dlt_00356 */
/* !req SWS_Dlt_00357 */
/* !req SWS_Dlt_00412 */
/* !req SWS_Dlt_00388 */
/* !req SWS_Dlt_00387 */
/* !req SWS_Dlt_00358 */
/* !req SWS_Dlt_00370 */
/* !req SWS_Dlt_00390 */
/* !req SWS_Dlt_00145 */
/* !req SWS_Dlt_00362 */
/* !req SWS_Dlt_00363 */
/* !req SWS_Dlt_00371 */
/* !req SWS_Dlt_00420 */
/* !req SWS_Dlt_00155 */
/* !req SWS_Dlt_00392 */
/* !req SWS_Dlt_00156 */
/* !req SWS_Dlt_00157 */
/* !req SWS_Dlt_00373 */
/* !req SWS_Dlt_00147 */
/* !req SWS_Dlt_00148 */
/* !req SWS_Dlt_00149 */
/* !req SWS_Dlt_00150 */
/* !req SWS_Dlt_00152 */
/* !req SWS_Dlt_00153 */
/* !req SWS_Dlt_00372 */
/* !req SWS_Dlt_00175 */
/* !req SWS_Dlt_00176 */
/* !req SWS_Dlt_00177 */
/* !req SWS_Dlt_00414 */
/* !req SWS_Dlt_00364 */
/* !req SWS_Dlt_00160 */
/* !req SWS_Dlt_00161 */
/* !req SWS_Dlt_00374 */
/* !req SWS_Dlt_00170 */
/* !req SWS_Dlt_00172 */
/* !req SWS_Dlt_00173 */
/* !req SWS_Dlt_00171 */
/* !req SWS_Dlt_00375 */
/* !req SWS_Dlt_00424 */
/* !req SWS_Dlt_00425 */
/* !req SWS_Dlt_00405 */
/* !req SWS_Dlt_00427 */
/* !req SWS_Dlt_00404 */
/* !req SWS_Dlt_00292 */
/* !req SWS_Dlt_00415 */

/* 7.7.7.1  Control messages */
/* !req SWS_Dlt_00194 */
/* !req SWS_Dlt_00196 */
/* !req SWS_Dlt_00383 */
/* !req SWS_Dlt_00381 */
/* !req SWS_Dlt_00382 */
/* !req SWS_Dlt_00197 */
/* !req SWS_Dlt_00198 */
/* !req SWS_Dlt_00494 */
/* !req SWS_Dlt_00199 */
/* !req SWS_Dlt_00200 */
/* !req SWS_Dlt_00201 */
/* !req SWS_Dlt_00501 */
/* !req SWS_Dlt_00502 */
/* !req SWS_Dlt_00202 */
/* !req SWS_Dlt_00503 */
/* !req SWS_Dlt_00489 */
/* !req SWS_Dlt_00203 */
/* !req SWS_Dlt_00204 */
/* !req SWS_Dlt_00504 */
/* !req SWS_Dlt_00505 */
/* !req SWS_Dlt_00207 */
/* !req SWS_Dlt_00208 */
/* !req SWS_Dlt_00209 */
/* !req SWS_Dlt_00210 */
/* !req SWS_Dlt_00506 */
/* !req SWS_Dlt_00211 */
/* !req SWS_Dlt_00212 */
/* !req SWS_Dlt_00507 */
/* !req SWS_Dlt_00213 */
/* !req SWS_Dlt_00214 */
/* !req SWS_Dlt_00508 */
/* !req SWS_Dlt_00215 */
/* !req SWS_Dlt_00216 */
/* !req SWS_Dlt_00509 */
/* !req SWS_Dlt_00217 */
/* !req SWS_Dlt_00218 */
/* !req SWS_Dlt_00219 */
/* !req SWS_Dlt_00220 */
/* !req SWS_Dlt_00487 */

/* 7.7.7.2  Timing messages */
/* !req SWS_Dlt_00221 */
/* !req SWS_Dlt_00222 */
/* !req SWS_Dlt_00394 */
/* !req SWS_Dlt_00395 */
/* !req SWS_Dlt_00492 */
/* !req SWS_Dlt_00492 */

/* 8.2 Service Interfaces */
/* !req SWS_Dlt_00498 */
/* !req SWS_Dlt_00496 */
/* !req SWS_Dlt_00497 */

/* 8.3 Types */
/* !req SWS_Dlt_00233*/

/* 8.4 Function interfaces */
/* !req SWS_Dlt_00243 */
/* !req SWS_Dlt_00244 */
/* !req SWS_Dlt_00245 */

/* 8.4.4 Provided interfaces for Dcm */
/* !req SWS_Dlt_00488 */
/* !req SWS_Dlt_00247 */
/* !req SWS_Dlt_00248 */
/* !req SWS_Dlt_00249 */
/* !req SWS_Dlt_00428 */

/* 8.5.1 Expected Interfaces from SW-Cs */
/* !req SWS_Dlt_00252 */
/* !req SWS_Dlt_00253 */
/* !req SWS_Dlt_00254 */
/* !req SWS_Dlt_00255 */
/* !req SWS_Dlt_00256 */
/* !req SWS_Dlt_00257 */
/* !req SWS_Dlt_00258 */
/* !req SWS_Dlt_00259 */
/* !req SWS_Dlt_00260 */
/* !req SWS_Dlt_00261 */
/* 8.6.2 Expected Interfaces from Dlt communication module */
/* !req SWS_Dlt_00265 */
/* !req SWS_Dlt_00264 */
/* !req SWS_Dlt_00485 */
/* !req SWS_Dlt_00250 */
/* !req SWS_Dlt_00251 */
/* 8.6.3 Expected Interfaces from Gpt */
/* !req SWS_Dlt_00513 */
/* !req SWS_Dlt_00514 */

/* @req SWS_Dlt_00445 */
/* @req SWS_Dlt_00440 */
/* @req SWS_Dlt_00441 */
/* @req SWS_Dlt_00442 */
/* @req SWS_Dlt_00443 */
/* @req SWS_Dlt_00511 */
/* @req SWS_Dlt_00500 */

/*lint -emacro(904,VALIDATE_NO_RV,VALIDATE_RV) *//*904 PC-Lint exception to MISRA 14.7 (validate DET macros).*/

#include "Platform_Types.h"
#include "Dlt.h"
#include "MemMap.h"
#include "string.h"
#include "SchM_Dlt.h"
//#if defined(USE_NVM)
//#include "NvM.h"
//#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if ( DLT_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
#endif

#if (DLT_HEADER_USE_TIMESTAMP==STD_ON)
#include "timer.h"
#endif

#include "cirq_buffer.h"
/* @req SWS_Dlt_00444 */
/* @req SWS_Dlt_00439 */
#if ( DLT_DEV_ERROR_DETECT == STD_ON )

#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(DLT_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_RV(_exp,_api,_err, _rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(DLT_MODULE_ID,0,_api,_err); \
          return _rv; \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#endif

/* @req SWS_Dlt_00229 */
#define DLT_VERBOSE_FLAG 0x08u

/* @req SWS_Dlt_00482 */
/* ----------------------------[Version check]------------------------------*/
#if !(((DLT_SW_MAJOR_VERSION == 1) && (DLT_SW_MINOR_VERSION == 0)) )
#error Dlt: Expected BSW module version to be 1.0.*
#endif

#if !(((DLT_AR_MAJOR_VERSION == 4) && (DLT_AR_MINOR_VERSION == 0)) )
#error Dlt: Expected AUTOSAR version to be 4.0.*
#endif

#define DLT_NOF_MESSAGES (DLT_MESSAGE_BUFFERSIZE / DLT_MAX_MESSAGE_LENGTH)

typedef enum {
    DLT_NOT_INITIALIZED, DLT_INITIALIZED, DLT_CONNECTED,
} Dlt_InternalStateType;

/* Static module specific variables */
static CirqBufferType cirqBuf;

static const Dlt_ConfigType *ptrConfig;

/* Variable used to hold the current message filtering status */
static uint8 filterMessages = 1u;/* Default on */
static uint8 defaultLogLevel = DLT_DEFAULT_MAX_LOG_LEVEL;

/*lint -esym(9003,messageCounter) Misra violation. Used by tests and cannot be defined at block scope */
#if (DLT_UNIT_TEST == STD_ON)
uint8 messageCounter = 0u;
#else
static uint8 messageCounter = 0u;
#endif

static Dlt_InternalStateType DltState = DLT_NOT_INITIALIZED;

boolean Dlt_ArcIsDltConnected(void) {
    return (DltState == DLT_CONNECTED);
}
/**
 * Initializes the module
 * @param ConfigPtr - a pointer to the configuration
 */
void Dlt_Init(const Dlt_ConfigType *ConfigPtr) {
    /* @req SWS_Dlt_00239 */
    VALIDATE_NO_RV((NULL != ConfigPtr), DLT_INIT_SERVICE_ID, DLT_E_PARAM_POINTER);
    static uint8 Dlt_SendRingBuffer[DLT_NOF_MESSAGES][DLT_MAX_MESSAGE_LENGTH];

    ptrConfig = ConfigPtr;

#if (DLT_HEADER_USE_TIMESTAMP==STD_ON)
    Timer_Init();
#endif

    /* init circular buffer */
    CirqBuff_Init(&cirqBuf, Dlt_SendRingBuffer, DLT_NOF_MESSAGES, DLT_MAX_MESSAGE_LENGTH);

    messageCounter = 0u;
    defaultLogLevel = DLT_DEFAULT_MAX_LOG_LEVEL;
    filterMessages = 1u;

    DltCom_Init();

    DltState = DLT_INITIALIZED;
}

/**
 * Maybe change endianess for an uint32
 * @param value - parameter to change endianess on
 */
#if 0
static uint32 CheckAndAdaptEndianess32(uint32 value)
{
    uint32 result;
#if (CPU_BYTE_ORDER != HIGH_BYTE_FIRST)
    result = (((value & 0x000000FFu) << 24u) |
            ((value & 0x0000FF00u) << 8u ) |
            ((value & 0x00FF0000u) >> 8u ) |
            ((value & 0xFF000000u) >> 24u) );
#else
    result = value;
#endif
    return result;
}

/**
 * Change endianess for an uint16
 * @param value - parameter to change endianess on
 */
static uint16 CheckAndAdaptEndianess16(uint16 value)
{
    uint16 result;
#if (CPU_BYTE_ORDER != HIGH_BYTE_FIRST)
    result = ( ((value & 0x00FFu) << 8u) |
            ((value & 0xFF00u) >> 8u) );
#else
    result = value;
#endif

    return result;
}
#endif

/**
 * Confirmation on sent messages
 * @param DltTxPduId - Pdu Id
 * @param Result - Result of the transmit
 */
void Dlt_ComTxConfirmation(PduIdType DltTxPduId, Std_ReturnType Result) {
    /* @req SWS_Dlt_00273 */
    (void) DltTxPduId;
    (void) Result; /* dont care about result, just free buffer */

    SchM_Enter_Dlt_EA_0();
    if (NULL != CirqBuff_PopLock(&cirqBuf)) {
        CirqBuff_PopRelease(&cirqBuf);
    }
    SchM_Exit_Dlt_EA_0();
}
/**
 * Create message and send
 * @param payload - a pointer to the payload buffer
 * @param len - length of the payload
 * @param session_id - session id
 */
static Dlt_ReturnType Dlt_CreateLogMessageAndSend(const uint8 *payload, uint16 len, Dlt_SessionIDType session_id,
        const Dlt_ExtendedHeaderType *extHeader) {
    /* @req SWS_Dlt_00301 */
    /* @req SWS_Dlt_00302 */
    /* @req SWS_Dlt_00458 */
    uint16 totlen = 4; /* Add place for mandatory parameters directly */
    uint8 *sendbuffer;
    Dlt_ReturnType result = DLT_E_OK;

    /* Since this function can be called by different context we need to protect the ring buffer
     * from being PushLock/PushRelease'd in the wrong order */
    SchM_Enter_Dlt_EA_0();

    sendbuffer = CirqBuff_PushLock(&cirqBuf);
    if (NULL == sendbuffer) {
        /* No buffer available, remove oldest message */
        /* @req SWS_Dlt_00297 */
        if (NULL != CirqBuff_PopLock(&cirqBuf)) {
            CirqBuff_PopRelease(&cirqBuf);
            /* Try push again */
            sendbuffer = CirqBuff_PushLock(&cirqBuf);
            if (NULL == sendbuffer) {
                /* Buffer is corrupted */
                SchM_Exit_Dlt_EA_0();
                /*lint -e{904} Return statement is necessary to avoid multiple if loops(limit cyclomatic complexity) and hence increase readability */
                return DLT_E_IF_NOT_AVAILABLE;
            }
        }
        else {
            /* Buffer is corrupted */
            SchM_Exit_Dlt_EA_0();
            /*lint -e{904} Return statement is necessary to avoid multiple if loops(limit cyclomatic complexity) and hence increase readability */
            return DLT_E_IF_NOT_AVAILABLE;
        }
    }

    /* Insert header */
    /* @req SWS_Dlt_00377 */
    /* @req SWS_Dlt_00318 *//* @req SWS_Dlt_00103 *//* @req SWS_Dlt_00104 *//* @req SWS_Dlt_00299 */
    sendbuffer[0] = DLT_VERS; /*header type *//* @req SWS_Dlt_00094 */
#if (DLT_HEADER_USE_MSBF==STD_ON)
    sendbuffer[0] |= DLT_MSBF; /* header type *//* @req SWS_Dlt_00094 *//* @req SWS_Dlt_00458 */
#endif
    sendbuffer[1] = messageCounter; /* mcnt */

#if (DLT_HEADER_USE_ECU_ID==STD_ON)
    sendbuffer[0] |= DLT_WEID;
    /*lint --e{9033} inhibit lint warning to avoid false Misra violation */
    sendbuffer[totlen] = (uint8)ptrConfig->EcuId[0];
    sendbuffer[totlen+1] = (uint8)ptrConfig->EcuId[1];
    sendbuffer[totlen+2] = (uint8)ptrConfig->EcuId[2];
    sendbuffer[totlen+3] = (uint8)ptrConfig->EcuId[3];
    totlen += 4;
#endif
#if (DLT_HEADER_USE_SESSION_ID==STD_ON)
    sendbuffer[0] |= DLT_WSID;
    sendbuffer[totlen] = (uint8)(session_id >> 24u);
    sendbuffer[totlen+1] = (uint8)(session_id >> 16u);
    sendbuffer[totlen+2] = (uint8)(session_id >> 8u);
    sendbuffer[totlen+3] = (uint8)session_id;
    totlen += 4;
#endif
#if (DLT_HEADER_USE_TIMESTAMP==STD_ON)
    sendbuffer[0] |= DLT_WTMS;
    /* use timer as solution  */
    uint32 timeStamp = Timer_GetTicks();
    sendbuffer[totlen] = (uint8)(timeStamp >> 24u);
    sendbuffer[totlen+1] = (uint8)(timeStamp >> 16u);
    sendbuffer[totlen+2] = (uint8)(timeStamp >> 8u);
    sendbuffer[totlen+3] = (uint8)timeStamp;
    totlen += 4;
#endif
#if (DLT_HEADER_USE_EXTENDED_HEADER==STD_ON)
    sendbuffer[0] |= DLT_UEH;
    sendbuffer[totlen] = extHeader->MSIN;
    sendbuffer[totlen+1] = extHeader->NOAR;
    sendbuffer[totlen+2] = extHeader->APID[0];
    sendbuffer[totlen+3] = extHeader->APID[1];
    sendbuffer[totlen+4] = extHeader->APID[2];
    sendbuffer[totlen+5] = extHeader->APID[3];
    sendbuffer[totlen+6] = extHeader->CTID[0];
    sendbuffer[totlen+7] = extHeader->CTID[1];
    sendbuffer[totlen+8] = extHeader->CTID[2];
    sendbuffer[totlen+9] = extHeader->CTID[3];
    totlen += 10;
#endif

//#if (DLT_HEADER_USE_MSBF==STD_ON)

    /* standard payload just copy to buffer and send */
    /* @req SWS_Dlt_00080 */
    if (len > 0) {
        memcpy(&sendbuffer[totlen], payload, len);
        totlen += len;
    }

    sendbuffer[2] = (uint8) (totlen >> 8u); /* len *//* @req SWS_Dlt_00320 *//* SWS_Dlt_00107 */
    sendbuffer[3] = (uint8) (totlen); /* len *//* @req SWS_Dlt_00320 *//* SWS_Dlt_00107 */

    /* Message Counter */
    /* @req SWS_Dlt_00319 *//* @req SWS_Dlt_00105*//* @req SWS_Dlt_00106*/
    messageCounter++;

    CirqBuff_PushRelease(&cirqBuf);

    SchM_Exit_Dlt_EA_0();

    /* Send using DltCom */
    Std_ReturnType res;
    PduInfoType pduInfo;
    pduInfo.SduDataPtr = sendbuffer;
    pduInfo.SduLength = totlen;
    res = DltCom_Transmit(messageCounter/*pdu id not really needed, use mcnt for id*/, &pduInfo);
    if (res != E_OK) {
        result = DLT_E_IF_NOT_AVAILABLE;
    }
    return result;
}

/**
 * The service represents the interface to be used by basic software modules or by
 * software component to send log messages.
 * @param session_id
 * @param log_info
 * @param log_data
 * @param log_data_length
 */
Dlt_ReturnType Dlt_SendLogMessage(Dlt_SessionIDType session_id, const Dlt_MessageLogInfoType* log_info,
        const uint8* log_data, uint16 log_data_length) {

    boolean status;
    status = TRUE;
    /* @req SWS_Dlt_00241 */
    /* @req SWS_Dlt_00107 */

    /* @req SWS_Dlt_00110 */
    Dlt_ReturnType ret = DLT_E_OK;
    Dlt_ExtendedHeaderType extHeader;

    VALIDATE_RV( (NULL != log_data), DLT_SENDLOGMESSAGE_SERVICE_ID, DLT_E_PARAM_POINTER, DLT_E_ERROR_UNKNOWN);

#if (DLT_IMPLEMENT_FILTER_MESSAGES == STD_ON)
    if((defaultLogLevel == 0) || (log_info->log_level > defaultLogLevel) || (filterMessages == 0u)) {
        status = FALSE;
        ret = DLT_E_OK;
    }
#endif

    if (status == TRUE) {
        extHeader.MSIN = ((uint8) DLT_TYPE_LOG << 1u) | (uint8) (log_info->log_level << 4u);
        if (0u != (log_info->options & DLT_VERBOSE_FLAG)) {
            extHeader.NOAR = (uint8) log_info->arg_count;
            extHeader.MSIN |= 0x01u; // set VERB flag
        }
        else {
            extHeader.NOAR = 0u;
        }
        extHeader.APID[0] = log_info->app_id[0];
        extHeader.APID[1] = log_info->app_id[1];
        extHeader.APID[2] = log_info->app_id[2];
        extHeader.APID[3] = log_info->app_id[3];
        extHeader.CTID[0] = log_info->context_id[0];
        extHeader.CTID[1] = log_info->context_id[1];
        extHeader.CTID[2] = log_info->context_id[2];
        extHeader.CTID[3] = log_info->context_id[3];

        /* @req SWS_Dlt_00014 *//* @req SWS_Dlt_00096 */
        if ((0u != (log_info->options & DLT_VERBOSE_FLAG)) && (0u != (log_info->options & DLT_UEH))) {
    #if ((DLT_USE_VERBOSE_MODE == STD_ON) && (DLT_HEADER_USE_EXTENDED_HEADER == STD_ON))
            /* verbose handling */
            if ((log_data_length <= DLT_MAX_MESSAGE_LENGTH)) {
                ret = Dlt_CreateLogMessageAndSend(log_data, log_data_length, session_id, &extHeader);
            }
            else {
                /* @req SWS_Dlt_00081 */
                ret = DLT_E_MSG_TOO_LARGE;
            }
    #else
            /* @req SWS_Dlt_00090 */
            ret = DLT_E_NOT_IN_VERBOSE_MOD;
    #endif
        }
        else {
            /* @req SWS_Dlt_00460 *//* @req SWS_Dlt_00329*//* Non verbose handling */
            /* @req SWS_Dlt_00031 *//* Message IDs used for Dem (0x00000001) and Det (0x00000002)are reserved and not usable for SW-Cs.. */
            /*lint --e{9007} inhibit lint warning to avoid false Misra violation */
            if ((0 != memcmp(log_info->app_id, "DEM", 3)) && (0 != memcmp(log_info->app_id, "DET", 3))) {
                /*lint --e{9033} inhibit lint warning to avoid false Misra violation */
                uint32 messageId = ((uint32) (log_data[0]) << 24u) | ((uint32) (log_data[1]) << 16u)
                        | ((uint32) (log_data[2]) << 8u) | (uint32) (log_data[3]);

                VALIDATE_RV( (messageId != DLT_DEM_MESSAGE_ID) && (messageId != DLT_DET_MESSAGE_ID), DLT_SENDLOGMESSAGE_SERVICE_ID, DLT_E_PARAM_POINTER, DLT_E_ERROR_UNKNOWN);
            }

            if ((log_data_length <= DLT_MAX_MESSAGE_LENGTH)) {
                ret = Dlt_CreateLogMessageAndSend(log_data, log_data_length, session_id, &extHeader);
            }
            else {
                /* @req SWS_Dlt_00081 */
                ret = DLT_E_MSG_TOO_LARGE;
            }
        }
    } else {
        /* return DLT_E_OK */
    }
    return ret;
}

/**
 * The service represents the interface to be used by basic software modules or by
 * software component to send trace messages.
 * @param session_id
 * @param trace_info
 * @param trace_data
 * @param trace_data_length
 */
Dlt_ReturnType Dlt_SendTraceMessage(Dlt_SessionIDType session_id, const Dlt_MessageTraceInfoType* trace_info,
        const uint8* trace_data, uint16 trace_data_length) {
    /* @req !SWS_Dlt_00243 */
    /* @req !SWS_Dlt_00333 */
    /* @req !SWS_Dlt_00011 */

    /* Not implemented yet */
    (void) session_id;
    (void) trace_info; /*lint !e920 Cast to void allowed here since not used */
    (void) trace_data; /*lint !e920 Cast to void allowed here since not used */
    (void) trace_data_length;
    return DLT_E_OK;
}

#if defined(USE_DEM)
/**
 * This service is provided by the Dem in order to call Dlt upon status changes.
 * @param EventId
 * @param EventStatusByteOld
 * @param EventStatusByteNew
 */
void Dlt_DemTriggerOnEventStatus(Dem_EventIdType EventId,
        Dem_EventStatusExtendedType EventStatusByteOld,
        Dem_EventStatusExtendedType EventStatusByteNew)
{
    uint16 totlen = 0;
    uint32 eventId_u32 = (uint32)EventId;
    uint32 messageId = DLT_DEM_MESSAGE_ID;
    static uint8 Dlt_TempBuffer[DLT_MAX_MESSAGE_LENGTH];

    /* ApplicationID =  “DEM”
     * ContextID =  “STD0”
     * MessageID =  0x00000001 */
    /* @req SWS_Dlt_00377 */
    const Dlt_MessageLogInfoType log_info = {
        .app_id = {'D','E','M',0u},
        .context_id = {'S','T','D',0u},
        .arg_count = 2u,
        .options = 0u,
        .log_level = DLT_LOG_WARN,
    };

    /* @req SWS_Dlt_00470 */
    /* @req SWS_Dlt_00475 */
    if((EventStatusByteOld != EventStatusByteNew) && (EventId != 0)) {
        /* @req SWS_Dlt_00476*/
        uint32 dtc = 0;
        uint8 bufsize;

        /* We need to protect the tempbuffer from being overwritten */
        SchM_Enter_Dlt_EA_0();

        Dlt_TempBuffer[totlen] = (uint8)(messageId >> 24u);
        Dlt_TempBuffer[totlen+1] = (uint8)(messageId >> 16u);
        Dlt_TempBuffer[totlen+2] = (uint8)(messageId >> 8u);
        Dlt_TempBuffer[totlen+3] = (uint8)(messageId);
        totlen += 4;

        Dlt_TempBuffer[totlen] = (uint8)(eventId_u32 >> 24u);
        Dlt_TempBuffer[totlen+1] = (uint8)(eventId_u32 >> 16u);
        Dlt_TempBuffer[totlen+2] = (uint8)(eventId_u32 >> 8u);
        Dlt_TempBuffer[totlen+3] = (uint8)(eventId_u32);
        totlen += 4;

        /* @req SWS_Dlt_00477 */
        (void)Dem_GetDTCOfEvent(EventId, DEM_DTC_FORMAT_UDS, &dtc);
        Dlt_TempBuffer[totlen] = (uint8)(dtc >> 24u);
        Dlt_TempBuffer[totlen+1] = (uint8)(dtc >> 16u);
        Dlt_TempBuffer[totlen+2] = (uint8)(dtc >> 8u);
        Dlt_TempBuffer[totlen+3] = (uint8)(dtc);
        totlen += 4;

        /* @req SWS_Dlt_00478 */
        bufsize = 255u;
        if(E_OK == Dem_DltGetAllExtendedDataRecords(EventId, &Dlt_TempBuffer[totlen], &bufsize)) {
            totlen += bufsize;
        }
        else {
            /* Operation failed */
        }

        /* @req SWS_Dlt_00479 */
        bufsize = 255;
        if(E_OK == Dem_DltGetMostRecentFreezeFrameRecordData(EventId, &Dlt_TempBuffer[totlen], &bufsize)) {
            totlen += bufsize;
        }
        else {
            /* Operation failed */
        }

        (void)Dlt_SendLogMessage(messageId, &log_info, Dlt_TempBuffer, totlen);

        SchM_Exit_Dlt_EA_0();
    }
}
#endif

/**
 * Service to forward error reports from Det to Dlt.
 * @param ModuleId
 * @param InstanceId
 * @param ApiId
 * @param ErrorId
 */
void Dlt_DetForwardErrorTrace(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId) {
    /* @req SWS_Dlt_00432 */
    /* @req SWS_Dlt_00430 */
    /* @req SWS_Dlt_00480 */
    /* @req SWS_Dlt_00376 */
    /* The ApplicationID, ContextID and MessageID of the send log
     message shall have the following values:
     ApplicationID =  “DET”
     ContextID =  “STD”
     MessageID =  0x00000002 */
    const Dlt_MessageLogInfoType log_info = { .app_id = { 'D', 'E', 'T', 0u }, .context_id = { 'S', 'T', 'D', 0u },
            .arg_count = 1u, .options = 0u, .log_level = DLT_LOG_ERROR, };
    uint32 messageId = DLT_DET_MESSAGE_ID;
    uint8 payload[9];
    uint16 totlen = 0;

    payload[totlen] = (uint8) (messageId >> 24u);
    payload[totlen + 1] = (uint8) (messageId >> 16u);
    payload[totlen + 2] = (uint8) (messageId >> 8u);
    payload[totlen + 3] = (uint8) (messageId);
    totlen += 4u;

    payload[totlen] = (uint8) (ModuleId >> 8u);
    payload[totlen + 1] = (uint8) ModuleId;
    payload[totlen + 2] = (uint8) InstanceId;
    payload[totlen + 3] = (uint8) ApiId;
    payload[totlen + 4] = (uint8) ErrorId;
    totlen += 5u;

    /* SWS_Dlt_00431 */
    (void) Dlt_SendLogMessage(messageId, &log_info, payload, totlen);
}

/**
 * The service has to be called when a software module wants to use services
 * offered by DLT software component for a specific context. If a context id is being
 * registered for an already registered application id then app_description can be
 * NULL and len_app_description zero.
 * @param session_id
 * @param app_id
 * @param context_id
 * @param app_description
 * @param len_app_description
 * @param context_description
 * @param len_context_description
 */
Dlt_ReturnType Dlt_RegisterContext(Dlt_SessionIDType session_id, Dlt_ApplicationIDType app_id,
        Dlt_ContextIDType context_id, const uint8* app_description, uint8 len_app_description,
        const uint8* context_description, uint8 len_context_description) {
    /* !req SWS_Dlt_00245 */
    /* Not implemented yet */
    (void) session_id;
    (void) app_id;/*lint !e920 Cast to void allowed here since not used */
    (void) context_id;/*lint !e920 Cast to void allowed here since not used */
    (void) app_description;/*lint !e920 Cast to void allowed here since not used */
    (void) len_app_description;
    (void) context_description;/*lint !e920 Cast to void allowed here since not used */
    (void) len_context_description;
    return DLT_E_OK;
}

/**
 * Create control message and send
 * @param payload - a pointer to the payload buffer
 * @param len - length of the payload
 * @param session_id - session id
 */
static Dlt_ReturnType Dlt_CreateControlMessageAndSend(uint8 *buffer, uint16 len, Dlt_SessionIDType session_id,
        const Dlt_ExtendedHeaderType *extHeader) {
    uint16 indx = 4; /* Add place for mandatory parameters directly */
    Dlt_ReturnType result = DLT_E_OK;

    /* Insert header */
    /* @req SWS_Dlt_00377 */
    /* @req SWS_Dlt_00318 *//* @req SWS_Dlt_00103 *//* @req SWS_Dlt_00104 *//* @req SWS_Dlt_00299 */
    buffer[0] = DLT_VERS | DLT_MSBF; /*header type *//* @req SWS_Dlt_00094 */
    buffer[1] = 0; /* mcnt not used for control messages */
    buffer[2] = (uint8) (len >> 8u); /* len *//* @req SWS_Dlt_00320 *//* SWS_Dlt_00107 */
    buffer[3] = (uint8) (len); /* len *//* @req SWS_Dlt_00320 *//* SWS_Dlt_00107 */

#if (DLT_HEADER_USE_ECU_ID==STD_ON)
    buffer[0] |= DLT_WEID;
    /*lint --e{9033} inhibit lint warning to avoid false Misra violation */
    buffer[indx] = (uint8)ptrConfig->EcuId[0];
    buffer[indx+1] = (uint8)ptrConfig->EcuId[1];
    buffer[indx+2] = (uint8)ptrConfig->EcuId[2];
    buffer[indx+3] = (uint8)ptrConfig->EcuId[3];
    indx += 4;
#endif
#if (DLT_HEADER_USE_SESSION_ID==STD_ON)
    buffer[0] |= DLT_WSID;
    buffer[indx] = (uint8)(session_id >> 24u);
    buffer[indx+1] = (uint8)(session_id >> 16u);
    buffer[indx+2] = (uint8)(session_id >> 8u);
    buffer[indx+3] = (uint8)session_id;
    indx += 4;
#endif
#if (DLT_HEADER_USE_TIMESTAMP==STD_ON)
    buffer[0] |= DLT_WTMS;
    /* use timer as solution  */
    uint32 timeStamp = Timer_GetTicks();
    buffer[indx] = (uint8)(timeStamp >> 24u);
    buffer[indx+1] = (uint8)(timeStamp >> 16u);
    buffer[indx+2] = (uint8)(timeStamp >> 8u);
    buffer[indx+3] = (uint8)timeStamp;
    indx += 4;
#endif
#if (DLT_HEADER_USE_EXTENDED_HEADER==STD_ON)
    buffer[0] |= DLT_UEH;
    buffer[indx] = extHeader->MSIN;
    buffer[indx+1] = extHeader->NOAR;
    buffer[indx+2] = extHeader->APID[0];
    buffer[indx+3] = extHeader->APID[1];
    buffer[indx+4] = extHeader->APID[2];
    buffer[indx+5] = extHeader->APID[3];
    buffer[indx+6] = extHeader->CTID[0];
    buffer[indx+7] = extHeader->CTID[1];
    buffer[indx+8] = extHeader->CTID[2];
    buffer[indx+9] = extHeader->CTID[3];
#endif

    /* Send using DltCom */
    Std_ReturnType res;
    PduInfoType pduInfo;
    pduInfo.SduDataPtr = buffer;
    pduInfo.SduLength = len;
    res = DltCom_Transmit(DLT_ARC_MAGIC_CONTROL_NUMBER, &pduInfo);
    if (res != E_OK) {
        result = DLT_E_IF_NOT_AVAILABLE;
    }

    return result;
}

void Dlt_ArcProcessIncomingMessage(const Dlt_StandardHeaderType *header, const uint8 *payload) {
    uint32 serviceId;
    uint16 inIndex = 0u;
    uint16 outIndex = 0u;
    Dlt_SessionIDType sessionId = 0u;
    uint8 Dlt_RespBuffer[60];
    Dlt_ExtendedHeaderType extHeader;
    static boolean busy_processing = FALSE;

    /* Make room for inserting fields in header */
    /* Add room for standard header */
    outIndex += 4u;

    /* SWS_Dlt_00188] *//* In the Standard Header the Application ID, Context ID, Session ID and Timestamp may be left blank */
    if ((header->HeaderType & DLT_WEID) != 0) {
        /* Ecu Id */
        inIndex += 4u;
    }
    if ((header->HeaderType & DLT_WSID) != 0) {
        /* Session Id */
        inIndex += 4u;
    }
    if ((header->HeaderType & DLT_WTMS) != 0) {
        /* Time stamp */
        inIndex += 4u;
    }
    /* Use Extended Header */
    if ((header->HeaderType & DLT_UEH) != 0) {
        inIndex += 10u;
    }

#if (DLT_HEADER_USE_ECU_ID==STD_ON)
    outIndex += 4u;
#endif
#if (DLT_HEADER_USE_SESSION_ID==STD_ON)
    outIndex += 4u;
#endif
#if (DLT_HEADER_USE_TIMESTAMP==STD_ON)
    outIndex += 4u;
#endif
#if (DLT_HEADER_USE_EXTENDED_HEADER==STD_ON)
    outIndex += 10u;
#endif

    extHeader.MSIN = ((uint8) DLT_TYPE_CONTROL << 1u) | (2u << 4u);
    extHeader.NOAR = 0u;
    extHeader.APID[0] = 0;
    extHeader.APID[1] = 0;
    extHeader.APID[2] = 0;
    extHeader.APID[3] = 0;
    extHeader.CTID[0] = 0;
    extHeader.CTID[1] = 0;
    extHeader.CTID[2] = 0;
    extHeader.CTID[3] = 0;

    /*lint --e{9033} inhibit lint warning to avoid false Misra violation */
    serviceId = ((uint32) (payload[inIndex]) << 24u) | ((uint32) (payload[inIndex + 1]) << 16u)
            | ((uint32) (payload[inIndex + 2]) << 8u) | (uint32) (payload[inIndex + 3]);

    if ((header->HeaderType & DLT_MSBF) == 0) {
        /* little endian payload */
        serviceId = (((serviceId & 0x000000FFu) << 24u) | ((serviceId & 0x0000FF00u) << 8u)
                | ((serviceId & 0x00FF0000u) >> 8u) | ((serviceId & 0xFF000000u) >> 24u));
    }

    /* @req SWS_Dlt_00191 */
    Dlt_RespBuffer[outIndex] = (uint8) (serviceId >> 24u);
    Dlt_RespBuffer[outIndex + 1] = (uint8) (serviceId >> 16u);
    Dlt_RespBuffer[outIndex + 2] = (uint8) (serviceId >> 8u);
    Dlt_RespBuffer[outIndex + 3] = (uint8) (serviceId);
    outIndex += 4u;
    inIndex += 4u;

    if (TRUE == busy_processing) {
        Dlt_RespBuffer[outIndex] = DLT_RESP_ERROR;
        outIndex += 1u;
        (void) Dlt_CreateControlMessageAndSend(Dlt_RespBuffer, outIndex, sessionId, &extHeader);
    }
    else {
        busy_processing = TRUE;

        switch (serviceId) {
            /* Get software version */
            case 0x00000013u:
            {
                uint16 lenver = 7u;
                Dlt_RespBuffer[outIndex] = DLT_RESP_OK;
                Dlt_RespBuffer[outIndex + 1] = 0u;
                Dlt_RespBuffer[outIndex + 2] = 0u;
                Dlt_RespBuffer[outIndex + 3] = (uint8) (lenver >> 8u);
                Dlt_RespBuffer[outIndex + 4] = (uint8) (lenver);
                memcpy(&Dlt_RespBuffer[outIndex + 5], "1234567", lenver);
                outIndex += 5u + lenver;

                if (DltState != DLT_CONNECTED) {
                    DltState = DLT_CONNECTED;
                }
            }
                break;
                /* Set_DefaulLogLevel */
            case 0x00000011u:
            {
                defaultLogLevel = payload[inIndex];
                Dlt_RespBuffer[outIndex] = DLT_RESP_OK;
                outIndex += 1u;
            }
                break;
                /* Get_DefaulLogLevel */
            case 0x00000004u:
            {
                Dlt_RespBuffer[outIndex + 1] = defaultLogLevel;
                Dlt_RespBuffer[outIndex] = DLT_RESP_OK;
                outIndex += 2u;
            }
                break;
                /* FilterMessages */
            case 0x0000000Au:
            {
#if DLT_IMPLEMENT_FILTER_MESSAGES == STD_ON
                filterMessages = payload[inIndex];
                Dlt_RespBuffer[outIndex] = DLT_RESP_OK;
#else
                Dlt_RespBuffer[outIndex] = DLT_RESP_NOT_SUPPORTED;
#endif
                outIndex += 1u;
            }
                break;
            default:
                Dlt_RespBuffer[outIndex] = DLT_RESP_NOT_SUPPORTED;
                outIndex += 1u;
                break;
        }

        (void) Dlt_CreateControlMessageAndSend(Dlt_RespBuffer, outIndex, sessionId, &extHeader);
        busy_processing = FALSE;
    }

}
