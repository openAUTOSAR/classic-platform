/*
 * smsc9512.h
 *
 *  Created on: 2013-11-1
 *      Author: Zhang Shuzhou
 */

#include "usb_util.h"

/* The SMSC LAN9512 has an integrated USB hub and is technically a compound
 * device.  Here we instead use the vendor ID and product ID of the
 * vendor-specific class device attached to the hub that is the device we
 * actually need to communicate with.  */

/** idVendor in the USB device descriptor for this device */
#define SMSC9512_VENDOR_ID  0x0424

/** idProduct in the USB device descriptor for this device */
#define SMSC9512_PRODUCT_ID 0xEC00

/** TODO */
#define SMSC9512_TX_OVERHEAD 8

/** TODO */
#define SMSC9512_RX_OVERHEAD 4

/** TODO */
#define SMSC9512_HS_USB_PKT_SIZE 512

/** TODO */
#define SMSC9512_DEFAULT_HS_BURST_CAP_SIZE (16 * 1024 + 5 * SMSC9512_HS_USB_PKT_SIZE)

/** TODO */
#define SMSC9512_DEFAULT_BULK_IN_DELAY 0x2000

#define SMSC9512_MAX_TX_REQUESTS 1
/* TODO */
//#define SMSC9512_MAX_RX_REQUESTS (DIV_ROUND_UP(60 * 1518, SMSC9512_DEFAULT_HS_BURST_CAP_SIZE))
#define SMSC9512_MAX_RX_REQUESTS 1

usb_status_t smsc9512_write_reg(struct usb_device *udev, uint32_t index, uint32_t data);

usb_status_t smsc9512_read_reg(struct usb_device *udev, uint32_t index, uint32_t *data);

usb_status_t smsc9512_modify_reg(struct usb_device *udev, uint32_t index,
                                 uint32_t mask, uint32_t set);
usb_status_t smsc9512_set_reg_bits(struct usb_device *udev, uint32_t index, uint32_t set);

usb_status_t smsc9512_wait_device_attached(ushort minor);

usb_status_t smsc9512_set_mac_address(struct usb_device *udev, const uint8_t *macaddr);
usb_status_t smsc9512_get_mac_address(struct usb_device *udev, uint8_t *macaddr);

struct usb_xfer_request;

void smsc9512_rx_complete(struct usb_xfer_request *req);
void smsc9512_tx_complete(struct usb_xfer_request *req);


static inline void
__smsc9512_dump_reg(struct usb_device *udev, uint32_t index, const char *name)
{
    uint32_t val = 0;
    smsc9512_read_reg(udev, index, &val);
    //kprintf("SMSC9512: %s = 0x%08x\n", name, val);
}

#define smsc9512_dump_reg(udev, index) __smsc9512_dump_reg(udev, index, #index)

/****************************************************************************/


/*
 * Transmitted Ethernet frames (as written to the SMSC LAN9512's Bulk OUT
 * endpoint) must be prefixed with an 8-byte header containing the "Tx command
 * word A" followed by the "Tx command word B".  It apparently is possible to
 * use these command words to set up the transmission of multiple Ethernet
 * frames in a single USB Bulk transfer, although we do not use this
 * functionality in this driver.
 */

/* Tx command word A */
#define TX_CMD_A_DATA_OFFSET           0x001F0000
#define TX_CMD_A_FIRST_SEG             0x00002000
#define TX_CMD_A_LAST_SEG              0x00001000
#define TX_CMD_A_BUF_SIZE              0x000007FF

/* Tx command word B */
#define TX_CMD_B_CSUM_ENABLE           0x00004000
#define TX_CMD_B_ADD_CRC_DISABLE       0x00002000
#define TX_CMD_B_DISABLE_PADDING       0x00001000
#define TX_CMD_B_PKT_BYTE_LENGTH       0x000007FF

/****************************************************************************/

/*
 * Received Ethernet frames (as read from the SMSC LAN9512's Bulk IN endpoint)
 * are prefixed with a 4-byte Rx Status word containing the flags below.  A
 * single USB Bulk IN transfer may contain multiple Ethernet frames (provided
 * that HW_CFG_MEF is set in HW_CFG), each of which is prepended by a Rx Status
 * word and padded to a 4-byte boundary.
 */

