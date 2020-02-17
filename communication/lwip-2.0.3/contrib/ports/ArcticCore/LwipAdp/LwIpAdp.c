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

#include "LwIpAdp.h"
#if defined(USE_ETHSM)
#include "EthSM.h"
#include "EthSM_Cbk.h"
#if (ETHSM_NOF_ETHIF_CONTROLLERS != 1)
#error "LWIP used assumes to have only one instance of Ethernet interface at the moment"
#endif
#endif


typedef struct {
    LinkState         linkState;
} LwIpAdp_InternalType;

static LwIpAdp_InternalType LwIpAdp_Intern = {
    .linkState  = LINK_DOWN
};

/**
  * @brief Update function of  tranciever link state from LWIP on behalf of ASR Eth Trcv module
  * @param  state
  * @retval none
  */
void LwIpAdp_LinkStateUpdate(LinkState State){

    if( (LwIpAdp_Intern.linkState != LINK_UP) && (State == LINK_UP)){
        LwIpAdp_Intern.linkState = LINK_UP;
#if defined(USE_ETHSM) &&  (ETHSM_DUMMY_MODE == STD_OFF)
        EthSM_TrcvLinkStateChg((ETHSM_NOF_ETHIF_CONTROLLERS-1),ETHTRCV_LINK_STATE_ACTIVE);
#endif
    }
    else if((LwIpAdp_Intern.linkState == LINK_UP) && (State != LINK_UP)){
        LwIpAdp_Intern.linkState = State;
#if defined(USE_ETHSM) &&  (ETHSM_DUMMY_MODE == STD_OFF)
        EthSM_TrcvLinkStateChg((ETHSM_NOF_ETHIF_CONTROLLERS-1),ETHTRCV_LINK_STATE_DOWN);
#endif
    }
    else{
        /* Do nothing */
    }
}

