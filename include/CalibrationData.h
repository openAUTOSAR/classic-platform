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


#ifndef CALIBRATIONDATA_H_
#define CALIBRATIONDATA_H_

#include "Calibration_Settings.h"

#ifdef CALIBRATION_ENABLED
/* Section data from linker script. */
extern char __CALIB_RAM_START;
extern char __CALIB_RAM_END;
extern char __CALIB_ROM_START;


#ifdef __CWCC__
#pragma section RW ".calibration_data"     ".calibration"
#define ARC_DECLARE_CALIB(type, name)   __declspec(section ".calibration_data") type name
#else
#define ARC_DECLARE_CALIB(type, name) type __attribute__((section (".calibration"))) name
#define ARC_DECLARE_CALIB_SHARED(type, name) type __attribute__((section (".calib_shared"))) name
#define ARC_DECLARE_CALIB_EXTERN(type, name) extern type name
#define ARC_DECLARE_CALIB_COMPONENT(type, name) type __attribute__((section (".calib_component"))) name
#endif/* __CWCC__ */

#else
#define ARC_DECLARE_CALIB(type, name) type name
#define ARC_DECLARE_CALIB_SHARED(type, name) type name
#define ARC_DECLARE_CALIB_EXTERN(type, name) extern type name
#define ARC_DECLARE_CALIB_COMPONENT(type, name) type name
#endif /* CALIBRATION_ENABLED */

#endif /* CALIBRATIONDATA_H_ */
