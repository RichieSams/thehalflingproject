/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/halfling_model_file.h"

#include "common/file_io_util.h"
#include "common/memory_stream.h"
#include "common/endian.h"

#include <string>
#include <fstream>

namespace Common {

Common::Model *Common::HalflingModelFile::Load(ID3D11Device *device, Common::TextureManager *textureManager, const wchar *filePath) {
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
	assert(fileFormatVersion == kFileFormatVersion);

	// Flags
	uint64 flags;
	fin.readUInt64(&flags);

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

	// Vertex data
	char *vertexData = new char[vertexBufferDesc.ByteWidth];
	fin.read(vertexData, vertexBufferDesc.ByteWidth);

	// Index data
	char *indexData = new char[indexBufferDesc.ByteWidth];
	fin.read(indexData, indexBufferDesc.ByteWidth);

	// Material table
	MaterialTableData *materialTable = nullptr;
	if ((flags & HAS_MATERIAL_TABLE) == HAS_MATERIAL_TABLE) {
		uint32 numMaterials;
		fin.readUInt32(&numMaterials);

		materialTable = new MaterialTableData[numMaterials];

		for (uint i = 0; i < numMaterials; ++i) {
			fin.readUInt32(&materialTable[i].HMATFilePathIndex);
			
			uint32 numTextures;
			fin.readUInt32(&numTextures);

			for (uint j = 0; j < numTextures; ++j) {
				TextureData data;
				fin.readUInt32(&data.FilePathIndex);
				fin.readByte(&data.Sampler);
				materialTable[i].Textures.push_back(data);
			}
		}
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

		MaterialTableData materialData = materialTable[subsets[i].MaterialIndex];

		// TODO: Actually figure out how to do shader index....
		std::string hmatFilePath = stringTable[materialData.HMATFilePathIndex];

		for (uint j = 0; j < materialData.Textures.size(); ++j) {
			std::wstring wideFileName(stringTable[materialData.Textures[j].FilePathIndex].begin(), stringTable[materialData.Textures[j].FilePathIndex].end());
			modelSubsets[i].TextureSRVs.push_back(textureManager->GetSRVFromFile(device, wideFileName, D3D11_USAGE_IMMUTABLE));
			modelSubsets[i].TextureSamplers.push_back(static_cast<TextureSampler>(materialData.Textures[j].Sampler));
		}
	}

	// Cleanup
	delete[] stringTable;
	delete[] materialTable;
	delete[] subsets;

	// Create the model with the read data
	Common::Model *model = new Common::Model();

	model->CreateVertexBuffer(device, vertexData, numVertices, vertexBufferDesc);
	model->CreateIndexBuffer(device, (uint *)indexData, numIndices, indexBufferDesc);
	model->CreateSubsets(modelSubsets, numSubsets);

	return model;
}


void HalflingModelFile::Write(const wchar *filepath, uint numVertices, uint numIndices, D3D11_BUFFER_DESC *vertexBufferDesc, D3D11_BUFFER_DESC *indexBufferDesc, D3D11_BUFFER_DESC *instanceBufferDesc, void *vertexData, void *indexData, void *instanceData, std::vector<Subset> &subsets, std::vector<std::string> &stringTable, std::vector<MaterialTableData> &materialTable) {
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

		BinaryWriteUInt32(fout, stringTableSize);
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

	// Vertex data
	fout.write(reinterpret_cast<const char *>(vertexData), vertexBufferDesc->ByteWidth);

	// Index data
	fout.write(reinterpret_cast<const char *>(indexData), indexBufferDesc->ByteWidth);

	// Material table
	size_t materialTableSize = materialTable.size();
	if (materialTableSize > 0) {
		flags |= HAS_MATERIAL_TABLE;

		BinaryWriteUInt32(fout, materialTableSize);
		for (uint i = 0; i < materialTableSize; ++i) {
			BinaryWriteUInt32(fout, materialTable[i].HMATFilePathIndex);
			
			size_t textureListSize = materialTable[i].Textures.size();
			BinaryWriteUInt32(fout, textureListSize);

			for (uint j = 0; j < textureListSize; ++j) {
				BinaryWriteUInt32(fout, materialTable[i].Textures[j].FilePathIndex);
				BinaryWriteByte(fout, materialTable[i].Textures[j].Sampler);
			}
			
		}
	}

	// Subsets
	BinaryWriteUInt32(fout, subsets.size());
	fout.write(reinterpret_cast<const char *>(&subsets[0]), sizeof(Subset) * subsets.size());

	// Go back and re-write flags
	fout.seekp(flagsPos);
	BinaryWriteInt64(fout, flags);

	// Cleanup
	fout.flush();
	fout.close();
}

void HalflingModelFile::VerifyFileIntegrity(const wchar *filepath) {
	// Read the entire file into memory
	DWORD bytesRead;
	char *fileBuffer = ReadWholeFile(filepath, &bytesRead);
	assert(fileBuffer != NULL);

	Common::MemoryInputStream fin(fileBuffer, bytesRead);

	// Read in the file data

	// Check that this is a 'HFM' file
	uint32 fileId;
	fin.readUInt32(&fileId);
	assert(fileId == MKTAG('\0', 'F', 'M', 'H'));

	// File format version
	byte fileFormatVersion;
	fin.readByte(&fileFormatVersion);
	assert(fileFormatVersion == kFileFormatVersion);

	// Flags
	uint64 flags;
	fin.readUInt64(&flags);

	// String table
	std::string *stringTable = nullptr;
	uint32 numStrings;
	if ((flags & HAS_STRING_TABLE) == HAS_STRING_TABLE) {
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

	// Vertex buffer desc
	D3D11_BUFFER_DESC vertexBufferDesc;
	fin.read((char *)&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

	// Index buffer desc
	D3D11_BUFFER_DESC indexBufferDesc;
	fin.read((char *)&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

	// Vertex data
	char *vertexData = new char[vertexBufferDesc.ByteWidth];
	fin.read(vertexData, vertexBufferDesc.ByteWidth);

	// Index data
	char *indexData = new char[indexBufferDesc.ByteWidth];
	fin.read(indexData, indexBufferDesc.ByteWidth);

	// Material table
	MaterialTableData *materialTable = nullptr;
	uint32 numMaterials;
	if ((flags & HAS_MATERIAL_TABLE) == HAS_MATERIAL_TABLE) {
		fin.readUInt32(&numMaterials);

		materialTable = new MaterialTableData[numMaterials];

		for (uint i = 0; i < numMaterials; ++i) {
			fin.readUInt32(&materialTable[i].HMATFilePathIndex);

			uint32 numTextures;
			fin.readUInt32(&numTextures);

			for (uint j = 0; j < numTextures; ++j) {
				TextureData data;
				fin.readUInt32(&data.FilePathIndex);
				fin.readByte(&data.Sampler);
				materialTable[i].Textures.push_back(data);
			}
		}
	}

	// Num subsets
	uint32 numSubsets;
	fin.readUInt32(&numSubsets);

	// Subset data
	Subset *subsets = new Subset[numSubsets];
	fin.read((char *)subsets, sizeof(Subset) * numSubsets);

	// Process the subsets
	for (uint i = 0; i < numSubsets; ++i) {
		assert(subsets[i].VertexCount > 0);
		assert(subsets[i].IndexCount > 0);

		MaterialTableData materialData = materialTable[subsets[i].MaterialIndex];

		assert(materialData.HMATFilePathIndex < numStrings);

		for (uint j = 0; j < materialData.Textures.size(); ++j) {
			assert(materialData.Textures[j].FilePathIndex < numStrings);
			assert(materialData.Textures[j].Sampler >= LINEAR_CLAMP && materialData.Textures[j].Sampler <= ANISOTROPIC_WRAP);
		}
	}

	// Cleanup
	delete[] stringTable;
	delete[] materialTable;
	delete[] subsets;
}

} // End of namespace Common
