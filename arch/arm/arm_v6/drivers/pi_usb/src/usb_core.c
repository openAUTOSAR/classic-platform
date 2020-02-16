/*
 * This is my last try for usb at the
 * first day of Chinese new year.
 * I hope can be successful at last
 * God bless me!
 *
 *                         ----shuzhou
 *                          2014-01-31
 *
 * this file contain the usb core functions
 */
#include "Os.h"
#include "usb_memory.h"
#include "usb_core_driver.h"
#include "usb_hcdi.h"
#include "usb_hub_driver.h"
#include "usb_std_defs.h"
#include "usb_subsystem.h"
#include "Uart.h"
#include "bcm2835.h"
boolean Message_Done = false;
/** Maximum number of simultaneous USB devices supported.  */
#define MAX_NUSBDEV 16

/** Maximum number of simultaneous USB device drivers supported.  */
#define MAX_NUSBDRV 16

/** Table of USB device structures that can be dynamically assigned to actual
 * devices as needed.  */
static struct usb_device usb_devices[MAX_NUSBDEV];

/** Table of USB device drivers that have been registered with the USB core. */
static const struct usb_device_driver *usb_device_drivers[MAX_NUSBDEV];

/** Number of currently registered USB device drivers.  There is currently no
 * way to remove a registered device driver; hence this can only increase.  */
static uint usb_num_device_drivers = 0;

/** Pointer to the root hub, or NULL if the USB subsystem has not yet been
 * successfully initialized.  */
struct usb_device *usb_root_hub = NULL;

/**
 * @ingroup usbcore
 *
 * Performs one-time initialization on a USB transfer request allocated by the
 * device driver in some way other than by usb_alloc_xfer_request().
 *
 * @param req
 *      Pointer to the USB transfer request to initialize.
 */
void
usb_init_xfer_request(struct usb_xfer_request *req)
{
    bzero(req, sizeof(struct usb_xfer_request));
    /* TODO: HCD-specific variables need to be handled better.  */
    req->deferer_thread_tid = BADTID;
    req->deferer_thread_sema = SYSERR;
}

/**
 * @ingroup usbcore
 *
 * Dynamically allocates a struct usb_xfer_request, including a data buffer.
 *
 * @param bufsize
 *      Length of the data buffer for sending and/or receiving.
 *
 * @return
 *      A pointer to the resulting struct usb_xfer_request, or NULL if out of
 *      memory.
 */
struct usb_xfer_request *
usb_alloc_xfer_request(uint bufsize)
{
    struct usb_xfer_request *req;

    req = memget(sizeof(struct usb_xfer_request) + bufsize);
    if (req == (void*)SYSERR)
    {
    	pi_printf("error: fail to usb alloc xfer request\r\n");
        return NULL;
    }
    usb_init_xfer_request(req);
    req->sendbuf = (uint8_t*)(req + 1);
    req->size = bufsize;
    return req;
}

/**
 * @ingroup usbcore
 *
 * Frees a struct usb_xfer_request allocated by usb_alloc_xfer_request().
 *
 * @param req
 *      A pointer to the struct usb_xfer_request to free.  It must not be
 *      currently pending.  NULL can be passed as a no-op.
 */
void usb_free_xfer_request(struct usb_xfer_request *req)
{
    if (req != NULL)
    {
        /* TODO: HCD-specific variables need to be handled better.  */
        //kill(req->deferer_thread_tid);
        semfree(req->deferer_thread_sema);
        memfree(req, sizeof(struct usb_xfer_request) + req->size);
        //pi_printf("infor: usb free request memory\r\n ");

    }
}

/**
 * @ingroup usbcore
 *
 * Submit a USB transfer request to be completed.
 *
 * This is intended to be an asynchronous interface where the transfer request
 * is queued for later processing by the Host Controller.  This function should
 * immediately return and the transfer request should be asynchronously
 * completed at some later time, making use of interrupts between the CPU and
 * host controller to accomplish this efficiently.  However, exactly how the
 * transfer request is completed is ultimately up to the Host Controller Driver.
 * Synchronous completion is possible, although not recommended.
 *
 * When the transfer has been completed or has failed, the @ref
 * usb_xfer_request::completion_cb_func "completion_cb_func" function of the @p
 * req will be called.  This callback function should process the completed or
 * failed request and either free its memory or re-submit it to start another
 * transfer.
 *
 * Not all USB transfer types and speeds are necessarily supported.  This
 * depends on the Host Controller Driver, which is expected to make sure each
 * request is supported when passed it and return ::USB_STATUS_INVALID_PARAMETER
 * if not.  Currently, the Host Controller Driver for the Synopsys DesignWare
 * High-Speed USB 2.0 On-The-Go Controller (the one on the Raspberry Pi) is
 * intended to support control, bulk, and interrupt transfers at low, full, and
 * high speeds; however, not all combinations of transfers and speeds have
 * actually been tested yet.
 *
 * @param req
 *      Pointer to the USB transfer request to submit.  Its members documented
 *      as input must have been filled in.
 *
 * @retval ::USB_STATUS_SUCCESS
 *      The request was submitted successfully.
 * @retval ::USB_STATUS_INVALID_PARAMETER
 *      @p req was not filled in correctly.
 * @retval ::USB_STATUS_DEVICE_DETACHED
 *      The device has been detached.
 */
