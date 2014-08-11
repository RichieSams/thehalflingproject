/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "graphics/dxerr.h"

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


namespace Graphics {

HRESULT LoadVertexShader(const wchar *fileName, ID3D11Device *device, ID3D11VertexShader **vertexShader, ID3D11InputLayout **inputLayout = nullptr, D3D11_INPUT_ELEMENT_DESC *vertexDesc = nullptr, uint numElements = 0);
HRESULT LoadPixelShader(const wchar *fileName, ID3D11Device *device, ID3D11PixelShader **pixelShader);
HRESULT LoadComputeShader(const wchar *fileName, ID3D11Device *device, ID3D11ComputeShader **computeShader);

// Returns a size suitable for creating a constant buffer, by rounding up
// to the next multiple of 16
inline size_t CBSize(size_t size) {
	return ((size + 15) / 16) * 16;
}

} // End of namespace Graphics
