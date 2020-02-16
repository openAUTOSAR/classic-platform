/*
 * usb_core_driver.h
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */

#ifndef USB_CORE_DRIVER_H_
#define USB_CORE_DRIVER_H_

#include "usb_semaphore.h"
//#include <thread.h>
#include "usb_std_defs.h"
#include "usb_util.h"

extern struct usb_device *usb_root_hub;


struct usb_xfer_request;

/**
 * @ingroup usbcore
 *
 * Type of function called when an asynchronous (interrupt-driven) USB transfer
 * has completed or failed.  This will be called with IRQs disabled.  This
 * callback would generally be expected to handle the received data in some way
 * (if the transfer was input and was successful), then either free or re-submit
 * the struct usb_xfer_request.
 */
typedef void (*usb_xfer_completed_t)(struct usb_xfer_request *req);

/**
 * @ingroup usbcore
 *
 * Specification of an asynchronous (interrupt-driven) USB transfer request.
 * This can be a control, bulk, or interrupt request.  To get one of these
 * structures, either call usb_alloc_xfer_request(), or allocate memory manually
 * in another way and call usb_init_xfer_request().  Submit to to the USB core
 * using usb_submit_xfer_request() after filling in the
 * @ref usb_xfer_request::dev "dev",
 * @ref usb_xfer_request::endpoint_desc "endpoint_desc",
 * @ref usb_xfer_request::sendbuf "sendbuf" or
 * @ref usb_xfer_request::recvbuf "recvbuf",
 * @ref usb_xfer_request::size "size",
 * and
 * @ref usb_xfer_request::completion_cb_func "completion_cb_func" members,
 * and optionally the
 * @ref usb_xfer_request::private "private" and
 * @ref usb_xfer_request::setup_data "setup_data" members.
 */
struct usb_xfer_request
{

    /*********************
     * Input variables   *
     *********************/

    /** USB device to communicate with.  */
    struct usb_device *dev;

    /** Pointer to the descriptor of the endpoint to communicate with.  You can
     * get a pointer to one of these by searching the endpoints array of the
     * struct usb_device.  Use @c NULL to specify the default control endpoint.
     * */
    const struct usb_endpoint_descriptor *endpoint_desc;

    /** Data buffer: whether the sendbuf or recvbuf is used depends on whether
     * the endpoint_number specifies an IN or OUT endpoint.  */
    union {
        /** Buffer of data to send.  Ignored if size is 0.  */
        void *sendbuf;
        /** Buffer into which to write the received data.  Ignored if size is 0.
         * */
        void *recvbuf;
    };

    /** Size of sendbuf or recvbuf.  For IN endpoints, this will be the maximum
     * number of bytes of data to receive.  For OUT endpoints, this will be the
     * exact number of bytes of data to send.  */
    uint size;

    /** Setup data for the USB control request.  Must be filled in for control
     * transfers; ignored otherwise.  Note: consider using usb_control_msg() for
     * control transfers instead.  */
    struct usb_control_setup_data setup_data;

    /** Callback function that will be called when this USB transfer has been
     * successfully completed or has failed.  */
    usb_xfer_completed_t completion_cb_func;

    /** USB device driver private data to be saved for the completion callback.
     * Setting this is optional.  */
    void *private;

    /*********************
     * Output variables   *
     *********************/

    /** Status of the transfer: ::USB_STATUS_SUCCESS if successful, or another
     * ::usb_status_t error code if the transfer failed.  ::USB_STATUS_SUCCESS
     * is set if exactly the requested number of bytes were transferred with no
     * error, or if the transfer was device-to-host (IN) and completed with no
     * error but returned fewer bytes than requested.  */
    usb_status_t status;

    /** Actual size of the data transferred.  This should be tested after a
     * successful device-to-host (IN) transfer, since it is valid for such a
     * transfer to complete with less than the number of bytes requested if for
     * whatever reason the device was unable to provide the full size.  */
    uint actual_size;

