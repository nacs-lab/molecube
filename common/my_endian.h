#ifndef _MY_ENDIAN_H
#define _MY_ENDIAN_H
//define endianness like in endian.h on gcc / Linux
//but our platforms don't all have endian.h

#ifndef _ENDIAN_H
#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321

#ifdef CONFIG_PC
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#ifdef __arm__
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#ifdef __PPC__
#define __BYTE_ORDER __BIG_ENDIAN
#endif

#endif //! _ENDIAN_H

#if !((__BYTE_ORDER == __LITTLE_ENDIAN) || (__BYTE_ORDER == __BIG_ENDIAN))
#error("Either (__BYTE_ORDER == __LITTLE_ENDIAN) OR (__BYTE_ORDER == __BIG_ENDIAN)")
#endif

#if ((__BYTE_ORDER == __LITTLE_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN))
#error("(__BYTE_ORDER == __LITTLE_ENDIAN) AND (__BYTE_ORDER == __BIG_ENDIAN) is not correct")
#endif

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define NATIVE_TO_BIG_ENDIAN16(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define NATIVE_TO_BIG_ENDIAN32(x) ((((x) & 0xff) << 24) | \
                     	 	 	 	 (((x) & 0xff00) << 8) | \
                     	 	 	 	 (((x) & 0xff0000UL) >> 8) | \
                     	 	 	 	 (((x) & 0xff000000UL) >> 24))
#else
#define NATIVE_TO_BIG_ENDIAN16(x) (x)
#define NATIVE_TO_BIG_ENDIAN32(x) (x)
#endif

#define BIG_ENDIAN_TO_NATIVE16(x) NATIVE_TO_BIG_ENDIAN16(x)
#define BIG_ENDIAN_TO_NATIVE32(x) NATIVE_TO_BIG_ENDIAN32(x)

#endif //_MY_ENDIAN_H
