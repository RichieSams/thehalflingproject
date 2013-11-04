/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/d3d_util.h"

#include <fstream>


namespace Common {

HRESULT LoadVertexShader(const char *fileName, ID3D11Device *device, D3D11_INPUT_ELEMENT_DESC *vertexDesc, uint numElements, ID3D11VertexShader **vertexShader, ID3D11InputLayout **inputLayout) {
	std::ifstream fin(fileName, std::ios::in | std::ios::binary | std::ios::ate);

	if (fin.is_open()) {
		int size = (int)fin.tellg();
		fin.seekg(0, std::ios::beg);

		// Create a buffer to hold the data
		char *buffer = new char[size];

		fin.read(buffer, size);
		fin.close();

		HRESULT result = device->CreateVertexShader(buffer, size, NULL, vertexShader);
		if (result != S_OK) {
			return result;
		}

		// Create the vertex input layout.
		result = device->CreateInputLayout(vertexDesc, numElements, buffer, size, inputLayout);

		delete[] buffer;
		return result;
	}

	return S_FALSE;
}

HRESULT LoadPixelShader(const char *fileName, ID3D11Device *device, ID3D11PixelShader **pixelShader) {
	std::ifstream fin(fileName, std::ios::in | std::ios::binary | std::ios::ate);

	if (fin.is_open()) {
		int size = (int)fin.tellg();
		fin.seekg(0, std::ios::beg);

		// Create a buffer to hold the data
		char *buffer = new char[size];

		fin.read(buffer, size);
		fin.close();

		HRESULT result = device->CreatePixelShader(buffer, size, NULL, pixelShader);

		delete[] buffer;
		return result;
	}

	return S_FALSE;
}

} // End of namespace Common
