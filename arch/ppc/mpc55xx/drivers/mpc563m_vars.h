/**************************************************************************
 * FILE NAME: $RCSfile: mpc563m_vars.h,v $   COPYRIGHT (c) Freescale 2005 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * Variables that define some features of the MPC563M.                    *
 * !!!!This file must only be included once in every project!!!!          *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson [r47354]                                *
 * $Log: mpc563m_vars.h,v $
 * Revision 1.1  2007/11/09 08:48:41  r47354
 * *** empty log message ***
 *

 *
 **************************************************************************/

/* eTPU characteristics definition */
struct eTPU_struct *eTPU = (struct eTPU_struct *)0xC3FC0000;

uint32_t fs_etpu_code_start =     0xC3FD0000;
uint32_t fs_etpu_data_ram_start = 0xC3FC8000;
uint32_t fs_etpu_data_ram_end =   0xC3FC89FC;
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


