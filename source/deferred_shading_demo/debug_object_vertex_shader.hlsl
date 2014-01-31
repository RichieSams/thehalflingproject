/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"


DebugObjectShaderPixelIn DebugObjectVS(DebugObjectShaderVertexIn input) {
	DebugObjectShaderPixelIn output;
	
	output.positionClip = mul(float4(input.position, 1.0f), input.instanceWorldViewProj);
	output.color = input.instanceColor;

	return output;
}