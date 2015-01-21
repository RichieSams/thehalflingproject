/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "scene/materials.h"

#include <unordered_set>
#include <mutex>


namespace Engine {

class MaterialCache {
private:
	std::unordered_set<Scene::Material, Scene::MaterialHasher> m_materialCache;
	std::mutex m_cacheLock;

public:
	const Scene::Material *getMaterial(Graphics::MaterialShader *shader, std::vector<ID3D11ShaderResourceView *> &textureSRVs, std::vector<ID3D11SamplerState *> &textureSamplers);
};

} // End of namespace Engine
