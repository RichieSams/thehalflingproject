/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "graphics/shader.h"

#include <unordered_map>
#include <vector>
#include <mutex>


namespace Engine {

class MaterialShaderManager {
public:
	~MaterialShaderManager();

private:
	Graphics::MaterialShader *m_defaultMaterialShader;

	std::unordered_map<std::wstring, size_t> m_shaderIdLookup;
	std::vector<Graphics::PixelShader<> *> m_shaderCache;
	std::mutex m_cacheLock;

public:
	void Initialize(ID3D11Device *device, const wchar *defaultMaterialShaderFilePath);
	/**
	 * Creates a MaterialShader from the filePath and returns a handle to the
	 * created shader. If the shader was already created, the Manager will return the
	 * handle of the previously created shader
	 *
	 * @param device      The DirectX device
	 * @param filePath    The path to the shader file
	 * @return            The handle to the loaded shader
	 */
	size_t CreateShader(ID3D11Device *device, const std::wstring filePath);
	/**
	 * Returns the shader for the given handle. If the handle is not valid, a default 
	 * material shader will be returned instead
	 *
	 * @param handle    The handle to the material shader
	 * @return          The material shader
	 */
	Graphics::MaterialShader *GetShader(size_t handle);
};

} // End of namespace Engine
