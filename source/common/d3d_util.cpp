/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/d3d_util.h"

#include "common/file_io_util.h"


namespace Common {

HRESULT LoadVertexShader(const wchar *fileName, ID3D11Device *device, ID3D11VertexShader **vertexShader, ID3D11InputLayout **inputLayout, D3D11_INPUT_ELEMENT_DESC *vertexDesc, uint numElements) {
	DWORD bytesRead;
	char *fileBuffer = ReadWholeFile(fileName, &bytesRead);
	if (fileBuffer == nullptr) {
		return -1;
	}

	HRESULT result = device->CreateVertexShader(fileBuffer, bytesRead, nullptr, vertexShader);
	if (result != S_OK) {
		return result;
	}

	if (numElements != 0) {
		// Create the vertex input layout.
		result = device->CreateInputLayout(vertexDesc, numElements, fileBuffer, bytesRead, inputLayout);
	}

	delete[] fileBuffer;
	return result;
}

HRESULT LoadPixelShader(const wchar *fileName, ID3D11Device *device, ID3D11PixelShader **pixelShader) {
	DWORD bytesRead;
	char *fileBuffer = ReadWholeFile(fileName, &bytesRead);
	if (fileBuffer == nullptr) {
		return -1;
	}

	HRESULT result = device->CreatePixelShader(fileBuffer, bytesRead, NULL, pixelShader);

	delete[] fileBuffer;
	return result;
}

} // End of namespace Common
