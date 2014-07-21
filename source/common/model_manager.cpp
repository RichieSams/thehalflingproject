/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/model_manager.h"

#include "common/halfling_model_file.h"


namespace Common {

ModelManager::~ModelManager() {
	for (auto iter = m_modelCache.begin(); iter != m_modelCache.end(); ++iter) {
		delete iter->second;
	}
}

Model *ModelManager::GetModel(ID3D11Device *device, Common::TextureManager *textureManager, std::wstring filePath) {
	// First check the cache
	auto iter = m_modelCache.find(filePath);
	if (iter != m_modelCache.end()) {
		return iter->second;
	}

	// Else create it from scratch
	Model *newModel = Common::HalflingModelFile::Load(device, textureManager, filePath.c_str());

	// Lock the cache before writing
	std::lock_guard<std::mutex> guard(m_cacheLock);

	// Write
	m_modelCache[filePath] = newModel;

	// The mutex will unlock when 'guard' goes out of scope and destructs

	return newModel;
}

} // End of namespace Common
