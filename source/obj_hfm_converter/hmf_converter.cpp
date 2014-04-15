/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "obj_hfm_converter/hmf_converter.h"

#include "common/typedefs.h"
#include "common/halfling_model_file.h"

#include "obj_hfm_converter/util.h"

#define DINI_MAX_LINE 1000
#include "INIReader.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>


namespace ObjHmfConverter {

bool ConvertToHMF(std::tr2::sys::path &baseDirectory, std::tr2::sys::path &inputFilePath, std::tr2::sys::path &iniFilePath, std::tr2::sys::path &outputFilePath) {
	std::tr2::sys::path inputDirectory(inputFilePath.parent_path());
	if (!inputFilePath.has_parent_path()) {
		inputDirectory = std::tr2::sys::current_path<std::tr2::sys::path>();
	}

	// If the output path doesn't exist, just use the input path with the .hmf extension
	if (outputFilePath.empty()) {
		outputFilePath = inputFilePath;
		outputFilePath.replace_extension("hmf");
	}

	std::tr2::sys::path outputDirectory(outputFilePath.parent_path());
	if (!outputFilePath.has_parent_path()) {
		outputDirectory = std::tr2::sys::current_path<std::tr2::sys::path>();
	}

	// Set default values
	ImporterIniFile iniFile;

	// Process the ini file
	if (!iniFilePath.empty()) {
		std::cout << "Parsing ini file... ";

		INIReader reader(iniFilePath);
		if (reader.ParseError() != 0) {
			std::cout << "Ini file parse error" << std::endl;
			return false;
		}

		iniFile.GenNormals = reader.GetBoolean("post-processing", "gennormals", iniFile.GenNormals);
		iniFile.CalcTangents = reader.GetBoolean("post-processing", "calctangents", iniFile.CalcTangents);

		iniFile.UseMaterialAmbientColor = reader.GetBoolean("materialpropertyoverrides", "ambientcolor", iniFile.UseMaterialAmbientColor);
		iniFile.UseMaterialDiffuseColor = reader.GetBoolean("materialpropertyoverrides", "diffusecolor", iniFile.UseMaterialDiffuseColor);
		iniFile.UseMaterialSpecColor = reader.GetBoolean("materialpropertyoverrides", "speccolor", iniFile.UseMaterialSpecColor);
		iniFile.UseMaterialOpacity = reader.GetBoolean("materialpropertyoverrides", "opacity", iniFile.UseMaterialOpacity);
		iniFile.UseMaterialSpecPower = reader.GetBoolean("materialpropertyoverrides", "specpower", iniFile.UseMaterialSpecPower);
		iniFile.UseMaterialSpecIntensity = reader.GetBoolean("materialpropertyoverrides", "specintensity", iniFile.UseMaterialSpecIntensity);

		iniFile.UseDiffuseColorMap = reader.GetBoolean("textureoverrides", "diffusecolormap", iniFile.UseDiffuseColorMap);
		iniFile.UseNormalMap = reader.GetBoolean("textureoverrides", "normalmap", iniFile.UseNormalMap);
		iniFile.UseDisplacementMap = reader.GetBoolean("textureoverrides", "displacementmap", iniFile.UseDisplacementMap);
		iniFile.UseAlphaMap = reader.GetBoolean("textureoverrides", "alphamap", iniFile.UseAlphaMap);
		iniFile.UseSpecColorMap = reader.GetBoolean("textureoverrides", "speccolormap", iniFile.UseSpecColorMap);
		iniFile.UseSpecPowerMap = reader.GetBoolean("textureoverrides", "specpowermap", iniFile.UseSpecPowerMap);

		iniFile.VertexBufferUsage = ParseUsageFromString(reader.Get("bufferdesc", "vertexbufferusage", "immutable"));
		iniFile.IndexBufferUsage = ParseUsageFromString(reader.Get("bufferdesc", "indexbufferusage", "immutable"));

		iniFile.DiffuseColorMapTextureType = ParseTextureTypeFromString(reader.Get("texturemapredirects", "diffusecolormap", "diffuse"), aiTextureType_DIFFUSE);
		iniFile.NormalMapTextureType = ParseTextureTypeFromString(reader.Get("texturemapredirects", "normalmap", "normal"), aiTextureType_NORMALS);
		iniFile.DisplacementMapTextureType = ParseTextureTypeFromString(reader.Get("texturemapredirects", "displacementmap", "displacement"), aiTextureType_DISPLACEMENT);
		iniFile.AlphaMapTextureType = ParseTextureTypeFromString(reader.Get("texturemapredirects", "alphamap", "alpha"), aiTextureType_OPACITY);
		iniFile.SpecColorMapTextureType = ParseTextureTypeFromString(reader.Get("texturemapredirects", "speccolormap", "specColor"), aiTextureType_SPECULAR);
		iniFile.SpecPowerMapTextureType = ParseTextureTypeFromString(reader.Get("texturemapredirects", "specpowermap", "specPower"), aiTextureType_SHININESS);

		std::cout << "Done" << std::endl;
	}

	uint postProcessingFlags = aiProcess_ConvertToLeftHanded |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FindInvalidData |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph;

	if (iniFile.GenNormals) {
		postProcessingFlags |= aiProcess_GenSmoothNormals;
	}


	std::cout << "Importing model... ";

	// Import the model file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inputFilePath.file_string(), postProcessingFlags);

	// Generate tangents
	if (iniFile.CalcTangents) {
		scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
	}

	// If the import failed, report it
	if (!scene) {
		std::cout << importer.GetErrorString();
		return false;
	}

	std::cout << "Done" << std::endl << "Converting... ";

	// Extract the data from the assimp scene
	std::vector<Vertex> vertices;
	std::vector<uint> indices;
	std::vector<Common::HalflingModelFile::Subset> subsets;
	std::vector<std::string> stringTable;

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

		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

		aiColor3D color;
		float value;
		aiString string;

		if (iniFile.UseMaterialAmbientColor && material->Get(AI_MATKEY_COLOR_AMBIENT, color) == aiReturn_SUCCESS) {
			subset.MatAmbientColor = DirectX::XMFLOAT3(color.r, color.g, color.b);
		}
		if (iniFile.UseMaterialDiffuseColor && material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS) {
			subset.MatDiffuseColor = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f);
		}
		if (iniFile.UseMaterialSpecColor && material->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS) {
			subset.MatSpecColor = DirectX::XMFLOAT3(color.r, color.g, color.b);
		}
		if (iniFile.UseMaterialOpacity && material->Get(AI_MATKEY_OPACITY, value) == aiReturn_SUCCESS) {
			subset.MatDiffuseColor.w = value;
		}
		if (iniFile.UseMaterialSpecPower && material->Get(AI_MATKEY_SHININESS, value) == aiReturn_SUCCESS) {
			subset.MatSpecPower = value;
		}
		if (iniFile.UseMaterialSpecIntensity && material->Get(AI_MATKEY_SHININESS_STRENGTH, value) == aiReturn_SUCCESS) {
			subset.MatSpecIntensity = value;
		}

