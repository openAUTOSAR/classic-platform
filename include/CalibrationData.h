#ifndef CALIBRATIONDATA_H_
#define CALIBRATIONDATA_H_

/* Section data from linker script. */
extern char __CALIB_RAM_START;
extern char __CALIB_RAM_END;
extern char __CALIB_ROM_START;

#define ARC_DECLARE_CALIB(type, name) const type __attribute__((section (".calibration"))) name

#endif /* CALIBRATIONDATA_H_ */
