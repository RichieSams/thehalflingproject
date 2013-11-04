/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_D3D_UTIL_H
#define COMMON_D3D_UTIL_H

#include "common/dxerr.h"

#include "d3d11.h"
#include "DirectXMath.h"


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

namespace Colors {

XMGLOBALCONST DirectX::XMFLOAT4 White = {1.0f, 1.0f, 1.0f, 1.0f};
XMGLOBALCONST DirectX::XMFLOAT4 Black = {0.0f, 0.0f, 0.0f, 1.0f};
XMGLOBALCONST DirectX::XMFLOAT4 Red = {1.0f, 0.0f, 0.0f, 1.0f};
XMGLOBALCONST DirectX::XMFLOAT4 Green = {0.0f, 1.0f, 0.0f, 1.0f};
XMGLOBALCONST DirectX::XMFLOAT4 Blue = {0.0f, 0.0f, 1.0f, 1.0f};
XMGLOBALCONST DirectX::XMFLOAT4 Yellow = {1.0f, 1.0f, 0.0f, 1.0f};
XMGLOBALCONST DirectX::XMFLOAT4 Cyan = {0.0f, 1.0f, 1.0f, 1.0f};
XMGLOBALCONST DirectX::XMFLOAT4 Magenta = {1.0f, 0.0f, 1.0f, 1.0f};

XMGLOBALCONST DirectX::XMFLOAT4 Silver = {0.75f, 0.75f, 0.75f, 1.0f};
XMGLOBALCONST DirectX::XMFLOAT4 LightSteelBlue = {0.69f, 0.77f, 0.87f, 1.0f};

} // End of namespace Colors

#endif
