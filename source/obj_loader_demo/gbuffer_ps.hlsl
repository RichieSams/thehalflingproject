/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "types.hlsli"
#include "common/shaders/materials.hlsli"
#include "obj_loader_demo/hlsl_util.hlsli"


struct GBuffer {
	float3 diffuse     : SV_Target0;
	float4 spec        : SV_Target1;
	float2 normal      : SV_Target2;
};

cbuffer cbPerObject : register(b1) {
	BlinnPhongMaterial gMaterial;
	uint gTextureFlags;
};

Texture2D gDiffuseTexture : register(t0);
Texture2D gSpecColorTexture : register(t1);
Texture2D gSpecPowerTexture : register(t2);
Texture2D gAlphaTexture : register(t3);
Texture2D gDisplacementTexture : register(t4);
Texture2D gNormalTexture : register(t5);

SamplerState gDiffuseSampler : register(s0);
SamplerState gSpecColorSampler : register(s1);
SamplerState gSpecPowerSampler : register(s2);
SamplerState gAlphaSampler : register(s3);
SamplerState gDisplacementSampler : register(s4);
SamplerState gNormalSampler : register(s5);

void GBufferPS(GBufferShaderPixelIn input, out GBuffer gbuffer) {
	[flatten]
	if ((gTextureFlags & 0x01) == 0x01) {
		gbuffer.diffuse = gMaterial.Diffuse.xyz * gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord).xyz;
	} else {
		gbuffer.diffuse = gMaterial.Diffuse.xyz;
	}

	[flatten]
	if ((gTextureFlags & 0x02) == 0x02) {
		gbuffer.spec.xyz = gMaterial.Specular.xyz * gSpecColorTexture.Sample(gSpecColorSampler, input.texCoord).xyz;
		gbuffer.spec.w = gMaterial.Specular.w / MAX_SPEC_POWER;
	} else {
		gbuffer.spec.xyz = gMaterial.Specular.xyz;
		gbuffer.spec.w = gMaterial.Specular.w / MAX_SPEC_POWER;
	}

	float3 cartesianNormal = normalize(input.normal);

	[flatten]
	if ((gTextureFlags & 0x20) == 0x20) {
		float3 normalMapSample = gNormalTexture.Sample(gNormalSampler, input.texCoord).xyz;

		cartesianNormal = PerturbNormal(cartesianNormal, normalMapSample, input.tangent);
	}
	
	gbuffer.normal = CartesianToSpherical(cartesianNormal);
}