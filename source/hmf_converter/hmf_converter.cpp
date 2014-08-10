/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "hmf_converter/hmf_converter.h"

#include "hmf_converter/util.h"

#include "common/typedefs.h"
#include "common/halfling_model_file.h"
#include "common/file_io_util.h"
#include "common/memory_stream.h"

#include <json/reader.h>
#include <json/value.h>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>


using filepath = std::tr2::sys::path;

namespace ObjHmfConverter {

bool ConvertToHMF(filepath &baseDirectory, filepath &inputFilePath, filepath &jsonFilePath, filepath &outputFilePath) {
	filepath inputDirectory(inputFilePath.parent_path());
	if (!inputFilePath.has_parent_path()) {
		inputDirectory = std::tr2::sys::current_path<filepath>();
	}

	// If the output path doesn't exist, just use the input path with the .hmf extension
	if (outputFilePath.empty()) {
		outputFilePath = inputFilePath;
		outputFilePath.replace_extension("hmf");
	}

	filepath outputDirectory(outputFilePath.parent_path());
	if (!outputFilePath.has_parent_path()) {
		outputDirectory = std::tr2::sys::current_path<filepath>();
	}

	// Process the json file
	if (jsonFilePath.empty()) {
		std::cout << "Json file required" << std::endl;
		return false;
	}

	std::cout << "Parsing json file... ";

	// Read the entire file into memory
	DWORD bytesRead;
	std::string str(jsonFilePath);
	std::wstring wideStr(str.begin(), str.end());
	char *fileBuffer = Common::ReadWholeFile(wideStr.c_str(), &bytesRead);

	// TODO: Add error handling
	if (fileBuffer == NULL) {
		std::cout << "Json file open error" << std::endl;
		return false;
	}

	Common::MemoryInputStream fin(fileBuffer, bytesRead);

	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(fin, root, false)) {
		std::cout << "Json file parse error: " << std::endl << reader.getFormatedErrorMessages() << std::endl;
		return false;
	}

	// Structures to store the data
	std::vector<Vertex> vertices;
	std::vector<uint> indices;
	std::vector<Common::HalflingModelFile::Subset> subsets;
	std::vector<std::string> stringTable;
	std::unordered_map<std::string, size_t> stringLookup;
	std::vector<Common::HalflingModelFile::MaterialTableData> materialTable;
	std::unordered_map<std::string, size_t> materialLookup;

	ImporterJsonFile jsonFile;
	jsonFile.GenNormals = root.get("GenNormals", jsonFile.GenNormals).asBool();
	jsonFile.CalcTangents = root.get("CalcTangents", jsonFile.CalcTangents).asBool();

	jsonFile.VertexBufferUsage = ParseUsageFromString(root.get("VertexBufferUsage", "immutable").asString());
	jsonFile.IndexBufferUsage = ParseUsageFromString(root.get("IndexBufferUsage", "immutable").asString());

	for (uint i = 0; i < root["MaterialDefinitions"].size(); ++i) {
		Json::Value materialDefinition = root["MaterialDefinitions"][i];

		std::string materialName = materialDefinition["MaterialName"].asString();

		if (materialLookup.find(materialName) != materialLookup.end()) {
			std::cout << "Error - Duplicate material: " << materialName << std::endl;
			return false;
		}

		Common::HalflingModelFile::MaterialTableData materialData;

		// Store the hmat file path
		std::string hmatFilePath = materialDefinition["HMATFilePath"].asString();
		// See if it already exists in the string table
		auto stringIter = stringLookup.find(hmatFilePath);
		if (stringIter != stringLookup.end()) {
			materialData.HMATFilePathIndex = stringIter->second;
		} else {
			size_t size = stringTable.size();
			materialData.HMATFilePathIndex = size;
			stringLookup[hmatFilePath] = size;
			stringTable.push_back(hmatFilePath);
		}

		// Store the textures
		for (uint j = 0; j < materialDefinition["TextureDefinitions"].size(); ++j) {
			Json::Value textureDefinition = materialDefinition["TextureDefinitions"][j];

			Common::HalflingModelFile::TextureData data;
			data.Sampler = ParseSamplerTypeFromString(textureDefinition["Sampler"].asString(), Common::LINEAR_WRAP);

			// Guarantee it's a dds file
			std::string fileString(ConvertToDDS(textureDefinition["FilePath"].asString().c_str(), baseDirectory, inputDirectory, outputDirectory));

			// See if it already exists
			stringIter = stringLookup.find(fileString);
			if (stringIter != stringLookup.end()) {
				data.FilePathIndex = stringIter->second;
			} else {
				size_t size = stringTable.size();
				data.FilePathIndex = size;
				stringLookup[fileString] = size;
				stringTable.push_back(fileString);
			}

			materialData.Textures.push_back(data);
		}

		// Store the index
		materialLookup[materialName] = materialTable.size();

		// Push the material onto the table
		materialTable.push_back(materialData);
	}


