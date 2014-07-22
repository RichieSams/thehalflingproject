/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/model_manager.h"

#include "common/halfling_model_file.h"
#include "common/texture_manager.h"
#include "common/material_shader_manager.h"

#include <fastformat/fastformat.hpp>


namespace Common {

ModelManager::~ModelManager() {
	for (auto iter = m_modelCache.begin(); iter != m_modelCache.end(); ++iter) {
		delete iter->second;
	}
}

Model *ModelManager::GetModel(ID3D11Device *device, Common::TextureManager *textureManager, Common::MaterialShaderManager *materialShaderManager, std::wstring filePath) {
	// First check the cache
	auto iter = m_modelCache.find(filePath);
	if (iter != m_modelCache.end()) {
		return iter->second;
	}

	// Else create it from scratch
	Model *newModel = Common::HalflingModelFile::Load(device, textureManager, materialShaderManager, filePath.c_str());

	// Lock the cache before writing
	std::lock_guard<std::mutex> guard(m_cacheLock);

	// Write
	m_modelCache[filePath] = newModel;

	// The mutex will unlock when 'guard' goes out of scope and destructs

	return newModel;
}

Model *ModelManager::CreateUnnamedModel() {
	// Lock the cache before writing
	std::lock_guard<std::mutex> guard(m_cacheLock);

	// Write
	std::wstring newModelName;
	fastformat::write(newModelName, L"unnamedModel", m_unnamedModelIncrementer++);

	Model *newModel = new Model();

	m_modelCache[newModelName] = newModel;

	// The mutex will unlock when 'guard' goes out of scope and destructs

	return newModel;
}

} // End of namespace Common