#define RX_STS_FF                      0x40000000    /* Filter Fail */
#define RX_STS_FL                      0x3FFF0000    /* Frame Length */
#define RX_STS_ES                      0x00008000    /* Error Summary */
#define RX_STS_BF                      0x00002000    /* Broadcast Frame */
#define RX_STS_LE                      0x00001000    /* Length Error */
#define RX_STS_RF                      0x00000800    /* Runt Frame */
#define RX_STS_MF                      0x00000400    /* Multicast Frame */
#define RX_STS_TL                      0x00000080    /* Frame too long */
#define RX_STS_CS                      0x00000040    /* Collision Seen */
#define RX_STS_FT                      0x00000020    /* Frame Type */
#define RX_STS_RW                      0x00000010    /* Receive Watchdog */
#define RX_STS_ME                      0x00000008    /* Mii Error */
#define RX_STS_DB                      0x00000004    /* Dribbling */
#define RX_STS_CRC                     0x00000002    /* CRC Error */

/****************************************************************************/

/**
 * Offset of Device ID / Revision Register.  TODO
 */
#define ID_REV                         0x00
#define ID_REV_CHIP_ID_MASK            0xFFFF0000
#define ID_REV_CHIP_REV_MASK           0x0000FFFF
#define ID_REV_CHIP_ID_9500            0x9500
#define ID_REV_CHIP_ID_9500A           0x9E00
#define ID_REV_CHIP_ID_9512            0xEC00
#define ID_REV_CHIP_ID_9530            0x9530
#define ID_REV_CHIP_ID_89530           0x9E08
#define ID_REV_CHIP_ID_9730            0x9730

/****************************************************************************/

/**
 * Offset of Interrupt Status Register.  TODO
 */
#define INT_STS                        0x08
#define INT_STS_TX_STOP                0x00020000
#define INT_STS_RX_STOP                0x00010000
#define INT_STS_PHY_INT                0x00008000
#define INT_STS_TXE                    0x00004000
#define INT_STS_TDFU                   0x00002000
#define INT_STS_TDFO                   0x00001000
#define INT_STS_RXDF                   0x00000800
#define INT_STS_GPIOS                  0x000007FF
#define INT_STS_CLEAR_ALL              0xFFFFFFFF

/****************************************************************************/

/** Offset of Receive Configuration Register.  */
#define RX_CFG                         0x0C

/** Most likely, software can write 1 to this flag discard all the Rx packets
 * currently buffered by the device.  */
#define RX_FIFO_FLUSH                  0x00000001

/****************************************************************************/

/** Offset of Transmit Configuration Register.  */
#define TX_CFG                         0x10

/** Transmit On flag.  Software can write 1 here to enable transmit
 * functionality (at the PHY layer?).  Writing 0 is ignored.  Reads as current
 * on (1) / off (0) state.  However, to actually allow packets to be
 * transmitted, software also must set the ::MAC_CR_TXEN flag in the ::MAC_CR
 * register.  */
#define TX_CFG_ON                      0x00000004

/** Transmit Stop flag.  Software can write 1 here to turn transmit
 * functionality off.  Writing 0 is ignored.  Always reads as 0.  */
#define TX_CFG_STOP                    0x00000002

/** Most likely, software can write 1 to this flag to discard all the Tx packets
 * currently buffered by the device.  */
#define TX_CFG_FIFO_FLUSH              0x00000001

/****************************************************************************/

/** Offset of Hardware Configuration Register.  As implied by the name, this
 * contains a number of flags that software can modify to configure the Ethernet
 * Adapter.   After reset, this register contains all 0's.  */
#define HW_CFG                         0x14

/** TODO: this is set by SMSC's Linux driver.  I don't know what BIR stands for,
 * but the BI might stand for Bulk In.  The observed behavior is that if you
 * don't set this flag, latency for Rx, Tx, or both appears to increase, and
 * Bulk IN transfers can complete immediately with 0 length even when no data
 * has been received.  */
#define HW_CFG_BIR                     0x00001000

/** TODO */
#define HW_CFG_LEDB                    0x00000800

/** Rx packet offset:  Software can modify this 2-bit field to cause Rx packets
 * to be offset by the specified number of bytes.  This is apparently intended
 * to allow software to align the IP header on a 4 byte boundary.  */
