/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "types.hlsli"
#include "common/shaders/materials.hlsli"
#include "common/shaders/hlsl_util.hlsli"


struct GBuffer {
	float4 diffuseAndRoughness    : SV_Target0;
	float4 specAndMetal           : SV_Target1;
	float2 normal                 : SV_Target2;
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
	float3 diffuse = float3(0.5f, 0.5f, 0.5f);
	float3 specular = float3(0.5f, 0.5f, 0.5f);
	float3 normal = float3(0.0f, 0.0f, 1.0f);
	float metallic = 0.0f;
	float roughness = 0.5f;
	float opacity = 1.0f;

	GetMaterialInfo(normalize(input.normal), input.tangent, textureSamples, diffuse, specular, normal, metallic, roughness, opacity);

	gbuffer.diffuseAndRoughness = float4(diffuse, roughness);
	gbuffer.specAndMetal = float4(specular, metallic);
	gbuffer.normal = CartesianToSpherical(normal);

}