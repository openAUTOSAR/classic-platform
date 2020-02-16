/*
 * usb_util.h
 *
 *  Created on: 31 jan 2014
 *      Author: zsz
 */

#ifndef USB_UTIL_H_
#define USB_UTIL_H_

#include <stdint.h>
#include "Std_Types.h"
//from kernel.h
/** Type-independent macro to calculate the minimum of 2 values.  */
#define min(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); \
                            (_a < _b) ? _a : _b; })

/** Type-independent macro to calculate the maximum of 2 values.  */
#define max(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); \
                            (_a > _b) ? _a : _b; })

/** Perform integer division, rounding up the quotient.  */
#define DIV_ROUND_UP(num, denom) (((num) + (denom) - 1) / (denom))

/** Get the number of elements in an array (not dynamically allocated)  */
#define ARRAY_LEN(array) (sizeof(array) / sizeof((array)[0]))


//from thread.h
#define BADTID      (-1)        /**< used when invalid tid needed       */


/**********************************************************************
 * from compiler.h
 ***********************************************************************/
/** Packed attribute:
 * Annotate a structure with this to lay it out with no padding bytes.  */
#define __packed                    __attribute__((packed))

/** Type of expression:
 * typeof(x) evaluates to the C type of x.  */
#define typeof __typeof__

/** Static assertion:
 * Assert that a predicate is true at compilation time.  This generates no code
 * in the resulting binary.  */
#define STATIC_ASSERT(condition) ((void)sizeof(char[1 - 2*!(condition)]))

/** Aligned attribute:
 * Annotate a member or buffer with this to align it on the specified byte
 * boundary.  */
#define __aligned(n)                __attribute__((aligned(n)))


/**********************************************************************/
/* macro to get offset to struct members */
#define offset(type, member) ((ulong)&(((type *)0)->member))

/* Base type definitions */
typedef unsigned char uchar;    /**< unsigned char type                 */
typedef unsigned short ushort;  /**< unsigned short type                */
typedef unsigned int uint;      /**< unsigned int type                  */
typedef unsigned long ulong;    /**< unsigned long type                 */

//typedef unsigned long size_t;   /**< size type                          */

/* Function declaration return types */
typedef int syscall;            /**< system call declaration            */
typedef int devcall;            /**< device call declaration            */
typedef int shellcmd;           /**< shell command declaration          */
typedef int thread;             /**< thread declaration                 */
typedef void interrupt;         /**< interrupt procedure                */
typedef void exchandler;        /**< exception procedure                */
typedef int message;            /**< message passing content            */

typedef int tid_typ;            /**< thread ID type                     */

///* Boolean type and constants */
//#define FALSE        0          /**< boolean false                      */
//#define TRUE         1          /**< boolean true                       */

/* Universal return constants */
#define OK        1             /**< system call ok                     */
#define SYSERR   (-1)           /**< system call failed                 */
//#define EOF      (-2)           /**< End-of-file (usually from read)    */
#define TIMEOUT  (-3)           /**< system call timed out              */
#define NOMSG    (-4)           /**< no message to receive              */

/**********************************************************************/

struct usb_device;

/**********************************************************************/

/* Configuration variables (could be moved somewhere else).  */

/** Enable USB "embedded" mode: if set to TRUE, there will be no USB debugging,
 * info, or error messages, and the 'usbinfo' shell command will not be
 * available.  This will reduce the compiled code size significantly.  */
#define USB_EMBEDDED                0

/** Minimum priority for USB messages.  Only messages with priority greater than
 * or equal to this will be printed.  */
#define USB_MIN_LOG_PRIORITY        4

/**********************************************************************/

/** Priority for USB error messages.  */
#define USB_LOG_PRIORITY_ERROR      3

/** Priority for USB informational messages, such as a device being attached or
 * detached.  */
#define USB_LOG_PRIORITY_INFO       2

/** Priority for USB debugging messages.  */
#define USB_LOG_PRIORITY_DEBUG      1

#if USB_EMBEDDED && USB_MIN_LOG_PRIORITY <= USB_LOG_PRIORITY_ERROR
#  undef USB_MIN_LOG_PRIORITY
#  define USB_MIN_LOG_PRIORITY USB_LOG_PRIORITY_ERROR + 1
#endif

#if USB_LOG_PRIORITY_ERROR >= USB_MIN_LOG_PRIORITY
void usb_log(int priority, const char *func,
             struct usb_device *dev, const char *format, ...)
                __printf_format(4, 5);
#endif

#if USB_LOG_PRIORITY_ERROR >= USB_MIN_LOG_PRIORITY
#  define usb_dev_error(dev, format, ...) \
        usb_log(USB_LOG_PRIORITY_ERROR, __func__, dev, format, ##__VA_ARGS__)
#else
#  define usb_dev_error(dev, format, ...)
#endif

#if USB_LOG_PRIORITY_INFO >= USB_MIN_LOG_PRIORITY
#  define usb_dev_info(dev, format, ...) \
        usb_log(USB_LOG_PRIORITY_INFO, __func__, dev, format, ##__VA_ARGS__)
#else
#  define usb_dev_info(dev, format, ...)
#endif

#if USB_LOG_PRIORITY_DEBUG >= USB_MIN_LOG_PRIORITY
#  define usb_dev_debug(dev, format, ...) \
        usb_log(USB_LOG_PRIORITY_DEBUG, __func__, dev, format, ##__VA_ARGS__)
#else
#  define usb_dev_debug(dev, format, ...)
#endif

#define usb_error(format, ...) usb_dev_error(NULL, format, ##__VA_ARGS__)
#define usb_info(format, ...)  usb_dev_info (NULL, format, ##__VA_ARGS__)
#define usb_debug(format, ...) usb_dev_debug(NULL, format, ##__VA_ARGS__)

/** Status code returned by many functions in the USB subsystem.  The generic
 * Xinu SYSERR does not provide enough information in many cases.  */
typedef enum usb_status {

    /** Function successful.  */
    USB_STATUS_SUCCESS                   =  0,

    /** USB device was detached.  */
    USB_STATUS_DEVICE_DETACHED           = -1,

    /** USB device is unsupported by the driver.  */
    USB_STATUS_DEVICE_UNSUPPORTED        = -2,

    /** Hardware error of some form occurred.  */
    USB_STATUS_HARDWARE_ERROR            = -3,

    /** Invalid data was received.  */
    USB_STATUS_INVALID_DATA              = -4,

    /** An invalid parameter was passed to the function.  */
    USB_STATUS_INVALID_PARAMETER         = -5,

    /** The USB transfer has not yet been processed.  */
    USB_STATUS_NOT_PROCESSED             = -6,

    /** Failed to allocated needed memory.  */
    USB_STATUS_OUT_OF_MEMORY             = -7,

    /** The operation timed out.  */
    USB_STATUS_TIMEOUT                   = -8,

    /** The request is unsupported.  */
    USB_STATUS_UNSUPPORTED_REQUEST       = -9,
} usb_status_t;

#if !USB_EMBEDDED
const char *usb_status_string(usb_status_t status);
#endif


#endif /* USB_UTIL_H_ */
