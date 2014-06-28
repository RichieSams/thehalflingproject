/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/typedefs.h"

// Only include the base windows libraries
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
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

#include <sstream>
#define AssertMsg(condition, message)                                                                                                            \
	do {                                                                                                                                         \
		if (!(condition)) {                                                                                                                      \
			std::wstringstream debugStream;                                                                                                      \
			debugStream << "Assertion `" #condition "` failed in " << __FILE__  << " line " << __LINE__ << ": "                                  \
			            << message << std::endl << std::endl << "Do you want to debug the application?";                                         \
			std::wstring debugMessage(debugStream.str());                                                                                        \
			int nResult = MessageBoxW(GetForegroundWindow(), debugMessage.c_str(), L"Unexpected error encountered", MB_YESNO | MB_ICONERROR );   \
			if (nResult == IDYES)                                                                                                                \
			DebugBreak();                                                                                                                        \
		}                                                                                                                                        \
	} while (false)
