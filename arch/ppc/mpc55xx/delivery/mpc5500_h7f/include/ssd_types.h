
/*************************************************************************
 *            (c) Copyright Motorola 2005, All Rights Reserved           *
 *************************************************************************
 *                                                                       *
 *   Motorola reserves the right to make changes without further notice  *
 *   to any product herein to improve reliability, function or design.   *
 *   Motorola does not assume any liability arising out of the           *
 *   application or use of any product, circuit, or software described   *
 *   herein; neither does it convey any license under its patent rights  *
 *   nor the rights of others.                                           *
 *                                                                       *
 *   Motorola products are not designed, intended, or authorized for     *
 *   use as components in systems intended for surgical implant into     *
 *   the body, or other applications intended to support life, or for    *
 *   any other application in which the failure of the Motorola product  *
 *   could create a situation where personal injury or death may occur.  *
 *                                                                       *
 *   Should Buyer purchase or use Motorola products for any such         *
 *   unintended or unauthorized application, Buyer shall indemnify and   *
 *   hold Motorola and its officers, employees, subsidiaries,            *
 *   affiliates, and distributors harmless against all claims costs,     *
 *   damages, and expenses, and reasonable attorney fees arising out     *
 *   of, directly or indirectly, any claim of personal injury or death   *
 *   associated with such unintended or unauthorized use, even if such   *
 *   claim alleges that Motorola was negligent regarding the design      *
 *   or manufacture of the part.                                         *
 *                                                                       *
 *   Motorola and the Motorola logo* are registered trademarks of        *
 *   Motorola Ltd.                                                       *
 *                                                                       *
 *************************************************************************

 *************************************************************************
 *                                                                       *
 *               Standard Software H7F Driver for MPC55xx                *
 *                                                                       *
 * FILE NAME     :  ssd_types.h                                          *
 * DATE          :  June 23, 2005                                        *
 *                                                                       *
 * AUTHOR        :  Flash Team,                                          *
 *                  Global Software Group, China, Motorola Inc.          *
 * E-mail        :  flash@sc.mcel.mot.com                                *
 *                                                                       *
 *************************************************************************/

/******************************* CHANGES *********************************
 3.20   2005.06.23       Cloud Li        Initial Version
 *************************************************************************/

#ifndef _SSD_TYPES_H_
#define _SSD_TYPES_H_

/*************************************************************************/
/*  SSD general data types                                               */
/*************************************************************************/

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

typedef unsigned char BOOL;

typedef signed char INT8;
typedef unsigned char UINT8;
typedef volatile signed char VINT8;
typedef volatile unsigned char VUINT8;

typedef signed short INT16;
typedef unsigned short UINT16;
typedef volatile signed short VINT16;
typedef volatile unsigned short VUINT16;

typedef signed long INT32;
typedef unsigned long UINT32;
typedef volatile signed long VINT32;
typedef volatile unsigned long VUINT32;

typedef signed long long INT64;
typedef unsigned long long UINT64;
typedef volatile signed long long VINT64;
typedef volatile unsigned long long VUINT64;

#endif  /* _SSD_TYPES_H_ */
