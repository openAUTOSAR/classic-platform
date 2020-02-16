/*
 * usb_hub.c
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */

/**
 * @addtogroup usbhub
 *
 * This is the USB Hub driver. Hubs are one of the fundamental devices in USB
 * and are used to provide connection points (ports) for additional devices.
 * Note that even if no "external" hub is plugged in, the USB still will have at
 * least one logical hub (the root hub) and usually additional "internal" hubs.
 * That is, a USB is a tree of devices where the root node and all non-leaf
 * nodes are hubs.  A port on a USB hub may correspond to a port you can
 * physically plug a device into or may correspond to an internal port.
 *
 * This hub driver is an example of a USB device driver, but it is somewhat
 * special as it mandatory to include this driver if USB is supported at all.
 * This is because it would be impossible to access any USB devices if a hub
 * driver were not available.  This hub driver also uses some interfaces in the
 * core driver, such as usb_attach_device(), that are not useful to any other
 * USB device driver.
 *
 * The initial entry point of this USB hub driver is hub_bind_device(), which is
 * called when the USB core has configured a newly attached USB device that may
 * be a hub.  hub_bind_device() is responsible for checking if the device is a
 * hub, and if so, then doing hub-specific setup, including one-time driver
 * initialization, reading the hub descriptor, powering on the ports, and
 * submitting an asynchronous USB interrupt transfer request to the hub's status
 * change endpoint.
 *
 * Everything else this hub driver does happens asynchronously as a response to
 * a status change request being completed.  Every USB hub has exactly one IN
 * interrupt endpoint called the "status change endpoint".  The hub responds on
 * this endpoint whenever the status of the hub or one of the hub's ports has
 * changed--- for example, when a USB device has been connected or disconnected
 * from a port.
 *
 * At the hardware level, when a hub has data to send on its status change
 * endpoint, an interrupt will come in from the USB host controller.  This
 * eventually will result in the status change transfer being completed and
 * hub_status_changed() being called.  Thus, the detection of status changes is
 * interrupt-driven and is not implemented by polling at the software level.
 * (At the hardware level, USB is still a polled bus, but the host controller
 * hardware handles that for us.)  Upon detecting a status change on one or more
 * ports on a hub, the hub driver then must submit one or more control messages
 * to the hub to determine exactly what changed on the affected ports.  However,
 * we defer this work by passing it to a separate thread in order to avoid doing
 * too much synchronous work in interrupt handlers.
 */

//#include <stdlib.h>
#include "Os.h"
#include "usb_core_driver.h"
#include "usb_hub_defs.h"
#include "usb_hub_driver.h"
#include "usb_std_defs.h"
#include "usb_memory.h"
#include "bcm2835.h"
#include "Uart.h"
boolean hub_onetime_init_flag = false;
boolean HUB_TASK_FLAG = false;
/** Maximum number of ports per hub supported by this driver.  (USB 2.0
 * theoretically allows up to 255 ports per hub.)  */
#define HUB_MAX_PORTS 16

/** This driver's representation of a USB port.  */
struct usb_port
{
    /** Pointer to the USB hub this port is attached to.  */
    struct usb_hub *hub;

    /** Number of this port (1-based)  */
    uint8_t number;

    /** Pointer to the USB device attached to this port, or NULL if there is
     * none.  */
    struct usb_device *child;

    /** Status of this port.  */
    struct usb_port_status status;
};

/** This driver's representation of a USB hub.  */
struct usb_hub
{
    /** TRUE if this hub structure is being used.  */
    boolean inuse;

    /** Pointer to the USB device of this hub.  */
    struct usb_device *device;

    /** Descriptor of this hub.  */
    struct usb_hub_descriptor descriptor;

    /** Reserve space for the variable-length data at the end of the hub
     * descriptor.  That is, 'descriptor' may overflow into this field.  */
    uint8_t descriptor_varData[64];

