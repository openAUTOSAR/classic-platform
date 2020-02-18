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
/** @tagSettings DEFAULT_ARCHITECTURE=RH850F1H|JACINTO6|MPC5748G */

/* This file Supports only CLAUSE 22 MDIO frame type */
/* This file Supports only one PHY access, no multiple PHY access */

#include "Eth.h"
#include "CDD_EthTrcv.h"
#if defined(CFG_JAC6)
#include "Eth_jacinto.h" /* Can directly access the ETH registers for the link status  in J6 */
#endif
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
#define ENABLE_AUTO_NEGOTIATION          0x1000u
#define ENABLE_100MBPS_DUPLEX            0x2100u
#define AUTO_NEG_COMPLETE                0x0020u
#if defined(CFG_ETH_PHY_DP83848C)
#define PHY_STATUS_REGISTER     	     0x10u
#define PHY_STATUS_LINK                  0x0001u
#define PHY_STATUS_NO_AUTO_100           0x0005u
#define PHY_STATUS_LOOPBACK              0x0008u
#elif defined (CFG_ETH_PHY_TJA1100)
#define PHY_STATUS_REGISTER     	     0x01u
#define PHY_EXTENDED_STATUS_REGISTER     15u
#define PHY_EXTENDED_CONTROL_REGISTER    17u
#define PHY_CONFIG_REGISTER1             18u
#define PHY_STATUS_LINK                  0x0004u
#define PHY_STATUS_NO_AUTO_100           0xE600u /*100BASE-T4,100BASE-X_FD,100BASE-X_HD,100BASE-T2_FD,100BASE-T2_HD*/
#define PHY_EXTENDED_STATUS_NO_AUTO_100  0x0080u
#define PHY_CONFIG_EN_BIT                (2u)
#define PHY_CONFIG_MASTER_SLAVE_BIT      (15u)
#else /* CFG_ETH_PHY_DP83865 */
#define PHY_STATUS_REGISTER     	     0x11u
#define PHY_STATUS_LINK                  0x0004u
#define PHY_STATUS_NO_AUTO_100           0x000Eu
#define PHY_STATUS_LOOPBACK              0x0040u
#define RGMII_INBAND_STATUS_EN           0x0100U
#endif

/* CLAUSE 45 access via CLAUSE 22 reserved registers */
#define PHY_MMD_ACCESS_CTRL_REG          13u
#define PHY_MMD_ACCESS_ADRS_REG	         14u

/* For MMD access control register */
#define PHY_MMD_DEVAD_MASK                  0x001Fu
#define PHY_MMD_FUNCTION_ADDRESS_SHIFT      14u
#define PHY_MMD_FUNCTION_ADDRESS            00u
#define PHY_MMD_FUNCTION_DATA_NO_POST_INC   01u
#define PHY_MMD_FUNCTION_DATA_POST_INC_RW   02u
#define PHY_MMD_FUNCTION_DATA_POST_INC_W    03u

/* MMD addresses */
#define PHY_MMD_RESERVED1                00u
#define PHY_MMD_PMA_PMD                  01u
#define PHY_MMD_WIS                      02u
#define PHY_MMD_PCS                      03u
#define PHY_MMD_COM_MAC                  04u /* PHY_MMD_PHY_XS */
#define PHY_MMD_DTE_XS                   05u
#define PHY_MMD_TC                       06u
#define PHY_MMD_AUTONEG                  07u
#define PHY_MMD_CLAUSE22_EXT             29u
#define PHY_MMD_VENDOR_SPECIFIC_1        30u
#define PHY_MMD_VENDOR_SPECIFIC_2        31u

