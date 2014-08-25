/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "scene/model.h"

#include <unordered_map>
#include <mutex>


namespace Engine {

class TextureManager;
class MaterialShaderManager;

class ModelManager {
public:
	ModelManager()
		: m_unnamedModelIncrementer(0u) {
	}
	~ModelManager();

private:
	std::unordered_map<std::wstring, Scene::Model *> m_modelCache;
	std::mutex m_cacheLock;

	uint m_unnamedModelIncrementer;

public:
	Scene::Model *GetModel(ID3D11Device *device, Engine::TextureManager *textureManager, Engine::MaterialShaderManager *materialShaderManager, Graphics::SamplerStateManager *samplerStateManager, std::wstring filePath);
	Scene::Model *CreateUnnamedModel();
};

} // End of namespace Engine