    /** Array of this hub's ports.  Only the first descriptor.bNbrPorts entries
     * will actually be used.  */
    struct usb_port ports[HUB_MAX_PORTS];
};

/** Maximum number of USB hubs that can be attached to the USB at the same time.
 */
#define MAX_NUSBHUBS 16

/** Hub status change data buffers.  */
static uint8_t                 hub_status_change_data[MAX_NUSBHUBS][8];

/** Hub status change transfer requests.  */
static struct usb_xfer_request hub_status_change_requests[MAX_NUSBHUBS];

/** Hub-specific data structures.  */
static struct usb_hub          hub_structs[MAX_NUSBHUBS];

/** Bitmask of hubs that have status changes pending.  Note: this can be
 * modified from an interrupt handler in hub_status_changed().  */
static unsigned int hub_status_change_pending;

/** Semaphore for signaling hub thread when a status change has occurred.  */
static semaphore hub_status_change_sema;

/** Thread ID of the hub thread (hub_thread()).  */
static tid_typ hub_thread_tid = BADTID;

/* Allocate a hub structure and associated status change request.  */
static int hub_alloc(void)
{
    uint i;
    static uint nexthub;

    /* We don't disable IRQs here since this is only called from the bind_device
     * callback, which is serialized by the USB core.  */

    for (i = 0; i < MAX_NUSBHUBS; i++)
    {
        nexthub = (nexthub + 1) % MAX_NUSBHUBS;
        if (!hub_structs[nexthub].inuse)
        {
            bzero(&hub_structs[nexthub], sizeof(struct usb_hub));
            hub_structs[nexthub].inuse = TRUE;
            return nexthub;
        }
    }
    pi_printf("error: fail to hub alloc\r\n");
    return SYSERR;
}

/* Marks a hub structure as free.  */
static void hub_free(int hubid)
{
    pi_printf("infor: hub free\r\n ");
    hub_structs[hubid].inuse = FALSE;
}

/** Stack size of USB hub thread.  This shouldn't need to be very large, but the
 * hub thread can call into USB device drivers' bind_device and
 * unbind_device callbacks.  */
#define HUB_THREAD_STACK_SIZE 8192

/** Priority of USB hub thread.  This should be fairly high so that changes in
 * USB connectivity can be responded to quickly.  */
#define HUB_THREAD_PRIORITY   60

/** Name of USB hub thread.  */
#define HUB_THREAD_NAME "USB hub thread"

/* Reads the hub descriptor and saves it in hub->descriptor.  Note: the hub
 * descriptor is a class-specific descriptor and is NOT the same as the generic
 * device descriptor.  */
static usb_status_t
hub_read_descriptor(struct usb_hub *hub)
{
    usb_status_t status;
    //usb_dev_debug(hub->device, "Reading hub descriptor.\n");
//    printf("[Hub Device] %u: reading hub descriptor\r\n", hub->device->address);
    status = usb_get_descriptor(hub->device,
                                USB_HUB_REQUEST_GET_DESCRIPTOR,
                                USB_BMREQUESTTYPE_DIR_IN |
                                    USB_BMREQUESTTYPE_TYPE_CLASS |
                                    USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
                                (USB_DESCRIPTOR_TYPE_HUB << 8), 0,
                                &hub->descriptor, sizeof(hub->descriptor) +
                                                  sizeof(hub->descriptor_varData));
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_dev_error(hub->device, "Failed to read hub descriptor: %s\n", usb_status_string(status));
    	printf("[Hub Device] %u, Failed to read hub descriptor: %s\r\n", hub->device->address, usb_status_string(status));
    }
    return status;
}