usb_status_t
usb_submit_xfer_request(struct usb_xfer_request *req)
{
#if USB_MIN_LOG_PRIORITY <= USB_LOG_PRIORITY_DEBUG
    enum usb_transfer_type type;
    enum usb_direction dir;
#endif
    irqmask im;
    usb_status_t status;

    if (!req || !req->dev || !req->completion_cb_func || !req->dev->inuse)
    {
//      usb_error("Bad usb_xfer_request: no device or completion callback "
//                  "function\n");
//    	pi_printf("req = "); pi_print((int )req, 0);
//    	pi_printf("req->dev = "); pi_print((int )req->dev, 0);
//    	pi_printf("req->completion_cb_func = "); pi_print((int )req->completion_cb_func, 0);
//    	pi_printf("req->dev->inuse = "); pi_print((boolean)req->dev->inuse, 1);

//		printf("infor: req = %d  ", *req);
//		printf("  req->dev = %d\r\n",  *req->dev);
//		printf("req->completion_cb_func = %d", *req->completion_cb_func);
//		printf("req->dev->inuse = %d", req->dev->inuse);
    	printf("infor: req = %d, req->dev->address = %d, req->completion_cb_func = %d, req->dev->inuse = %d",
    			*req, req->dev->address, req->completion_cb_func, req->dev->inuse);
		pi_printf("error: bad usb xfer request\r\n");
        return USB_STATUS_INVALID_PARAMETER;
    }

    im = disable();

    /* Don't allow submitting new transfers to devices that are going away.  */
    if (req->dev->state == USB_DEVICE_DETACHMENT_PENDING)
    {
//        usb_dev_debug(req->dev, "Device detachment pending; "
//                      "refusing new xfer\n");
        pi_printf("error: refusing new xfer\r\n");
        restore(im);
        return USB_STATUS_DEVICE_DETACHED;
    }


#if  USB_MIN_LOG_PRIORITY <= USB_LOG_PRIORITY_DEBUG
    if (req->endpoint_desc)
    {
        type = req->endpoint_desc->bmAttributes & 0x3;
        dir = req->endpoint_desc->bEndpointAddress >> 7;
    }
    else
    {
        type = USB_TRANSFER_TYPE_CONTROL;
        dir = req->setup_data.bmRequestType >> 7;
    }
//    usb_dev_debug(req->dev, "Submitting xfer request (%u bytes, "
//                  "type=%s, dir=%s)\n",
//                  req->size,
//                  usb_transfer_type_to_string(type),
//                  usb_direction_to_string(dir));
	printf( "Device %u, Submitting xfer request (%u bytes, type=%s, dir=%s)\r\n",
			req->dev->address, req->size,
			usb_transfer_type_to_string(type),
			usb_direction_to_string(dir));
    if (type == USB_TRANSFER_TYPE_CONTROL)
    {
//        usb_dev_debug(req->dev, "Control message: {.bmRequestType=0x%02x, "
//                      ".bRequest=0x%02x, wValue=0x%04x, wIndex=0x%04x, wLength=0x%04x}\n",
//                      req->setup_data.bmRequestType,
//                      req->setup_data.bRequest,
//                      req->setup_data.wValue,
//                      req->setup_data.wIndex,
//                      req->setup_data.wLength);
//		printf("Device %u, Control message: {.bmRequestType=0x%02x, .bRequest=0x%02x, wValue=0x%04x, wIndex=0x%04x, wLength=0x%04x}\n",
//				req->dev->address,
//				req->setup_data.bmRequestType,
//				req->setup_data.bRequest,
//				req->setup_data.wValue,
//				req->setup_data.wIndex,
//				req->setup_data.wLength);
    }
#endif
    req->status = USB_STATUS_NOT_PROCESSED;
    req->actual_size = 0;
    req->complete_split = 0;
    req->control_phase = 0;
    ++req->dev->xfer_pending_count;
    status = hcd_submit_xfer_request(req);
    if (status != USB_STATUS_SUCCESS)
    {
        --req->dev->xfer_pending_count;
    }
    restore(im);
    return status;
}

/**
 * @ingroup usbcore
 *
 * Signal the device driver that a USB transfer has successfully completed or
 * has encountered an error.  Intended to be called by Host Controller Drivers
 * only.
 *
 * @param req
 *        USB transfer to signal completion on.
 */
