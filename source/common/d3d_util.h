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


namespace Common {

HRESULT LoadVertexShader(const char *fileName, ID3D11Device *device, D3D11_INPUT_ELEMENT_DESC *vertexDesc, uint numElements, ID3D11VertexShader **vertexShader, ID3D11InputLayout **inputLayout);

HRESULT LoadPixelShader(const char *fileName, ID3D11Device *device, ID3D11PixelShader **pixelShader);

} // End of namespace Common

#endif
