/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/halfling_model_file.h"

#include "common/file_io_util.h"
#include "common/memory_stream.h"
#include "common/endian.h"

#include <string>
#include <fstream>

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
			stringTable[i].resize(stringLength + 1); // Allow space for the null terminator
			fin.read(&stringTable[i][0], stringLength);

			// Manually create the null terminator
			stringTable[i][stringLength] = '\0';
		}
	}

	// Num vertices
	uint32 numVertices;
	fin.readUInt32(&numVertices);

	// Num indices
	uint32 numIndices;
	fin.readUInt32(&numIndices);

	// Num vertex elements
	// TODO: Do we want to store this?

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

	// Process the subsets
	ModelSubset *modelSubsets = new ModelSubset[numSubsets];
	for (uint i = 0; i < numSubsets; ++i) {
		ZeroMemory(&modelSubsets[i], sizeof(ModelSubset));

		modelSubsets[i].VertexStart = subsets[i].VertexStart;
		modelSubsets[i].VertexCount = subsets[i].VertexCount;
		modelSubsets[i].IndexStart = subsets[i].IndexStart;
		modelSubsets[i].IndexCount = subsets[i].IndexCount;

		modelSubsets[i].AABB_min = subsets[i].AABB_min;
		modelSubsets[i].AABB_max = subsets[i].AABB_max;

		modelSubsets[i].Material.Ambient = DirectX::XMFLOAT4(subsets[i].MatAmbientColor.x, subsets[i].MatAmbientColor.y, subsets[i].MatAmbientColor.z, subsets[i].MatSpecIntensity);
		modelSubsets[i].Material.Diffuse = subsets[i].MatDiffuseColor;
		modelSubsets[i].Material.Specular = DirectX::XMFLOAT4(subsets[i].MatSpecColor.x, subsets[i].MatSpecColor.y, subsets[i].MatSpecColor.z, subsets[i].MatSpecPower);

		if (subsets[i].DiffuseColorMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].DiffuseColorMapIndex].begin(), stringTable[subsets[i].DiffuseColorMapIndex].end());
			modelSubsets[i].DiffuseColorSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::DIFFUSE_COLOR;
		}
		if (subsets[i].SpecColorMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].SpecColorMapIndex].begin(), stringTable[subsets[i].SpecColorMapIndex].end());
			modelSubsets[i].SpecularColorSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::SPEC_COLOR;
		}
		if (subsets[i].SpecPowerMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].SpecPowerMapIndex].begin(), stringTable[subsets[i].SpecPowerMapIndex].end());
			modelSubsets[i].SpecularPowerSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::SPEC_POWER;
		}
		if (subsets[i].AlphaMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].AlphaMapIndex].begin(), stringTable[subsets[i].AlphaMapIndex].end());
			modelSubsets[i].AlphaMapSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::ALPHA_MAP;
		}
		if (subsets[i].DisplacementMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].DisplacementMapIndex].begin(), stringTable[subsets[i].DisplacementMapIndex].end());
			modelSubsets[i].DisplacementMapSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::DISPLACEMENT_MAP;
		}
		if (subsets[i].NormalMapIndex != -1) {
			std::wstring wideFileName(stringTable[subsets[i].NormalMapIndex].begin(), stringTable[subsets[i].NormalMapIndex].end());
			modelSubsets[i].NormalMapSRV = textureManager->GetSRVFromFile(device, context, wideFileName, D3D11_USAGE_IMMUTABLE);
			modelSubsets[i].TextureFlags |= Common::TextureFlags::NORMAL_MAP;
		}
	}

	// Create the model with the read data
	Common::Model *model = new Common::Model();

	model->CreateVertexBuffer(device, vertexData, numVertices, vertexBufferDesc);
	model->CreateIndexBuffer(device, (uint *)indexData, numIndices, indexBufferDesc);
	if (hasInstanceBuffer) {
		model->CreateInstanceBuffer(device, instanceBufferDesc, instanceData);
	}
	model->CreateSubsets(modelSubsets, numSubsets);

	return model;
}


void HalflingModelFile::Write(const wchar *filepath, uint numVertices, uint numIndices, D3D11_BUFFER_DESC *vertexBufferDesc, D3D11_BUFFER_DESC *indexBufferDesc, D3D11_BUFFER_DESC *instanceBufferDesc, void *vertexData, void *indexData, void *instanceData, std::vector<Subset> &subsets, std::vector<std::string> &stringTable) {
	std::ofstream fout(filepath, std::ios::out | std::ios::binary);

	// File Id
	BinaryWriteUInt32(fout, MKTAG('\0', 'F', 'M', 'H'));

	// File format version
	BinaryWriteByte(fout, kFileFormatVersion);

	// Flags placeholder
	std::streamoff flagsPos = fout.tellp();
	uint64 flags = 0;
	BinaryWriteInt64(fout, flags);

	// String table
	size_t stringTableSize = stringTable.size();
	if (stringTableSize > 0) {
		flags |= HAS_STRING_TABLE;

		BinaryWriteUInt32(fout, stringTable.size());
		for (uint i = 0; i < stringTableSize; ++i) {
			BinaryWriteUInt16(fout, stringTable[i].size());
			fout.write(stringTable[i].c_str(), stringTable[i].size());
		}
	}

	// Num vertices
	BinaryWriteUInt32(fout, numVertices);

	// Num indices 
	BinaryWriteUInt32(fout, numIndices);

	// Num vertex elements
	// TODO: Do we want to store this?

	// Vertex buffer desc
	fout.write(reinterpret_cast<const char *>(vertexBufferDesc), sizeof(D3D11_BUFFER_DESC));

	// Index buffer desc
	fout.write(reinterpret_cast<const char *>(indexBufferDesc), sizeof(D3D11_BUFFER_DESC));

	// Instance buffer desc
	if (instanceBufferDesc != nullptr) {
		flags |= HAS_INSTANCE_BUFFER;

		fout.write(reinterpret_cast<const char *>(instanceBufferDesc), sizeof(D3D11_BUFFER_DESC));
	}

	// Vertex data
	fout.write(reinterpret_cast<const char *>(vertexData), vertexBufferDesc->ByteWidth);

	// Index data
	fout.write(reinterpret_cast<const char *>(indexData), indexBufferDesc->ByteWidth);

	// Instance data
	if (instanceBufferDesc != nullptr && instanceData != nullptr) {
		flags |= HAS_INSTANCE_BUFFER_DATA;

		fout.write(reinterpret_cast<const char *>(instanceData), instanceBufferDesc->ByteWidth);
	}

	// Subsets
	BinaryWriteUInt32(fout, subsets.size());
	for (uint i = 0; i < subsets.size(); ++i) {
		fout.write(reinterpret_cast<const char *>(&subsets[i]), sizeof(Subset));
	}

	// Go back and re-write flags
	fout.seekp(flagsPos);
	BinaryWriteInt64(fout, flags);

	// Cleanup
	fout.flush();
	fout.close();
}

} // End of namespace Common