void
usb_complete_xfer(struct usb_xfer_request *req)
{
    irqmask im;

    im = disable();

    --req->dev->xfer_pending_count;

    /* Override transfer status if device detachment pending.  */
    if (req->dev->state == USB_DEVICE_DETACHMENT_PENDING)
    {
        req->status = USB_STATUS_DEVICE_DETACHED;
    }

//    usb_dev_debug(req->dev,
//                  "Calling completion callback (Actual transfer size %u "
//                  "of %u bytes, type=%s, dir=%s, status=%d)\n",
//                  req->actual_size, req->size,
//                  usb_transfer_type_to_string(
//                        req->endpoint_desc ?
//                            (req->endpoint_desc->bmAttributes & 0x3) :
//                            USB_TRANSFER_TYPE_CONTROL),
//                  usb_direction_to_string(
//                        req->endpoint_desc ?
//                            (req->endpoint_desc->bEndpointAddress >> 7) :
//                            req->setup_data.bmRequestType >> 7),
//                  req->status);

//     printf("Calling completion callback (Actual transfer size %u of %u bytes, type=%s, dir=%s, status=%d)\r\n",
//    		 req->actual_size, req->size,
//             usb_transfer_type_to_string(req->endpoint_desc ? (req->endpoint_desc->bmAttributes & 0x3) : USB_TRANSFER_TYPE_CONTROL),
//             usb_direction_to_string( req->endpoint_desc ? (req->endpoint_desc->bEndpointAddress >> 7) : req->setup_data.bmRequestType >> 7),
//             req->status);

    /* Tally error if not successful.  */
    if (req->status != USB_STATUS_SUCCESS)
    {
        req->dev->error_count++;
        req->dev->last_error = req->status;
        pi_printf("error: fail to transfer\r\n");
    }
    /* Actually call the completion function.  */
    (*req->completion_cb_func)(req);
    /* If device is being detached and we just completed the last pending
     * transfer to it, signal the thread waiting in usb_free_device().  */
    if (req->dev->state == USB_DEVICE_DETACHMENT_PENDING &&
        req->dev->xfer_pending_count == 0)
    {
        //send(req->dev->quiescent_state_waiter, 0);
    	pi_printf("[Attention]: usb_complete_xfer!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
    }

    restore(im);
}

static void
signal_control_msg_done(struct usb_xfer_request *req)
{
//	printf("infor: wait req->private\r\n");
	usb_sem_signal(req->private);
//	printf("infor: signal req->private\r\n");

}

/**
 * @ingroup usbcore
 *
 * Synchronously performs a control transfer to or from a USB device.  Control
 * messages are one of the four fundamental transfer types of USB and are
 * documented in various places in the USB 2.0 specification.
 *
 * This is a synchronous interface, so the thread will be blocked until the
 * control transfer has completed, timed out, or encountered another error.
 * Currently the request timeout is chosen by the Host Controller Driver and
 * cannot be passed as a parameter.
 *
 * @param dev
 *      Pointer to the USB device to which to submit the control message.
 * @param endpoint_desc
 *      Pointer to the endpoint descriptor for the endpoint to which to submit
 *      the control transfer, or NULL to specify the default control endpoint
 *      (which has no endpoint descriptor).
 * @param bRequest
 *      Request to make.  Standard device requests are documented in Section 9.4
 *      of the USB 2.0 specification; in this code they are the enum
 *      ::usb_device_request values.  Other values, such as class-specific
 *      requests, can also be used for this parameter.
 * @param bmRequestType
 *      Type of the request to make.  Standard types are documented in Section
 *      9.3.1 of the USB 2.0 specification; in this code they are the enum
 *      ::usb_bmRequestType_fields values.
 * @param wValue
 *      Request-specific data.
 * @param wIndex
 *      Request-specific data.
 * @param data
 *      Pointer to a buffer that, depending on the request, either contains the
 *      additional data to be sent as part of the message, or receives the
 *      additional data that will be received from the USB device.   This
 *      parameter is ignored if @p wLength is 0.
 * @param wLength
 *      Length of the @p data buffer.  This is interpreted as the exact number
 *      of additional bytes of data that must be transferred.  This can be 0 if
 *      no additional data needs to be transferred to satisfy the request; for
 *      example, a SetAddress request passes information in wValue but uses no
 *      additional data buffer.
 *
 * @return
 *      Status of the transfer.  The possible return values include:
 *
 * @retval ::USB_STATUS_SUCCESS
 *      The transfer completed successfully with exactly the amount of data
 *      specified by @p wLength.
 * @retval ::USB_STATUS_OUT_OF_MEMORY
 *      Failed to allocate memory or create a semaphore.
 * @retval ::USB_STATUS_INVALID_DATA
 *      The transfer completed, but not with the exact amount of data specified.
 * @retval ::USB_STATUS_UNSUPPORTED_REQUEST
 *      The transfer was to the root hub, but the request was not implemented.
 * @retval ::USB_STATUS_HARDWARE_ERROR
 *      A hardware error occurred.
 * @retval ::USB_STATUS_DEVICE_DETACHED
 *      The USB device was detached.
 */
usb_status_t
usb_control_msg(struct usb_device *dev,
                const struct usb_endpoint_descriptor *endpoint_desc,
                uint8_t bRequest, uint8_t bmRequestType,
                uint16_t wValue, uint16_t wIndex, void *data, uint16_t wLength)
{
	//pi_printf("usb control msg(): call\r\n");
    usb_status_t status;
    struct usb_xfer_request *req;
    semaphore sem;
    sem_err err;
//    sem = semcreate(0);
//
//    if (isbadsem(sem))
//    {
//        return USB_STATUS_OUT_OF_MEMORY;
//    }

//    pi_printf("infor: hold msg sem\r\n");
    err = usb_sem_new(&sem, 0);
    if(err != SEM_OK){
        pi_printf("infor: fail to create msg sem\r\n");
    	return USB_STATUS_OUT_OF_MEMORY;
    }
//    pi_printf("infor: get new sem\r\n");
    req = usb_alloc_xfer_request(wLength);
    if (req == NULL)
    {
        semfree(sem);
        return USB_STATUS_OUT_OF_MEMORY;
    }
    req->dev = dev;
    req->endpoint_desc = endpoint_desc;
    req->recvbuf = data;
    req->size = wLength;
    req->setup_data.bmRequestType = bmRequestType;
    req->setup_data.bRequest = bRequest;
    req->setup_data.wValue = wValue;
    req->setup_data.wIndex = wIndex;
    req->setup_data.wLength = wLength;
    req->completion_cb_func = signal_control_msg_done;
    req->private = &sem;
    status = usb_submit_xfer_request(req);
    if (status == USB_STATUS_SUCCESS)
    {
        /* Wait for transfer to complete (or fail).  */
        //wait(sem);
//    	pi_printf("infor: wait msg sem\r\n");
    	usb_sem_wait(&sem, 0);

//    	pi_printf("infor: get msg sem\r\n");
//    	while(Message_Done == false){
//    		pi_printf("hope is needed!\r\n");
//    	}
    	//does the req value change somehow
    	//after passing to usb_submit_xfer_request
    	//dwc_schedule_xfer_requests(req);
        status = req->status;

        /* Force error if actual size was not the same as requested size.  */
        if (status == USB_STATUS_SUCCESS && req->actual_size != req->size)
        {
            status = USB_STATUS_INVALID_DATA;
            req->dev->error_count++;
            req->dev->last_error = status;
            pi_printf("error: invalid data\r\n");
        }
    }
//    printf("3.....[Req],[Dev],[Check Device]: %u,%u,%u\r\n", (int)req, (int)req->dev,req->dev->address);
    usb_free_xfer_request(req);
//    printf("4.....[Req],[Dev],[Check Device]: %u,%u,%u\r\n", (int)req, (int)req->dev,req->dev->address);
//    semfree(sem);
    usb_sem_free(&sem);
//    Message_Done = false;
    //pi_printf("usb control msg(): done!\r\n");
    return status;
}

/**
 * @ingroup usbcore
 *
 * Reads a descriptor from a USB device.  This is a wrapper around
 * usb_control_msg() that automatically handles reading the descriptor header
 * and requesting the appropriate length.
 *
 * @param dev
 *      USB device from which to read the descriptor.
 * @param bRequest
 *      See usb_control_msg().
 * @param bmRequestType
 *      See usb_control_msg().
 * @param wValue
 *      See usb_control_msg().
 * @param wIndex
 *      See usb_control_msg().
 * @param buf
 *      See usb_control_msg().
 * @param buflen
 *      See usb_control_msg().
 *
 * @return
 *      Same possible return values as usb_control_msg(), or
 *      ::USB_STATUS_INVALID_DATA if the bLength field in the descriptor header
 *      is less than the size of the descriptor header itself.
 */
usb_status_t
usb_get_descriptor(struct usb_device *dev, uint8_t bRequest, uint8_t bmRequestType,
                   uint16_t wValue, uint16_t wIndex, void *buf, uint16_t buflen)
{
    usb_status_t status;
    uint16_t len;

    if (buflen > sizeof(struct usb_descriptor_header))
    {
        /* Get descriptor length.  */
        struct usb_descriptor_header hdr;

        status = usb_control_msg(dev, NULL, bRequest, bmRequestType,
                                 wValue, wIndex, &hdr, sizeof(hdr));
        if (status != USB_STATUS_SUCCESS)
        {
        	pi_printf("error: usb_get_descriptor(), wrong status\r\n");
            return status;
        }

        if (hdr.bLength < sizeof(hdr))
        {
            //usb_dev_error(dev, "Descriptor length too short\n");
            pi_printf("error: descriptor length too short\r\n");
            return USB_STATUS_INVALID_DATA;
        }

        /* Length to read is the minimum of the descriptor's actual length and
         * the buffer length.  */
        len = min(hdr.bLength, buflen);
    }
    else
    {
        len = buflen;
    }
    /* Read the descriptor for real.  */
    return usb_control_msg(dev, NULL, bRequest, bmRequestType, wValue, wIndex,
                           buf, len);
}

/* Read a USB device's device descriptor, or a prefix of it, into
 * dev->descriptor.  */
static usb_status_t
usb_read_device_descriptor(struct usb_device *dev, uint16_t maxlen)
{
    /* Note: we do not really need to use usb_get_descriptor() here because we
     * never read more than the minimum length of the device descriptor.  */
    return usb_control_msg(dev, NULL,
                           USB_DEVICE_REQUEST_GET_DESCRIPTOR,
                           USB_BMREQUESTTYPE_DIR_IN |
                               USB_BMREQUESTTYPE_TYPE_STANDARD |
                               USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
                           USB_DESCRIPTOR_TYPE_DEVICE << 8, 0,
                           &dev->descriptor, maxlen);
}

/* Read the specified configuration descriptor, or a prefix of it, from a USB
 * device into a buffer.  */
static usb_status_t
usb_get_configuration_descriptor(struct usb_device *dev, uint8_t configuration_idx,
                                 void *buf, uint16_t buflen)
{
    return usb_control_msg(dev, NULL, USB_DEVICE_REQUEST_GET_DESCRIPTOR,
                           USB_BMREQUESTTYPE_DIR_IN |
                               USB_BMREQUESTTYPE_TYPE_STANDARD |
                               USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
                           (USB_DESCRIPTOR_TYPE_CONFIGURATION << 8) | configuration_idx,
                           0, buf, buflen);
}

/**
 * Reads the first configuration descriptor of a USB device into the dynamically
 * allocated dev->config_descriptor buffer.  Assumes this buffer is initially
 * unallocated.
 *
 * @param dev
 *      Pointer to the USB device structure for the device from which to read
 *      the configuration descriptor.
 * @param configuration
 *      Index of configuration to read.
 *
 * @return
 *      ::USB_STATUS_OUT_OF_MEMORY if memory for the configuration descriptor
 *      could not be allocated, ::USB_STATUS_INVALID_PARAMETER if the
 *      configuration descriptor is invalid, or any value that can be returned
 *      by usb_control_msg().
 */
static usb_status_t
usb_read_configuration_descriptor(struct usb_device *dev, uint8_t configuration)
{
    struct usb_configuration_descriptor desc;
    usb_status_t status;
    int interface_idx;
    int endpoint_idx;
    uint i;
    struct usb_descriptor_header *hdr;
    boolean in_alternate_setting;

    /* Get configuration descriptor size */
    status = usb_get_configuration_descriptor(dev, configuration,
                                              &desc, sizeof(desc));
    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: fail to get configuration descriptor\r\n");
        return status;
    }

    /* Allocate buffer for full configuration descriptor */
    dev->config_descriptor = memget(desc.wTotalLength);
    if (dev->config_descriptor == (void*)SYSERR)
    {
    	pi_printf("error: fail to configuration descriptor memory\r\n");
        return USB_STATUS_OUT_OF_MEMORY;
    }

    /* Get configuration descriptor for real */
    status = usb_get_configuration_descriptor(dev, configuration,
                                              dev->config_descriptor,
                                              desc.wTotalLength);
    if (status != USB_STATUS_SUCCESS)
    {
    	pi_printf("error: fail to get configuration descriptor for real\r\n");
        return status;
    }

    /* Set up pointers to the interface descriptors and endpoint descriptors.
     * */
    interface_idx = -1;
    endpoint_idx = -1;
    in_alternate_setting = FALSE;
    for (i = 0;
         i + sizeof(struct usb_descriptor_header) <= desc.wTotalLength;
         i += hdr->bLength)
    {
        hdr = (struct usb_descriptor_header*)((uint8_t*)dev->config_descriptor + i);

        if (hdr->bLength < sizeof(struct usb_descriptor_header))
        {
            goto out_invalid;
        }

        switch (hdr->bDescriptorType)
        {
            case USB_DESCRIPTOR_TYPE_INTERFACE:
                if (i + sizeof(struct usb_interface_descriptor) > desc.wTotalLength)
                {
                    goto out_invalid;
                }
                if (interface_idx >= 0 && !in_alternate_setting &&
                    endpoint_idx + 1 !=
                            dev->interfaces[interface_idx]->bNumEndpoints)
                {
                    //usb_dev_debug(dev, "Number of endpoints incorrect\n");
                    pi_printf("error: number of endpoints incorrect\r\n");
                    goto out_invalid;
                }
                if (((struct usb_interface_descriptor*)hdr)->bAlternateSetting != 0)
                {
                    in_alternate_setting = TRUE;
                }
                else
                {
                    in_alternate_setting = FALSE;
                    if (++interface_idx >= USB_DEVICE_MAX_INTERFACES)
                    {
//                        usb_dev_error(dev,
//                                      "Too many interfaces (this driver only "
//                                      "supports %u per configuration)\n",
//                                      USB_DEVICE_MAX_INTERFACES);
                    	pi_printf("error: too many interfaces\r\n");
                        return USB_STATUS_DEVICE_UNSUPPORTED;
                    }
                    dev->interfaces[interface_idx] =
                                (struct usb_interface_descriptor*)hdr;
                    endpoint_idx = -1;
                }
                break;
            case USB_DESCRIPTOR_TYPE_ENDPOINT:
                if (interface_idx < 0)
                {
                    goto out_invalid;
                }
                if (i + sizeof(struct usb_endpoint_descriptor) > desc.wTotalLength)
                {
                    goto out_invalid;
                }
                if (!in_alternate_setting)
                {
                    if (++endpoint_idx >= USB_DEVICE_MAX_ENDPOINTS)
                    {
//                        usb_dev_error(dev,
//                                      "Too many endpoints (this driver only "
//                                      "supports %u per interface)\n",
//                                      USB_DEVICE_MAX_ENDPOINTS);
                        pi_printf("error: too many endpoints\r\n");
                        return USB_STATUS_DEVICE_UNSUPPORTED;
                    }
                    dev->endpoints[interface_idx][endpoint_idx] =
                                (struct usb_endpoint_descriptor*)hdr;
                }
                break;
            default:
                break;
        }
    }
    if (interface_idx + 1 != dev->config_descriptor->bNumInterfaces)
    {
        //usb_dev_debug(dev, "Number of interfaces incorrect (interface_idx=%d)\n", interface_idx);
        pi_printf("error: number of interfaces incorrect\r\n");
        goto out_invalid;
    }

    return USB_STATUS_SUCCESS;
out_invalid:
    //usb_dev_error(dev, "Configuration descriptor invalid\n");
    pi_printf("error: configuration descriptor invalid\r\n");
    return USB_STATUS_INVALID_DATA;
}

