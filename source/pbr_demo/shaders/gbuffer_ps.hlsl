/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "types.hlsli"
#include "common/shaders/materials.hlsli"
#include "common/shaders/hlsl_util.hlsli"


struct GBuffer {
	// RBG base color
	// Diffuse and Spec colors are calculated as follows:
	// DiffuseColor = BaseColor - BaseColor * Metallic;
	// SpecColor = lerp(0.08 * Specular.xxx, BaseColor, Metallic)
	float3 baseColor                : SV_Target0;
	// R: Roughness
	// G: Specular
	// B: Metallic
	float3 roughnessSpecAndMetal    : SV_Target1;
	// RG spherical encode normal
	float2 normal                   : SV_Target2;
};


Texture2D gTextures[TEXTURE_COUNT] : register(t0);
SamplerState gSamplers[TEXTURE_COUNT] : register(s0);


void GBufferPS(GBufferShaderPixelIn input, out GBuffer gbuffer) {
	float4 textureSamples[TEXTURE_COUNT];

	[unroll]
	for (uint i = 0; i < TEXTURE_COUNT; ++i) {
		textureSamples[i] = gTextures[i].Sample(gSamplers[i], input.texCoord);
	}

	// Initialize
	float3 baseColor = float3(0.5f, 0.5f, 0.5f);
	float specular = 0.5f;
	float3 normal = float3(0.0f, 0.0f, 1.0f);
	float metallic = 0.0f;
	float roughness = 0.5f;

	GetMaterialInfo(normalize(input.normal), input.tangent, textureSamples, baseColor, specular, normal, metallic, roughness);

	gbuffer.baseColor = baseColor;
	gbuffer.roughnessSpecAndMetal = float4(roughness, specular, metallic, 0.0f);
	gbuffer.normal = CartesianToSpherical(normal);
}