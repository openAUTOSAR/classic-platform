
/**************************************************************************
 * FILE NAME: $RCSfile: mpc5554_vars.h,v $     COPYRIGHT (c) Freescale 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * Variables that define some features of the MPC5554.                    *
 * !!!!This file must only be included once in every project!!!!          *
 *========================================================================*
 * ORIGINAL AUTHOR: Jeff Loeliger [r12110]                                *
 * $Log: mpc5554_vars.h,v $
 * Revision 1.1  2004/12/02 13:45:26  r12110
 * -First version of file.
 *
 **************************************************************************/

/* eTPU characteristics definition */
struct eTPU_struct *eTPU = (struct eTPU_struct *)0xC3FC0000;

uint32_t fs_etpu_code_start =     0xC3FD0000;
uint32_t fs_etpu_data_ram_start = 0xC3FC8000;
uint32_t fs_etpu_data_ram_end =   0xC3FC8BFC;
uint32_t fs_etpu_data_ram_ext =   0xC3FCC000;

/*********************************************************************
 *
 * Copyright:
 *	Freescale Semiconductor, INC. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is
 *  retained without alteration in any modified and/or redistributed
 *  versions, and that such modified versions are clearly identified
 *  as such. No licenses are granted by implication, estoppel or
 *  otherwise under any patents or trademarks of Freescale
 *  Semiconductor, Inc. This software is provided on an "AS IS"
 *  basis and without warranty.
 *
 *  To the maximum extent permitted by applicable law, Freescale
 *  Semiconductor DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,
 *  INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *  PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
 *  REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF)
 *  AND ANY ACCOMPANYING WRITTEN MATERIALS.
 *
 *  To the maximum extent permitted by applicable law, IN NO EVENT
 *  SHALL Freescale Semiconductor BE LIABLE FOR ANY DAMAGES WHATSOEVER
 *  (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
 *  BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER
 *  PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.
 *
 *  Freescale Semiconductor assumes no responsibility for the
 *  maintenance and support of this software
 *
 ********************************************************************/