/* MMD PMA/PMD register space */
#define PHY_PMA_PMD_CONTROL1             00u
#define PHY_PMA_PMD_STATUS1              01u
#define PHY_PMA_PMD_DEV_IDENTIFIER1      02u
#define PHY_PMA_PMD_DEV_IDENTIFIER2      03u
#define PHY_PMA_PMD_100BASET1_CONTROL    (0x0834)
#define PHY_PMA_PMD_100BASET1_TMODE_CONTROL     (0x0836)
#define PHY_PMA_PMD_10MBPS               00u
#define PHY_PMA_PMD_100MBPS              01u
#define PHY_PMA_PMD_1000MBPS             02u
#define PHY_PMA_PMD_100BASET1_MASTER     (1<<14)
#define PHY_PMA_PMD_TMODE_NORMAL_OPN     (0)

/* MMD COM PORT register space */
#define PHY_COM_PORT_CONTROL_REG         (0x0800)

/* MMD CONTROL register space */
#define PHY_PCS_COPPER_CONTROL           00u
#define PHY_100BASET1_STATUS1            (0x8008)
#define PHY_100BASET1_STATUS2            (0x8009)
#define PHY_POWER_DOWN                   (1<<11)


/* MMD AUTO NEGOTIATION space  */
#define PHY_AUTONEG_CONTROL              (0x0200)
#define PHY_AUTONEG_STATUS               (0x0201)
#define PHY_AUTONEG_ADV_REG1             (0x0202)
#define PHY_AUTONEG_ADV_REG2             (0x0203)
#define PHY_AUTONEG_ENABLE               (1u<<12)
#define PHY_RESTART_AUTONEG              (1u<<9)
#define PHY_AUTO_NEG_MASTER              (1<<4)


#define PHY_COMMON_CONTROL_REG           (0x0000) /* Control register for device PMA/PMD and PCS */


#ifdef CFG_BRD_ZYNQ_ZC702
#define PORT_MIO_52                      0xF80007D0
#define PORT_MIO_53                      0xF80007D4
#endif

#if defined(CFG_JAC6)
#define GMAC_SW_MDIO_LINKSEL             0x00000080U
#define GMAC_SW_MDIO_LINKINTENB          0x00000040U
#define GMAC_SW_MDIO_PHYADDRMON_MASK     0x0000001FU
static GmacSw_HwRegType * const hwPtr = (GmacSw_HwRegType*) GMAC_SW_BASE;
#endif

/* PHY device - MARVELL-88E1111 */
#define RGMII_TXRX_CLK_DELAY    	     0x0030
#define ASYMMETRIC_PAUSE    		     0x0800
#define PAUSE   					     0x0400
#define LINKSPEED_1000MBPS		         0x0040
#define LINKSPEED_100MBPS			     0x2000
#define LINKSPEED_10MBPS			     0x0000

/* Max wait cycles for actions to occur in busy wait***** */
#define ETH_MAX_BUSY_WAIT_CYCLES        (uint32)1000

#define ETH_MDIO_WAIT_CYCLES             50u

/* Macro for busy waiting for a while */
#define BUSY_WAIT_A_WHILE(_regPtr, _mask, _val, _retPtr)\
    *(_retPtr) = E_NOT_OK;\
    for( uint32 i = 0; i < ETH_MAX_BUSY_WAIT_CYCLES; i++) {\
        if( (*(_regPtr) & (_mask)) == (_val) ) {\
            *(_retPtr) = E_OK;\
            break;\
        }\
    }\

static uint8  PhyAddress = ETHTRCV_MAX_ADDRESS; /* Invalid address or Phy is not initialised in this context */
static LinkState  linkStatus = NO_LINK;

static boolean bMdioClause45 = FALSE;

static Eth_ReturnType readMii45(uint8 CtrlIdx, uint8 Device, uint16 RegIdx, uint16* RegValPtr);
static Eth_ReturnType writeMii45(uint8 CtrlIdx,uint8 Device,uint16 RegIdx, uint16 RegVal);

/**
 * Makes Soft reset on the required MMD
 * @param CtrlIdx
 * @param Device
 * @return void
 */
