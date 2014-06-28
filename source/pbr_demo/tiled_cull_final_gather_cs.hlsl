/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

/* This code is heavily influenced by Andrew Lauritzen's 2010 code published with his
 * article "Intersecting Lights with Pixels". It also uses several elements from 
 * Matt Pettineo's IndexedDeferred Sample code.
 */


#include "pbr_demo/types.hlsli"
#include "common/shaders/lights.hlsli"
#include "common/shaders/light_functions.hlsli"
#include "pbr_demo/hlsl_util.hlsli"
#include "pbr_demo/shader_defines.h"


cbuffer cbPerFrame : register(b0) {
	float4x4 gWorldView : packoffset(c0);
	float4x4 gProjection : packoffset(c4);
	float4x4 gInvViewProjection : packoffset(c8);

	DirectionalLight gDirectionalLight : packoffset(c12);

	float3 gEyePosition : packoffset(c15);
	uint gNumPointLightsToDraw : packoffset(c15.w);

	float2 gCameraClipPlanes : packoffset(c16);
	uint gNumSpotLightsToDraw : packoffset(c16.z);
	uint gVisualizeLightCount : packoffset(c16.w);
}

#ifdef MSAA_
	Texture2DMS<float3> gBufferDiffuse         : register(t0);
	Texture2DMS<float4> gBufferSpecAndPower    : register(t1);
	Texture2DMS<float2> gGBufferNormal         : register(t2);
	Texture2DMS<float> gGBufferDepth           : register(t3);
#else
	Texture2D<float3> gBufferDiffuse         : register(t0);
	Texture2D<float4> gBufferSpecAndPower    : register(t1);
	Texture2D<float2> gGBufferNormal         : register(t2);
	Texture2D<float> gGBufferDepth           : register(t3);
#endif

StructuredBuffer<PointLight> gPointLights : register(t4);
StructuredBuffer<SpotLight> gSpotLights : register(t5);

RWTexture2D<float4> gOutputBuffer : register(u0);

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTilePointLightIndices[MAX_LIGHTS_PER_TILE];
groupshared uint sTileNumPointLights;
groupshared uint sTileSpotLightIndices[MAX_LIGHTS_PER_TILE];
groupshared uint sTileNumSpotLights;


