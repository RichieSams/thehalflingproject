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
	GraphicsState()
			: MaterialShader(nullptr),
			  IndexBuffer(nullptr),
			  BlendState(BlendState::BLEND_DISABLED),
			  SampleMask(0xFFFFFFFF),
			  RasterizerState(RasterizerState::CULL_BACKFACES),
			  DepthStencilState(DepthStencilState::REVERSE_DEPTH_WRITE_ENABLED) {
		// WORKAROUND: We have to manually initialize because VS 2013 compiler doesn't support array initialization in the class initializer list
		VertexBuffers[0] = nullptr;
		VertexBuffers[1] = nullptr;

		BlendFactor[0] = 1.0f;
		BlendFactor[1] = 1.0f;
		BlendFactor[2] = 1.0f;
		BlendFactor[3] = 1.0f;
	}

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
