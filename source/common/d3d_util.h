/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_D3D_UTIL_H
#define COMMON_D3D_UTIL_H

#include "common/dxerr.h"

//---------------------------------------------------------------------------------------
// Simple d3d error checker 
//---------------------------------------------------------------------------------------

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x) {                                                         \
	HRESULT hr = (x);                                                   \
	if (FAILED(hr)) {                                                   \
	DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true);   \
	}                                                                   \
}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

//---------------------------------------------------------------------------------------
// Convenience macro for releasing COM objects.
//---------------------------------------------------------------------------------------

#define ReleaseCOM(x) { if(x){ x->Release(); x = nullptr; } }

#endif
