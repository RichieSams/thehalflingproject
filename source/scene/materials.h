/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/std_vector_compare.h"
#include "common/hash.h"

#include "graphics/shader.h"

#include <vector>


namespace Scene {

struct Material {
	Material(Graphics::MaterialShader *shader, std::vector<ID3D11ShaderResourceView *> &textureSRVs, std::vector<ID3D11SamplerState *> &textureSamplers)
		: Shader(shader),
		  TextureSRVs(textureSRVs),
		  TextureSamplers(textureSamplers) {
	}

	Graphics::MaterialShader *Shader;
	std::vector<ID3D11ShaderResourceView *> TextureSRVs;
	std::vector<ID3D11SamplerState *> TextureSamplers;

	bool operator==(const Material &rhs) const {
		return Shader == rhs.Shader && Common::CompareVectors(TextureSRVs, rhs.TextureSRVs) && Common::CompareVectors(TextureSamplers, rhs.TextureSamplers);
	}
};

class MaterialHasher {
public:
	size_t operator()(const Material &key) const {
		size_t hash = (size_t)key.Shader;

		for (auto iter = key.TextureSRVs.begin(); iter != key.TextureSRVs.end(); ++iter) {
			hash = hash_combiner(hash, (size_t)(*iter));
		}

		for (auto iter = key.TextureSamplers.begin(); iter != key.TextureSamplers.end(); ++iter) {
			hash = hash_combiner(hash, (size_t)(*iter));
		}

		return hash;
	}
};

} // End of namespace Scene