/* Retrieves the status of a USB port and saves it in port->status.  */
static usb_status_t
port_get_status(struct usb_port *port)
{
    usb_status_t status;

    //usb_dev_debug(port->hub->device, "Retrieving status of port %u\n", port->number);
//    pi_printf("infor: retrieving status of port ");  pi_print(port->number, 1);

    status = usb_control_msg(port->hub->device, NULL,
                             USB_HUB_REQUEST_GET_STATUS,
                             USB_BMREQUESTTYPE_DIR_IN |
                                 USB_BMREQUESTTYPE_TYPE_CLASS |
                                 USB_BMREQUESTTYPE_RECIPIENT_OTHER,
                             0, port->number,
                             &port->status, sizeof(port->status));
    //usb_dev_debug(port->hub->device, "Got port status\n");
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_dev_error(port->hub->device, "Failed to get status for port %u: %s\n", port->number, usb_status_string(status));
        pi_printf("error: fail to get status for port\r\n");
    }
    return status;
}

/**
 * Enables or disables a feature of a USB port.  This corresponds to making a
 * ClearPortFeature or SetPortFeature request as documented in section 11.24 of
 * the USB 2.0 specification.
 *
 * @param port
 *      Pointer to the USB port structure.
 * @param feature
 *      The feature to enable or disable.
 * @param enable
 *      TRUE to enable the feature; FALSE to disable the feature.
 *
 * @return
 *      See usb_control_msg().
 */
static usb_status_t
port_change_feature(struct usb_port *port, enum usb_port_feature feature,
                    boolean enable)
{
    uint8_t bRequest = (enable) ? USB_HUB_REQUEST_SET_FEATURE :
                                  USB_HUB_REQUEST_CLEAR_FEATURE;
    return usb_control_msg(port->hub->device, NULL,
                           bRequest,
                           USB_BMREQUESTTYPE_DIR_OUT |
                               USB_BMREQUESTTYPE_TYPE_CLASS |
                               USB_BMREQUESTTYPE_RECIPIENT_OTHER,
                           feature, port->number, NULL, 0);
}

static usb_status_t
port_set_feature(struct usb_port *port, enum usb_port_feature feature)
{
    return port_change_feature(port, feature, TRUE);
}

static usb_status_t
port_clear_feature(struct usb_port *port, enum usb_port_feature feature)
{
    return port_change_feature(port, feature, FALSE);
}

/** Maximum milliseconds to wait for a port to reset (800 is the same value that
 * Linux uses).  */
#define USB_PORT_RESET_TIMEOUT 800

/** Milliseconds between each status check on the port while waiting for it to
 * finish being reset (Linux uses several values, but 10 in the default case).
 * */
#define USB_PORT_RESET_DELAY    10

/**
 * Resets a USB port.
 *
 * @param port
 *      Pointer to the USB port structure.
 *
 * @return
 *      Any value that can be returned by usb_control_msg(), as well as
 *      ::USB_STATUS_TIMEOUT if the hardware is taking too long to acknowledge
 *      that it has finished resetting.
 */
static usb_status_t
port_reset(struct usb_port *port)
{
    usb_status_t status;
    uint i;

    //usb_dev_debug(port->hub->device, "Resetting port %u\n", port->number);
//    pi_printf("infor: reset port "); mini_uart_sendDec(port->number); pi_printf("\r\n");
    /* Tell the hardware to reset.  */
    status = port_set_feature(port, USB_PORT_RESET);
    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: fail to port set feature\r\n");
        return status;
    }

    /* Wait until the hardware clears the USB_PORT_RESET flag in the port
     * status.  USB_STATUS_TIMEOUT is returned if this does not happen after a
     * certain amount of time.  */
    for (i = 0; i < USB_PORT_RESET_TIMEOUT && port->status.reset; i += USB_PORT_RESET_DELAY)
    {
        //sleep(USB_PORT_RESET_DELAY);
    	pi_printf("infor: sleep\r\n");
    	for(int j = 0; j < USB_PORT_RESET_DELAY; j ++){
    		bcm2835_Sleep(1000);
    	}
        status = port_get_status(port);
        if (status != USB_STATUS_SUCCESS)
        {
        	pi_printf("error: fail to port get status\r\n");
            return status;
        }
    }

    if (i >= USB_PORT_RESET_TIMEOUT)
    {
        return USB_STATUS_TIMEOUT;
    }

    /* From Section 9.2.6.2 of the USB 2.0 specification:
     *
     *   "After a port is reset or resumed, the USB System Software is expected
     *   to provide a "recovery" interval of 10 ms before the device attached to
     *   the port is expected to respond to data transfers. The device may
     *   ignore any data transfers during the recovery interval."
     *
     * Apparently, some devices are even slower than this, so we provide even
     * more recovery time than this.
     */
    //sleep(30);
	for (int k = 0; k < 30; k ++) {
		delayMicroseconds(1000);
	}

    return USB_STATUS_SUCCESS;
}

