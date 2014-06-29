/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/materials.h"
#include "common/lights.h"

#include "DirectXMath.h"


namespace PBRDemo {

// Gbuffer pass
struct GBufferVertexShaderObjectConstants {
	DirectX::XMMATRIX WorldViewProj;
	DirectX::XMMATRIX World;
};

struct InstancedGBufferVertexShaderFrameConstants {
	DirectX::XMMATRIX ViewProj;
};

struct InstancedGBufferVertexShaderObjectConstants {
	uint StartVector;
};

struct GBufferPixelShaderObjectConstants {
	Common::BlinnPhongMaterial Material;
	uint TextureFlags;
};


// Tiled cull final gather pass
struct TiledCullFinalGatherComputeShaderFrameConstants {
	DirectX::XMMATRIX WorldView;
	DirectX::XMMATRIX Projection;
	DirectX::XMMATRIX InvViewProjection;

	Common::DirectionalLight DirectionalLight;

	DirectX::XMFLOAT3 EyePosition;
	uint NumPointLightsToDraw;

	DirectX::XMFLOAT2 CameraClipPlanes;
	uint NumSpotLightsToDraw;
	uint pad;
};


} // End of namespace PBRDemo
