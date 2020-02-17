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

#ifndef CANTRCV_TJA1145_H_
#define CANTRCV_TJA1145_H_

/* CAN transceiver - TJA1145 Register declaration */
#define REGISTER_0x20_ADDR          0x20u
#define REGISTER_20h_CPNC_SET       0x10u
#define REGISTER_20h_PNCOK_SET      0x20u
#define REGISTER_20h_CPNC_CLR       0x00u
#define REGISTER_20h_PNCOK_CLR      0x00u
#define REGISTER_0x22_ADDR          0x22u
#define REGISTER_22h_CPNERR_MASK    0x40u
#define REGISTER_22h_CPNS_MASK      0x20u
#define REGISTER_22h_COCS_MASK      0x10u
#define REGISTER_22h_CTS_MASK       0x80u
#define REGISTER_22h_CBSS_MASK      0x08u
#define REGISTER_0x23_ADDR          0x23u
#define REGISTER_0x23_CWE_SET       0x01u
#define REGISTER_0x23_CFE_SET       0x02u
#define REGISTER_0x23_CBSE_SET      0x10u
#define REGISTER_0x23_CWE_CLR       0x00u
#define REGISTER_0x04_ADDR          0x04u
#define REGISTER_0x04_SPIFE_SET     0x02u
#define REGISTER_0x04_SPIFE_CLR     0x00u
#define REGISTER_0x04_OTWE_SET      0x04u
#define REGISTER_0x04_OTWE_CLR      0x00u
#define REGISTER_0x4c_ADDR          0x4cu
#define REGISTER_0x4C_WPFE_SET      0x01u
#define REGISTER_0x4C_WPRE_SET      0x02u
#define REGISTER_0x61_ADDR          0x61u
#define REGISTER_0x63_ADDR          0x63u
#define REGISTER_0x64_ADDR          0x64u
#define REGISTER_0x61_PO_SET        0x10u
#define REGISTER_0x61_OTW_SET       0x40u
#define REGISTER_0x61_SPIF_SET      0x20u
#define REGISTER_0x63_PNFDE_SET     0x20u
#define REGISTER_0x63_CW_SET        0x01u
#define REGISTER_0x63_CBS_SET       0x10u
#define REGISTER_0x64_WPR_SET       0x02u
#define REGISTER_0x64_WPF_SET       0x01u
#define REGISTER_0x03_ADDR          0x03u
#define REGISTER_0x22_ADDR          0x22u
#define REGISTER_0x22_CBSS_SET      0x08u
#define REGISTER_0x22_CFS_SET       0x01u
#define REGISTER_0x4B_ADDR          0x4Bu
#define REGISTER_0x26_ADDR          0x26u
#define REGISTER_0x26_CDR_50        0x00u
#define REGISTER_0x26_CDR_100       0x01u
#define REGISTER_0x26_CDR_125       0x02u
#define REGISTER_0x26_CDR_250       0x03u
#define REGISTER_0x26_CDR_500       0x05u
#define REGISTER_0x26_CDR_1000      0x07u
#define REGISTER_0x01_ADDR          0x01u
#define REGISTER_0x01_MC_NORMAL     0x07u /* System control registers */
#define REGISTER_0x01_MC_STANDBY    0x04u /* System control registers */
#define REGISTER_0x01_MC_SLEEP      0x01u /* System control registers */
#define REGISTER_0x01_MODE_MASK     0x07u
#define REGISTER_0x60_ADDR          0x60u
#define REGISTER_0x01_SYSE_SET      0x01u
#define REGISTER_0x01_TRXE_SET      0x04u
#define REGISTER_0x01_WPE_SET       0x08u
#define REGISTER_0x2F_ADDR          0x2Fu /* Frame control register (address 2Fh) */
#define REGISTER_0x2F_PNDM          0x40u /* Frame control register (address 2Fh) */
#define REGISTER_0x2F_IDE_STD       0x00u /* Frame control register (address 2Fh) */
#define REGISTER_0x2F_IDE_EXT       0x80u /* Frame control register (address 2Fh) */
#define REGISTER_0x27_ADDR          0x27u /* ID register 0 (address 27h) */
#define REGISTER_0x28_ADDR          0x28u /* ID register 1 (address 28h) */
#define REGISTER_0x29_ADDR          0x29u /* ID register 1 (address 29h) */
#define REGISTER_0x2A_ADDR          0x2Au /* ID register 1 (address 2Ah) */
#define REGISTER_0x2B_ADDR          0x2Bu /* Mask register 0 (address 2Bh) */
#define REGISTER_0x2C_ADDR          0x2Cu /* Mask register 1 (address 2Ch) */
#define REGISTER_0x2D_ADDR          0x2Du /* Mask register 2 (address 2Dh) */
#define REGISTER_0x2E_ADDR          0x2Eu /* Mask register 3 (address 2Eh) */
#define REGISTER_20h_CMC_SET        0x01u

#define REGISTER_0x68_ADDR          0x68u /* Data mask registers (addresses 68h to 6Fh) for byte 0 */
#define REGISTER_0x69_ADDR          0x69u /* Data mask registers (addresses 68h to 6Fh) for byte 1 */
#define REGISTER_0x6A_ADDR          0x6Au /* Data mask registers (addresses 68h to 6Fh) for byte 2 */
#define REGISTER_0x6B_ADDR          0x6Bu /* Data mask registers (addresses 68h to 6Fh) for byte 3 */
#define REGISTER_0x6C_ADDR          0x6Cu /* Data mask registers (addresses 68h to 6Fh) for byte 4 */
#define REGISTER_0x6D_ADDR          0x6Du /* Data mask registers (addresses 68h to 6Fh) for byte 5 */
#define REGISTER_0x6E_ADDR          0x6Eu /* Data mask registers (addresses 68h to 6Fh) for byte 6 */
#define REGISTER_0x6F_ADDR          0x6Fu /* Data mask registers (addresses 68h to 6Fh) for byte 7 */


#endif /* CANTRCV_TJA1145_H_ */
