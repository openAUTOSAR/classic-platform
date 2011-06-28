/*************************************************************************
 *	          			Freescale Confidential Proprietary      						 *
 *           Copyright (C) 2005-2007 by Freescale. All rights reserved.  *
 *************************************************************************
 *                                                                       *
 *   Freescale reserves the right to make changes without further notice *
 *   to any product herein to improve reliability, function or design.   *
 *   Freescale does not assume any liability arising out of the          *
 *   application or use of any product, circuit, or software described   *
 *   herein; neither does it convey any license under its patent rights  *
 *   nor the rights of others.                                           *
 *                                                                       *
 *   Freescale products are not designed, intended, or authorized for    *
 *   use as components in systems intended for surgical implant into     *
 *   the body, or other applications intended to support life, or for    *
 *   any other application in which the failure of the Freescale product *
 *   could create a situation where personal injury or death may occur.  *
 *                                                                       *
 *   Should Buyer purchase or use Freescale products for any such        *
 *   unintended or unauthorized application, Buyer shall indemnify and   *
 *   hold Freescale and its officers, employees, subsidiaries,           *
 *   affiliates, and distributors harmless against all claims costs,     *
 *   damages, and expenses, and reasonable attorney fees arising out     *
 *   of, directly or indirectly, any claim of personal injury or death   *
 *   associated with such unintended or unauthorized use, even if such   *
 *   claim alleges that Freescale was negligent regarding the design     *
 *   or manufacture of the part.                                         *
 *                                                                       *
 *   Freescale and the Freescale logo* are registered trademarks of      *
 *   Freescale Semicondutor, Inc.                                        *
 *                                                                       *
 *************************************************************************

 *************************************************************************
 *                                                                       *
 *               Standard Software H7F Driver for MPC55xx                *
 *                                                                       *
 * FILE NAME     :  ssd_c90fl_internal.h                                     *
 * DATE          :  December 07, 2007                                    *
 * AUTHOR        :  Arvind Awasthi                                       *
 * E-mail        :  r56611@freescale.com                                 *
 *                                                                       *
 *************************************************************************/

/******************************* CHANGES *********************************
 1.0   2007.12.07               Arvind Awasthi          Initial Version
 *************************************************************************/

#ifndef _C90FL_INTERNAL_H_
#define _C90FL_INTERNAL_H_

/* cycles counter used in FlashResume */
/* about 500ns at 400MHz system clock */
#define FLASH_RESUME_WAIT   15

/*************************************************************************/
/*  CallBack function period                                             */
/*************************************************************************/
#define CALLBACK_BC         90
#define CALLBACK_PV         80
#define CALLBACK_CS         120

#endif  /* _C90FL_INTERNAL_H_ */
