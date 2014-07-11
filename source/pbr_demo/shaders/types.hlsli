/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef PBR_DEMO_TYPES_H
#define PBR_DEMO_TYPES_H


#define MAX_SPEC_POWER 512

struct VertexIn {
	float3 position  : POSITION;
	float3 normal    : NORMAL;
	float2 texCoord  : TEXCOORD;
	float3 tangent   : TANGENT;
};

struct InstancedVertexIn {
	float3 position  : POSITION;
	float3 normal    : NORMAL;
	float2 texCoord  : TEXCOORD;
	float3 tangent   : TANGENT;
	uint instanceId  : SV_INSTANCEID;
};

struct GBufferShaderPixelIn {
	float4 positionClip   : SV_POSITION;
	float3 normal         : NORMAL;
	float3 tangent        : TANGENT;
	float2 texCoord       : TEXCOORD;
};

struct CalculatedTrianglePixelIn {
	float4 positionClip  : SV_POSITION;
	float2 texCoord      : TEXCOORD;
};

#endif