/* Sets the bus address of a USB device.  */
static usb_status_t
usb_set_address(struct usb_device *dev, uint8_t address)
{
    usb_status_t status;
    status = usb_control_msg(dev, NULL, /* default control endpoint */
                             USB_DEVICE_REQUEST_SET_ADDRESS,
                             USB_BMREQUESTTYPE_DIR_OUT |
                                USB_BMREQUESTTYPE_TYPE_STANDARD |
                                USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
                             address, /* wValue: address */
                             0, NULL, 0); /* wIndex, buf, wLength: unused */
    if (status == USB_STATUS_SUCCESS)
    {
        dev->address = address;
//        printf("infor:  dev->address = %u\r\n", dev->address);
    }
    return status;
}

/* Configures a USB device with the specified configuration, as specified by the
 * bConfigurationValue field in one of the device's configuration descriptors.
 */
static usb_status_t
usb_set_configuration(struct usb_device *dev, uint8_t configuration)
{
    usb_status_t status;

    status = usb_control_msg(dev,
                             NULL, /* endpoint 0 */
                             USB_DEVICE_REQUEST_SET_CONFIGURATION,
                             USB_BMREQUESTTYPE_DIR_OUT |
                                USB_BMREQUESTTYPE_TYPE_STANDARD |
                                USB_BMREQUESTTYPE_RECIPIENT_DEVICE,
                             configuration, /* wValue: configuration index */
                             0, NULL, 0); /* wIndex, data, wLength: unused */
    if (status == USB_STATUS_SUCCESS)
    {
        dev->configuration = configuration;
    }
    return status;
}

