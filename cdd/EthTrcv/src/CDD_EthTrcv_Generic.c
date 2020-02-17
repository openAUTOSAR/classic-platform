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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=NONE */
/** @tagSettings DEFAULT_ARCHITECTURE=ZYNQ */

/* This file Supports only CLAUSE 22 MDIO frame type */
/* This file Supports only one PHY access, no multiple PHY access */

#include "Eth.h"
#include "CDD_EthTrcv.h"
#include "LwIpAdp.h"
#if defined(USE_DET)
#include "Det.h"
#define DET_REPORT_ERROR(_api,_err) (void)Det_ReportError(ETH_MODULE_ID,0,(_api),(_err))
#else
#define DET_REPORT_ERROR(_api,_err)
#endif

#define BMCR_REGISTER    			     0u
#define BMSR_REGISTER    			     1u
#define PHY_IDR1                         2u
#define PHY_IDR2                         3u
#define AUTONEG_ADVERTISE_REGISTER	     4u
#define PHY_SOFT_RESET                	 0x8000u
#define AUTONEG_RESTART                	 0x0200u
#define ENABLE_AUTO_NEGOTIATION          0x1000u
#define ENABLE_100MBPS_DUPLEX            0x2100u
#define AUTO_NEG_COMPLETE                0x0020u

#define RGMII_TXRX_CLK_DELAY    	     0x0030
#define ASYMMETRIC_PAUSE    		     0x0800
#define PAUSE   					     0x0400
#define LINKSPEED_1000MBPS		         0x0040
#define LINKSPEED_100MBPS			     0x2000
#define LINKSPEED_10MBPS			     0x0000

#define LINK_STATUS                      0x0004u

static uint8  PhyAddress = ETHTRCV_MAX_ADDRESS; /* Invalid address or Phy is not initialised in this context */
static LinkState  linkStatus = NO_LINK;

/**
 * Fetch PHY address:: mock up function for ETH tranciever
 * @param ctrlIdx
 * @return phyAddr
 */
uint8 EthTrcv_FindPhyAdrs(uint8 CtrlIdx)
{
    uint8 phyAddr;
      for (phyAddr = 0; phyAddr < ETHTRCV_MAX_ADDRESS; phyAddr++) {
          uint8 Status;
          uint16 Reg1;
          uint16 Reg2;
          Status =  Eth_ReadMii(CtrlIdx,phyAddr, PHY_IDR1, &Reg1);
          Status |= Eth_ReadMii(CtrlIdx,phyAddr, PHY_IDR2, &Reg2); /* change it */

          if ((Status == ETH_OK) &&
              (Reg1 > 0x0000) && (Reg1 < 0xffff) &&
              (Reg2 > 0x0000) && (Reg2 < 0xffff)) {
              return phyAddr;
          }
      }
      return phyAddr;
}

/**
 * Fetch PHY address from the configuration
 * @param CtrlIdx
 * @return phyAddr
 */
#if (STD_OFF == ETHTRCV_AUTO_FIND_ADDRESS)
uint8 EthTrcv_FindConfiguredPhyAdrs(uint8 CtrlIdx)
{
    for(uint32 i = 0; i < ETHTRCV_ADDRESS_CNT; i++){
		uint8 Status;
		uint16 Reg1;
		uint16 Reg2;
		Status =  Eth_ReadMii(CtrlIdx,EthTrcvConfigSet.TrcvAdrs[i].address, 2, &Reg1);
		Status |= Eth_ReadMii(CtrlIdx,EthTrcvConfigSet.TrcvAdrs[i].address, 3, &Reg2); /* change it */

		if ((Status == ETH_OK) &&
			(Reg1 > 0x0000) && (Reg1 < 0xffff) &&
			(Reg2 > 0x0000) && (Reg2 < 0xffff)) {
			return EthTrcvConfigSet.TrcvAdrs[i].address;
		}
    }
    return ETHTRCV_MAX_ADDRESS;

}
#endif

/**
 * Link Setting up - Needed to be called from a cyclic routine
 * precondition - phyAddress is valid
 * @param CtrlIdx
 * @return
 */
