
#ifndef CRC_CFG_H_
#define CRC_CFG_H_

#include "MemMap.h"

/* @req 4.0.3/CRC040 */

/* CRC 8 Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_8_HARDWARE (0x01) /* Not supported */
#define CRC_8_RUNTIME  (0x02)
#define CRC_8_TABLE    (0x04) /* Default value */


#define CRC_8_MODE      CRC_8_TABLE


/* CRC 8H2F Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_8H2F_HARDWARE (0x01) /* Not supported */
#define CRC_8H2F_RUNTIME  (0x02)
#define CRC_8H2F_TABLE    (0x04) /* Default value */


#define CRC_8H2F_MODE      CRC_8H2F_TABLE


/* CRC 16 Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_16_HARDWARE (0x01) /* Not supported */
#define CRC_16_RUNTIME  (0x02)
#define CRC_16_TABLE    (0x04) /* Default value */


#define CRC_16_MODE      CRC_16_TABLE


/* CRC 32 Configuration
 * Possible values and the mode decides what method to be used
 */
#define CRC_32_HARDWARE (0x01)  /* Not supported */
#define CRC_32_RUNTIME  (0x02)
#define CRC_32_TABLE    (0x04)  /* Default value */


#define CRC_32_MODE      CRC_32_TABLE


/* Version info configuration */
#define CRC_VERSION_INFO_API    STD_ON


#endif /* CRC_CFG_H_ */

