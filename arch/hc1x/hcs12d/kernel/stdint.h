
#ifndef _STDINT_H
#define _STDINT_H

#include <limits.h>

#if (SCHAR_MAX == 0x7f)
typedef signed char int8_t;
typedef unsigned char uint8_t;
#else
#error BAD TYPES
#endif

#if (SHRT_MAX == 0x7fff)
typedef signed short int16_t;
typedef unsigned short uint16_t;
#else
#error BAD TYPES
#endif

/* HC12 supports both */
#if INT_MAX == 0x7fffL
typedef long int int32_t;
typedef unsigned long int uint32_t;
#else
typedef int int32_t;
typedef unsigned int uint32_t;
#endif

/* 64-bit */
#if (LONG_MAX > 0x7fffffff)
#define __longis_64
#else
#define __longis_32
#endif

#if defined(__longis_64)
typedef signed long int64_t;
typedef unsigned long uint64_t;
#else
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#endif

/* Least types.. */
typedef uint8_t       		uint_least8_t;
typedef uint16_t      		uint_least16_t;
typedef uint32_t      		uint_least32_t;
typedef int8_t        		int_least8_t;
typedef int16_t       		int_least16_t;
typedef int32_t       		int_least32_t;

#endif /* _STDINT_H */
