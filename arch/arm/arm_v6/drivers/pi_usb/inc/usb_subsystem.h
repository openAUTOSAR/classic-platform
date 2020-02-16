/*
 * usb_subsystem.h
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */

#ifndef USB_SUBSYSTEM_H_
#define USB_SUBSYSTEM_H_

#include "usb_util.h"

syscall usbinit(void);
syscall usbinfo(void);

#endif /* USB_SUBSYSTEM_H_ */