#define HW_CFG_RXDOFF                  0x00000600

/** TODO */
#define HW_CFG_DRP                     0x00000040

/** Multiple Ethernet Frames:  Software can set this flag in HW_CFG to allow
 * multiple Ethernet frames to be received in a single USB Bulk In transfer.
 * The default value after reset is 0, meaning that the hardware will by default
 * provide each received Ethernet frame in a separate USB Bulk In transfer.  */
#define HW_CFG_MEF                     0x00000020

/** "Lite" Reset flag.  Software can write 1 to this flag in HW_CFG to start a
 * "lite" reset on the device, whatever that means.  The hardware will
 * automatically clear this flag when the device has finished resetting, which
 * should take no longer than 1 second.  */
#define HW_CFG_LRST                    0x00000008

/** TODO */
#define HW_CFG_PSEL                    0x00000004

/** TODO: this is set by SMSC's Linux driver at the same time as HW_CFG_MEF.  I
 * have no idea what it stands for or what it does.  */
#define HW_CFG_BCE                     0x00000002

/** TODO */
#define HW_CFG_SRST                    0x00000001

/****************************************************************************/

/** TODO */
#define RX_FIFO_INF                    0x18

/****************************************************************************/

/** Offset of Power Management Control Register.  TODO */
#define PM_CTRL                        0x20
#define PM_CTL_RES_CLR_WKP_STS         0x00000200
#define PM_CTL_DEV_RDY                 0x00000080
#define PM_CTL_SUS_MODE                0x00000060
#define PM_CTL_SUS_MODE_0              0x00000000
#define PM_CTL_SUS_MODE_1              0x00000020
#define PM_CTL_SUS_MODE_2              0x00000040
#define PM_CTL_SUS_MODE_3              0x00000060

/** PHY Reset flag:  Software can write 1 here to start a PHY reset on the
 * device.  The hardware will automatically clear this flag when the PHY has
 * finished resetting, which should take no longer than 1 second.  */
#define PM_CTL_PHY_RST                 0x00000010
#define PM_CTL_WOL_EN                  0x00000008
#define PM_CTL_ED_EN                   0x00000004
#define PM_CTL_WUPS                    0x00000003
#define PM_CTL_WUPS_NO                 0x00000000
#define PM_CTL_WUPS_ED                 0x00000001
#define PM_CTL_WUPS_WOL                0x00000002
#define PM_CTL_WUPS_MULTI              0x00000003

/****************************************************************************/

/** Offset of LED General Purpose I/O Configuration Register.  */
#define LED_GPIO_CFG                   0x24
#define LED_GPIO_CFG_SPD_LED           0x01000000
#define LED_GPIO_CFG_LNK_LED           0x00100000
#define LED_GPIO_CFG_FDX_LED           0x00010000

/****************************************************************************/

/** Offset of General Purpose I/O Configuration Register.  */
#define GPIO_CFG                       0x28

/****************************************************************************/

/** Offset of (Advanced?) Flow Control Configuration Register.
 * After reset, this register is 0.  */
#define AFC_CFG                        0x2C

/**
 * Value written to AFC_CFG by the Linux driver, with the following explanation:
 *
 *     Hi watermark = 15.5Kb (~10 mtu pkts)
 *     low watermark = 3k (~2 mtu pkts)
 *     backpressure duration = ~ 350us
 *     Apply FC on any frame.
 */
#define AFC_CFG_DEFAULT                0x00F830A1

/****************************************************************************/

/** TODO */
#define E2P_CMD                        0x30
#define E2P_CMD_BUSY                   0x80000000
#define E2P_CMD_MASK                   0x70000000
#define E2P_CMD_READ                   0x00000000
#define E2P_CMD_EWDS                   0x10000000
#define E2P_CMD_EWEN                   0x20000000
#define E2P_CMD_WRITE                  0x30000000
#define E2P_CMD_WRAL                   0x40000000
#define E2P_CMD_ERASE                  0x50000000
#define E2P_CMD_ERAL                   0x60000000
#define E2P_CMD_RELOAD                 0x70000000
#define E2P_CMD_TIMEOUT                0x00000400
#define E2P_CMD_LOADED                 0x00000200
#define E2P_CMD_ADDR                   0x000001FF

