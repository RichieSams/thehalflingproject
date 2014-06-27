/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "types.hlsli"
#include "obj_loader_demo/hlsl_util.hlsli"


cbuffer cbPerFrame : register(b0) {
	float4x4 gProj;
	float4x4 gInvViewProjection;
	uint gGBufferIndex;
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

float4 RenderGBuffersPS(CalculatedTrianglePixelIn input) : SV_TARGET {
	float2 gbufferDim;
	#ifdef MSAA_
		uint dummy;
		gGBufferDepth.GetDimensions(gbufferDim.x, gbufferDim.y, dummy);
	#else
		gGBufferDepth.GetDimensions(gbufferDim.x, gbufferDim.y);
	#endif
	
	float2 pixelCoord = input.texCoord * gbufferDim;

	[branch]
	if (gGBufferIndex == 0) {
		// Render albedo color
		#ifdef MSAA_
			return float4(gBufferDiffuse.Load(pixelCoord, 0).xyz, 1.0f);
		#else
				return float4(gBufferDiffuse[pixelCoord].xyz, 1.0f);
		#endif
	} else if (gGBufferIndex == 1) {
		// Render spec color
		#ifdef MSAA_
			return float4(gBufferSpecAndPower.Load(pixelCoord, 0).xyz, 1.0f);
		#else
			return float4(gBufferSpecAndPower[pixelCoord].xyz, 1.0f);
		#endif
	} else if (gGBufferIndex == 2) {
		// Render Spherical Coord Normal
		#ifdef MSAA_
			float2 sphericalCoord = gGBufferNormal.Load(pixelCoord, 0).xy;
		#else
			float2 sphericalCoord = gGBufferNormal[pixelCoord].xy;
		#endif
		
		// Normalize to renderable values
		sphericalCoord.x = (sphericalCoord.x / 3.141592f) * 0.5f + 0.5f;
		sphericalCoord.y = sphericalCoord.y / 3.141592f;

		return float4(sphericalCoord, 0.0f, 1.0f);
	} else if (gGBufferIndex == 3) {
		// Render Cartesian Coord Normal
		#ifdef MSAA_
			float3 cartesianCoords = SphericalToCartesian(gGBufferNormal.Load(pixelCoord, 0).xy);
		#else
			float3 cartesianCoords = SphericalToCartesian(gGBufferNormal[pixelCoord].xy);
		#endif

		// Normalize to renderable values
		cartesianCoords = cartesianCoords * 0.5f + 0.5f;

		return float4(cartesianCoords, 1.0f);

	} else if (gGBufferIndex == 4) {
		// Render depth
		#ifdef MSAA_
			float zw = (float)(gGBufferDepth.Load(pixelCoord, 0));
		#else
			float zw = (float)(gGBufferDepth[pixelCoord]);
		#endif

		return float4(zw, zw, zw, 1.0f);
	}

	return float4(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
}