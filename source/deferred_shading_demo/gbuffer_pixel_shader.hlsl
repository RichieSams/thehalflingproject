/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"
#include "common/shaders/materials.hlsli"
#include "deferred_shading_demo/hlsl_util.hlsli"


struct GBuffer {
	float4 albedoMaterialIndex    : SV_Target0;
	float2 normal                 : SV_Target1;
};

cbuffer cbPerObject : register(b1) {
	uint gMaterialIndex;
};

Texture2D gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

void GBufferPS(GBufferShaderPixelIn input, out GBuffer gbuffer) {
	gbuffer.albedoMaterialIndex = gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord);
	gbuffer.albedoMaterialIndex.w = (float)gMaterialIndex;

	gbuffer.normal = CartesianToSpherical(input.normal);
}