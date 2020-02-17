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

#warning "This default file may only be used as an example!"

/* @req SWS_RamTst_00080 */
#if !(((RAMTST_SW_MAJOR_VERSION == 2) && (RAMTST_SW_MINOR_VERSION == 0)) )
#error RamTst: Configuration file expected BSW module version to be 2.0.*
#endif

#if !(((RAMTST_AR_MAJOR_VERSION == 4) && (RAMTST_AR_MINOR_VERSION == 1)) )
#error RamTst: Configuration file expected AUTOSAR version to be 4.1.*
#endif

#ifndef RAMTST_CFG_H_
#define RAMTST_CFG_H_

#define RAMTST_DEV_ERROR_DETECT                             STD_ON
#define RAMTST_ALLOW_API                                    STD_ON
#define RAMTST_CHANGE_NUMBER_OF_TESTED_CELLS_API            STD_OFF
#define RAMTST_GET_ALG_PARAMS_API                           STD_OFF
#define RAMTST_GET_EXECUTION_STATUS_API                     STD_OFF
#define RAMTST_GET_NUMBER_OF_TESTED_CELLS_API               STD_OFF
#define RAMTST_GET_TEST_ALGORITHM_API                       STD_OFF
#define RAMTST_GET_TEST_RESULT_API                          STD_OFF
#define RAMTST_GET_TEST_RESULT_PER_BLOCK_API                STD_OFF
#define RAMTST_GET_VERSION_INFO_API                         STD_ON
#define RAMTST_RESUME_API                                   STD_OFF
#define RAMTST_RUN_FULL_TEST_API                            STD_ON
#define RAMTST_RUN_PARTIAL_TEST_API                         STD_OFF
#define RAMTST_SELECT_ALG_PARAMS_API                        STD_OFF
#define RAMTST_STOP_API                                     STD_ON
#define RAMTST_SUSPEND_API                                  STD_OFF

#define RAMTST_ABRAHAM_TEST_SELECTED                        STD_OFF
#define RAMTST_CHECKERBOARD_TEST_SELECTED                   STD_OFF
#define RAMTST_GALPAT_TEST_SELECTED                         STD_OFF
#define RAMTST_MARCH_TEST_SELECTED                          STD_ON
#define RAMTST_TRANSP_GALPAT_TEST_SELECTED                  STD_OFF
#define RAMTST_WALK_PATH_TEST_SELECTED                      STD_OFF


extern const RamTst_ConfigParamsType RamTst_ConfigParams;

#define RAMTST_NUM_BLOCKS 1

#endif /*RAMTST_CFG_H_*/
