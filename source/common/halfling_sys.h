/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_HALFLING_SYS_H
#define COMMON_HALFLING_SYS_H

#include "common/typedefs.h"

// Only include the base windows libraries
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <comip.h>

// GDI+
#include <gdiplus.h>

// Un-define min and max from the windows headers
#ifdef min
	#undef min
#endif

#ifdef max
	#undef max
#endif
#endif
