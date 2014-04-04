/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/typedefs.h"

#include "common/console_progress_bar.h"
#include "common/halfling_model_file.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DirectXMath.h>

#include <vector>


struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 tangent;
};

int main(int argc, char *argv[]) {
	rlutil::cls();
	std::cout << "Converting..." << std::endl;
	
	Common::ConsoleProgressBar progressBar;
	progressBar.SetVerticalOffset(3);

	// Import the obj file
	Assimp::Importer importer;
	importer.ReadFile("sponza.obj", aiProcess_ConvertToLeftHanded | 
	                                aiProcess_Triangulate |
	                                aiProcess_JoinIdenticalVertices |
	                                aiProcess_GenSmoothNormals |
	                                aiProcess_ValidateDataStructure |
	                                aiProcess_ImproveCacheLocality |
	                                aiProcess_RemoveRedundantMaterials |
	                                aiProcess_FindInvalidData |
	                                aiProcess_OptimizeMeshes |
	                                aiProcess_OptimizeGraph);

	// Generate tangents
	const aiScene* scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);

	// If the import failed, report it
	if (!scene) {
		std::cout << importer.GetErrorString();
		return 1;
	}

	std::cout << "Done Importing" << std::endl << "Start Processing" << std::endl;

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

		for (uint j = 0; j < mesh->mNumVertices; ++j) {
			Vertex vertex;
			vertex.pos = DirectX::XMFLOAT3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
			vertex.normal = mesh->HasNormals() ? DirectX::XMFLOAT3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z) : DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertex.texCoord = mesh->HasTextureCoords(0) ? DirectX::XMFLOAT2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y) : DirectX::XMFLOAT2(0.0f, 0.0f);
			vertex.tangent = mesh->HasTangentsAndBitangents() ? DirectX::XMFLOAT3(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z) : DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			
			vertices.push_back(vertex);
		}

		for (uint j = 0; j < mesh->mNumFaces; ++j) {
			for (uint k = 0; k < mesh->mFaces[j].mNumIndices; ++k) {
				indices.push_back(mesh->mFaces[j].mIndices[k]);
			}
		}

		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

		aiColor3D color;
		float value;
		aiString string;

		if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == aiReturn_SUCCESS) {
			subset.MatAmbientColor = DirectX::XMFLOAT3(color.r, color.g, color.b);
		}
		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS) {
			subset.MatDiffuseColor = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f);
		}
		if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS) {
			subset.MatSpecColor = DirectX::XMFLOAT3(color.r, color.g, color.b);
		}
		if (material->Get(AI_MATKEY_OPACITY, value) == aiReturn_SUCCESS) {
			subset.MatDiffuseColor.w = value;
		}
		if (material->Get(AI_MATKEY_SHININESS, value) == aiReturn_SUCCESS) {
			subset.MatSpecPower = value;
		}
		if (material->Get(AI_MATKEY_SHININESS_STRENGTH, value) == aiReturn_SUCCESS) {
			subset.MatSpecIntensity = value;
		} else {
			subset.MatSpecIntensity = 1.0f;
		}

		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.diffuseColorMapIndex = stringTable.size();
			stringTable.push_back(string.data);
		} else {
			subset.diffuseColorMapIndex = -1;
		}
		if (material->GetTexture(aiTextureType_NORMALS, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.normalMapIndex = stringTable.size();
			stringTable.push_back(string.data);
		} else {
			subset.normalMapIndex = -1;
		}
		if (material->GetTexture(aiTextureType_DISPLACEMENT, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.displacementMapIndex = stringTable.size();
			stringTable.push_back(string.data);
		} else {
			subset.displacementMapIndex = -1;
		}
		if (material->GetTexture(aiTextureType_OPACITY, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.alphaMapIndex = stringTable.size();
			stringTable.push_back(string.data);
		} else {
			subset.alphaMapIndex = -1;
		}
		if (material->GetTexture(aiTextureType_SPECULAR, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.specColorMapIndex = stringTable.size();
			stringTable.push_back(string.data);
		} else {
			subset.specColorMapIndex = -1;
		}
		if (material->GetTexture(aiTextureType_SHININESS, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.specPowerMapIndex = stringTable.size();
			stringTable.push_back(string.data);
		} else {
			subset.specPowerMapIndex = -1;
		}

		subsets.push_back(subset);
	}

	std::cout << "Done Processing" << std::endl << "Writing to file" << std::endl;

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(D3D11_BUFFER_DESC));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(D3D11_BUFFER_DESC));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(uint)* indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	Common::HalflingModelFile::Write(L"sponza.hmf", vertices.size(), indices.size(), &vbd, &ibd, nullptr, &vertices[0], &indices[0], nullptr, subsets, stringTable);

	std::cout << "Finished write" << std::endl << "Press any key to exit..." << std::endl;

	rlutil::anykey();
}