    /*****************************************************************
     * Private variables (mainly for Host Controller Drivers; do not *
     * touch from device drivers).  TODO: a better design might      *
     * allow HCDs to customize the variables they can use, perhaps   *
     * by embedding the usb_xfer_request in another struct.          *
     *****************************************************************/
    void *cur_data_ptr;
    uint8_t complete_split : 1;
    uint8_t short_attempt  : 1;
    uint8_t need_sof       : 1;
    uint8_t control_phase  : 2;
    uint8_t next_data_pid  : 2;
    uint attempted_size;
    uint attempted_packets_remaining;
    uint attempted_bytes_remaining;
    uint csplit_retries;
    tid_typ deferer_thread_tid;
    semaphore deferer_thread_sema;
};

/**
 * @ingroup usbcore
 *
 * Information about a USB device driver.  This should be declared staticly by
 * the driver, then registered with the USB core driver using
 * usb_register_device_driver().  The callback functions specified in the
 * structure will then be called automatically by the USB core driver at
 * appropriate times.
 */
struct usb_device_driver
{
    /** Name of the driver, for informational messages only.  This can be @c
     * NULL.  */
    const char *name;

    /**
     * Function called to possibly bind this USB device driver to a USB device.
     * All USB device drivers are required to implement this function, so it
     * cannot be @c NULL.
     *
     * Implementations of this function must first check if the USB device is
     * supported by the driver and return ::USB_STATUS_DEVICE_UNSUPPORTED if it
     * is not.  To determine whether the USB device is supported,
     * implementations can examine the @ref usb_device_descriptor::idVendor
     * "vendor" and @ref usb_device_descriptor::idProduct "product" IDs in the
     * @ref usb_device::descriptor "device descriptor", or the available @ref
     * usb_device::interfaces "interfaces" and @ref usb_device::endpoints
     * "endpoints".
     *
     * If the USB device is indeed supported by the driver, this function must
     * do any necessary device-specific setup, such as configuring the device
     * with device-specific or class-specific control messages, and allocate any
     * needed resources, such as @ref ::usb_xfer_request
     * "USB transfer requests", needed to support the device.  If fully
     * successful, ::USB_STATUS_SUCCESS must be returned; otherwise, any
     * resources allocated for the device must be released and another
     * ::usb_status_t error code must be returned.  If necessary, device drivers
     * can store per-device, driver-specific data in the @ref
     * usb_device::driver_private "driver_private" member of the USB device
     * structure.
     *
     * This function may continue to be called by the USB core with @a different
     * USB devices even after the driver has been successfully bound to a USB
     * device.  If the driver does not support controlling multiple USB devices
     * simultaneously, this function must return ::USB_STATUS_DEVICE_UNSUPPORTED
     * in such cases.  This function will not, however, be called @a
     * concurrently with other calls to @ref usb_device_driver::bind_device
     * "bind_device" or @ref usb_device_driver::unbind_device
     * "unbind_device".
     *
     * The USB device will already be configured with its first listed
     * configuration when this function is called.  There is currently no way
     * for device drivers to choose a different configuration.
     */
    usb_status_t (*bind_device)(struct usb_device *dev);

