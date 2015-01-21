/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "scene/model_loading.h"

#include "scene/geometry_generator.h"
#include "scene/model.h"

#include "engine/model_manager.h"
#include "engine/texture_manager.h"
#include "engine/material_shader_manager.h"
#include "engine/material_cache.h"

#include "graphics/device_states.h"


namespace Scene {
	
TextureSampler ParseSamplerTypeFromString(std::string &inputString, TextureSampler defaultType) {
	if (_stricmp(inputString.c_str(), "linear_clamp") == 0) {
		return LINEAR_CLAMP;
	} else if (_stricmp(inputString.c_str(), "linear_border") == 0) {
		return LINEAR_BORDER;
	} else if (_stricmp(inputString.c_str(), "linear_wrap") == 0) {
		return LINEAR_WRAP;
	} else if (_stricmp(inputString.c_str(), "point_clamp") == 0) {
		return POINT_CLAMP;
	} else if (_stricmp(inputString.c_str(), "point_wrap") == 0) {
		return POINT_WRAP;
	} else if (_stricmp(inputString.c_str(), "anisotropic_wrap") == 0) {
		return ANISOTROPIC_WRAP;
	} else {
		return defaultType;
	}
}

ID3D11SamplerState *GetSamplerStateFromSamplerType(TextureSampler samplerType, Graphics::SamplerStateManager *samplerStateManager) {
	switch (samplerType) {
	case LINEAR_CLAMP:
		return samplerStateManager->LinearClamp();
		break;
	case LINEAR_BORDER:
		return samplerStateManager->LinearBorder();
		break;
	case LINEAR_WRAP:
		return samplerStateManager->Linear();
		break;
	case POINT_CLAMP:
		return samplerStateManager->Point();
		break;
	case POINT_WRAP:
		return samplerStateManager->PointWrap();
		break;
	case ANISOTROPIC_WRAP:
	default:
		return samplerStateManager->Anisotropic();
		break;
	}
}


Model *FileModelToLoad::CreateModel(ID3D11Device *device, Engine::TextureManager *textureManager, Engine::ModelManager *modelManager, Engine::MaterialShaderManager *materialShaderManager, Engine::MaterialCache *materialCache, Graphics::SamplerStateManager *samplerStateManager) {
	return m_modelManager->GetModel(m_device, m_textureManager, materialShaderManager, materialCache, samplerStateManager, m_filePath.c_str());
}

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 tangent;
};

Model *PlaneModelToLoad::CreateModel(ID3D11Device *device, Engine::TextureManager *textureManager, Engine::ModelManager *modelManager, Engine::MaterialShaderManager *materialShaderManager, Engine::MaterialCache *materialCache, Graphics::SamplerStateManager *samplerStateManager) {
	GeometryGenerator::MeshData meshData;
	ModelSubset *subset = new ModelSubset[1];

	GeometryGenerator::CreateGrid(m_width, m_depth, m_x_subdivisions, m_z_subdivisions, &meshData, m_x_textureTiling, m_z_textureTiling);
	subset->AABB_min = DirectX::XMFLOAT3(-m_width * 0.5f, 0.0f, -m_depth * 0.5f);
	subset->AABB_max = DirectX::XMFLOAT3(m_width * 0.5f, 0.0f, m_depth * 0.5f);

	subset->IndexStart = 0u;
	subset->IndexCount = static_cast<uint>(meshData.Indices.size());
	subset->VertexStart = 0u;
	subset->VertexCount = static_cast<uint>(meshData.Vertices.size());

	Graphics::MaterialShader *shader = materialShaderManager->GetShader(device, m_material.HMATFilePath);
	std::vector<ID3D11ShaderResourceView *> textureSRVs;
	std::vector<ID3D11SamplerState *> textureSamplers;
	for (uint i = 0; i < m_material.Textures.size(); ++i) {
		textureSRVs.push_back(textureManager->GetSRVFromFile(device, m_material.Textures[i].FilePath, D3D11_USAGE_IMMUTABLE));
		textureSamplers.push_back(GetSamplerStateFromSamplerType(m_material.Textures[i].Sampler, samplerStateManager));
	}

	subset->Material = materialCache->getMaterial(shader, textureSRVs, textureSamplers);

	Vertex *vertices = new Vertex[meshData.Vertices.size()];
	for (uint i = 0; i < meshData.Vertices.size(); ++i) {
		vertices[i].pos = meshData.Vertices[i].Position;
		vertices[i].normal = meshData.Vertices[i].Normal;
		vertices[i].texCoord = meshData.Vertices[i].TexCoord;
		vertices[i].tangent = meshData.Vertices[i].Tangent;
	}

	Model *newModel = modelManager->CreateUnnamedModel();
	newModel->CreateVertexBuffer(device, vertices, sizeof(Vertex), static_cast<uint>(meshData.Vertices.size()));
	newModel->CreateIndexBuffer(device, &meshData.Indices[0], static_cast<uint>(meshData.Indices.size()), DisposeAfterUse::NO);
	newModel->CreateSubsets(subset, 1);

	return newModel;
}

