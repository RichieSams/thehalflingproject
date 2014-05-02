/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_loader_demo/types.hlsli"
#include "common/shaders/lights.hlsli"
#include "common/shaders/light_functions.hlsli"
#include "obj_loader_demo/hlsl_util.hlsli"

cbuffer cbPerFrame : register(b0) {
	float4x4 gProjection : packoffset(c0);
	float4x4 gInvViewProjection : packoffset(c4);
	DirectionalLight gDirectionalLight : packoffset(c8);
	float3 gEyePosition : packoffset(c12);
	uint gNumPointLightsToDraw : packoffset(c13.x);
	uint gNumSpotLightsToDraw : packoffset(c13.y);
}

Texture2DMS<float3> gGBufferAlbedo       : register(t0);
Texture2DMS<float2> gGBufferNormal       : register(t1);
Texture2DMS<uint> gGBufferMaterialId    : register(t2);
Texture2DMS<float> gGBufferDepth         : register(t3);

StructuredBuffer<PointLight> gPointLights : register(t4);
StructuredBuffer<SpotLight> gSpotLights : register(t5);

StructuredBuffer<BlinnPhongMaterial> gMaterialList : register(t6);

float4 NoCullFinalGatherPS(FullScreenTrianglePixelIn input) : SV_TARGET {
	float2 gbufferDim;
	uint dummy;
	gGBufferDepth.GetDimensions(gbufferDim.x, gbufferDim.y, dummy);

	float2 pixelCoord = input.texCoord * gbufferDim;

	// Sample from the Depth GBuffer and calculate position
	float zw = (float)(gGBufferDepth.Load(pixelCoord, 0));

	// Discard pixels that have infinite depth.
	if (zw == 0.0f)
		discard;

	float3 positionWS = PositionFromDepth(zw, pixelCoord, gbufferDim, gInvViewProjection);

	// Sample from the Albedo-Specular Power GBuffer
	float4 albedo = float4(gGBufferAlbedo.Load(pixelCoord, 0).xyz, 1.0f);
	BlinnPhongMaterial material = gMaterialList[gGBufferMaterialId.Load(pixelCoord, 0).x];

	// Sample from the Normal-Specular Intensity GBuffer
	float2 normalSphericalCoords = gGBufferNormal.Load(pixelCoord, 0).xy;
	float3 normal = SphericalToCartesian(normalSphericalCoords);

	float3 toEye = normalize(gEyePosition - positionWS);

	// Initialize
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the contribution from each light source
	uint lightIndex;

	AccumulateBlinnPhongDirectionalLight(material, gDirectionalLight, normal, toEye, ambient, diffuse, spec);

	for (lightIndex = 0; lightIndex < gNumPointLightsToDraw; ++lightIndex) {
        PointLight light = gPointLights[lightIndex];
		AccumulateBlinnPhongPointLight(material, light, positionWS, normal, toEye, diffuse, spec);
    }

	for (lightIndex = 0; lightIndex < gNumSpotLightsToDraw; ++lightIndex) {
        SpotLight light = gSpotLights[lightIndex];
		AccumulateBlinnPhongSpotLight(material, light, positionWS, normal, toEye, diffuse, spec);
    }

	// Combine
	float4 litColor = albedo * (ambient + diffuse) + spec;

	// Take alpha from diffuse material
	litColor.a = material.Diffuse.a;

	return litColor;
}