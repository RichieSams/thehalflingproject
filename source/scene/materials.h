/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "graphics/shader.h"

#include <vector>


namespace Scene {

struct Material {
	Graphics::MaterialShader *Shader;
	std::vector<ID3D11ShaderResourceView *> TextureSRVs;
	std::vector<ID3D11SamplerState *> TextureSamplers;
};

} // End of namespace Scene

