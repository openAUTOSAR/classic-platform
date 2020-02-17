/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 *
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef LDCOM_INTERNAL_H_
#define LDCOM_INTERNAL_H_

 typedef enum {
    LDCOM_STATE_UNINIT,   /* Status of LdCom module before LdCom_Init function*/
    LDCOM_STATE_INIT,     /* Status of LdCom module after LdCom_Init function called*/
}LdCom_StateType;

 typedef struct {
    LdCom_StateType initStatus;  /* var to hold LdCom module status */
}LdCom_InternalType;

#endif /*LDCOM_INTERNAL_H*/
