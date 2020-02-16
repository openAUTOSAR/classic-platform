/*
 * usb_debug.c
 *
 *  Created on: 14 jul 2014
 *      Author: sse
 */
#include <stdlib.h>
#include <stdio.h>
#include "usb_core_driver.h"
#include "usb_hub_driver.h"
#include "usb_std_defs.h"
#include "usb_subsystem.h"

/**
 * @ingroup usbcore
 *
 * Translates a ::usb_status_t into a string describing it.
 *
 * @param status
 *      A ::usb_status_t error code returned by one of the functions in Xinu's
 *      USB subsystem.
 *
 * @return
 *      A constant string describing the error, or "unknown error" if the error
 *      code is not recognized.
 */
const char *
usb_status_string(usb_status_t status)
{
    switch (status)
    {
        case USB_STATUS_SUCCESS:
            return "success";
        case USB_STATUS_OUT_OF_MEMORY:
            return "out of memory";
        case USB_STATUS_UNSUPPORTED_REQUEST:
            return "unsupported request";
        case USB_STATUS_DEVICE_UNSUPPORTED:
            return "device unsupported";
        case USB_STATUS_TIMEOUT:
            return "request timed out";
        case USB_STATUS_HARDWARE_ERROR:
            return "hardware error";
        case USB_STATUS_INVALID_DATA:
            return "invalid data";
        case USB_STATUS_INVALID_PARAMETER:
            return "invalid parameter";
        case USB_STATUS_NOT_PROCESSED:
            return "transfer not processed yet";
        case USB_STATUS_DEVICE_DETACHED:
            return "device was detached";
    }
    return "unknown error";
}


/**
 * @ingroup usbcore
 *
 * Translates a USB class code into a string describing it.
 *
 * @param class_code
 *      A USB class code constant.
 * @return
 *      A constant string describing the USB class, or "Unknown" if the class
 *      code was not recognized.
 */
const char *
usb_class_code_to_string(enum usb_class_code class_code)
{
    switch (class_code)
    {
        case USB_CLASS_CODE_INTERFACE_SPECIFIC:
            return "None (see interface descriptors)";
        case USB_CLASS_CODE_AUDIO:
            return "Audio";
        case USB_CLASS_CODE_COMMUNICATIONS_AND_CDC_CONTROL:
            return "Communications and CDC Control";
        case USB_CLASS_CODE_HID:
            return "HID (Human Interface Device)";
        case USB_CLASS_CODE_IMAGE:
            return "Image";
        case USB_CLASS_CODE_PRINTER:
            return "Printer";
        case USB_CLASS_CODE_MASS_STORAGE:
            return "Mass Storage";
        case USB_CLASS_CODE_HUB:
            return "Hub";
        case USB_CLASS_CODE_VIDEO:
            return "Video";
        case USB_CLASS_CODE_WIRELESS_CONTROLLER:
            return "Wireless Controller";
        case USB_CLASS_CODE_MISCELLANEOUS:
            return "Miscellaneous";
        case USB_CLASS_CODE_VENDOR_SPECIFIC:
            return "Vendor Specific";
    }
    return "Unknown";
}

/**
 * @ingroup usbcore
 *
 * Translates a USB speed constant into a string describing it.
 *
 * @param speed
 *      A USB speed constant.
 *
 * @return
 *      "high", "full", "low", or "unknown".
 */
const char *
usb_speed_to_string(enum usb_speed speed)
{
    switch (speed)
    {
        case USB_SPEED_HIGH:
            return "high";
        case USB_SPEED_FULL:
            return "full";
        case USB_SPEED_LOW:
            return "low";
    }
    return "unknown";
}

/**
 * @ingroup usbcore
 *
 * Translates a USB transfer type constant into a string describing it.
 *
 * @param type
 *      A USB transfer type constant.
 *
 * @return
 *      "Control", "Isochronous", "Bulk", "Interrupt", or "Unknown".
 */