	std::cout << "Done" << std::endl;

	uint postProcessingFlags = aiProcess_ConvertToLeftHanded |
	                           aiProcess_Triangulate |
	                           aiProcess_JoinIdenticalVertices |
	                           aiProcess_ValidateDataStructure |
	                           aiProcess_ImproveCacheLocality |
	                           aiProcess_RemoveRedundantMaterials |
	                           aiProcess_FindInvalidData |
	                           aiProcess_OptimizeMeshes |
	                           aiProcess_OptimizeGraph;

	if (jsonFile.GenNormals) {
		postProcessingFlags |= aiProcess_GenSmoothNormals;
	}


	std::cout << "Importing model... ";

	// Import the model file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inputFilePath.file_string(), postProcessingFlags);

	// Generate tangents
	if (jsonFile.CalcTangents) {
		scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
	}

	// If the import failed, report it
	if (!scene) {
		std::cout << importer.GetErrorString();
		return false;
	}

	std::cout << "Done" << std::endl << "Converting... ";


	// Extract the data from the assimp scene
	for (uint i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh *mesh = scene->mMeshes[i];

		Common::HalflingModelFile::Subset subset;
		subset.VertexCount = mesh->mNumVertices;
		subset.VertexStart = vertices.size();
		subset.IndexStart = indices.size();
		subset.IndexCount = mesh->mNumFaces * 3;

		DirectX::XMVECTOR AABB_min = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR AABB_max = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		for (uint j = 0; j < mesh->mNumVertices; ++j) {
			Vertex vertex;
			vertex.pos = DirectX::XMFLOAT3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
			vertex.normal = mesh->HasNormals() ? DirectX::XMFLOAT3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z) : DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertex.texCoord = mesh->HasTextureCoords(0) ? DirectX::XMFLOAT2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y) : DirectX::XMFLOAT2(0.0f, 0.0f);
			vertex.tangent = mesh->HasTangentsAndBitangents() ? DirectX::XMFLOAT3(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z) : DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

			AABB_min = DirectX::XMVectorMin(AABB_min, DirectX::XMLoadFloat3(&vertex.pos));
			AABB_max = DirectX::XMVectorMax(AABB_max, DirectX::XMLoadFloat3(&vertex.pos));

			vertices.push_back(vertex);
		}

		DirectX::XMStoreFloat3(&subset.AABB_min, AABB_min);
		DirectX::XMStoreFloat3(&subset.AABB_max, AABB_max);

		for (uint j = 0; j < mesh->mNumFaces; ++j) {
			for (uint k = 0; k < mesh->mFaces[j].mNumIndices; ++k) {
				indices.push_back(mesh->mFaces[j].mIndices[k]);
			}
		}
		
		aiString name;
		scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, name);

		// See if the material exists within the 
		std::string materialName(name.C_Str());
		auto iter = materialLookup.find(materialName);
		if (iter == materialLookup.end()) {
			std::cout << "Error - Material \"" << materialName << "\" is not defined in the json file" << std::endl;
			return false;
		}
		
		subset.MaterialIndex = iter->second;

		subsets.push_back(subset);
	}
	
	std::cout << "Done" << std::endl << "Writing to file... ";

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(D3D11_BUFFER_DESC));
	vbd.Usage = jsonFile.VertexBufferUsage;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(D3D11_BUFFER_DESC));
	ibd.Usage = jsonFile.IndexBufferUsage;
	ibd.ByteWidth = sizeof(uint)* indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	std::string outputPathStr(outputFilePath.file_string());
	std::wstring wideString(outputPathStr.begin(), outputPathStr.end());
	Common::HalflingModelFile::Write(wideString.c_str(), vertices.size(), indices.size(), &vbd, &ibd, nullptr, &vertices[0], &indices[0], nullptr, subsets, stringTable, materialTable);

	std::cout << "Done" << std::endl << "Verifying file integrity... ";

	Common::HalflingModelFile::VerifyFileIntegrity(wideString.c_str());

	std::cout << "Done" << std::endl << "Finished" << std::endl;

	return true;
}

} // End of namespace ObjHmfConverter
