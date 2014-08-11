/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "engine/material_shader_manager.h"


namespace Engine {

MaterialShaderManager::~MaterialShaderManager() {
	for (auto iter = m_shaderCache.begin(); iter != m_shaderCache.end(); ++iter) {
		delete *iter;
	}
}

void MaterialShaderManager::Initialize(ID3D11Device *device, const wchar *defaultMaterialShaderFilePath) {
	m_defaultMaterialShader = new Graphics::MaterialShader(defaultMaterialShaderFilePath, device, false, false);
}

size_t MaterialShaderManager::CreateShader(ID3D11Device *device, const std::wstring filePath) {
	// First check the cache
	auto iter = m_shaderIdLookup.find(filePath);
	if (iter != m_shaderIdLookup.end()) {
		return iter->second;
	}

	// Else create it from scratch
	Graphics::MaterialShader *newShader = new Graphics::MaterialShader(filePath.c_str(), device, false, false);

	// Lock the cache before writing
	std::lock_guard<std::mutex> guard(m_cacheLock);

	// Write
	size_t newHandle = m_shaderCache.size();
	
	m_shaderIdLookup[filePath] = newHandle;
	m_shaderCache.push_back(newShader);

	// The mutex will unlock when 'guard' goes out of scope and destructs

	return newHandle;
}

Graphics::MaterialShader *MaterialShaderManager::GetShader(size_t handle) {
	// We have to lock the cache because reads from vector are not 
	// thread-safe if we can't guarantee that it won't be written to during a read
	//
	// While we *sort-of* know this won't happen since (atm) the shaders are created
	// at load, this imposes a restriction on potential future run-time loading
	std::lock_guard<std::mutex> guard(m_cacheLock);

	if (handle >= m_shaderCache.size()) {
		return m_defaultMaterialShader;
	}

	// The mutex will unlock when 'guard' goes out of scope and destructs

	return m_shaderCache[handle];
}

} // End of namespace Engine