/* This function is called when a new USB device has been connected to a USB
 * port.  It must reset the port, then call into the USB core driver to address
 * and configure the new device attached to the port.  */
static void
port_attach_device(struct usb_port *port)
{
    usb_status_t status;
    struct usb_device *new_device;

    status = port_reset(port);
    if (status != USB_STATUS_SUCCESS)
    {
//        usb_dev_error(port->hub->device, "Failed to reset port %u: %s\n",
//                      port->number, usb_status_string(status));
        pi_printf("error: fail to reset port\r\n");
        return;
    }

    new_device = usb_alloc_device(port->hub->device);
    if (new_device == NULL)
    {
        //usb_error("Too many USB devices attached\n");
        pi_printf("error: too many USB devices\r\n");
        status = USB_STATUS_OUT_OF_MEMORY;
        port_clear_feature(port, USB_PORT_ENABLE);
        return;
    }

    status = port_get_status(port);
    if (status != USB_STATUS_SUCCESS)
    {
        usb_free_device(new_device);
        port_clear_feature(port, USB_PORT_ENABLE);
        return;
    }

     /* Record the speed at which the device is attached.  The hardware knows
     * what it is; we just need to get it from the port status structure. */
    if (port->status.high_speed_attached)
    {
        new_device->speed = USB_SPEED_HIGH;
    }
    else if (port->status.low_speed_attached)
    {
        new_device->speed = USB_SPEED_LOW;
    }
    else
    {
        new_device->speed = USB_SPEED_FULL;
    }

    //usb_dev_info(port->hub->device, "New %s-speed device connected to port %u\n", usb_speed_to_string(new_device->speed), port->number);

    new_device->port_number = port->number;

//    printf("[Port Hub Device] %u, New %s-speed device connected to port %u\n",
//    		port->hub->device->address, usb_speed_to_string(new_device->speed), port->number);

    status = usb_attach_device(new_device);

    if (status != USB_STATUS_SUCCESS)
    {
//        usb_dev_error(port->hub->device,
//                      "Failed to attach new device to port %u: %s\n",
//                      port->number, usb_status_string(status));
    	printf("[Port Hub Device] %u, Failed to attach new device to port %u: %s\r\n",
    			port->hub->device->address, port->number, usb_status_string(status));
        usb_free_device(new_device);
        port_clear_feature(port, USB_PORT_ENABLE);
        return;
    }
    usb_lock_bus();
    port->child = new_device;
    usb_unlock_bus();
}

/* Unbinds the driver from the USB device, if any, attached to a USB port and
 * frees the device structure.  */
static void
port_detach_device(struct usb_port *port)
{
    if (port->child != NULL)
    {
        usb_lock_bus();
//        usb_dev_debug(port->hub->device, "Port %u: device detached.\n",
//                      port->number);
//        usb_info("Detaching %s\n", usb_device_description(port->child));
        pi_printf("infor: detach device\r\n");
        usb_free_device(port->child);
        port->child = NULL;
        usb_unlock_bus();
    }
}