const char *
usb_transfer_type_to_string(enum usb_transfer_type type)
{
    switch (type)
    {
        case USB_TRANSFER_TYPE_CONTROL:
            return "Control";
        case USB_TRANSFER_TYPE_ISOCHRONOUS:
            return "Isochronous";
        case USB_TRANSFER_TYPE_BULK:
            return "Bulk";
        case USB_TRANSFER_TYPE_INTERRUPT:
            return "Interrupt";
    }
    return "Unknown";
}



/**
 * @ingroup usbcore
 *
 * Translates a USB direction constant into a string describing it.
 *
 * @param dir
 *      A USB direction constant.
 *
 * @return
 *      "OUT", "IN", or "Unknown".
 */
const char *
usb_direction_to_string(enum usb_direction dir)
{
    switch (dir)
    {
        case USB_DIRECTION_OUT:
            return "OUT";
        case USB_DIRECTION_IN:
            return "IN";
    }
    return "Unknown";
}

/**
 * Translates a bcdUSB (binary-coded-decimal USB version) value into a
 * human-readable string.
 *
 * @param bcdUSB
 *      The bcdUSB value (e.g. from a USB device descriptor) to translate.
 *
 * @return
 *      A pointer to a staticly allocated string describing the USB version.  It
 *      will be changed on the next call to this function.
 */
static const char *
usb_bcd_version_to_string(uint16_t bcdUSB)
{
    static char string[3 + 1 + 2 + 1 + 2 + 1];
    char *p = string;
    p += sprintf(string, "%u.%u",
                 (bcdUSB >> 8) & 0xff,  /* At most 3 digits */
                 (bcdUSB >> 4) & 0xf);  /* At most 2 digits */
                                        /* (plus period)    */
    if (bcdUSB & 0xf)
    {
        sprintf(p, ".%u", bcdUSB & 31);  /* At most 2 digits (plus period)  */
    }
    return string;
}
/**
 * @ingroup usbcore
 *
 * Returns a fairly detailed, human-readable description of a USB device.
 *
 * @param dev
 *      USB device to get a description of.
 *
 * @return
 *      A string describing the device.  The returned string is statically
 *      allocated and will be changed on the next call to this function.
 */
const char *
usb_device_description(const struct usb_device *dev)
{
    uint i;
    enum usb_class_code class;
    static char device_description[512];
    char *p;

    p = device_description;

    /* Start with speed and USB version information.  */
    p += sprintf(p, "%s-speed USB %s",
                 usb_speed_to_string(dev->speed),
                 usb_bcd_version_to_string(dev->descriptor.bcdUSB));

    /* Try to find a class description of the device, taking into account that
     * the class may be stored either in the device descriptor or an interface
     * descriptor.  */
    class = dev->descriptor.bDeviceClass;
    if (class == 0)
    {
        for (i = 0; i < dev->config_descriptor->bNumInterfaces; i++)
        {
            if (dev->interfaces[i]->bInterfaceClass != 0)
            {
                class = dev->interfaces[i]->bInterfaceClass;
            }
        }
    }

    /* Add the class description if we found one and it was not something
     * meaningless like the vendor specific class.  */
    if (class != 0 &&
        class != USB_CLASS_CODE_VENDOR_SPECIFIC &&
        class != USB_CLASS_CODE_MISCELLANEOUS)
    {
        p += sprintf(p, " %s class",
                     usb_class_code_to_string(class));
    }

    /* This is indeed a device.  */
    p += sprintf(p, " device");

    /* Add the product name, if the device provides it.  */
    if (dev->product[0] != '\0')
    {
        p += sprintf(p, " (%s)", dev->product);
    }

    /* Add vendor and product IDs.  */
    p += sprintf(p, " (idVendor=0x%04x, idProduct=0x%04x)",
                 dev->descriptor.idVendor,
                 dev->descriptor.idProduct);

    /* Return the resulting string.  */
    return device_description;
}
