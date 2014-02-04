/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"

float4 DebugObjectPS(DebugObjectShaderPixelIn input) : SV_TARGET {
	return input.color;
}