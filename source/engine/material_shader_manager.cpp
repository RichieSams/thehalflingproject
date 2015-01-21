/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "engine/material_shader_manager.h"


namespace Engine {

void MaterialShaderManager::Initialize(ID3D11Device *device, const wchar *defaultMaterialShaderFilePath) {
	m_defaultMaterialShader = new Graphics::MaterialShader(defaultMaterialShaderFilePath, device, false, false);
}

Graphics::MaterialShader *MaterialShaderManager::GetShader(ID3D11Device *device, const std::wstring &filePath) {
	// First check the cache
	auto iter = m_shaderCache.find(filePath);
	if (iter != m_shaderCache.end()) {
		return &iter->second;
	}

	// Else create it from scratch
	// Lock the cache before writing
	std::lock_guard<std::mutex> guard(m_cacheLock);

	// Write
	auto returnArg = m_shaderCache.emplace(std::piecewise_construct,
										   std::forward_as_tuple(filePath),
										   std::forward_as_tuple(filePath.c_str(), device, false, false));

	// Return the created element
	return &((*returnArg.first).second);

	// The mutex will unlock when 'guard' goes out of scope and destructs
}

} // End of namespace Engine