/**
 * @ingroup usbcore
 *
 * Allocate a USB device structure from the pool of available device structures.
 *
 * @param parent
 *      Pointer to the USB device structure for the parent hub, or @c NULL if
 *      the device being allocated is the root hub.

 * @return
 *      Pointer to an initialized USB device structure, or @c NULL if no more
 *      USB device structures are available.
 */
struct usb_device *
usb_alloc_device(struct usb_device *parent)
{
    uint i;
    struct usb_device *dev;
    irqmask im;

    dev = NULL;
    im = disable();
    for (i = 0; i < MAX_NUSBDEV; i++)
    {
        if (!usb_devices[i].inuse)
        {
            dev = &usb_devices[i];
            bzero(dev, sizeof(struct usb_device));
            dev->inuse = TRUE;
            dev->speed = USB_SPEED_HIGH; /* Default to high-speed unless
                                            overridden later */
            dev->parent = parent;
            if (parent != NULL)
            {
                dev->depth = parent->depth + 1;
            }
            dev->last_error = USB_STATUS_SUCCESS;
            dev->state = USB_DEVICE_ATTACHED;
            dev->quiescent_state_waiter = BADTID;
            break;
        }
    }
    restore(im);
    return dev;
}

/**
 * @ingroup usbcore
 *
 * Detaches and frees a USB device.  This unbinds the device driver, if one is
 * bound, and returns the USB device structure to the pool of available device
 * structures.  Only the hub driver should need to call this function.
 *
 * @param dev
 *      Pointer to the USB device structure to detach.
 */
