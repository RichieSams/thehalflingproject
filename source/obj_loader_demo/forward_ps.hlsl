/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "types.hlsli"
#include "hlsl_util.hlsli"

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

StructuredBuffer<PointLight> gPointLights : register(t8);
StructuredBuffer<SpotLight> gSpotLights : register(t9);


float4 ForwardPS(ForwardPixelIn input) : SV_TARGET {
	// Interpolating can unnormalize
	float3 cartesianNormal = normalize(input.normal);

	[flatten]
	if ((gTextureFlags & 0x20) == 0x20) {
		float3 normalMapSample = gNormalTexture.Sample(gNormalSampler, input.texCoord).xyz;

		cartesianNormal = PerturbNormal(cartesianNormal, normalMapSample, input.tangent);
	}

	float3 toEye = normalize(gEyePosition - input.positionWorld);

	// Initialize
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sample the texture
	float4 textureColor;
	[flatten]
	if (gTextureFlags & 0x01 == 0x01) {
		textureColor = gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord);
	} else {
		textureColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Sum the contribution from each light source
	uint lightIndex;

	AccumulateBlinnPhongDirectionalLight(gMaterial, gDirectionalLight, cartesianNormal, toEye, ambient, diffuse, spec);

	for (lightIndex = 0; lightIndex < gNumPointLightsToDraw; ++lightIndex) {
		PointLight light = gPointLights[lightIndex];
		AccumulateBlinnPhongPointLight(gMaterial, light, input.positionWorld, cartesianNormal, toEye, diffuse, spec);
	}

	for (lightIndex = 0; lightIndex < gNumSpotLightsToDraw; ++lightIndex) {
		SpotLight light = gSpotLights[lightIndex];
		AccumulateBlinnPhongSpotLight(gMaterial, light, input.positionWorld, cartesianNormal, toEye, diffuse, spec);
	}

	// Combine
	float4 litColor = textureColor * (ambient + diffuse) + spec;

	// Take alpha from diffuse material
	litColor.a = gMaterial.Diffuse.a;

	return litColor;
}