#define MAX_EEPROM_SIZE                512

/****************************************************************************/

/** TODO */
#define E2P_DATA                       0x34
#define E2P_DATA_MASK                  0x000000FF

/****************************************************************************/

/** Offset of Burst Cap Register.
 *
 * When multiple Ethernet frames per USB bulk transfer are enabled, this
 * register must be set by software to specify the maximum number of USB (not
 * networking!) packets the hardware will provide in a single Bulk In transfer.
 *
 * This register is ignored if HW_CFG_MEF is not set.  Otherwise, this must be
 * set to at least 5, which represents a maximum of 5 * 512 = 2560 bytes of data
 * per transfer from the high speed Bulk In endpoint.  */
#define BURST_CAP                      0x38

/****************************************************************************/

/** TODO */
#define GPIO_WAKE                      0x64

/****************************************************************************/

/** TODO */
#define INT_EP_CTL                     0x68
#define INT_EP_CTL_INTEP               0x80000000
#define INT_EP_CTL_MACRTO              0x00080000
#define INT_EP_CTL_TX_STOP             0x00020000
#define INT_EP_CTL_RX_STOP             0x00010000
#define INT_EP_CTL_PHY_INT             0x00008000
#define INT_EP_CTL_TXE                 0x00004000
#define INT_EP_CTL_TDFU                0x00002000
#define INT_EP_CTL_TDFO                0x00001000
#define INT_EP_CTL_RXDF                0x00000800
#define INT_EP_CTL_GPIOS               0x000007FF

/****************************************************************************/

/**
 * Offset of Bulk In Delay Register.
 *
 * The low 16 bits of this register contain a value that indicates the maximum
 * amount of time the hardware waits for additional packets before responding to
 * a Bulk In request once a packet has been received.  From experiment, the time
 * is specified on a linear scale where each unit is approximately 17
 * nanoseconds.  The default value in this register after reset is 0x800 which
 * indicates a delay of about 34.8 microseconds, assuming that the scale is
 * 0-based.  SMSC's Linux driver changes this to 0x2000, or a delay of about 139
 * microseconds.
 *
 * The high 16 bits of this register are ignored, as far as I can tell.
 *
 * The value in this register no effect if HW_CFG_MEF is not set in the
 * HW_CFG register.
 */
#define BULK_IN_DLY                    0x6C

/****************************************************************************/

/** Offset of Media Access Control Control Register  */
#define MAC_CR                         0x100

/** ??? */
#define MAC_CR_RXALL                   0x80000000

/** Half duplex mode. */
#define MAC_CR_RCVOWN                  0x00800000

/** Loopback mode. */
#define MAC_CR_LOOPBK                  0x00200000

/** Full duplex mode. */
#define MAC_CR_FDPX                    0x00100000

/** Multicast pass: receive all multicast packets.  */
#define MAC_CR_MCPAS                   0x00080000

/** Promiscuous mode. */
#define MAC_CR_PRMS                    0x00040000

/** Inverse filtering. */
#define MAC_CR_INVFILT                 0x00020000

/** Pass on bad frames. */
#define MAC_CR_PASSBAD                 0x00010000

/** ??? */
#define MAC_CR_HFILT                   0x00008000

/** Filter received multicast packets by the set of addresses specified by HASHH
 * and HASHL.  */
#define MAC_CR_HPFILT                  0x00002000

/** ??? */
#define MAC_CR_LCOLL                   0x00001000

/** Receive broadcast packets?  */
#define MAC_CR_BCAST                   0x00000800

/** ??? */
#define MAC_CR_DISRTY                  0x00000400

/** ??? */
#define MAC_CR_PADSTR                  0x00000100

/** ??? */
#define MAC_CR_BOLMT_MASK              0x000000C0

/** ??? */
#define MAC_CR_DFCHK                   0x00000020

/** Transmit enabled at the MAC layer.  Software can write 1 to enable or write
 * 0 to disable.  However, to actually allow packets to be transmitted, software
 * also must set the ::TX_CFG_ON flag in the ::TX_CFG register.  */
#define MAC_CR_TXEN                    0x00000008

