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
Texture2D gSpecColorTexture : register(t1);
Texture2D gSpecPowerTexture : register(t2);
Texture2D gAlphaTexture : register(t3);
Texture2D gDisplacementTexture : register(t4);
Texture2D gNormalTexture : register(t5);

SamplerState gSpecColorSampler : register(s1);
SamplerState gDiffuseSampler : register(s0);
SamplerState gSpecPowerSampler : register(s2);
SamplerState gAlphaSampler : register(s3);
SamplerState gDisplacementSampler : register(s4);
SamplerState gNormalSampler : register(s5);

void GBufferPS(GBufferShaderPixelIn input, out GBuffer gbuffer) {
	[flatten]
	if ((gTextureFlags & 0x01) == 0x01) {
		gbuffer.albedo = gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord).xyz;
	} else {
		gbuffer.albedo = float3(1.0f, 1.0f, 1.0f);
	}

	float3 cartesianNormal;

	[flatten]
	if ((gTextureFlags & 0x20) == 0x20) {
		float3 normalMapSample = gNormalTexture.Sample(gNormalSampler, input.texCoord).xyz;

		cartesianNormal = PerturbNormal(normalize(input.normal), normalMapSample, input.tangent);
	} else {
		cartesianNormal = normalize(input.normal);
	}
	
	gbuffer.normal = CartesianToSpherical(cartesianNormal);
	gbuffer.materialId = gMaterialIndex;
}