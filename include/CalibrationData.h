#ifndef CALIBRATIONDATA_H_
#define CALIBRATIONDATA_H_

#include "Calibration_Settings.h"

#ifdef CALIBRATION_ENABLED
/* Section data from linker script. */
extern char __CALIB_RAM_START;
extern char __CALIB_RAM_END;
extern char __CALIB_ROM_START;
#endif /* CALIBRATION_ENABLED */


#pragma section data_type ".calib_shared"    ".calib_shared"
#pragma section data_type ".calibration"     ".calibration"
#pragma section data_type ".calib_component" ".calib_component"


#define ARC_DECLARE_CALIB(type, name) type __attribute__((section (".calibration"))) name
#define ARC_DECLARE_CALIB_SHARED(type, name) type __attribute__((section (".calib_shared"))) name
#define ARC_DECLARE_CALIB_EXTERN(type, name) extern type name
#define ARC_DECLARE_CALIB_COMPONENT(type, name) type __attribute__((section (".calib_component"))) name
#endif /* CALIBRATIONDATA_H_ */
