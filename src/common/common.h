#ifndef _COMMON_COMMON_H
#define _COMMON_COMMON_H

#include "platform.h"
#include "config.h"
#include "version.h"

#include <stdlib.h>

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

typedef signed char s8;
typedef signed int s16;
typedef signed long s32;

typedef unsigned char byte;
typedef unsigned int word;
typedef unsigned int uint;
typedef int bool;

#define true 1
#define false 0

#define const

#define CDECL cdecl
#define FAR far


#define CHECK_PARAM_RANGE(v, inclusive_min, exclusive_max)
#define ASSERT(x)
#define SET_UNINITIALIZED_8(x)
#define CHECK_UNINITIALIZED_8(x)
#define SET_UNINITIALIZED_16(x)
#define CHECK_UNINITIALIZED_16(x)


#include "debug.h"

#include "util.h"

#define ARRAYSIZE(array) ((int)(sizeof(array) / sizeof((array)[0])))

#define _TEXT(addr, text) text
#define _STATIC_TEXT(addr, text) text

// A string that the original keeps in another module's data: in the DOS build it is that
// module's array S_<addr> (declared extern by the file's DGROUP block, tools/strings.py);
// elsewhere it is an ordinary string.
#define _XTEXT(addr, text) S_##addr
#define _XSTATIC_TEXT(addr, text) S_##addr

// A "return;" in a function that returns a value: the original returns whatever AX holds
// there. Elsewhere the function returns the value it would have had.
#define RETURN_AX(value) return

#endif