static void softReset(uint8 CtrlIdx, uint8 Device) /* Device can be 1,3,4,7 */{
	uint16 regAddr = 0x0000;
	uint16 regData = 0;

	if (Device == PHY_MMD_AUTONEG){
		regAddr = PHY_AUTONEG_CONTROL; /* soft  reset for T unit */
	}else if (Device == PHY_MMD_COM_MAC){
		/* soft reset outside Copper PCS , that means reset for RGMII/RMII */
		regAddr = PHY_COM_PORT_CONTROL_REG;
	}else{
		regAddr = PHY_COMMON_CONTROL_REG; /* affects all - dev1,dev3 and dev7 */
	}

	(void) readMii45(CtrlIdx, Device, regAddr, &regData);
	regData = regData | PHY_SOFT_RESET;  //bits 15 for reset
	(void) writeMii45(CtrlIdx, Device, regAddr,regData);

	(void) readMii45(CtrlIdx, Device, regAddr, &regData);
	while ((regData & PHY_SOFT_RESET) == PHY_SOFT_RESET) {
		(void) readMii45(CtrlIdx, Device, regAddr, &regData);
	}	
}

/**
 * Reads a IEEE 803.u clause 45 compliant transceiver register via IEEE 803.u clause 22
 * Clause 22 STA <------ Clause 45 MMD
 * @param CtrlIdx
 * @param Device
 * @param RegIdx
 * @param RegValPtr
 */
static Eth_ReturnType readMii45(uint8 CtrlIdx, uint8 Device, uint16 RegIdx, uint16* RegValPtr){
	uint8 status;
	uint16 reg = 0u;

	reg = Device | (PHY_MMD_FUNCTION_ADDRESS << PHY_MMD_FUNCTION_ADDRESS_SHIFT);

	status =  Eth_WriteMii(CtrlIdx,PhyAddress, PHY_MMD_ACCESS_CTRL_REG, reg);
	status |= Eth_WriteMii(CtrlIdx,PhyAddress, PHY_MMD_ACCESS_ADRS_REG, RegIdx);

	reg = Device | (PHY_MMD_FUNCTION_DATA_NO_POST_INC << PHY_MMD_FUNCTION_ADDRESS_SHIFT);

	status |=  Eth_WriteMii(CtrlIdx,PhyAddress, PHY_MMD_ACCESS_CTRL_REG, reg);
	status |=  Eth_ReadMii (CtrlIdx,PhyAddress, PHY_MMD_ACCESS_ADRS_REG, RegValPtr);
	if (status != ETH_OK) {
		DET_REPORT_ERROR(ETH_GLOBAL_ID, ETH_E_READ_MII_FAIL);
	}
	return status;
}

/**
 * Writes in to a IEEE 803.u clause 45 compliant transceiver register via IEEE 803.u clause 22
 * Clause 22 STA ------> Clause 45 MMD
 * @param CtrlIdx
 * @param Device
 * @param RegIdx
 * @param RegVal
 */
static Eth_ReturnType writeMii45(uint8 CtrlIdx,uint8 Device,uint16 RegIdx, uint16 RegVal){
	uint8 status;
	uint16 reg = 0u;

	reg = Device | (PHY_MMD_FUNCTION_ADDRESS << PHY_MMD_FUNCTION_ADDRESS_SHIFT);

	status =  Eth_WriteMii(CtrlIdx,PhyAddress, PHY_MMD_ACCESS_CTRL_REG, reg);
	status |= Eth_WriteMii(CtrlIdx,PhyAddress, PHY_MMD_ACCESS_ADRS_REG, RegIdx);

	reg = Device | (PHY_MMD_FUNCTION_DATA_NO_POST_INC << PHY_MMD_FUNCTION_ADDRESS_SHIFT);

	status |=  Eth_WriteMii(CtrlIdx,PhyAddress, PHY_MMD_ACCESS_CTRL_REG, reg);
	status |=  Eth_WriteMii (CtrlIdx,PhyAddress, PHY_MMD_ACCESS_ADRS_REG, RegVal);
	if (status != ETH_OK) {
		DET_REPORT_ERROR(ETH_GLOBAL_ID, ETH_E_WRITE_MII_FAIL);
	}
	return status;
}

