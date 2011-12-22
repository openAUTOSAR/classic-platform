/* -------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2009-2011 ArcCore AB <contact@arccore.com>
 * Licensed under ArcCore Embedded Software License Agreement.
 * -------------------------------- Arctic Core ------------------------------*/

#ifndef IO_H_
#define IO_H_

#define WRITE8(address, value)      (*(uint8_t*)(address) = (value))
#define READ8(address)              ((uint8_t)(*(uint8_t*)(address)))

#define WRITE16(address, value)     (*(vuint16_t*)(address) = (value))
#define READ16(address)             ((uint16_t)(*(vuint16_t*)(address)))

#define WRITE32(address, value)     (*(vuint32_t*)(address) = (value))
#define READ32(address)             ((uint32_t)(*(vuint32_t*)(address)))

#define WRITE64(address, value)     (*(vuint64_t*)(address) = (value))
#define READ64(address)             ((uint64_t)(*(vuint64_t*)(address)))


#endif /* IO_H_ */
