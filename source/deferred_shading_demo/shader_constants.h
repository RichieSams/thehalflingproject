/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef DEFERRED_SHADING_DEMO_SHADING_CONSTANTS_H
#define DEFERRED_SHADING_DEMO_SHADING_CONSTANTS_H

#include "common/materials.h"

#include "DirectXMath.h"


namespace DeferredShadingDemo {

// Forward Shading
struct ForwardVertexShaderObjectConstants {
	DirectX::XMMATRIX WorldViewProj;
	DirectX::XMMATRIX World;
};

struct ForwardPixelShaderFrameConstants {
	Common::DirectionalLight DirectionalLight;
	DirectX::XMFLOAT3 EyePosition;
};

struct ForwardPixelShaderObjectConstants {
	Common::BlinnPhongMaterial Material;
};


// Gbuffer pass
struct GBufferVertexShaderObjectConstants {
	DirectX::XMMATRIX WorldViewProj;
	DirectX::XMMATRIX World;
};

struct GBufferPixelShaderObjectConstants {
	uint MaterialIndex;
};

// No cull final gather pass
struct NoCullFinalGatherPixelShaderFrameConstants {
	DirectX::XMMATRIX gProjection;
	DirectX::XMMATRIX gInvViewProjection;

	Common::DirectionalLight gDirectionalLight;
	DirectX::XMFLOAT3 gEyePosition;
};

} // End of namespace DeferredShadingDemo

#endif