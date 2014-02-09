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

Texture2DMS<float4> gGBufferAlbedoMaterialIndex    : register(t0);
Texture2DMS<float2> gGBufferNormal                 : register(t1);
Texture2DMS<float> gGBufferDepth                   : register(t2);

float4 RenderGBuffersPS(TransformedFullScreenTrianglePixelIn input) : SV_TARGET {
	float2 gbufferDim;
	uint dummy;
	gGBufferDepth.GetDimensions(gbufferDim.x, gbufferDim.y, dummy);
	
	float2 pixelCoord = input.texCoord * gbufferDim;

	[branch]
	if (gGBufferIndex == 0) {
		// Render albedo color
		float4 albedoMaterialIndex = gGBufferAlbedoMaterialIndex.Load(pixelCoord, 0).xyzw;

		return float4(albedoMaterialIndex.xyz, 1.0f);
	} else if (gGBufferIndex == 1) {
		// Render material id
		float4 albedoMaterialIndex = gGBufferAlbedoMaterialIndex.Load(pixelCoord, 0).xyzw;
		uint materialId = (uint)albedoMaterialIndex.w;

		// Interpret the material id as an 8-bit color
		uint red = materialId & 0xFFF00000;
		uint green = materialId & 0x000FFF00;
		uint blue = materialId & 0x000000FF;

		return float4((float)red / 8.0f, (float)green / 8.0f, (float)blue / 4.0f, 1.0f);
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