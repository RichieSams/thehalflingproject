/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "graphics/shader.h"

#include <map>


namespace Graphics {

struct GraphicsState {
	MaterialShader *MaterialShader;
	ID3D11Buffer *VertexBuffers[2];
	ID3D11Buffer *IndexBuffer;

	std::map<uint, ID3D11ShaderResourceView *> TextureSRVs;
	std::map<uint, ID3D11SamplerState *> TextureSamplers;

	BlendState BlendState;
	float BlendFactor[4];
	uint SampleMask;
	RasterizerState RasterizerState;
	DepthStencilState DepthStencilState;
};


} // End of namespace Graphics
