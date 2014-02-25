/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"
#include "common/shaders/materials.hlsli"
#include "common/shaders/lights.hlsli"
#include "common/shaders/light_functions.hlsli"


cbuffer cbPerFrame : register(b0) {
	DirectionalLight gDirectionalLight : packoffset(c0);
	float3 gEyePosition : packoffset(c4);
	uint gNumPointLightsToDraw : packoffset(c5.x);
	uint gNumSpotLightsToDraw : packoffset(c5.y);
}

cbuffer cbPerObject : register(b1) {
	BlinnPhongMaterial gMaterial;
	uint gTextureFlags;
};

Texture2D gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

StructuredBuffer<PointLight> gPointLights : register(t3);
StructuredBuffer<SpotLight> gSpotLights : register(t4);


float4 ForwardPS(ForwardPixelIn input) : SV_TARGET {
	// Interpolating can unnormalize
	input.normal = normalize(input.normal);

	float3 toEye = normalize(gEyePosition - input.positionWorld);

	// Initialize
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sample the texture
	float4 textureColor = gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord);

	// Sum the contribution from each light source
	uint lightIndex;

	AccumulateBlinnPhongDirectionalLight(gMaterial, gDirectionalLight, input.normal, toEye, ambient, diffuse, spec);

	for (lightIndex = 0; lightIndex < gNumPointLightsToDraw; ++lightIndex) {
		PointLight light = gPointLights[lightIndex];
		AccumulateBlinnPhongPointLight(gMaterial, light, input.positionWorld, input.normal, toEye, diffuse, spec);
	}

	for (lightIndex = 0; lightIndex < gNumSpotLightsToDraw; ++lightIndex) {
		SpotLight light = gSpotLights[lightIndex];
		AccumulateBlinnPhongSpotLight(gMaterial, light, input.positionWorld, input.normal, toEye, diffuse, spec);
	}

	// Combine
	float4 litColor = textureColor * (ambient + diffuse) + spec;

	// Take alpha from diffuse material
	litColor.a = gMaterial.Diffuse.a;

	return litColor;
}