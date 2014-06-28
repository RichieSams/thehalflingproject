/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/model.h"

#include "common/texture_manager.h"

#include <unordered_map>


namespace Common {

class ModelManager {
public:
	~ModelManager();

private:
	std::unordered_map<std::wstring, Model *> m_modelCache;

public:
	Model *GetModel(ID3D11Device *device, Common::TextureManager *textureManager, std::wstring filePath);
};

} // End of namespace Common
