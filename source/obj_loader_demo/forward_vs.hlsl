/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"

cbuffer cbPerObject : register(b1) {
	float4x4 gWorldViewProjMatrix;
    float4x4 gWorldMatrix;
};


ForwardPixelIn ForwardVS(VertexIn input) {
	ForwardPixelIn output;

	output.positionClip = mul(float4(input.position, 1.0f), gWorldViewProjMatrix);
    output.positionWorld = mul(float4(input.position, 1.0f), gWorldMatrix).xyz;
    output.normal = normalize(mul(float4(input.normal, 0.0f), gWorldMatrix).xyz);
	output.tangent = normalize(mul(float4(input.tangent, 0.0f), gWorldMatrix).xyz);
	output.texCoord = input.texCoord;
	
	return output;
}