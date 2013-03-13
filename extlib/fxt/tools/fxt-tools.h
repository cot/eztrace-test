/*
 *  Copyright (C) 2003, 2004, 2011-2012 Samuel Thibault <samuel.thibault@ens-lyon.org>
 *
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the program ; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __FXT_TOOLS_H__
#define __FXT_TOOLS_H__
#include <stdint.h>
#include <errno.h>
#ifndef __MINGW32__
#include <arpa/inet.h>
#endif

#define EPRINTF(fmt,...)	do {\
	fprintf(stderr, "=== " fmt, ## __VA_ARGS__); \
	printf("=== " fmt, ## __VA_ARGS__); \
	fflush(stdout); \
} while (0)

#define DEFAULT_TRACE_FILE "trace_file"

#define WRITE(n,value) do { \
	int res; \
	uint##n##_t __val = (typeof(__val)) (value); \
	res = write(fd, (void *)&__val, sizeof(__val)); \
	if (res < 0) return -1; \
	if (res < sizeof(__val)) { \
		errno = EINVAL; \
		return -1; \
	} \
} while (0)

#define READRAW(n,value) do { \
	uint##n##_t __val; \
	int res; \
	res = fread((void *)&__val, sizeof(__val), 1, fstream); \
	if (res < 0) return -1; \
	if (res < 1) { \
		errno = EINVAL; \
		return -1; \
	} \
	(value) = (typeof(value)) __val; \
} while (0)

extern void unsupported_swap(void);
#define DO_SWAP(n, var) { \
	uint##n##_t ___val = (var); \
	switch(n) { \
	case 64: ___val = \
			(___val & 0x00000000000000ffULL) << 56 | \
			(___val & 0x000000000000ff00ULL) << 40 | \
			(___val & 0x0000000000ff0000ULL) << 24 | \
			(___val & 0x00000000ff000000ULL) << 8  | \
			(___val & 0x000000ff00000000ULL) >> 8  | \
			(___val & 0x0000ff0000000000ULL) >> 24 | \
			(___val & 0x00ff000000000000ULL) >> 40 | \
			(___val & 0xff00000000000000ULL) >> 56; \
		break; \
	case 32: ___val = \
			(___val & 0x000000ffUL) << 24 | \
			(___val & 0x0000ff00UL) << 8  | \
			(___val & 0x00ff0000UL) >> 8  | \
			(___val & 0xff000000UL) >> 24; \
		break; \
	case 16: ___val = \
			(___val & 0x00ffU) << 8 | \
			(___val & 0xff00U) >> 8; \
		break; \
	case 8: break; \
	default: unsupported_swap(); \
	} \
	var = ___val; \
}

#ifdef IS_BIG_ENDIAN
#define SWAP(n, var) do { \
	if (fxt->infos.arch < 0x10000) \
		/* little endian trace, swap */ \
		DO_SWAP(n, var); \
} while(0)
#else
#define SWAP(n, var) do { \
	if (fxt->infos.arch >= 0x10000) \
		/* big endian trace, swap */ \
		DO_SWAP(n, var); \
} while(0)
#endif

#ifdef __MINGW32__
#define myswap32(v) ({ \
	uint32_t ___val = (v); \
	(___val & 0x000000ffUL) << 24 | \
	(___val & 0x0000ff00UL) << 8  | \
	(___val & 0x00ff0000UL) >> 8  | \
	(___val & 0xff000000UL) >> 24; \
})
#define myntohl(v) myswap32(v)
#define myhtonl(v) myswap32(v)
#else
#define myntohl(v) ntohl(v)
#define myhtonl(v) htonl(v)
#endif

#define READ(n,value) do { \
	uint##n##_t __val; \
	int res; \
	res = fread((void *)&__val, sizeof(__val), 1, fstream); \
	if (res < 0) return -1; \
	if (res < 1) { \
		errno = EINVAL; \
		return -1; \
	} \
	SWAP(n,__val); \
	(value) = (typeof(value)) __val; \
} while (0)

#endif /* __FXT_TOOLS_H__ */
