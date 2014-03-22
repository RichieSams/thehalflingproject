/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/halfling_model_file.h"

#include "common/texture_manager.h"
#include "common/file_io_util.h"
#include "common/memory_stream.h"
#include "common/endian.h"

#include <string>

namespace Common {

Common::Model *Common::HalflingModelFile::Load(ID3D11Device *device, ID3D11DeviceContext *context, Common::TextureManager *textureManager, const wchar *filePath) {
	// Read the entire file into memory
	DWORD bytesRead;
	char *fileBuffer = ReadWholeFile(filePath, &bytesRead);
	if (fileBuffer == NULL) {
		return NULL;
	}

	Common::MemoryInputStream fin(fileBuffer, bytesRead);

	// Read in the file data

	// Check that this is a 'HFM' file
	uint32 fileId;
	fin.readUInt32(&fileId);
	if (fileId != MKTAG('\0', 'F', 'M', 'H')) {
		return NULL;
	}

	// File format version
	byte fileFormatVersion;
	fin.readByte(&fileFormatVersion);

	// Flags
	uint64 flags;
	fin.readUInt64(&flags);

	bool hasInstanceBuffer = (flags & HAS_INSTANCE_BUFFER) == HAS_INSTANCE_BUFFER;
	bool hasInstanceData = (flags & HAS_INSTANCE_BUFFER_DATA) == HAS_INSTANCE_BUFFER_DATA;

	// Num vertices
	uint64 numVertices;
	fin.readUInt64(&numVertices);

	// Num indices
	uint64 numIndices;
	fin.readUInt64(&numIndices);

	// Num instances
	uint64 numInstances;
	fin.readUInt64(&numInstances);

	// Num vertex elements
	uint16 numVertexElements;
	fin.readUInt16(&numVertexElements);

	// Vertex buffer desc
	D3D11_BUFFER_DESC vertexBufferDesc;
	fin.read((char *)&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

	// Index buffer desc
	D3D11_BUFFER_DESC indexBufferDesc;
	fin.read((char *)&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

	// Instance buffer desc
	D3D11_BUFFER_DESC instanceBufferDesc;
	ZeroMemory(&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));

	if (hasInstanceBuffer) {
		fin.read((char *)&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));
	}

	// Vertex data
	char *vertexData = new char[vertexBufferDesc.ByteWidth];
	fin.read(vertexData, vertexBufferDesc.ByteWidth);

	// Index data
	char *indexData = new char[indexBufferDesc.ByteWidth];
	fin.read(indexData, indexBufferDesc.ByteWidth);

	// Instance data
	char *instanceData = nullptr;
	if (hasInstanceData) {
		instanceData = new char[instanceBufferDesc.ByteWidth];
		fin.read(instanceData, instanceBufferDesc.ByteWidth);
	}

	// Num subsets
	uint32 numSubsets;
	fin.readUInt32(&numSubsets);

	// Subset data
	Subset *subsets = new Subset[numSubsets];
	fin.read((char *)subsets, sizeof(Subset) * numSubsets);

	// String table
	std::string *stringTable = nullptr;
	if ((flags & HAS_STRING_TABLE) == HAS_STRING_TABLE) {
		uint32 numStrings;
		fin.readUInt32(&numStrings);

		stringTable = new std::string[numStrings];

		for (uint i = 0; i < numStrings; ++i) {
			uint16 stringLength;
			fin.readUInt16(&stringLength);

			// Read in the string characters
			stringTable[i].reserve(stringLength + 1); // Allow space for the null terminator
			fin.read(&stringTable[i][0], stringLength);

			// Manually create the null terminator
			stringTable[i][stringLength] = '\0';
		}
	}

	// Process the subsets
	ModelSubset *modelSubsets = new ModelSubset[numSubsets];
	for (uint i = 0; i < numSubsets; ++i) {
		modelSubsets[i].VertexStart = subsets[i].VertexStart;
		modelSubsets[i].VertexCount = subsets[i].VertexCount;
		modelSubsets[i].IndexStart = subsets[i].IndexStart;
		modelSubsets[i].IndexCount = subsets[i].IndexCount;

		modelSubsets[i].Material.Ambient.x = subsets[i].MatAmbientColor[0];
		modelSubsets[i].Material.Ambient.y = subsets[i].MatAmbientColor[1];
		modelSubsets[i].Material.Ambient.z = subsets[i].MatAmbientColor[2];
		modelSubsets[i].Material.Ambient.w = subsets[i].MatSpecIntensity;

		modelSubsets[i].Material.Diffuse.x = subsets[i].MatDiffuseColor[0];
		modelSubsets[i].Material.Diffuse.y = subsets[i].MatDiffuseColor[1];
		modelSubsets[i].Material.Diffuse.z = subsets[i].MatDiffuseColor[2];
		modelSubsets[i].Material.Diffuse.w = subsets[i].MatDiffuseColor[3];

		modelSubsets[i].Material.Specular.x = subsets[i].MatSpecColor[0];
		modelSubsets[i].Material.Specular.y = subsets[i].MatSpecColor[1];
		modelSubsets[i].Material.Specular.z = subsets[i].MatSpecColor[2];
		modelSubsets[i].Material.Specular.w = subsets[i].MatSpecPower;

		if (subsets[i].diffuseColorMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].diffuseColorMapIndex].begin(), stringTable[subsets[i].diffuseColorMapIndex].end());
			modelSubsets[i].DiffuseColorSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::DIFFUSE_COLOR;
		}
		if (subsets[i].specColorMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].specColorMapIndex].begin(), stringTable[subsets[i].specColorMapIndex].end());
			modelSubsets[i].SpecularColorSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::SPEC_COLOR;
		}
		if (subsets[i].specPowerMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].specPowerMapIndex].begin(), stringTable[subsets[i].specPowerMapIndex].end());
			modelSubsets[i].SpecularPowerSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::SPEC_POWER;
		}
		if (subsets[i].alphaMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].alphaMapIndex].begin(), stringTable[subsets[i].alphaMapIndex].end());
			modelSubsets[i].AlphaMapSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::ALPHA_MAP;
		}
		if (subsets[i].displacementMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].displacementMapIndex].begin(), stringTable[subsets[i].displacementMapIndex].end());
			modelSubsets[i].DisplacementMapSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::DISPLACEMENT_MAP;
		}
		if (subsets[i].normalMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].normalMapIndex].begin(), stringTable[subsets[i].normalMapIndex].end());
			modelSubsets[i].NormalMapSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::NORMAL_MAP;
		}
	}

	// Create the model with the read data
	Common::Model *model = new Common::Model();

	model->CreateVertexBuffer(device, vertexData, numVertices, vertexBufferDesc);
	model->CreateIndexBuffer(device, (uint *)indexData, numIndices, indexBufferDesc);
	if (hasInstanceBuffer) {
		model->CreateInstanceBuffer(device, numInstances, instanceBufferDesc, instanceData);
	}
	model->CreateSubsets(modelSubsets, numSubsets);

	return model;
}

} // End of namespace Common