/**
 * Finds dynamically if the PHY is Clause 45 MDIO compliant or not
 * @param CtrlIdx
 * @return boolean
 */
static boolean isMdioClause45(uint8 CtrlIdx){
	uint8 status;
	uint16 reg1 = 0u;
	uint16 reg2 = 0u;
    boolean ret = TRUE;
    status =   Eth_ReadMii(CtrlIdx,PhyAddress, PHY_IDR1, &reg1);
    status |=  Eth_ReadMii(CtrlIdx,PhyAddress, PHY_IDR2, &reg2);
	if ((status == ETH_OK) &&
		(reg1 > 0x0000) && (reg1 < 0xffff) &&
		(reg2 > 0x0000) && (reg2 < 0xffff)) {
		ret = FALSE;
	}
	return ret;

	/* , for clause 45 read - let it be here
	status =  readMii45(CtrlIdx,PHY_MMD_PMA_PMD,PHY_IDR1, &reg1);
	status |= readMii45(CtrlIdx,PHY_MMD_PMA_PMD,PHY_IDR2, &reg2);
	if ((reg1 > 0x0000) && (reg1 < 0xffff) &&
		(reg2 > 0x0000) && (reg2 < 0xffff)) {
		return TRUE;
	}
	*/
}


/**
 * Fetch PHY address:: mock up function for ETH tranciever
 * @param ctrlIdx
 * @return phyAddr
 */
