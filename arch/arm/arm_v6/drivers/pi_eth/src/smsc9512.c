/*
 * smsc9512.c
 *
 *  Created on: 2013-11-1
 *      Author: Zhang Shuzhou
 */

#include "smsc9512.h"
#include "usb_core_driver.h"
#include "Uart.h"
#include "bcm2835.h"

/**
 * @ingroup etherspecific
 *
 * Write to a register on the SMSC LAN9512 USB Ethernet Adapter.
 *
 * @param udev
 *      USB device for the adapter
 * @param index
 *      Index of the register to write
 * @param data
 *      Value to write to the register
 *
 * @return
 *      ::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *      code.
 */
usb_status_t
smsc9512_write_reg(struct usb_device *udev, uint32_t index, uint32_t data)
{
    return usb_control_msg(udev, NULL,
                           SMSC9512_VENDOR_REQUEST_WRITE_REGISTER,
                           USB_BMREQUESTTYPE_DIR_OUT |
                               USB_BMREQUESTTYPE_TYPE_VENDOR |
                               USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
                           0, index, &data, sizeof(uint32_t));
}

/**
 * @ingroup etherspecific
 *
 * Read from a register on the SMSC LAN9512 USB Ethernet Adapter.
 *
 * @param udev
 *      USB device for the adapter
 * @param index
 *      Index of the register to read
 * @param data
 *      Pointer into which to write the register's value
 *
 * @return
 *      ::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *      code.
 */
usb_status_t
smsc9512_read_reg(struct usb_device *udev, uint32_t index, uint32_t *data)
{
    return usb_control_msg(udev, NULL,
                           SMSC9512_VENDOR_REQUEST_READ_REGISTER,
                           USB_BMREQUESTTYPE_DIR_IN |
                               USB_BMREQUESTTYPE_TYPE_VENDOR |
                               USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
                           0, index, data, sizeof(uint32_t));
}

/**
 * @ingroup etherspecific
 *
 * Modify the value contained in a register on the SMSC LAN9512 USB Ethernet
 * Adapter.
 *
 * @param udev
 *      USB device for the adapter
 * @param index
 *      Index of the register to modify
 * @param mask
 *      Mask that contains 1 for the bits where the old value in the register
 *      will be kept rather than cleared (unless those bits also appear in @p
 *      set, in which case they will still be set).
 * @param set
 *      Mask of bits to set in the register.
 *
 * @return
 *      ::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *      code.
 */
usb_status_t
smsc9512_modify_reg(struct usb_device *udev, uint32_t index, uint32_t mask, uint32_t set)
{
    usb_status_t status;
    uint32_t val = 0;
//    char* s = NULL;
    status = smsc9512_read_reg(udev, index, &val);
//    bcm2835_delayMicroseconds(1000);
//    bcm2835_delayMicroseconds(1000);
    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: smsc9512 modify reg fail\r\n");
        return status;
    }
    val &= mask;
    val |= set;
    return smsc9512_write_reg(udev, index, val);
}

/**
 * @ingroup etherspecific
 *
 * Set bits in a register on the SMSC LAN9512 USB Ethernet Adapter.
 *
 * @param udev
 *      USB device for the adapter
 * @param index
 *      Index of the register to modify
 * @param set
 *      Bits to set in the register.  At positions where there is a 0, the old
 *      value in the register will be written.
 *
 * @return
 *      ::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *      code.
 */
usb_status_t
smsc9512_set_reg_bits(struct usb_device *udev, uint32_t index, uint32_t set)
{
    return smsc9512_modify_reg(udev, index, 0xffffffff, set);
}

/**
 * @ingroup etherspecific
 *
 * Change the MAC address of the SMSC LAN9512 USB Ethernet Adapter.
 *
 * @param udev
 *      USB device for the adapter
 * @param macaddr
 *      New MAC address to set (6 bytes long)
 *
 * @return
 *      ::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *      code.  On failure the existing MAC address may have been partially
 *      modified.
 */
usb_status_t
smsc9512_set_mac_address(struct usb_device *udev, const uint8_t *macaddr)
{
    usb_status_t status;
    uint32_t addrl, addrh;

    addrl = macaddr[0] | macaddr[1] << 8 | macaddr[2] << 16 | macaddr[3] << 24;
    addrh = macaddr[4] | macaddr[5] << 8;

    status = smsc9512_write_reg(udev, ADDRL, addrl);
//    bcm2835_delayMicroseconds(1000);
//    bcm2835_delayMicroseconds(1000);
    //pi_printf("infor: smsc9512 write reg\r\n");
    if (status != USB_STATUS_SUCCESS)
    {
        return status;
    }
    return smsc9512_write_reg(udev, ADDRH, addrh);
}

/**
 * @ingroup etherspecific
 *
 * Reads the MAC address of the SMSC LAN9512 USB Ethernet Adapter.
 *
 * @param udev
 *      USB device for the adapter
 * @param macaddr
 *      Pointer into which to write the MAC address (6 bytes long)
 *
 * @return
 *      ::USB_STATUS_SUCCESS on success; otherwise another ::usb_status_t error
 *      code.
 */
usb_status_t
smsc9512_get_mac_address(struct usb_device *udev, uint8_t *macaddr)
{
    usb_status_t status;
    uint32_t addrl, addrh;
    char* s = NULL;

    status = smsc9512_read_reg(udev, ADDRL, &addrl);
    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: smsc9512 get mac wrong\r\n");
        return status;
    }
    status = smsc9512_read_reg(udev, ADDRH, &addrh);
    if (status != USB_STATUS_SUCCESS)
    {
        return status;
    }
    macaddr[0] = (addrl >> 0)  & 0xff;
    macaddr[1] = (addrl >> 8)  & 0xff;
    macaddr[2] = (addrl >> 16) & 0xff;
    macaddr[3] = (addrl >> 24) & 0xff;
    macaddr[4] = (addrh >> 0)  & 0xff;
    macaddr[5] = (addrh >> 8)  & 0xff;
    return USB_STATUS_SUCCESS;
}



