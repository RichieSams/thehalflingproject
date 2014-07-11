/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "pbr_demo/shaders/example.hmat.hlsli"

Texture2D gTextureOne : register(t0);
Texture2D gTextureTwo : register(t1);

SamplerState gSamplerOne : register(s0);
SamplerState gSamplerTwo : register(s1);


float4 main(float2 texCoord : TEXCOORD) : SV_TARGET {
	float4 textureSamples[2];
	textureSamples[0] = gTextureOne.Sample(gSamplerOne, texCoord);
	textureSamples[1] = gTextureTwo.Sample(gSamplerTwo, texCoord);

	// Default values:
	// diffuseColor = (0.5f, 0.5f, 0.5f)
	// specular = 0.5f
	// normal = (0.0f, 0.0f, 1.0f)
	// metallic = 0.0f
	// roughness = 0.5f
	// opacity = 1.0f;

	float3 diffuseColor = float3(0.5f, 0.5f, 0.5f);
	float specular = 0.5f;
	float3 normal = float3(0.0f, 0.0f, 1.0f);
	float metallic = 0.0f;
	float roughness = 0.5f;
	float opacity = 1.0f;

	GetMaterialInfo(textureSamples, diffuseColor, specular, normal, metallic, roughness, opacity);

	return float4(dot(diffuseColor, normal), specular, metallic * roughness, opacity);
}