/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef DEFERRED_SHADING_DEMO_TYPES_H
#define DEFERRED_SHADING_DEMO_TYPES_H

struct VertexIn {
	float3 position  : POSITION;
	float3 normal    : NORMAL;
	float2 texCoord  : TEXCOORD;
};

struct ForwardPixelIn {
	float4 positionClip   : SV_POSITION;
	float3 positionWorld  : POSITION_WORLD;
	float3 normal         : NORMAL;
	float2 texCoord       : TEXCOORD;
};

struct GBufferShaderPixelIn {
	float4 positionClip   : SV_POSITION;
	float3 normal         : NORMAL;
	float2 texCoord       : TEXCOORD;
};

struct FullScreenTrianglePixelIn {
	float4 positionClip   : SV_POSITION;
};

#endif