/* Respond to status change on a USB port.  */
static void
port_status_changed(struct usb_port *port)
{
    usb_status_t status;

    /* Retrieve the port status by sending a USB control message.  (This is
     * stored in port->status and is not the same as the 'status' value
     * returned, which is the status of getting the port status...)  */
    status = port_get_status(port);
    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: fail to port get status\r\n");
        return;
    }

//    usb_dev_debug(port->hub->device,
//                  "Port %u: {wPortStatus=0x%04x, wPortChange=0x%04x}\n",
//                  port->number,
//                  port->status.wPortStatus,
//                  port->status.wPortChange);
    /* Handle the various types of status changes.  */

    if (port->status.connected_changed)
    {
        /* Connection changed: a device was connected or disconnected.  */

//      usb_dev_debug(port->hub->device, "Port %u: device now %s\n",
//                      port->number,
//                      (port->status.connected ? "connected" : "disconnected"));

//    	pi_printf("port ");mini_uart_sendDec(port->number); pi_printf(": device now ");pi_printf(port->status.connected ? "connected\r\n" : "disconnected\r\n");
        /* Clear/acknowledge connection changed flag.  */
        port_clear_feature(port, USB_C_PORT_CONNECTION);

        /* Detach old device (if any).  */
        port_detach_device(port);

        if (port->status.connected)
        {
            /* Attach new device.  */
            port_attach_device(port);
        }
    }


    if (port->status.enabled_changed)
    {
//    	printf("infor: USB_C_PORT_ENABLE\r\n");
        port_clear_feature(port, USB_C_PORT_ENABLE);
    }
//
    if (port->status.reset_changed)
    {
//    	printf("infor: USB_C_PORT_RESET\r\n");
        port_clear_feature(port, USB_C_PORT_RESET);
    }
//
    if (port->status.suspended_changed)
    {
    	pi_printf("USB_C_PORT_SUSPEND\r\n");
        port_clear_feature(port, USB_C_PORT_SUSPEND);
    }
//
    if (port->status.overcurrent_changed)
    {
    	pi_printf("USB_C_PORT_OVER_CURRENT\r\n");
        port_clear_feature(port, USB_C_PORT_OVER_CURRENT);
    }

}

/**
 * @ingroup usbhub
 *
 * Call a function on each USB device in the tree.  This can be used to iterate
 * through every device on the USB.  The calling code is responsible for making
 * sure devices cannot be detached from the USB while this is executing (e.g. by
 * executing with interrupts disabled or by calling usb_lock_bus()).
 *
 * @param dev
 *      Root of the USB device tree at which to do the iteration.
 * @param callback
 *      Callback function to execute on each device.
 */
void usb_hub_for_device_in_tree(struct usb_device *dev,
                                usb_status_t (*callback)(struct usb_device *))
{
    if (dev != NULL)
    {
        (*callback)(dev);
        if (is_hub(dev))
        {
            int hub_id = (int)dev->driver_private;
            struct usb_hub *hub = &hub_structs[hub_id];
            uint i;

            for (i = 0; i < hub->descriptor.bNbrPorts; i++)
            {
                usb_hub_for_device_in_tree(hub->ports[i].child, callback);
            }
        }
    }
}

/**
 * Routine executed by the hub thread, of which one instance exists no matter
 * how many hubs there are on the USB.  The hub thread is responsible for
 * repeatedly retrieving hub status change requests that have completed, then
 * processing them by retrieving more detailed information about the ports that
 * have had status changes and responsing to them.
 *
 * Each status change requests is re-submitted after it has been processed.
 *
 * This thread intentionally executes with IRQs enabled so that it can be
 * preempted.
 *
 * @return
 *      This thread does not return.
 */