Model *BoxModelToLoad::CreateModel(ID3D11Device *device, Engine::TextureManager *textureManager, Engine::ModelManager *modelManager, Engine::MaterialShaderManager *materialShaderManager, Engine::MaterialCache *materialCache, Graphics::SamplerStateManager *samplerStateManager) {
	GeometryGenerator::MeshData meshData;
	ModelSubset *subset = new ModelSubset[1];

	GeometryGenerator::CreateBox(m_width, m_height, m_depth, &meshData);
	subset->AABB_min = DirectX::XMFLOAT3(-m_width * 0.5f, -m_height * 0.5f, -m_depth * 0.5f);
	subset->AABB_max = DirectX::XMFLOAT3(m_width * 0.5f, m_height * 0.5f, m_depth * 0.5f);

	subset->IndexStart = 0u;
	subset->IndexCount = static_cast<uint>(meshData.Indices.size());
	subset->VertexStart = 0u;
	subset->VertexCount = static_cast<uint>(meshData.Vertices.size());

	Graphics::MaterialShader *shader = materialShaderManager->GetShader(device, m_material.HMATFilePath);
	std::vector<ID3D11ShaderResourceView *> textureSRVs;
	std::vector<ID3D11SamplerState *> textureSamplers;
	for (uint i = 0; i < m_material.Textures.size(); ++i) {
		textureSRVs.push_back(textureManager->GetSRVFromFile(device, m_material.Textures[i].FilePath, D3D11_USAGE_IMMUTABLE));
		textureSamplers.push_back(GetSamplerStateFromSamplerType(m_material.Textures[i].Sampler, samplerStateManager));
	}

	subset->Material = materialCache->getMaterial(shader, textureSRVs, textureSamplers);

	Vertex *vertices = new Vertex[meshData.Vertices.size()];
	for (uint i = 0; i < meshData.Vertices.size(); ++i) {
		vertices[i].pos = meshData.Vertices[i].Position;
		vertices[i].normal = meshData.Vertices[i].Normal;
		vertices[i].texCoord = meshData.Vertices[i].TexCoord;
		vertices[i].tangent = meshData.Vertices[i].Tangent;
	}

	Model *newModel = modelManager->CreateUnnamedModel();
	newModel->CreateVertexBuffer(device, vertices, sizeof(Vertex), static_cast<uint>(meshData.Vertices.size()));
	newModel->CreateIndexBuffer(device, &meshData.Indices[0], static_cast<uint>(meshData.Indices.size()), DisposeAfterUse::NO);
	newModel->CreateSubsets(subset, 1);

	return newModel;
}

Model *SphereModelToLoad::CreateModel(ID3D11Device *device, Engine::TextureManager *textureManager, Engine::ModelManager *modelManager, Engine::MaterialShaderManager *materialShaderManager, Engine::MaterialCache *materialCache, Graphics::SamplerStateManager *samplerStateManager) {
	GeometryGenerator::MeshData meshData;
	ModelSubset *subset = new ModelSubset[1];

	GeometryGenerator::CreateSphere(m_radius, m_sliceCount, m_stackCount, &meshData);
	subset->AABB_min = DirectX::XMFLOAT3(-m_radius, -m_radius, -m_radius);
	subset->AABB_max = DirectX::XMFLOAT3(m_radius, m_radius, m_radius);

	subset->IndexStart = 0u;
	subset->IndexCount = static_cast<uint>(meshData.Indices.size());
	subset->VertexStart = 0u;
	subset->VertexCount = static_cast<uint>(meshData.Vertices.size());

	Graphics::MaterialShader *shader = materialShaderManager->GetShader(device, m_material.HMATFilePath);
	std::vector<ID3D11ShaderResourceView *> textureSRVs;
	std::vector<ID3D11SamplerState *> textureSamplers;
	for (uint i = 0; i < m_material.Textures.size(); ++i) {
		textureSRVs.push_back(textureManager->GetSRVFromFile(device, m_material.Textures[i].FilePath, D3D11_USAGE_IMMUTABLE));
		textureSamplers.push_back(GetSamplerStateFromSamplerType(m_material.Textures[i].Sampler, samplerStateManager));
	}

	subset->Material = materialCache->getMaterial(shader, textureSRVs, textureSamplers);

	Vertex *vertices = new Vertex[meshData.Vertices.size()];
	for (uint i = 0; i < meshData.Vertices.size(); ++i) {
		vertices[i].pos = meshData.Vertices[i].Position;
		vertices[i].normal = meshData.Vertices[i].Normal;
		vertices[i].texCoord = meshData.Vertices[i].TexCoord;
		vertices[i].tangent = meshData.Vertices[i].Tangent;
	}

	Model *newModel = modelManager->CreateUnnamedModel();
	newModel->CreateVertexBuffer(device, vertices, sizeof(Vertex), static_cast<uint>(meshData.Vertices.size()));
	newModel->CreateIndexBuffer(device, &meshData.Indices[0], static_cast<uint>(meshData.Indices.size()), DisposeAfterUse::NO);
	newModel->CreateSubsets(subset, 1);

	return newModel;
}

} // End of namespace Scene
