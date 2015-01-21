/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "engine/material_cache.h"


namespace Engine {

const Scene::Material *MaterialCache::getMaterial(Graphics::MaterialShader *shader, std::vector<ID3D11ShaderResourceView *> &textureSRVs, std::vector<ID3D11SamplerState *> &textureSamplers) {
	// Lock the cache
	std::lock_guard<std::mutex> guard(m_cacheLock);

	return &(*(m_materialCache.emplace(shader, textureSRVs, textureSamplers).first));

	// The mutex will unlock when 'guard' goes out of scope and destructs
}

} // End of namespace Engine