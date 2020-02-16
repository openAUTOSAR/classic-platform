/*
 * usb_hcdi.h
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */

#ifndef USB_HCDI_H_
#define USB_HCDI_H_

#include <usb_util.h>

struct usb_xfer_request;

/**
 * @ingroup usbhcd
 *
 * Powers on, resets, and initializes the USB Host Controller, placing it in a
 * known state ready to be used.  This must be called after hcd_power_on() but
 * before calling any other Host Controller Driver Interface functions.
 *
 * @return
 *      ::USB_STATUS_SUCCESS if successful; another ::usb_status_t error code
 *      otherwise.
 */
usb_status_t hcd_start(void);

/**
 * @ingroup usbhcd
 *
 * Stops the USB Host Controller, reversing any effects of a successful call to
 * hcd_start().  Intended to be used only if USB initialization fails at a later
 * stage.
 */
void hcd_stop(void);

/**
 * @ingroup usbhcd
 *
 * Submits a transfer request to the USB Host Controller for completion.  As
 * described in the documentation for usb_submit_xfer_request(), this is
 * intended to be an asynchronous interface.  Also, the Host Controller Driver
 * is responsible for any intelligent scheduling of requests.
 *
 * @param req
 *      Pointer to the request to complete.  See the implementation of
 *      usb_submit_xfer_request() in the USB core driver for the sanity checks
 *      that are performed on the request before passing it to the Host
 *      Controller Driver.
 *
 * @return
 *      ::USB_STATUS_SUCCESS if the request was successfully queued (not
 *      necessarily completed or even started yet);
 *      ::USB_STATUS_INVALID_PARAMETER if the Host Controller Driver or Host
 *      Controller hardware itself does not support the request type or speed.
 */
usb_status_t hcd_submit_xfer_request(struct usb_xfer_request *req);

void dwc_schedule_xfer_requests(struct usb_xfer_request *req);

#endif /* USB_HCDI_H_ */
