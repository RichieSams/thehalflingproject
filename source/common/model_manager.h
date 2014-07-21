/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/model.h"

#include "common/texture_manager.h"

#include <unordered_map>
#include <mutex>


namespace Common {

class ModelManager {
public:
	ModelManager()
		: m_unnamedModelIncrementer(0u) {
	}
	~ModelManager();

private:
	std::unordered_map<std::wstring, Model *> m_modelCache;
	std::mutex m_cacheLock;

	uint m_unnamedModelIncrementer;

public:
	Model *GetModel(ID3D11Device *device, Common::TextureManager *textureManager, std::wstring filePath);
	Model *CreateUnnamedModel();
};

} // End of namespace Common