    /**
     * Function called to unbind a USB device driver from a USB device, due to
     * the device being removed from the USB.  This can be called by the USB
     * core at any time after @ref usb_device_driver::bind_device
     * "bind_device" has returned successfully.  This function is responsible
     * for releasing any resources allocated in @ref
     * usb_device_driver::bind_device "bind_device" for the USB device.
     * Device drivers are not strictly required to implement this function and
     * can specify @c NULL here.  However, then any resources allocated in @ref
     * usb_device_driver::bind_device "bind_device" will be leaked if the
     * device is detached.
     *
     * When this function is called, it is guaranteed that there are no longer
     * any pending transfers to the corresponding USB device.  To implement
     * this, for a period of time before this function is called the USB core
     * ensures that any attempt to submit transfers to the device with either
     * usb_submit_xfer_request() or usb_control_msg() will fail with
     * ::USB_STATUS_DEVICE_DETACHED.  In the case of usb_submit_xfer_request(),
     * the completion callback will not be called.  In addition, the completion
     * callback will be called on all pending transfers and their status will be
     * set to ::USB_STATUS_DEVICE_DETACHED.  The result is that when this unbind
     * callback is called, all ::usb_xfer_request structures that may have been
     * allocated are owned by the driver itself.  Therefore, such structures can
     * immediately be freed, unless the driver itself holds additional
     * references to them.
     *
     * This function is always called with IRQs enabled, and it can be called
     * multiple times in succession to unbind @a different devices bound to the
     * same driver.  It will not, however, be called @a concurrently with other
     * calls to either @ref usb_device_driver::bind_device "bind_device" or
     * @ref usb_device_driver::unbind_device "unbind_device".
     */
    void (*unbind_device)(struct usb_device *dev);
};

/** Maximum number of interfaces per device.  */
#define USB_DEVICE_MAX_INTERFACES 8

/** Maximum number of endpoints per interface.  */
#define USB_DEVICE_MAX_ENDPOINTS  8

/**
 * @ingroup usbcore
 *
 * Information about a USB device.  This is provided by the USB core to device
 * drivers' @ref usb_device_driver::bind_device "bind_device" and @ref
 * usb_device_driver::unbind_device "unbind_device" callbacks.  Device drivers
 * (other than the hub driver) are not responsible for allocating or freeing
 * this structure.
 */
struct usb_device
{
    /** TRUE if this structure is in use; FALSE if it is free to allocate to a
     * new device.  Set by USB core.  */
    boolean inuse;

    /** Address of this device.  Set by USB core.  */
    uint address;

    /** Depth of this device (root hub is 0, next level hub is 1, ...).  Set by
     * USB core.  */
    uint depth;

    /** 1-based index of the USB port on the parent hub this device is plugged
     * into, or 0 if this is the root hub.  Set by USB core.  */
    uint port_number;

    /** Speed at which this device is attached to its hub.  Set by USB core.  */
    enum usb_speed speed;

    /** Hub this USB device is connected to, or @c NULL if this is the root hub.
     * Set by USB core.  */
    struct usb_device *parent;

    /** Current configuration index of this USB device.  Set by USB core.  */
    uint8_t configuration;

    /** Device descriptor of this device.  Set by USB core.  */
    struct usb_device_descriptor descriptor;

    /** Pointer to the full configuration descriptor for the first configuration
     * of this device.  Set by USB core.  */
    struct usb_configuration_descriptor *config_descriptor;

    /** Pointers to all interface descriptors of this device.  Set by USB core.  */
    struct usb_interface_descriptor *interfaces[USB_DEVICE_MAX_INTERFACES];

    /** Pointers to all endpoint descriptors of this device, arranged by
     * interface.  Set by USB core.  */
    struct usb_endpoint_descriptor *endpoints[
                            USB_DEVICE_MAX_INTERFACES][USB_DEVICE_MAX_ENDPOINTS];

    /** Null-terminated product string (ASCII encoded, and in English if
     * available) of this device.  Will be the empty string if the device has
     * no product string or it could not be read.  Set by USB core.  */
    char product[128];

    /** Null-terminated manufacturer string (ASCII encoded, and in English if
     * available) of this device.  Will be the empty string if the device has no
     * manufacturer string or it could not be read.  Set by USB core.  */
    char manufacturer[128];

    /** Private data for the driver of this USB device.  USB device drivers can
     * place a pointer to a private data structure here in their @ref
     * usb_device_driver::bind_device "bind_device" routines.  */
    void *driver_private;

    /** Driver bound to this device, if any.  Set by USB core.  */
    const struct usb_device_driver *driver;

    /** Number of USB transfers that have completed with failure status on this
     * device.  */
    ulong error_count;