void HubTask( void )
{
//	pi_printf("infor: HubTask\r\n");
    for (;;)
    {
        /* Wait for one or more hub status change messages to arrive, then
         * process them.  */
    	usb_sem_wait(&hub_status_change_sema, 0);
		if (HUB_TASK_FLAG == true) {
			while (hub_status_change_pending != 0) {
//				printf("infor: HubTask Starts\r\n");
				struct usb_xfer_request *req;
				struct usb_hub *hub;
				int hub_id;
				irqmask im;

				hub_id = 31 - __builtin_clz(hub_status_change_pending); //Count leading zeros in a word
				req = &hub_status_change_requests[hub_id];
				hub = &hub_structs[hub_id];
				/* Clear the status change pending bit for this hub, but temporarily
				 * disable interrupts to avoid a race with hub_status_changed()
				 * modifying the same bitmask for a *different* hub.  */
				im = disable();
				hub_status_change_pending &= ~(1 << hub_id);
				restore(im);

				if (req->status == USB_STATUS_SUCCESS) {
					unsigned int portmask;
					uint i;

					//usb_dev_debug(req->dev, "Processing hub status change\n");
//					pi_printf("infor: processing hub status change\r\n");
//					printf("Device: %u, Processing hub status change\r\n", req->dev->address);
					/* The format of the message is a bitmap that indicates which ports have
					 * had status changes.  We ignore bit 0, which indicates status change
					 * of the hub device itself.  */
					portmask = 0;
					for (i = 0; i < req->actual_size; i++) {
						portmask |= (unsigned int) ((uint8_t*) req->recvbuf)[i]	<< (i * 8);
					}

					/* Process ports on which a status change was detected.  */
					for (i = 0; i < hub->descriptor.bNbrPorts; i++) {
						if (portmask & (2 << i)) {
							port_status_changed(&hub->ports[i]);
						}
					}
				} else {
//                  usb_dev_error(req->dev, "Status change request failed: %s\n",
//                              usb_status_string(req->status));
					pi_printf("error: fail to change status request\r\n");
				}
				/* Re-submit the status change request.  */
				usb_submit_xfer_request(req);
			}
			HUB_TASK_FLAG = false;
		}
    }
}

/**
 * Callback function that is called when a hub has received data on its status
 * change pipe (which is an IN interrupt endpoint).
 *
 * Since this is called from an interrupt handler, we don't really want to
 * process the status change right in this function.  Instead, signal
 * hub_thread() that a status change has occurred.
 *
 * @param req
 *      USB transfer request from the hub's status change endpoint that has
 *      completed.
 */
static void
hub_status_changed(struct usb_xfer_request *req)
{
    int hub_id;

    hub_id = req - hub_status_change_requests;
    hub_status_change_pending |= 1 << hub_id;
//    printf("infor: hub_status_change_pending = %d\r\n", hub_status_change_pending);
    usb_sem_signal(&hub_status_change_sema);
    HUB_TASK_FLAG = true;
}

/**
 * Onetime initialization of the USB hub driver.
 */
static usb_status_t
hub_onetime_init(void)
{
    uint i;

    if (hub_onetime_init_flag == true)
    {
        /* Already initialized.  */
        return USB_STATUS_SUCCESS;
    }

    /* Create semaphore for signaling hub thread.  */
    //hub_status_change_sema = semcreate(0);
    usb_sem_new(&hub_status_change_sema, 0);
    if (SYSERR == hub_status_change_sema)
    {
        return USB_STATUS_OUT_OF_MEMORY;
    }

    /* Initialize available status change requests and hub structures.  */
    hub_status_change_pending = 0;
    for (i = 0; i < MAX_NUSBHUBS; i++)
    {
        usb_init_xfer_request(&hub_status_change_requests[i]);
        hub_status_change_requests[i].recvbuf = hub_status_change_data[i];
        hub_status_change_requests[i].size = sizeof(hub_status_change_data[i]);
        hub_status_change_requests[i].completion_cb_func = hub_status_changed;
        hub_structs[i].inuse = FALSE;
    }
    hub_onetime_init_flag = true;
    /* Create hub thread.  we in the autosar way, god bless shuzhou!*/
//    hub_thread_tid = create(hub_thread, HUB_THREAD_STACK_SIZE, HUB_THREAD_PRIORITY, HUB_THREAD_NAME, 0);
//    if (SYSERR == ready(hub_thread_tid, RESCHED_NO))
//    {
//        kill(hub_thread_tid);
//        hub_thread_tid = BADTID;
//        semfree(hub_status_change_sema);
//        return USB_STATUS_OUT_OF_MEMORY;
//    }
    return USB_STATUS_SUCCESS;
}

