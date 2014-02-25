/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"
#include "obj_loader_demo/hlsl_util.hlsli"


cbuffer cbPerFrame : register(b0) {
	float4x4 gProj;
	float4x4 gInvViewProjection;
	uint gGBufferIndex;
}

Texture2DMS<float3> gGBufferAlbedo       : register(t0);
Texture2DMS<float2> gGBufferNormal       : register(t1);
Texture2DMS<uint> gGBufferMaterialId    : register(t2);
Texture2DMS<float> gGBufferDepth         : register(t3);

float4 RenderGBuffersPS(TransformedFullScreenTrianglePixelIn input) : SV_TARGET {
	float2 gbufferDim;
	uint dummy;
	gGBufferDepth.GetDimensions(gbufferDim.x, gbufferDim.y, dummy);
	
	float2 pixelCoord = input.texCoord * gbufferDim;

	[branch]
	if (gGBufferIndex == 0) {
		// Render albedo color
		return float4(gGBufferAlbedo.Load(pixelCoord, 0).xyz, 1.0f);
	} else if (gGBufferIndex == 1) {
		// Render material id
		uint materialId = gGBufferMaterialId.Load(pixelCoord, 0).x;

		float colorValue = (float)materialId / 1000.0f;
		return float4(colorValue, colorValue, colorValue, 1.0f);
	} else if (gGBufferIndex == 2) {
		// Render Spherical Coord Normal
		return float4(gGBufferNormal.Load(pixelCoord, 0).xy, 0.0f, 1.0f);
	} else if (gGBufferIndex == 3) {
		// Render Cartesian Coord Normal
		float2 normalSphericalCoords = gGBufferNormal.Load(pixelCoord, 0).xy;
		return float4(SphericalToCartesian(normalSphericalCoords), 1.0f);

	} else if (gGBufferIndex == 4) {
		// Render depth
		float zw = (float)(gGBufferDepth.Load(pixelCoord, 0));
		float linearZ = LinearDepth(zw, gProj) * 0.001f;

		return float4(zw, zw, zw, 1.0f);
	} else {
		// Render position from depth
		float zw = (float)(gGBufferDepth.Load(pixelCoord, 0));

		return float4(PositionFromDepth(zw, pixelCoord, gbufferDim, gInvViewProjection), 1.0f);
	}
}