/** Receive enabled.  Software can write 1 to enable or write 0 to disable.  */
#define MAC_CR_RXEN                    0x00000004

/****************************************************************************/

/** Offset of Address High Register.  This contains the high 2 bytes of the MAC
 * address used by the device, stored in little endian order.
 *
 * As they are not part of the MAC address, the hardware ignores the values
 * written to the upper 2 bytes of this register and always reads them as 0.
 *
 * Software can change the MAC address used by the device by writing to the
 * ::ADDRH and ::ADDRL registers, and it can retrieve the current MAC address by
 * reading them.  On reset, the device will read its MAC address from the EEPROM
 * if one is attached; otherwise it will set its MAC address to 0xFFFFFFFFFFFF.
 * */
#define ADDRH                          0x104

/** Offset of Address Low Register.  This contains the low 4 bytes of the MAC
 * address used by the device, stored in little endian order.  See ::ADDRH.  */
#define ADDRL                          0x108

/****************************************************************************/

/** Offset of Hash High register, used together with HASHL to filter specific
 * multicast packets.  TODO */
#define HASHH                          0x10C

/** Offset of Hash Low register, used together with HASHH to filter specific
 * multicast packets.  TODO */
#define HASHL                          0x110

/****************************************************************************/

/** TODO */
#define MII_ADDR                       0x114
#define MII_WRITE                      0x02
#define MII_BUSY                       0x01
#define MII_READ                       0x00 /* ~of MII Write bit */

/** TODO */
#define MII_DATA                       0x118

/****************************************************************************/

/** TODO.  After reset, this is 0.  */
#define FLOW                           0x11C
#define FLOW_FCPT                      0xFFFF0000
#define FLOW_FCPASS                    0x00000004
#define FLOW_FCEN                      0x00000002
#define FLOW_FCBSY                     0x00000001

/****************************************************************************/

/** TODO */
#define VLAN1                           0x120

/** TODO */
#define VLAN2                           0x124

/****************************************************************************/

/** TODO */
#define WUFF                            0x128
#define LAN9500_WUFF_NUM                4
#define LAN9500A_WUFF_NUM               8

/****************************************************************************/

/** TODO */
#define WUCSR                          0x12C
#define WUCSR_WFF_PTR_RST              0x80000000
#define WUCSR_GUE                      0x00000200
#define WUCSR_WUFR                     0x00000040
#define WUCSR_MPR                      0x00000020
#define WUCSR_WAKE_EN                  0x00000004
#define WUCSR_MPEN                     0x00000002

/****************************************************************************/

/** Offset of Checksum Offload Engine/Enable Control Register.  This register
 * can be used to enable or disable Tx and Rx checksum offload.  These refer
 * specifically to the TCP/UDP checksums and not to the CRC32 calculated for
 * an Ethernet frame itself, which is controlled separately and is done by
 * default, unlike this which must be explicitly enabled.  */
#define COE_CR                         0x130

/** Transmit checksum offload enabled.  Software can write 1 here to enable or
 * write 0 here to disable.  After reset, this is disabled (0).  */
#define Tx_COE_EN                      0x00010000

/** TODO.  After reset, this is 0.  */
#define Rx_COE_MODE                    0x00000002

/** Receive checksum offload enabled.  Software can write 1 here to enable or
 * write 0 here to disable.  After reset, this is disabled (0).  */
#define Rx_COE_EN                      0x00000001

/****************************************************************************/

/* Vendor-specific PHY Definitions */

/* EDPD NLP / crossover time configuration (LAN9500A only) */
#define PHY_EDPD_CONFIG                 16
#define PHY_EDPD_CONFIG_TX_NLP_EN      ((u16)0x8000)
#define PHY_EDPD_CONFIG_TX_NLP_1000    ((u16)0x0000)
#define PHY_EDPD_CONFIG_TX_NLP_768     ((u16)0x2000)
#define PHY_EDPD_CONFIG_TX_NLP_512     ((u16)0x4000)
#define PHY_EDPD_CONFIG_TX_NLP_256     ((u16)0x6000)
#define PHY_EDPD_CONFIG_RX_1_NLP       ((u16)0x1000)
#define PHY_EDPD_CONFIG_RX_NLP_64      ((u16)0x0000)
#define PHY_EDPD_CONFIG_RX_NLP_256     ((u16)0x0400)
#define PHY_EDPD_CONFIG_RX_NLP_512     ((u16)0x0800)
#define PHY_EDPD_CONFIG_RX_NLP_1000    ((u16)0x0C00)
#define PHY_EDPD_CONFIG_EXT_CROSSOVER  ((u16)0x0001)
#define PHY_EDPD_CONFIG_DEFAULT         (PHY_EDPD_CONFIG_TX_NLP_EN | \
                                         PHY_EDPD_CONFIG_TX_NLP_768 | \
                                         PHY_EDPD_CONFIG_RX_1_NLP)

