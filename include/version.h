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

#ifndef VERSION_H_
#define VERSION_H_

#define _ARCTIC_CORE_MAJOR_         22
#define _ARCTIC_CORE_MINOR_         0
#define _ARCTIC_CORE_PATCHLEVEL_    0
#define _ARCTIC_CORE_BUILDTYPE_     REL

#define STR__(x)		#x
#define STRSTR__(x) 	STR__(x)

/* Test for 1.2.3 ARCTIC_CORE_VERSION > 10203 */
#define ARCTIC_CORE_VERSION 		(_ARCTIC_CORE_MAJOR_ * 10000 + _ARCTIC_CORE_MINOR_ * 100 + _ARCTIC_CORE_PATCHLEVEL_)
#define ARCTIC_CORE_VERSION_STR	STRSTR__(_ARCTIC_CORE_MAJOR_) "." STRSTR__(_ARCTIC_CORE_MINOR_) "." STRSTR__(_ARCTIC_CORE_PATCHLEVEL_) "." STRSTR__(_ARCTIC_CORE_BUILDTYPE_)

typedef struct Version {
    const char *string;
    const char *info;
    const char *buildDate;
    const char *optFlags;
} VersionType;

extern const VersionType ArcticCore_Version;


#endif /* VERSION_H_ */

