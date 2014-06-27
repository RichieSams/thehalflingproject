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

Texture2DMS<float3> gBufferDiffuse         : register(t0);
Texture2DMS<float4> gBufferSpecAndPower    : register(t1);
Texture2DMS<float2> gGBufferNormal         : register(t2);
Texture2DMS<float> gGBufferDepth           : register(t3);

StructuredBuffer<PointLight> gPointLights : register(t4);
StructuredBuffer<SpotLight> gSpotLights : register(t5);

StructuredBuffer<BlinnPhongMaterial> gMaterialList : register(t6);

float4 NoCullFinalGatherPS(CalculatedTrianglePixelIn input) : SV_TARGET {
	float2 gbufferDim;
	uint dummy;
	gGBufferDepth.GetDimensions(gbufferDim.x, gbufferDim.y, dummy);

	float2 pixelCoord = input.texCoord * gbufferDim;

	// Sample from the Depth GBuffer and calculate position
	float zw = (float)(gGBufferDepth.Load(pixelCoord, 0));

	// Discard pixels that have infinite depth.
	if (zw == 0.0f)
		discard;

	SurfaceProperties surfProps;

	// Sample from the diffuse albedo GBuffer
	surfProps.diffuseAlbedo = float4(gBufferDiffuse.Load(pixelCoord, 0).xyz, 1.0f);

	// Sample from the specular albedo GBuffer
	surfProps.specAlbedoAndPower = gBufferSpecAndPower.Load(pixelCoord, 0);
	// Decode the specular power
	surfProps.specAlbedoAndPower.w *= MAX_SPEC_POWER;

	// Sample from the Normal GBuffer
	float2 normalSphericalCoords = gGBufferNormal.Load(pixelCoord, 0).xy;
	surfProps.normal = SphericalToCartesian(normalSphericalCoords);
	surfProps.position = PositionFromDepth(zw, input.texCoord, gInvViewProjection);

	float3 toEye = normalize(gEyePosition - surfProps.position);

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
	return diffuse + spec;
}