/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "types.hlsli"


Texture2D<float4> gHDRInput : register(t0);


float4 PostProcessPS(CalculatedTrianglePixelIn input) : SV_TARGET {
	// For now, we're just going to let the hardware map the HDR to the backbuffer
	return gHDRInput[input.positionClip.xy];
}
