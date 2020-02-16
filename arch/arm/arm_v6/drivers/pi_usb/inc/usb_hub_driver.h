/*
 * usb_hub_driver.h
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */

#ifndef USB_HUB_DRIVER_H_
#define USB_HUB_DRIVER_H_

#include "usb_util.h"
#include "usb_core_driver.h"

void usb_hub_for_device_in_tree(struct usb_device *dev, usb_status_t (*callback)(struct usb_device *));

extern const struct usb_device_driver usb_hub_driver;

#endif /* USB_HUB_DRIVER_H_ */