uint8 EthTrcv_FindPhyAdrs(uint8 CtrlIdx)
{
    uint8 phyAddr;
      for (phyAddr = 0; phyAddr < ETHTRCV_MAX_ADDRESS; phyAddr++) {
#if !defined(CFG_JAC6)
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
#else
          for(uint32 i = 0; i < ETH_MDIO_WAIT_CYCLES; i++); /* Provide some time for MDIO module to find the PHYs available */
          if(hwPtr->MDIO.MDIO_ALIVE & (0x1u << phyAddr)){
              return phyAddr;
          }
#endif
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
#if !defined(CFG_JAC6)
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
#else
        for(uint32 i = 0; i < ETH_MDIO_WAIT_CYCLES; i++); /* Provide some time for MDIO module to find the PHYs available */
        if(hwPtr->MDIO.MDIO_ALIVE & (0x1u << EthTrcvConfigSet.TrcvAdrs[i].address)){
            return EthTrcvConfigSet.TrcvAdrs[i].address;
        }
#endif
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
#if !defined(CFG_JAC6)
        uint16 regValue;
        if(linkStatus == NO_LINK)
        {
            /* Set auto neg advert register */
            (void)Eth_WriteMii(CtrlIdx,PhyAddress, AUTONEG_ADVERTISE_REGISTER, 0x01e1);
            // Enable and start auto-negotiation
            Eth_ReadMii(CtrlIdx,PhyAddress,BMCR_REGISTER, &regValue);
            Eth_WriteMii(CtrlIdx,PhyAddress,BMCR_REGISTER, (regValue | 0x1200));
            linkStatus = AUTONEGOTIATE_RESTART;
        }
        // Wait for Auto-Neg complete flag
        Eth_ReadMii(CtrlIdx,PhyAddress,BMSR_REGISTER,&statusValue);
        if((statusValue & AUTO_NEG_COMPLETE )== AUTO_NEG_COMPLETE){
            linkStatus = AUTONEGOTIATE_COMPLETE;
        }
        Eth_ReadMii(CtrlIdx,PhyAddress,PHY_STATUS_REGISTER,&statusValue);
        if((statusValue & PHY_STATUS_LINK ) != 0){
            linkStatus = LINK_UP;
        }else{
            linkStatus = LINK_DOWN;
        }
        LwIpAdp_LinkStateUpdate(linkStatus);
#else
        if (bMdioClause45 == FALSE){
			Eth_ReadMii(CtrlIdx,PhyAddress,PHY_STATUS_REGISTER,&statusValue);
        }else{
        	/* for Marvell 88Q1010 */
        	(void)readMii45(CtrlIdx,PHY_MMD_PCS,PHY_100BASET1_STATUS2, &statusValue);
        }
        if((statusValue & PHY_STATUS_LINK ) != 0){
			linkStatus = LINK_UP;
		}else{
			linkStatus = LINK_DOWN;
		}
        LwIpAdp_LinkStateUpdate(linkStatus);
#endif
    }else{
        PhyAddress =  EthTrcv_FindPhyAdrs(CtrlIdx);
        if(PhyAddress != ETHTRCV_MAX_ADDRESS){
             (void)EthTrcv_TransceiverInit(CtrlIdx);
        }
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
    volatile uint32 timeout;
#if defined(CFG_ETH_PHY_TJA1100)
    uint16 extdPhyStatus;
#endif
    uint16 regValue;

#if (STD_OFF == ETHTRCV_AUTO_FIND_ADDRESS)
    PhyAddress = EthTrcv_FindConfiguredPhyAdrs(CtrlIdx);
#else
    PhyAddress = EthTrcv_FindPhyAdrs(CtrlIdx);
#endif
    if(PhyAddress == ETHTRCV_MAX_ADDRESS){ /* Could still not find the Phy connected */
        return E_NOT_OK;
    }
#if	defined(CFG_JAC6) /* otherwise it is always MDIO clause 22 */
    bMdioClause45 = isMdioClause45(CtrlIdx);
#endif

    /* Wait for completion, this is just a precaution to avoid hanging systems waiting for transceiver inits to finish.  */
    timeout = 100000;

	/*  Reset phy */
    if(bMdioClause45 == FALSE){
		(void)Eth_WriteMii(CtrlIdx,PhyAddress, BMCR_REGISTER, PHY_SOFT_RESET);
		for(volatile uint16 i=0; i <10u; i++){};
	} else {
		/* Init phy */
		(void) writeMii45(CtrlIdx, PHY_MMD_PCS, 0xFA0C, 0x5);
		(void) writeMii45(CtrlIdx, PHY_MMD_PCS, 0xFB04, 0x81);
		(void) writeMii45(CtrlIdx, PHY_MMD_PCS, 0x8000, 0x201); /* PCS auto correct polarity and Disable Jabber function */
		/* Soft reset */
		softReset(CtrlIdx, PHY_MMD_PCS);
    }
#if !defined(CFG_S32K148)
#if	defined(CFG_JAC6) && (ETHTRCV_AUTO_NEGOTIATE == STD_OFF)/*  ETH_NO_AUTO_NEG */
	/* No link check done via these registers
	hwPtr->MDIO.MDIO_USERPHYSEL0 |=  GMAC_SW_MDIO_LINKSEL;
	hwPtr->MDIO.MDIO_USERPHYSEL0 |= (GMAC_SW_MDIO_PHYADDRMON_MASK & PhyAddress);
	*/
	if(bMdioClause45 == FALSE){
		(void)Eth_ReadMii (CtrlIdx,PhyAddress, BMCR_REGISTER, &regValue);
		regValue &= ~ENABLE_AUTO_NEGOTIATION; // disable AN
		(void)Eth_WriteMii(CtrlIdx,PhyAddress, BMCR_REGISTER, (regValue | ENABLE_100MBPS_DUPLEX));
		for(volatile uint16 i=0; i < ETH_MDIO_WAIT_CYCLES; i++){};
		do
		{
			for(volatile uint16 i=0; i < ETH_MDIO_WAIT_CYCLES; i++){};
			if ((timeout--) == 0) {
				return E_NOT_OK;
			}
			Eth_ReadMii(CtrlIdx,PhyAddress, PHY_STATUS_REGISTER, &phyStatus);
#if defined (CFG_ETH_PHY_TJA1100)
			Eth_ReadMii(CtrlIdx,PhyAddress, PHY_EXTENDED_STATUS_REGISTER, &extdPhyStatus);
#endif
		}while
#if !defined (CFG_ETH_PHY_TJA1100)
			((phyStatus & PHY_STATUS_NO_AUTO_100) != PHY_STATUS_NO_AUTO_100);
#else
			(((phyStatus & PHY_STATUS_NO_AUTO_100) == 0u) && ((extdPhyStatus & PHY_EXTENDED_STATUS_NO_AUTO_100) != PHY_EXTENDED_STATUS_NO_AUTO_100));
#endif
	}else{
		/* for Marvell 88Q1010 */
#if (ETHTRCV_AUTO_NEGOTIATE == STD_ON)
        #if 0
		(void)writeMii45(CtrlIdx,PHY_MMD_AUTONEG,PHY_AUTONEG_ADV_REG2,(1 << 4)); /* not required -  advertise master or use it from HW strapping */
        #endif
		/* Use auto negotiation */
		(void)readMii45 (CtrlIdx,PHY_MMD_AUTONEG,PHY_AUTONEG_CONTROL, &regValue);
		regValue |= PHY_AUTONEG_ENABLE;
		(void)writeMii45(CtrlIdx,PHY_MMD_AUTONEG,PHY_AUTONEG_CONTROL, regValue);
		do{
			if ((timeout--) == 0) {
				return E_NOT_OK;
			}
			if (readMii45(CtrlIdx,PHY_MMD_AUTONEG,PHY_AUTONEG_STATUS, &phyStatus) != 0) {
				return E_NOT_OK;
			}
			if(phyStatus == 0xffff){
				return E_NOT_OK;
			}
		}while ((phyStatus & (AUTO_NEG_COMPLETE | PHY_STATUS_LINK)) != (AUTO_NEG_COMPLETE | PHY_STATUS_LINK)); /* wait until it is complete */
		/* link needs to be available */
#else
		/* Configuration is from HW strapping */
		/* check 3.8008 for the status of master or slave */
#endif
	}

#else //CFG_JAC6 (ETH_NO_AUTO_NEG) else

	/* Set auto neg advert register */
	(void)Eth_WriteMii(CtrlIdx,PhyAddress, AUTONEG_ADVERTISE_REGISTER, 0x01e1);

    /* enable auto-negotiation */
    (void)Eth_ReadMii (CtrlIdx,PhyAddress, BMCR_REGISTER, &regValue);
    (void)Eth_WriteMii(CtrlIdx,PhyAddress, BMCR_REGISTER, (regValue | 0x1200));


    do {
        for(volatile uint16 i=0; i < 1000; i++){
        }

        if ((timeout--) == 0) {
            return E_NOT_OK;
        }

		if (Eth_ReadMii(CtrlIdx,PhyAddress, BMSR_REGISTER, &phyStatus) != 0) {
			return E_NOT_OK;
		}
		if(phyStatus == 0xffff){
			return E_NOT_OK;
		}
	} while (!(phyStatus & AUTO_NEG_COMPLETE)); // Should be 0x786D
#endif //CFG_JAC6(ETH_NO_AUTO_NEG) end
#endif
#ifdef CFG_ETHTRCV_PHYLOOPBACK /* physical line test, set in build_config for use */
	(void)Eth_ReadMii (CtrlIdx,PhyAddress, BMCR_REGISTER, &regValue);
	regValue &= ~ENABLE_AUTO_NEGOTIATION; // disable AN
	(void)Eth_WriteMii(CtrlIdx,PhyAddress, BMCR_REGISTER, (regValue | 0x6100)); // loopback with speed 100
	do
	{
		for(volatile uint16 i=0; i < 1000; i++){};
		if ((timeout--) == 0) {
			return E_NOT_OK;
		}
		Eth_ReadMii(CtrlIdx,PhyAddress, PHY_STATUS_REGISTER, &phyStatus);
	}while (!(phyStatus & PHY_STATUS_LOOPBACK));
#endif /* ETH_PHYLOOPBACK physical line test */

    linkStatus = LINK_UP;
    LwIpAdp_LinkStateUpdate(LINK_UP);
    return E_OK;
}
