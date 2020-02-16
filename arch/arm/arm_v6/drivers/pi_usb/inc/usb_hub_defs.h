/*
 * usb_hub_defs.h
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */

#ifndef USB_HUB_DEFS_H_
#define USB_HUB_DEFS_H_

#include "usb_util.h"

#define USB_HUB_CHARACTERISTIC_IS_COMPOUND_DEVICE (1 << 2)

/** Standard format of USB hub descriptors.  See Table 11-13 in Section 11.23 of
 * the USB 2.0 specification.  */
struct usb_hub_descriptor {
    uint8_t  bDescLength;
    uint8_t  bDescriptorType;
    uint8_t  bNbrPorts;
    uint16_t wHubCharacteristics;
    uint8_t  bPwrOn2PwrGood;
    uint8_t  bHubContrCurrent;
    uint8_t  varData[]; /* Variable length field; 64 should be the maximum possible
                      length (255 ports => 2 x 32 bytes of data) */
} __packed;

/** USB port status, defined in Section 11.24.2.7 of the USB 2.0 specification.
 * */
struct usb_port_status {
    union {
        uint16_t wPortStatus;
        struct {
            uint16_t connected : 1;
            uint16_t enabled : 1;
            uint16_t suspended : 1;
            uint16_t overcurrent : 1;
            uint16_t reset : 1;
            uint16_t wPortStatus_reserved1 : 3;
            uint16_t powered : 1;
            uint16_t low_speed_attached : 1;
            uint16_t high_speed_attached : 1;
            uint16_t test_mode : 1;
            uint16_t indicator_control : 1;
            uint16_t wPortStatus_reserved2 : 3;
        };
    };
    union {
        uint16_t wPortChange;
        struct {
            uint16_t connected_changed : 1;
            uint16_t enabled_changed : 1;
            uint16_t suspended_changed : 1;
            uint16_t overcurrent_changed : 1;
            uint16_t reset_changed : 1;
            uint16_t wPortChange_reserved : 11;
        };
    };
} __packed;

/** USB port features.  see Table 11-17 in Section 11.24.2 of the USB 2.0
 * specification.  */
enum usb_port_feature {
    USB_PORT_CONNECTION     = 0,
    USB_PORT_ENABLE         = 1,
    USB_PORT_SUSPEND        = 2,
    USB_PORT_OVER_CURRENT   = 3,
    USB_PORT_RESET          = 4,
    USB_PORT_POWER          = 8,
    USB_PORT_LOW_SPEED      = 9,
    USB_C_PORT_CONNECTION   = 16,
    USB_C_PORT_ENABLE       = 17,
    USB_C_PORT_SUSPEND      = 18,
    USB_C_PORT_OVER_CURRENT = 19,
    USB_C_PORT_RESET        = 20,
    USB_PORT_TEST           = 21,
    USB_PORT_INDICATOR      = 22,
};

/** USB hub status, defined in Section 11.24.2.6 of the USB 2.0 specification.
 * */
struct usb_hub_status {
    union {
        uint16_t wHubStatus;
        struct {
            uint16_t local_power : 1;
            uint16_t overcurrent : 1;
            uint16_t wHubStatus_reserved : 14;
        };
    };
    union {
        uint16_t wHubChange;
        struct {
            uint16_t local_power_changed : 1;
            uint16_t overcurrent_changed : 1;
            uint16_t wHubChange_reserved : 14;
        };
    };
} __packed;

/** Standard USB hub requests.  See Table 11-16 in Section 11.24.2 of the USB
 * 2.0 specification.  */
enum usb_hub_request {
    USB_HUB_REQUEST_GET_STATUS       = 0,
    USB_HUB_REQUEST_CLEAR_FEATURE    = 1,
    USB_HUB_REQUEST_SET_FEATURE      = 3,
    USB_HUB_REQUEST_GET_DESCRIPTOR   = 6,
    USB_HUB_REQUEST_SET_DESCRIPTOR   = 7,
    USB_HUB_REQUEST_CLEAR_TT_BUFFER  = 8,
    USB_HUB_REQUEST_RESET_TT         = 9,
    USB_HUB_REQUEST_GET_TT_STATE     = 10,
    USB_HUB_REQUEST_STOP_TT          = 11,
};



#endif /* USB_HUB_DEFS_H_ */
