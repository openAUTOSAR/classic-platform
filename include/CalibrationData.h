#ifndef CALIBRATIONDATA_H_
#define CALIBRATIONDATA_H_

#include "Calibration_Settings.h"

#ifdef CALIBRATION_ACTIVE
/* Section data from linker script. */
extern char __CALIB_RAM_START;
extern char __CALIB_RAM_END;
extern char __CALIB_ROM_START;
#define ARC_DECLARE_CALIB(type, name) type __attribute__((section (".calibration"))) name
#else
#define ARC_DECLARE_CALIB(type, name) const type name
#endif /* CALIBRATION_ACTIVE */



#endif /* CALIBRATIONDATA_H_ */
