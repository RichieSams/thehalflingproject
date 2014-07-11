// Copyright 2010 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

/**
 * Modified for use in The Halfling Project - A Graphics Engine and Projects
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "pbr_demo/shaders/types.hlsli"


CalculatedTrianglePixelIn FullScreenTriangleVS(uint vertexID : SV_VertexID) {
	CalculatedTrianglePixelIn output;

	// Parametrically work out vertex location for full screen triangle
	output.positionClip.x = (float)(vertexID / 2) * 4.0f - 1.0f;
	output.positionClip.y = (float)(vertexID % 2) * 4.0f - 1.0f;
	output.positionClip.z = 0.0f;
	output.positionClip.w = 1.0f;

	output.texCoord.x = (float)(vertexID / 2) * 2.0f;
	output.texCoord.y = 1.0f - (float)(vertexID % 2) * 2.0f;

    return output;
}