		if (iniFile.UseDiffuseColorMap && material->GetTexture(iniFile.DiffuseColorMapTextureType, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.DiffuseColorMapIndex = stringTable.size();
			// Guarantee it's a dds file
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseNormalMap && material->GetTexture(iniFile.NormalMapTextureType, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.NormalMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseDisplacementMap && material->GetTexture(iniFile.DisplacementMapTextureType, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.DisplacementMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseAlphaMap && material->GetTexture(iniFile.AlphaMapTextureType, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.AlphaMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseSpecColorMap && material->GetTexture(iniFile.SpecColorMapTextureType, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.SpecColorMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseSpecPowerMap && material->GetTexture(iniFile.SpecPowerMapTextureType, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.SpecPowerMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}

		subsets.push_back(subset);
	}
	
	std::cout << "Done" << std::endl << "Writing to file... ";

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(D3D11_BUFFER_DESC));
	vbd.Usage = iniFile.VertexBufferUsage;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(D3D11_BUFFER_DESC));
	ibd.Usage = iniFile.IndexBufferUsage;
	ibd.ByteWidth = sizeof(uint)* indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	std::string outputPathStr(outputFilePath.file_string());
	std::wstring wideString(outputPathStr.begin(), outputPathStr.end());
	Common::HalflingModelFile::Write(wideString.c_str(), vertices.size(), indices.size(), &vbd, &ibd, nullptr, &vertices[0], &indices[0], nullptr, subsets, stringTable);

	std::cout << "Done" << std::endl << "Finished" << std::endl;

	return true;
}

} // End of namespace ObjHmfConverter
