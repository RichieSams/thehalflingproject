/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef DEFERRED_SHADING_DEMO_SHADING_CONSTANTS_H
#define DEFERRED_SHADING_DEMO_SHADING_CONSTANTS_H

#include "common/light_manager.h"
#include "common/materials.h"

#include "DirectXMath.h"


namespace DeferredShadingDemo {

struct VertexShaderFrameConstants {
	DirectX::XMMATRIX viewProj;
	DirectX::XMMATRIX proj;
};

struct VertexShaderObjectConstants {
	DirectX::XMMATRIX worldViewProj;
	DirectX::XMMATRIX world;
};

struct PixelShaderFrameConstants {
	Common::DirectionalLight directionalLight;
	DirectX::XMFLOAT3 eyePosition;
	float pad;
};

struct PixelShaderObjectConstants {
	Common::Material material;
};

} // End of namespace DeferredShadingDemo

#endif