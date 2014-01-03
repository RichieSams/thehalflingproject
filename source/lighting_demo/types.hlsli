/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef LIGHTING_DEMO_TYPES_H
#define LIGHTING_DEMO_TYPES_H

struct VertexIn {
	float3 position  : POSITION;
	float3 normal    : NORMAL;
};

struct PixelIn {
	float4 positionClip   : SV_POSITION;
	float3 positionWorld  : POSITION_WORLD;
	float3 normal         : NORMAL;
};

#endif
