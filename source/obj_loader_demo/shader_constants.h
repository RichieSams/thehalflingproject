/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/materials.h"
#include "common/lights.h"

#include "DirectXMath.h"


namespace ObjLoaderDemo {

// Forward Shading
struct ForwardVertexShaderObjectConstants {
	DirectX::XMMATRIX WorldViewProj;
	DirectX::XMMATRIX World;
};

struct InstancedForwardVertexShaderFrameConstants {
	DirectX::XMMATRIX ViewProj;
};

struct InstancedForwardVertexShaderObjectConstants {
	uint StartVector;
};

struct ForwardPixelShaderFrameConstants {
	Common::DirectionalLight DirectionalLight;
	DirectX::XMFLOAT3 EyePosition;
	float pad;

	uint NumPointLightsToDraw;
	uint NumSpotLightsToDraw;
	uint pad2[2];
};

struct ForwardPixelShaderObjectConstants {
	Common::BlinnPhongMaterial Material;
	uint TextureFlags;
};


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


// No cull final gather pass
struct NoCullFinalGatherPixelShaderFrameConstants {
	DirectX::XMMATRIX InvViewProjection;

	Common::DirectionalLight DirectionalLight;
	DirectX::XMFLOAT3 EyePosition;
	float pad;

	uint NumPointLightsToDraw;
	uint NumSpotLightsToDraw;
	uint pad2[2];
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
	uint VisualizeLightCount;
};


// Debug pass
struct TransformedFullScreenTriangleVertexShaderConstants {
	DirectX::XMFLOAT2 gClipTranslation;
	float gClipScale;
};

struct RenderGBuffersPixelShaderConstants {
	DirectX::XMMATRIX gInvViewProjection;
	uint gGBufferIndex;
};

} // End of namespace ObjLoaderDemo