/**
 * Initializes the port structures of a USB hub; also checks whether the hub has
 * more ports than are supported by this driver.
 */
static usb_status_t
hub_init_ports(struct usb_hub *hub)
{
    uint i;

    if (hub->descriptor.bNbrPorts > HUB_MAX_PORTS)
    {
//        usb_dev_error(hub->device,
//                      "Too many ports on hub (%u > HUB_MAX_PORTS=%u)\n",
//                      hub->descriptor.bNbrPorts, HUB_MAX_PORTS);
        pi_printf("error: too many ports on hub\r\n");
        return USB_STATUS_DEVICE_UNSUPPORTED;
    }

    for (i = 0; i < hub->descriptor.bNbrPorts; i++)
    {
        hub->ports[i].hub = hub;
        hub->ports[i].number = i + 1;
    }

    return USB_STATUS_SUCCESS;
}

/**
 * Powers on all ports of a USB hub.
 */
static usb_status_t
hub_power_on_ports(struct usb_hub *hub)
{
    uint i;
    usb_status_t status;

    //usb_dev_debug(hub->device, "Powering on %u USB ports\n", hub->descriptor.bNbrPorts);
//    pi_printf("infor: power on "); mini_uart_sendDec(hub->descriptor.bNbrPorts);  pi_printf(" ports\r\n");
    for (i = 0; i < hub->descriptor.bNbrPorts; i++)
    {
        status = port_set_feature(&hub->ports[i], USB_PORT_POWER);
        if (status != USB_STATUS_SUCCESS)
        {
            //usb_dev_error(hub->device, "Failed to power on port %u: %s\n", i, usb_status_string(status));
            pi_printf("error: fail to power on port\r\n");
            return status;
        }
    }

    /* According to the section 11.11 of the USB 2.0 specification,
     * bPwrOn2PwrGood of the hub descriptor is the "Time (in 2 ms intervals)
     * from the time the power-on sequence begins on a port until power is good
     * on that port."  Here we insert this required delay.  */
    //sleep(2 * hub->descriptor.bPwrOn2PwrGood);
	for (int j = 0; j < 2 * hub->descriptor.bPwrOn2PwrGood; j++) {
		bcm2835_Sleep(1000);
	}

    return USB_STATUS_SUCCESS;
}

/**
 * Attempts to bind a new USB device to the hub driver.  This the @ref
 * usb_device_driver::bind_device "bind_device" implementation for the hub
 * driver and therefore complies with its documented behavior.  However, an
 * important implementation detail is that the hub driver fully supports
 * multiple concurrent hubs (up to ::MAX_NUSBHUBS, to be exact).
 */