void
usb_free_device(struct usb_device *dev)
{
    irqmask im;

    /* Disallow new transfers to this device and wait for all pending
     * transfers to complete.  */
    im = disable();
    dev->state = USB_DEVICE_DETACHMENT_PENDING;
    if (dev->xfer_pending_count != 0)
    {
//        usb_dev_debug(dev, "Waiting for %u pending xfers to complete\n",
//                      dev->xfer_pending_count);
//        dev->quiescent_state_waiter = gettid();
//        receive();
    	pi_printf("usb_complete_xfer, receive\r\n");
    }
    restore(im);

    /* Unbind the device driver if needed.  */
    if (dev->driver != NULL && dev->driver->unbind_device != NULL)
    {
//        usb_dev_debug(dev, "Unbinding %s\n", dev->driver->name);
//    	pi_printf("infor: unbinding \r\n ");
//    	pi_printf(dev->driver->name);
        dev->driver->unbind_device(dev);
    }

    /* Free configuration descriptor and device structure.  */
    //usb_dev_debug(dev, "Releasing USB device structure.\n");
    pi_printf("infor: releasing USB device structure\r\n ");
    if (dev->config_descriptor != NULL)
    {
        memfree(dev->config_descriptor, dev->config_descriptor->wTotalLength);
        pi_printf("infor: usb free device memory\r\n ");
    }
    dev->inuse = FALSE;
}

/**
 * Tries to bind a USB device driver to a USB device.
 *
 * @param dev
 *      A newly addressed and configured USB device to find a driver for.
 *
 * @return
 *      ::USB_STATUS_SUCCESS on success if driver successfully bound or was
 *      already bound; ::USB_STATUS_DEVICE_UNSUPPORTED if no driver supports the
 *      device; or another error code returned by a driver initialization
 *      routine.
 */
static usb_status_t
usb_try_to_bind_device_driver(struct usb_device *dev)
{
    usb_status_t status;
    uint i;

    if (dev->driver != NULL)
    {
        /* Driver already bound.  */
    	pi_printf("infor: driver already bound\r\n");
        return USB_STATUS_SUCCESS;
    }

    status = USB_STATUS_DEVICE_UNSUPPORTED;
    for (i = 0; i < usb_num_device_drivers; i++)
    {
        //usb_dev_debug(dev, "Attempting to bind %s to device\n", usb_device_drivers[i]->name);
//        pi_printf("infor: attempt to bind "); pi_printf(usb_device_drivers[i]->name); pi_printf(" to device\r\n");
//        printf("Device: %u, Attempting to bind %s to device\r\n", dev->address, usb_device_drivers[i]->name);

        status = usb_device_drivers[i]->bind_device(dev);
        if (status != USB_STATUS_DEVICE_UNSUPPORTED)
        {
            if (status == USB_STATUS_SUCCESS)
            {
                dev->driver = usb_device_drivers[i];
                //usb_info("Bound %s to %s\n", dev->driver->name, usb_device_description(dev));
//                printf("Bound %s \r\n", dev->driver->name);
            }
            break;
        }
    }
    return status;
}

