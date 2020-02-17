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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.0.3 */
/** @tagSettings DEFAULT_ARCHITECTURE=RH850F1H */


#ifndef FR_MPC5XXX_H_
#define FR_MPC5XXX_H_


//CHI Command Vector Bit
#define CHI_CMD_NOT_ACCEPTED            0x00
#define CHI_CMD_CONFIG                  0x01
#define CHI_CMD_READY                   0x02
#define CHI_CMD_WAKEUP                  0x03
#define CHI_CMD_RUN                     0x04
#define CHI_CMD_ALL_SLOTS               0x05
#define CHI_CMD_HALT                    0x06
#define CHI_CMD_FREEZE                  0x07
#define CHI_CMD_SEND_MTS                0x08
#define CHI_CMD_ALLOW_COLDSTART         0x09
#define CHI_CMD_RESET_STATUS_INDICATORS 0x0A
#define CHI_CMD_MONITOR_MODE            0x0B
#define CHI_CMD_CLEAR_RAMS              0x0C


#define POC_CMD_ALLOW_COLDSTART         CHI_CMD_ALLOW_COLDSTART
#define POC_CMD_ALL_SLOTS               CHI_CMD_ALL_SLOTS
#define POC_CMD_CONFIG                  CHI_CMD_CONFIG
#define POC_CMD_FREEZE                  CHI_CMD_FREEZE
#define POC_CMD_READY                   CHI_CMD_READY
//#define POC_CMD_CONFIG_COMPLETE         4u
#define POC_CMD_RUN                     CHI_CMD_RUN
//#define POC_CMD_DEFAULT_CONFIG          6u
#define POC_CMD_HALT                    CHI_CMD_HALT
#define POC_CMD_WAKEUP                  CHI_CMD_WAKEUP


struct FLXA_reg {
    uint32 FROC;
    uint32 pad_0;
    uint32 FROS;
    uint32 FRTEST1;
    uint32 FRTEST2;
    uint32 pad_1;
    uint32 FRLCK;
    uint32 FREIR;
    uint32 FRSIR;
    uint32 FREILS;
    uint32 FRSILS;
    uint32 FREIES;
    uint32 FREIER;
    uint32 FRSIES;
    uint32 FRSIER;
    uint32 FRILE;
    uint32 FRT0C;
    uint32 FRT1C;
    uint32 FRSTPW1;
    uint32 FRSTPW2;
    uint32 pad_2[11];
    uint32 FRSUCC1;
    uint32 FRSUCC2;
    uint32 FRSUCC3;
    uint32 FRNEMC;
    uint32 FRPRTC1;
    uint32 FRPRTC2;
    uint32 FRMHDC;
    uint32 pad_3;
    uint32 FRGTUC1;
    uint32 FRGTUC2;
    uint32 FRGTUC3;
    uint32 FRGTUC4;
    uint32 FRGTUC5;
    uint32 FRGTUC6;
    uint32 FRGTUC7;
    uint32 FRGTUC8;
    uint32 FRGTUC9;
    uint32 FRGTUC10;
    uint32 FRGTUC11;
    uint32 pad_4[13];
    uint32 FRCCSV;
    uint32 FRCCEV;
    uint32 pad_5[2];
    uint32 FRSCV;
    uint32 FRMTCCV;
    uint32 FRRCV;
    uint32 FROCV;
    uint32 FRSFS;
    uint32 FRSWNIT;
    uint32 FRACS;
    uint32 pad_6;
    uint32 FRESID1;
    uint32 FRESID2;
    uint32 FRESID3;
    uint32 FRESID4;
    uint32 FRESID5;
    uint32 FRESID6;
    uint32 FRESID7;
    uint32 FRESID8;
    uint32 FRESID9;
    uint32 FRESID10;
    uint32 FRESID11;
    uint32 FRESID12;
    uint32 FRESID13;
    uint32 FRESID14;
    uint32 FRESID15;
    uint32 pad_7;
    uint32 FROSID1;
    uint32 FROSID2;
    uint32 FROSID3;
    uint32 FROSID4;
    uint32 FROSID5;
    uint32 FROSID6;
    uint32 FROSID7;
    uint32 FROSID8;
    uint32 FROSID9;
    uint32 FROSID10;
    uint32 FROSID11;
    uint32 FROSID12;
    uint32 FROSID13;
    uint32 FROSID14;
    uint32 FROSID15;
    uint32 pad_8;
    uint32 FRNMV1;
    uint32 FRNMV2;
    uint32 FRNMV3;
    uint32 pad_9[81];
    uint32 FRMRC;
    uint32 FRFRF;
    uint32 FRFRFM;
    uint32 FRFCL;
    uint32 FRMHDS;
    uint32 FRLDTS;
    uint32 FRFSR;
    uint32 FRMHDF;
    uint32 FRTXRQ1;
    uint32 FRTXRQ2;
    uint32 FRTXRQ3;
    uint32 FRTXRQ4;
    uint32 FRNDAT1;
    uint32 FRNDAT2;
    uint32 FRNDAT3;
    uint32 FRNDAT4;
    uint32 FRMBSC1;
    uint32 FRMBSC2;
    uint32 FRMBSC3;
    uint32 FRMBSC4;
    uint32 pad_10[44];
    uint8 FRWRDS[256]; /* Use the 8-bit access */
    uint32 FRWRHS1;
    uint32 FRWRHS2;
    uint32 FRWRHS3;
    uint32 pad_11;
    uint32 FRIBCM;
    uint32 FRIBCR;
    uint32 pad_12[58];
    uint8 FRRDDS[256]; /* Use the 8-bit access */
    uint32 FRRDHS1;
    uint32 FRRDHS2;
    uint32 FRRDHS3;
    uint32 FRMBS;
    uint32 FROBCM;
    uint32 FROBCR;
    uint32 pad_13[58];
    uint32 FRITC;
    uint32 FROTC;
    uint32 FRIBA;
    uint32 FRFBA;
    uint32 FROBA;
    uint32 FRIQC;
    uint32 FRUIR;
    uint32 FRUOR;
    uint32 FRITS;
    uint32 FROTS;
    uint32 FRAES;
    uint32 FRAEA;
    uint32 FRDA0;
    uint32 FRDA1;
    uint32 FRDA2;
    uint32 FRDA3;
    uint32 pad_14;
    uint32 FRT2C;
};



#endif /*FR_INTERNAL_H_*/