/* Mode Control/Status Register */
#define PHY_MODE_CTRL_STS               17
#define MODE_CTRL_STS_EDPWRDOWN        ((u16)0x2000)
#define MODE_CTRL_STS_ENERGYON         ((u16)0x0002)

#define SPECIAL_CTRL_STS                27
#define SPECIAL_CTRL_STS_OVRRD_AMDIX   ((u16)0x8000)
#define SPECIAL_CTRL_STS_AMDIX_ENABLE  ((u16)0x4000)
#define SPECIAL_CTRL_STS_AMDIX_STATE   ((u16)0x2000)

#define PHY_INT_SRC                     29
#define PHY_INT_SRC_ENERGY_ON          ((u16)0x0080)
#define PHY_INT_SRC_ANEG_COMP          ((u16)0x0040)
#define PHY_INT_SRC_REMOTE_FAULT       ((u16)0x0020)
#define PHY_INT_SRC_LINK_DOWN          ((u16)0x0010)

#define PHY_INT_MASK                    30
#define PHY_INT_MASK_ENERGY_ON         ((u16)0x0080)
#define PHY_INT_MASK_ANEG_COMP         ((u16)0x0040)
#define PHY_INT_MASK_REMOTE_FAULT      ((u16)0x0020)
#define PHY_INT_MASK_LINK_DOWN         ((u16)0x0010)
#define PHY_INT_MASK_DEFAULT           (PHY_INT_MASK_ANEG_COMP | \
                                         PHY_INT_MASK_LINK_DOWN)

#define PHY_SPECIAL                     31
#define PHY_SPECIAL_SPD                ((u16)0x001C)
#define PHY_SPECIAL_SPD_10HALF         ((u16)0x0004)
#define PHY_SPECIAL_SPD_10FULL         ((u16)0x0014)
#define PHY_SPECIAL_SPD_100HALF        ((u16)0x0008)
#define PHY_SPECIAL_SPD_100FULL        ((u16)0x0018)

/****************************************************************************/

/* SMSC LAN9512 USB Vendor Requests */

/** Write Register:  Specify as bRequest of a USB control message to write a
 * register on the SMSC LAN9512.  bmRequestType must specify a vendor-specific
 * request in the host-to-device direction, wIndex must specify the offset of
 * the register, and the transfer data must be 4 bytes containing the value to
 * write.  */
#define SMSC9512_VENDOR_REQUEST_WRITE_REGISTER       0xA0

/** Read Register:  Specify as bRequest of a USB control message to read a
 * register from the SMSC LAN9512.  bmRequestType must specify a vendor-specific
 * request in the device-to-host direction, wIndex must specify the offset of
 * the register, and the transfer data must be a 4-byte location in which to
 * store the register's contents.  */
#define SMSC9512_VENDOR_REQUEST_READ_REGISTER        0xA1

/** TODO */
#define SMSC9512_VENDOR_REQUEST_GET_STATS            0xA2

/****************************************************************************/

/* Interrupt Endpoint status word bitfields */
#define INT_ENP_TX_STOP                ((uint32_t)BIT(17))
#define INT_ENP_RX_STOP                ((uint32_t)BIT(16))
#define INT_ENP_PHY_INT                ((uint32_t)BIT(15))
#define INT_ENP_TXE                    ((uint32_t)BIT(14))
#define INT_ENP_TDFU                   ((uint32_t)BIT(13))
#define INT_ENP_TDFO                   ((uint32_t)BIT(12))
#define INT_ENP_RXDF                   ((uint32_t)BIT(11))

/***********************************/



