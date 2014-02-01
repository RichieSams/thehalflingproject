// Copyright 2010 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"


cbuffer cbPerObject : register(b1) {
	float2 gClipTranslation  : packoffset(c0.x);
    float gClipScale         : packoffset(c0.z);
};

TransformedFullScreenTrianglePixelIn FullScreenTriangleVS(uint vertexID : SV_VertexID) {
	TransformedFullScreenTrianglePixelIn output;

	uint vertexNumber = vertexID - ((vertexID + 1 & 4) / 2);

	// Parametrically work out vertex location for full screen triangle
    float2 grid = float2((vertexNumber << 1) & 2, vertexNumber & 2);
	output.texCoord = grid * 0.5;
	output.texCoord.y = 1.0f - output.texCoord.y;
	
	// Scale and translate
    output.positionClip = float4((grid * gClipScale) + gClipTranslation, 1.0f, 1.0f);
    
    return output;
}