void EthTrcv_TransceiverLinkUp(uint8 CtrlIdx)
{
    if(PhyAddress != ETHTRCV_MAX_ADDRESS){
        uint16 statusValue = 0;
        if(linkStatus == NO_LINK)
        {
        	(void)EthTrcv_TransceiverInit(CtrlIdx);
        }
        Eth_ReadMii(CtrlIdx,PhyAddress,BMSR_REGISTER,&statusValue);
        if((statusValue & LINK_STATUS ) != LINK_STATUS){
            linkStatus = LINK_UP;
        }else{
            linkStatus = LINK_DOWN;
        }

        LwIpAdp_LinkStateUpdate(linkStatus);
    }else{
    	(void)EthTrcv_TransceiverInit(CtrlIdx);
    }
}

/**
  * @brief  Mock up function for ETH Tranceiver : Initialise tranciever required
  * @param  ctrlIdx
  * @retval Std_ReturnType
  */
Std_ReturnType EthTrcv_TransceiverInit(uint8 CtrlIdx)
{
	uint16 phyStatus;
    uint16 regValue;

    (void)Eth_WriteMii(CtrlIdx,PhyAddress, BMCR_REGISTER, PHY_SOFT_RESET);
    for(volatile uint16 i=0; i <100u; i++){};

#if (STD_OFF == ETHTRCV_AUTO_FIND_ADDRESS)
    PhyAddress = EthTrcv_FindConfiguredPhyAdrs(CtrlIdx);
#else
    PhyAddress = EthTrcv_FindPhyAdrs(CtrlIdx);
#endif
    if(PhyAddress == ETHTRCV_MAX_ADDRESS){ /* Could still not find the Phy connected */
        return E_NOT_OK;
    }

    enum{
       ETH_SPEED_10MBPS,
       ETH_SPEED_100MBPS,
       ETH_SPEED_1000MBPS
    };
    uint16 speed = ETH_SPEED_100MBPS;

    Eth_ReadMii(CtrlIdx, PhyAddress, BMCR_REGISTER, &regValue);
    regValue &= ~LINKSPEED_1000MBPS;
    regValue &= ~LINKSPEED_100MBPS;
    regValue &= ~LINKSPEED_10MBPS;

    if(speed == ETH_SPEED_100MBPS)	{
        regValue |=  LINKSPEED_100MBPS;
    }
    else if (speed == ETH_SPEED_1000MBPS)    {
        regValue |=  LINKSPEED_1000MBPS;
    }
    else {
        regValue |=  LINKSPEED_10MBPS;
    }

#if (ETHTRCV_AUTO_NEGOTIATE == STD_OFF)
	regValue &= ~ENABLE_AUTO_NEGOTIATION; // Disable AN
#else
    Eth_WriteMii( CtrlIdx,PhyAddress, 9, 0); /* Dont advertise PHY speed of 1000 Mbps */
    Eth_WriteMii(CtrlIdx,PhyAddress, AUTONEG_ADVERTISE_REGISTER,  0x0100 |0x00E0); // advertise 100mbps and 10mbps
	regValue = regValue | AUTONEG_RESTART | ENABLE_AUTO_NEGOTIATION;  // Enable AN & Restart autoneg
#endif

#ifdef CFG_ETHTRCV_PHYLOOPBACK /* physical line test, set in build_config for use */
	regValue &= ~ENABLE_AUTO_NEGOTIATION; // disable AN
	(void)Eth_WriteMii(CtrlIdx,PhyAddress, BMCR_REGISTER, (regValue | 0x6100)); // loopback with speed 100
#endif

    Eth_WriteMii(CtrlIdx,PhyAddress,BMCR_REGISTER, regValue);

#if (ETHTRCV_AUTO_NEGOTIATE == STD_ON)
    do{
    	// Wait for Auto-Neg complete and link status flag
    	Eth_ReadMii(CtrlIdx,PhyAddress,BMSR_REGISTER,&phyStatus);
    }while ((phyStatus & (AUTO_NEG_COMPLETE | LINK_STATUS)) != (AUTO_NEG_COMPLETE | LINK_STATUS));
#else
    do{
    	// Wait for link status flag
    	Eth_ReadMii(CtrlIdx,PhyAddress,BMSR_REGISTER,&phyStatus);
    }while ((phyStatus & LINK_STATUS )!= LINK_STATUS);
#endif

    linkStatus = LINK_UP;
    LwIpAdp_LinkStateUpdate(LINK_UP);

    return E_OK;
}
