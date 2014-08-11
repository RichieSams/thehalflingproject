/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "engine/model_manager.h"

#include "scene/halfling_model_file.h"
#include "engine/texture_manager.h"
#include "engine/material_shader_manager.h"

#include <fastformat/fastformat.hpp>


namespace Engine {

ModelManager::~ModelManager() {
	for (auto iter = m_modelCache.begin(); iter != m_modelCache.end(); ++iter) {
		delete iter->second;
	}
}

Scene::Model *ModelManager::GetModel(ID3D11Device *device, TextureManager *textureManager, MaterialShaderManager *materialShaderManager, Graphics::SamplerStates *samplerStates, std::wstring filePath) {
	// First check the cache
	auto iter = m_modelCache.find(filePath);
	if (iter != m_modelCache.end()) {
		return iter->second;
	}

	// Else create it from scratch
	Scene::Model *newModel = Scene::HalflingModelFile::Load(device, textureManager, materialShaderManager, samplerStates, filePath.c_str());

	// Lock the cache before writing
	std::lock_guard<std::mutex> guard(m_cacheLock);

	// Write
	m_modelCache[filePath] = newModel;

	// The mutex will unlock when 'guard' goes out of scope and destructs

	return newModel;
}

Scene::Model *ModelManager::CreateUnnamedModel() {
	// Lock the cache before writing
	std::lock_guard<std::mutex> guard(m_cacheLock);

	// Write
	std::wstring newModelName;
	fastformat::write(newModelName, L"unnamedModel", m_unnamedModelIncrementer++);

	Scene::Model *newModel = new Scene::Model();

	m_modelCache[newModelName] = newModel;

	// The mutex will unlock when 'guard' goes out of scope and destructs

	return newModel;
}

} // End of namespace Engine
