/*
 * usb_stddef.h
 *
 *  Created on: 2013-10-31
 *      Author: Zhang Shuzhou
 */

#ifndef USB_STDDEF_H_
#define USB_STDDEF_H_

/* macro to get offset to struct members */
#define offsetof(type, member) ((size_t)&(((type *)0)->member))

/* Base type definitions */
typedef unsigned char uchar;    /**< unsigned char type                 */
typedef unsigned short ushort;  /**< unsigned short type                */
typedef unsigned int uint;      /**< unsigned int type                  */
typedef unsigned long ulong;    /**< unsigned long type                 */
//typedef char boolean;           /**< boolean type                       */
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

/* Boolean type and constants */
//#define FALSE        0          /**< boolean false                      */
//#define TRUE         1          /**< boolean true                       */

/* Universal return constants */
#define OK        1             /**< system call ok                     */
#define NULL      0             /**< null pointer for linked lists      */
#define SYSERR   (-1)           /**< system call failed                 */
#define EOF      (-2)           /**< End-of-file (usually from read)    */
#define TIMEOUT  (-3)           /**< system call timed out              */
#define NOMSG    (-4)           /**< no message to receive              */


#endif /* USB_STDDEF_H_ */
