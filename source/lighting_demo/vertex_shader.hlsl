/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"

cbuffer cbPerFrame : register(b0) {
    float4x4 gViewProjMatrix;
    float4x4 gProjMatrix;
};

cbuffer cbPerObject : register(b1) {
	float4x4 gWorldViewProjMatrix;
    float4x4 gWorldViewMatrix;
};


PixelIn VS(VertexIn input) {
	PixelIn output;

	output.positionClip = mul(float4(input.position, 1.0f), gWorldViewProjMatrix);
    output.positionView = mul(float4(input.position, 1.0f), gWorldViewMatrix).xyz;
    output.normalView   = mul(float4(input.normal, 0.0f), gWorldViewMatrix).xyz;
	
	return output;
}