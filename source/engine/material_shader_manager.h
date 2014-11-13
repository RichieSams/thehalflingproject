/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "graphics/shader.h"

#include <unordered_map>
#include <mutex>


namespace Engine {

class MaterialShaderManager {
private:
	Graphics::MaterialShader *m_defaultMaterialShader;

	std::unordered_map<std::wstring, Graphics::MaterialShader> m_shaderCache;
	std::mutex m_cacheLock;

public:
	void Initialize(ID3D11Device *device, const wchar *defaultMaterialShaderFilePath);
	/**
	 * Returns the shader for the given filePath. If the shader does not exist, 
	 * it creates a MaterialShader from the filePath
	 *
	 * @param device      The DirectX device
	 * @param filePath    The path to the shader file
	 * @return            The MaterialShader
	 */
	Graphics::MaterialShader *GetShader(ID3D11Device *device, const std::wstring &filePath);
};

} // End of namespace Engine
