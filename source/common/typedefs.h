/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_TYPEDEFS_H
#define COMMON_TYPEDEFS_H

typedef unsigned char byte;
typedef unsigned char uint8;
typedef signed char int8;

typedef unsigned short uint16;
typedef signed short int16;

typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned int uint;

typedef __int64				int64;
typedef unsigned __int64	uint64;

typedef wchar_t wchar;

namespace DisposeAfterUse {
	enum Flag { NO, YES };
}

#endif