[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void ComputeShaderTileCS(uint3 groupId : SV_GroupID,
                         uint groupIndex : SV_GroupIndex,
                         uint3 dispatchThreadId : SV_DispatchThreadID,
                         uint3 groupThreadId : SV_GroupThreadID) {
	uint2 pixelCoord = dispatchThreadId.xy;

	// Sample from the Depth GBuffer and calculate position
	#ifdef MSAA_
		float zw = (float)(gGBufferDepth.Load(pixelCoord, 0));
	#else
		float zw = (float)(gGBufferDepth[pixelCoord]);
	#endif

	float linearZ = LinearDepth(zw, gProjection);

	float2 gbufferDim;
	#ifdef MSAA_
		uint dummy;
		gGBufferDepth.GetDimensions(gbufferDim.x, gbufferDim.y, dummy);
	#else
		gGBufferDepth.GetDimensions(gbufferDim.x, gbufferDim.y);
	#endif

	float3 positionWS = PositionFromDepth(zw, (pixelCoord + 0.5f) / gbufferDim, gInvViewProjection);

	// Work out Z bounds for our samples
    float minZSample = min(gCameraClipPlanes.y, linearZ);
    float maxZSample = max(gCameraClipPlanes.x, linearZ);

	// Initialize shared memory light list and Z bounds
	if (groupIndex == 0) {
		sTileNumPointLights = 0;
		sTileNumSpotLights = 0;
		sMinZ = 0x7F7FFFFF;      // Max float
		sMaxZ = 0;
	}

	GroupMemoryBarrierWithGroupSync();
	
	if(maxZSample >= minZSample) {
        InterlockedMin(sMinZ, asuint(minZSample));
        InterlockedMax(sMaxZ, asuint(maxZSample));
    }

	GroupMemoryBarrierWithGroupSync();

	float minTileZ = asfloat(sMinZ);
	float maxTileZ = asfloat(sMaxZ);
	
	// NOTE: This is all uniform per-tile (i.e. no need to do it per-thread) but fairly inexpensive
	// We could just precompute the frusta planes for each tile and dump them into a constant buffer...
	// They don't change unless the projection matrix changes since we're doing it in view space.
	// Then we only need to compute the near/far ones here tightened to our actual geometry.
	// The overhead of group synchronization/LDS or global memory lookup is probably as much as this
	// little bit of math anyways, but worth testing.

	// Work out scale/bias from [0, 1]
	float2 tileScale = gbufferDim * rcp(float(2 * COMPUTE_SHADER_TILE_GROUP_DIM));
	float2 tileBias = tileScale - float2(groupId.xy);

	// Now work out composite projection matrix
	// Relevant matrix columns for this tile frusta
	float4 c1 = float4(gProjection._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
	float4 c2 = float4(0.0f, -gProjection._22 * tileScale.y, tileBias.y, 0.0f);
	float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

	// Derive frustum planes
	float4 frustumPlanes[6];

	// Sides
	frustumPlanes[0] = c4 - c1;
	frustumPlanes[1] = c4 + c1;
	frustumPlanes[2] = c4 - c2;
	frustumPlanes[3] = c4 + c2;

	// Near/far
	frustumPlanes[4] = float4(0.0f, 0.0f,  1.0f, -minTileZ);
	frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f,  maxTileZ);
	
	// Normalize frustum planes (near/far already normalized)
	[unroll] 
	for (uint i = 0; i < 4; ++i) {
		frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
	}
	
	// Cull lights for this tile
	uint lightIndex;
	// Point lights
	for (lightIndex = groupIndex; lightIndex < gNumPointLightsToDraw; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE) {
		PointLight light = gPointLights[lightIndex];
		float3 lightPosition = mul(float4(light.Position, 1.0f), gWorldView).xyz;

		// Cull PointLight sphere vs tile frustum
		bool inFrustum = true;
		[unroll] 
		for (uint i = 0; i < 6; ++i) {
			float d = dot(frustumPlanes[i], float4(lightPosition, 1.0f));
			inFrustum = inFrustum && (d >= -light.Range);
		}

		[branch] 
		if (inFrustum) {
			// Append light to list
			// Compaction might be better if we expect a lot of lights
			uint listIndex;
			InterlockedAdd(sTileNumPointLights, 1, listIndex);
			sTilePointLightIndices[listIndex] = lightIndex;
		}
	}
	// Spotlights
	for (lightIndex = groupIndex; lightIndex < gNumSpotLightsToDraw; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE) {
		SpotLight light = gSpotLights[lightIndex];
		float3 lightPosition = mul(float4(light.Position, 1.0f), gWorldView).xyz;
				
		// Cull SpotLight sphere vs tile frustum
		// TODO: Actually do cone vs tile frustum culling
		bool inFrustum = true;
		[unroll] 
		for (uint i = 0; i < 6; ++i) {
			float d = dot(frustumPlanes[i], float4(lightPosition, 1.0f));
			inFrustum = inFrustum && (d >= -light.Range);
		}

		[branch] 
		if (inFrustum) {
			// Append light to list
			// Compaction might be better if we expect a lot of lights
			uint listIndex;
			InterlockedAdd(sTileNumSpotLights, 1, listIndex);
			sTileSpotLightIndices[listIndex] = lightIndex;
		}
	}

	GroupMemoryBarrierWithGroupSync();
	
	uint numPointLights = sTileNumPointLights;
	uint numSpotLights = sTileNumSpotLights;

	// Only process onscreen pixels (tiles can span screen edges)
	[branch]
	if (all(pixelCoord < (uint)gbufferDim)) {
		[branch] 
		if (gVisualizeLightCount) {
			gOutputBuffer[pixelCoord] = float4(GetColor((float)(numPointLights + numSpotLights), 0.0f, 24.0f), 1.0f);
			return;
		} else if (zw == 0.0f) {
			return;
		} else {
			SurfaceProperties surfProps;
			surfProps.position = positionWS;

			#ifdef MSAA_
				// Sample from the diffuse albedo GBuffer
				surfProps.diffuseAlbedo = float4(gBufferDiffuse.Load(pixelCoord, 0).xyz, 1.0f);

				// Sample from the specular albedo GBuffer
				surfProps.specAlbedoAndPower = gBufferSpecAndPower.Load(pixelCoord, 0);
				// Decode the specular power
				surfProps.specAlbedoAndPower.w *= MAX_SPEC_POWER;

				// Sample from the Normal GBuffer
				float2 normalSphericalCoords = gGBufferNormal.Load(pixelCoord, 0).xy;
				surfProps.normal = SphericalToCartesian(normalSphericalCoords);
			#else
				// Sample from the diffuse albedo GBuffer
				surfProps.diffuseAlbedo = float4(gBufferDiffuse[pixelCoord].xyz, 1.0f);

				// Sample from the specular albedo GBuffer
				surfProps.specAlbedoAndPower = gBufferSpecAndPower[pixelCoord];
				// Decode the specular power
				surfProps.specAlbedoAndPower.w *= MAX_SPEC_POWER;

				// Sample from the Normal GBuffer
				float2 normalSphericalCoords = gGBufferNormal[pixelCoord].xy;
				surfProps.normal = SphericalToCartesian(normalSphericalCoords);
			#endif

			float3 toEye = normalize(gEyePosition - surfProps.position);

			// Initialize
			float4 diffuse = float4(0.0f, 0.0f, 0.0f, 1.0f);
			float4 spec = float4(0.0f, 0.0f, 0.0f, 1.0f);

			// Sum the contribution from each light source
			uint lightIndex;

			AccumulateBlinnPhongDirectionalLight(gDirectionalLight, surfProps, toEye, diffuse, spec);

			for (lightIndex = 0; lightIndex < numPointLights; ++lightIndex) {
				PointLight light = gPointLights[sTilePointLightIndices[lightIndex]];
				AccumulateBlinnPhongPointLight(light, surfProps, toEye, diffuse, spec);
			}

			for (lightIndex = 0; lightIndex < numSpotLights; ++lightIndex) {
				SpotLight light = gSpotLights[sTileSpotLightIndices[lightIndex]];
				AccumulateBlinnPhongSpotLight(light, surfProps, toEye, diffuse, spec);
			}

			// Combine and write sample 0 result
			gOutputBuffer[pixelCoord] = diffuse + spec;
		}
	}
}