/**
 * @ingroup usbcore
 *
 * Configure and initialize, or "enumerate", a newly allocated USB device.  The
 * physical device is initially assumed to be non-addressed and non-configured
 * and therefore accessible by sending control messages to the default address
 * of 0.
 *
 * @param dev
 *      New USB device to configure and initialize.
 * @return
 *      ::USB_STATUS_SUCCESS if successful; another ::usb_status_t error code
 *      otherwise.  Note that after a succesful return, the newly attached
 *      device may or may not have been bound to an actual device driver.
 */
usb_status_t
usb_attach_device(struct usb_device *dev)
{
    usb_status_t status;
    uint8_t address;

    /* To communicate with the USB device using control transfers, we need to
     * know the maximum packet size supported by the device.  This is nontrivial
     * because the maximum packet size is stored in the device descriptor, which
     * itself needs to be read with a control transfer.  To work around this,
     * according to the USB 2.0 specification the USB system software must read
     * only the first 8 bytes of the device descriptor in a single packet, which
     * will include the maximum packet size that should be used with further
     * transfers.  This works because the maximum packet size is guaranteed to
     * be at least 8 bytes.  */
//    usb_dev_debug(dev, "Getting maximum packet size from start of device descriptor\n");
//    printf("Device: %u, Getting maximum packet size from start of device descriptor\r\n", dev->address);
    dev->descriptor.bMaxPacketSize0 = 8;
    status = usb_read_device_descriptor(dev, 8);
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_dev_error(dev, "Failed to read start of device descriptor: %s\n", usb_status_string(status));
    	pi_printf("error: fail to read start of device descriptor\r\n");
        return status;
    }

//    usb_dev_debug(dev, "Using bMaxPacketSize0=%u\n", dev->descriptor.bMaxPacketSize0);
//    pi_printf("usb_attach_device(): using bMaxPacketSize0 "); mini_uart_sendDec(dev->descriptor.bMaxPacketSize0);pi_printf("\r\n");
//    printf("Device: %u, Using bMaxPacketSize0=%u\r\n", dev->address, dev->descriptor.bMaxPacketSize0);
    /* Assign an address to this device.  To get a unique address we just use
     * the 1-based index of the `struct usb_device' in the usb_devices table. */
    address = (dev - usb_devices) + 1;
//    usb_dev_debug(dev, "Assigning address %u to new device\n", address);
//    pi_printf("usb_attach_device(): assigning address "); mini_uart_sendhex(address, 1);
//    printf("Device: %u, Assigning address %u to new device\r\n", dev->address, address);
    status = usb_set_address(dev, address);
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_dev_error(dev, "Failed to assign address: %s\n", usb_status_string(status));
        pi_printf("error: fail to assign address\r\n");
        return status;
    }

    /* Read the device descriptor to find information about this device.  */
//    usb_debug("Reading device descriptor.\n");
//    pi_printf("usb_attach_device(): reading device descriptor\r\n");
//    printf("Reading device descriptor\r\n");
    status = usb_read_device_descriptor(dev, sizeof(dev->descriptor));
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_dev_error(dev, "Failed to read device descriptor: %s\n", usb_status_string(status));
        pi_printf("error: fail to read device descriptor\r\n");
        return status;
    }

#if 0//!USB_EMBEDDED
    /* Read product and manufacturer strings if present.  */
    if (dev->descriptor.iProduct != 0)
    {
        usb_debug("Reading product string.\n");
        usb_get_ascii_string(dev, dev->descriptor.iProduct, dev->product, sizeof(dev->product));
    }
    if (dev->descriptor.iManufacturer != 0)
    {
        usb_debug("Reading manufacturer string.\n");
        usb_get_ascii_string(dev, dev->descriptor.iManufacturer, dev->manufacturer, sizeof(dev->manufacturer));
    }
#endif

    /* Read the first configuration descriptor.  */
//    usb_debug("Reading configuration descriptor.\n");
//    printf("Reading configuration descriptor\r\n");
    status = usb_read_configuration_descriptor(dev, 0);
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_dev_error(dev, "Failed to read configuration descriptor: %s\n", usb_status_string(status));
        pi_printf("error: fail to read configuration descriptor\r\n");
        return status;
    }

    /* Configure the device with its first reported configuration.  */
//    usb_dev_debug(dev, "Assigning configuration %u (%u interfaces available)\n",
//                  dev->config_descriptor->bConfigurationValue,
//                  dev->config_descriptor->bNumInterfaces);
//    printf("Device: %u, Assigning configuration %u (%u interfaces available)\r\n",
//    		dev->config_descriptor->bConfigurationValue, dev->config_descriptor->bNumInterfaces);
    status = usb_set_configuration(dev, dev->config_descriptor->bConfigurationValue);
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_dev_error(dev, "Failed to set device configuration: %s\n", usb_status_string(status));
    	pi_printf("error: fail to set device configuration\r\n");
        return status;
    }

   /* Report the device attachment at an informational log level.  */