static usb_status_t
hub_bind_device(struct usb_device *dev)
{
    usb_status_t status;
    struct usb_hub *hub;
    int hub_id;

    /* Check whether the new device a hub or not.  */
    if (dev->descriptor.bDeviceClass != USB_CLASS_CODE_HUB ||
        dev->config_descriptor->bNumInterfaces != 1 ||
        dev->interfaces[0]->bNumEndpoints != 1 ||
        (dev->endpoints[0][0]->bmAttributes & 0x3) !=
                USB_TRANSFER_TYPE_INTERRUPT)
    {
    	printf("error: usb status device unsupported\r\n");
        return USB_STATUS_DEVICE_UNSUPPORTED;
    }

    /* Do one-time initialization of the hub driver.  */
    status = hub_onetime_init();
    if (status != USB_STATUS_SUCCESS)
    {
    	printf("error: fail to do one-time initialization\r\n");
        return status;
    }

    /* Allocate per-hub data.  */
    hub_id = hub_alloc();
    if (SYSERR == hub_id)
    {
        //usb_error("Too many hubs attached.\n");
        pi_printf("error: too many hubs attached\r\n");
        return USB_STATUS_DEVICE_UNSUPPORTED;
    }

    hub = &hub_structs[hub_id];
    hub->device = dev;

    /* Read the hub descriptor.  */
    status = hub_read_descriptor(hub);
    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: fail to read hub descriptor\r\n");
        hub_free(hub_id);
        return status;
    }

//    usb_dev_debug(dev, "Attaching %sUSB hub with %u ports\n",
//                  (hub->descriptor.wHubCharacteristics & USB_HUB_CHARACTERISTIC_IS_COMPOUND_DEVICE) ?
//                            "compound device " : "", hub->descriptor.bNbrPorts);
//    printf("Device %u, Attaching %s USB hub with %u ports\r\n",
//    		dev->address, (hub->descriptor.wHubCharacteristics & USB_HUB_CHARACTERISTIC_IS_COMPOUND_DEVICE) ?
//    				"compound device " : "", hub->descriptor.bNbrPorts);
    /* Initialize the appropriate number of USB port structures.  */
    status = hub_init_ports(hub);
    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: fail hub init port\r\n");
        hub_free(hub_id);
        return status;
    }

    /* Power on the ports attached to this hub.  */
    status = hub_power_on_ports(hub);

    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: fail hub power on port\r\n");
        hub_free(hub_id);
        return status;
    }

    /* Submit status change request.  */
    hub_status_change_requests[hub_id].dev = dev;

    hub_status_change_requests[hub_id].endpoint_desc = dev->endpoints[0][0];
    dev->driver_private = (void*)hub_id;
    status = usb_submit_xfer_request(&hub_status_change_requests[hub_id]);

    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: fail hub xfer request\r\n");
        hub_free(hub_id);
        return status;
    }
//    printf("infor: hub bind done\r\n");
    return USB_STATUS_SUCCESS;
}

/**
 * Unbinds the hub driver from a hub that has been detached.  This the @ref
 * usb_device_driver::unbind_device "unbind_device" implementation for the hub
 * driver and therefore complies with its documented behavior.  However, an
 * important detail that only the hub driver needs to deal with is recursively
 * detaching any child devices.
 */
static void
hub_unbind_device(struct usb_device *hub_device)
{
    int hub_id = (int)hub_device->driver_private;
    struct usb_hub *hub = &hub_structs[hub_id];
    irqmask im;
    uint i;

    /* Detach any devices attached to this hub (a.k.a. "child" devices).  */
    for (i = 0; i < hub->descriptor.bNbrPorts; i++)
    {
        if (hub->ports[i].child != NULL)
        {
            usb_free_device(hub->ports[i].child);
        }
    }

    /* Remove the status change request for the detached hub from the mask of
     * status change requests waiting to be processed by the hub thread.  To do
     * this, we must clear the corresponding bit in hub_status_change_pending.
     * Interrupts must be temporarily disabled to avoid racing with
     * hub_status_changed() trying to set bit for a *different* hub in the same
     * bitmask.  */
    im = disable();
    hub_status_change_pending &= ~(1 << hub_id);
    restore(im);

    /* Free the `struct usb_hub' for the detached hub.  */
    hub_free(hub_id);
}

/**
 * @ingroup usbhub
 * Declaration of the USB hub device driver.
 */
const struct usb_device_driver usb_hub_driver = {
    .name          = "USB Hub Driver",
    .bind_device   = hub_bind_device,
    .unbind_device = hub_unbind_device,
};