    /** Last error to occur on this device.  */
    usb_status_t last_error;

    /** USB core use only.  */
    enum {
        USB_DEVICE_ATTACHED,
        USB_DEVICE_DETACHMENT_PENDING,
    } state;
    /** USB core use only.  */
    uint xfer_pending_count;
    /** USB core use only.  */
    tid_typ quiescent_state_waiter;
};


/* The following functions are intended to be used by USB device drivers.  */

usb_status_t
usb_register_device_driver(const struct usb_device_driver *drv);

struct usb_xfer_request *
usb_alloc_xfer_request(uint bufsize);

void
usb_init_xfer_request(struct usb_xfer_request *req);

void
usb_free_xfer_request(struct usb_xfer_request *req);

usb_status_t
usb_submit_xfer_request(struct usb_xfer_request *req);


usb_status_t
usb_control_msg(struct usb_device *dev,
                const struct usb_endpoint_descriptor *desc,
                uint8_t bRequest, uint8_t bmRequestType,
                uint16_t wValue, uint16_t wIndex, void *data, uint16_t wLength);

usb_status_t
usb_get_descriptor(struct usb_device *dev, uint8_t bRequest,
		   uint8_t bmRequestType, uint16_t wValue, uint16_t wIndex,
		   void *buf, uint16_t buflen);

/* The following functions are available only in non-embedded builds of the USB
 * code.  */
#if !USB_EMBEDDED
usb_status_t
usb_get_string_descriptor(struct usb_device *dev, uint8_t index, uint16_t lang_id,
                          struct usb_string_descriptor *buf, uint16_t buflen);

usb_status_t
usb_get_ascii_string(struct usb_device *dev, uint32_t iString,
                     char *strbuf, uint32_t strbufsize);

const char *
usb_device_description(const struct usb_device *dev);

const char *
usb_class_code_to_string(enum usb_class_code class_code);

const char *
usb_transfer_type_to_string(enum usb_transfer_type type);

const char *
usb_direction_to_string(enum usb_direction dir);

const char *
usb_speed_to_string(enum usb_speed speed);

#endif /* !USB_EMBEDDED */

/* The following functions are intended to be used only by the USB hub device
 * driver.  */

struct usb_device *
usb_alloc_device(struct usb_device *parent);

usb_status_t
usb_attach_device(struct usb_device *dev);

void
usb_free_device(struct usb_device *dev);

/* The following functions are intended to be used only by the USB host
 * controller driver.  */

/**
 * Returns TRUE if a given USB transfer request is a control request; FALSE
 * otherwise.
 */
static inline boolean
usb_is_control_request(const struct usb_xfer_request *req)
{
    return req->endpoint_desc == NULL ||
           (req->endpoint_desc->bmAttributes & 0x3) == USB_TRANSFER_TYPE_CONTROL;
}

/**
 * Returns TRUE if a given USB transfer request is an interrupt request; FALSE
 * otherwise.
 */
static inline boolean
usb_is_interrupt_request(const struct usb_xfer_request *req)
{
    return req->endpoint_desc != NULL &&
           (req->endpoint_desc->bmAttributes & 0x3) == USB_TRANSFER_TYPE_INTERRUPT;
}

void usb_complete_xfer(struct usb_xfer_request *req);

void usb_lock_bus(void);

void usb_unlock_bus(void);

/* The following functions are primarily intended to used by the USB host
 * controller and hub drivers; other device drivers probably will not need them.
 * */

/**
 * Returns TRUE if a USB device is the root hub; FALSE otherwise.
 */
static inline boolean is_root_hub(const struct usb_device *dev)
{
    return dev->parent == NULL;
}

/**
 * Returns TRUE if a USB device is a hub; FALSE otherwise.
 */
static inline boolean is_hub(const struct usb_device *dev)
{
    return dev->descriptor.bDeviceClass == USB_CLASS_CODE_HUB;
}

#endif /* USB_CORE_DRIVER_H_ */
