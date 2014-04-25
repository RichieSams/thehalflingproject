/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"
#include "common/shaders/materials.hlsli"
#include "obj_loader_demo/hlsl_util.hlsli"


struct GBuffer {
	float3 albedo       : SV_Target0;
	float2 normal       : SV_Target1;
	uint materialId    : SV_Target2;
};

cbuffer cbPerObject : register(b1) {
	uint gMaterialIndex;
	uint gTextureFlags;
};

Texture2D gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

void GBufferPS(GBufferShaderPixelIn input, out GBuffer gbuffer) {
	[flatten]
	if ((gTextureFlags & 0x01) == 0x01) {
		gbuffer.albedo = gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord).xyz;
	} else {
		gbuffer.albedo = float3(1.0f, 1.0f, 1.0f);
	}

	gbuffer.normal = CartesianToSpherical(normalize(input.normal));
	gbuffer.materialId = gMaterialIndex;
}