//    usb_info("Attaching %s\n", usb_device_description(dev));
//    printf("Attaching %s\n", usb_device_description(dev));
    /* Try to bind a driver to the newly configured device. */
    status = usb_try_to_bind_device_driver(dev);

    if (status == USB_STATUS_DEVICE_UNSUPPORTED)
    {
        //usb_dev_info(dev, "No driver found for device.\n");
        pi_printf("infor: no driver found for device\r\n");
        /* No currently registered driver supports the new device.  However,
         * this should not be considered a failure to attach the device, since
         * the needed driver may just not be registered yet.  */
        status = USB_STATUS_SUCCESS;
    }
    else if (status != USB_STATUS_SUCCESS)
    {
        //usb_dev_error(dev, "Failed to bind driver to new USB device: %s\n", usb_status_string(status));
        pi_printf("error: fail to bind driver to new USB device\r\n");
    }
    return status;
}

static semaphore usb_bus_lock;

/**
 * @ingroup usbcore
 *
 * Prevent devices from being attached or detached to/from the USB.  This does
 * not prevent USB transfers from being issued on the bus.
 */
void usb_lock_bus(void)
{
    wait(usb_bus_lock);
}

/**
 * @ingroup usbcore
 *
 * Undo usb_lock_bus(), allowing devices to be attached or detached to/from the
 * USB.
 */
void usb_unlock_bus(void)
{
    //signal(usb_bus_lock);
}

/**
 * @ingroup usbcore
 *
 * Registers a USB device driver with the USB core.  At any time after this is
 * called, the USB core may execute the @ref usb_device_driver::bind_device
 * "bind_device" callback to try to bind the device driver to a USB device.
 *
 * This currently can safely be called before usbinit().
 *
 * @param drv
 *      Pointer to the USB device driver structure to register.
 *
 * @return
 *      ::USB_STATUS_SUCCESS if the driver was successfully registered or was
 *      already registered; otherwise another ::usb_status_t error code.
 */
usb_status_t
usb_register_device_driver(const struct usb_device_driver *drv)
{
    irqmask im;
    usb_status_t status;

    if (NULL == drv->bind_device)
    {
        //usb_error("bind_device function must be implemented\n");
    	printf("error: fail to bind_device function\r\n");
        return USB_STATUS_INVALID_PARAMETER;
    }

    im = disable();
    if (usb_num_device_drivers >= MAX_NUSBDRV)
    {
        //usb_error("Can't register new USB device driver: too many drivers already registered\n");
    	printf("error: fail to register new USB device\r\n");
        status = USB_STATUS_UNSUPPORTED_REQUEST;
    }
    else
    {
        boolean already_registered = FALSE;
        uint i;

        for (i = 0; i < usb_num_device_drivers; i++)
        {
            if (drv == usb_device_drivers[i])
            {
                already_registered = TRUE;
                break;
            }
        }
        if (!already_registered)
        {
            usb_device_drivers[usb_num_device_drivers++] = drv;
            //usb_info("Registered %s\n", drv->name);
            /* Check if a device compatible with this driver is already present
             * on the bus.  */
            usb_hub_for_device_in_tree(usb_root_hub, usb_try_to_bind_device_driver);
        }
        status = USB_STATUS_SUCCESS;
    }
    restore(im);
    return status;
}

/**
 * @ingroup usb
 *
 * Initializes the USB subsystem and corresponding USB and begins the USB device
 * enumeration process.  Since USB is a dynamic bus, the enumeration process
 * proceeds in an interrupt-driven manner and continues after this function
 * returns.  Thus, there is no guarantee that any given device will be
 * enumerated at any specific time, even after this function returns; this is
 * necessary by the design of USB itself.  USB device drivers will be bound to
 * devices as they are found; use usb_register_device_driver() to register a
 * device driver, which can be done at any time (either before or after calling
 * this function).
 *
 * @return
 *      OK on success; SYSERR if the USB subsystem was not successfully
 *      initialized.
 */
syscall usbinit(void)
{
    struct usb_device *root_hub;
    usb_status_t status;

    usb_bus_lock = semcreate(0);
//  pi_printf("infor: start usb init\r\n");

    if (isbadsem(usb_bus_lock))
    {
        goto err;
    }

    status = usb_register_device_driver(&usb_hub_driver);

    if (status != USB_STATUS_SUCCESS)
    {
        goto err_free_usb_bus_lock;
    }
//    printf("infor: start hcd init\r\n");
    status = hcd_start();
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_error("Failed to start USB host controller: %s\n", usb_status_string(status));
        pi_printf("error: fail to start USB host controller\r\n");
        goto err_free_usb_bus_lock;
    }

    //usb_debug("Successfully started USB host controller\n");
//    pi_printf("infor: Successfully started USB host controller\r\n");

    root_hub = usb_alloc_device(NULL);

    //usb_debug("Attaching root hub\n");
//    pi_printf("infor: attach root hub\r\n");
    status = usb_attach_device(root_hub);
    if (status != USB_STATUS_SUCCESS)
    {
        //usb_error("Failed to attach root hub: %s\n", usb_status_string(status));
        pi_printf("error: fail to attach root hub\r\n");
        goto err_free_root_hub;
    }

    usb_root_hub = root_hub;
    //usb_debug("Successfully initialized USB subsystem\n");

//    pi_printf("infor: Successfully initialized USB subsystem\r\n");
    usb_unlock_bus();
    return OK;

err_free_root_hub:
    usb_free_device(root_hub);
    hcd_stop();
err_free_usb_bus_lock:
    semfree(usb_bus_lock);
err:
    return SYSERR;
}
