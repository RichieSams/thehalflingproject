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
	// Alpha
	[flatten]
	if ((gTextureFlags & 0x08) == 0x08) {
		float alpha = gAlphaTexture.Sample(gAlphaSampler, input.texCoord).x;
		clip(alpha < 0.1f ? -1 : 1);
	}

	SurfaceProperties surfProps;
	surfProps.position = input.positionWorld;
	
	// Sample diffuse albedo
	surfProps.diffuseAlbedo = float4(gMaterial.Diffuse.xyz, 1.0f);
	[flatten]
	if ((gTextureFlags & 0x01) == 0x01) {
		surfProps.diffuseAlbedo *= float4(gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord).xyz, 1.0f);
	}

	// Sample spec albedo
	surfProps.specAlbedoAndPower.xyz = gMaterial.Specular.xyz;
	[flatten]
	if ((gTextureFlags & 0x02) == 0x02) {
		surfProps.specAlbedoAndPower.xyz *= gSpecColorTexture.Sample(gSpecColorSampler, input.texCoord).xyz;
	}
	surfProps.specAlbedoAndPower.w = gMaterial.Specular.w;

	// Interpolating can unnormalize
	surfProps.normal = normalize(input.normal);

	[flatten]
	if ((gTextureFlags & 0x20) == 0x20) {
		float3 normalMapSample = gNormalTexture.Sample(gNormalSampler, input.texCoord).xyz;

		surfProps.normal = PerturbNormal(surfProps.normal, normalMapSample, input.tangent);
	}

	float3 toEye = normalize(gEyePosition - input.positionWorld);

	// Initialize
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the contribution from each light source
	uint lightIndex;

	AccumulateBlinnPhongDirectionalLight(gDirectionalLight, surfProps, toEye, diffuse, spec);

	for (lightIndex = 0; lightIndex < gNumPointLightsToDraw; ++lightIndex) {
		PointLight light = gPointLights[lightIndex];
		AccumulateBlinnPhongPointLight(light, surfProps, toEye, diffuse, spec);
	}

	for (lightIndex = 0; lightIndex < gNumSpotLightsToDraw; ++lightIndex) {
		SpotLight light = gSpotLights[lightIndex];
		AccumulateBlinnPhongSpotLight(light, surfProps, toEye, diffuse, spec);
	}

	// Combine
	float4 litColor = diffuse + spec;

	// Take alpha from diffuse material
	litColor.a = gMaterial.Diffuse.a;

	return litColor;
}