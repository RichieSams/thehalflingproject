/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "types.hlsli"
#include "hlsl_util.hlsli"


#define NUM_VECTORS_PER_INSTANCE 3u

cbuffer cbPerFrame : register(b0) {
	float4x4 gViewProjMatrix;
}

cbuffer cbPerObject : register(b1) {
	uint gStartVector;
};

StructuredBuffer<float4> gInstanceBuffer : register(t0);


GBufferShaderPixelIn InstancedGBufferVS(InstancedVertexIn input) {
	GBufferShaderPixelIn output;

	uint worldMatrixOffset = input.instanceId * NUM_VECTORS_PER_INSTANCE + gStartVector;

	float4 c0 = gInstanceBuffer[worldMatrixOffset];
	float4 c1 = gInstanceBuffer[worldMatrixOffset + 1];
	float4 c2 = gInstanceBuffer[worldMatrixOffset + 2];

	float4x4 world = CreateMatrixFromCols(c0, c1, c2, float4(0.0f, 0.0f, 0.0f, 1.0f));
	float4x4 worldViewProj = mul(world, gViewProjMatrix);

	output.positionClip = mul(float4(input.position, 1.0f), worldViewProj);
    output.normal = normalize(mul(float4(input.normal, 0.0f), world).xyz);
	output.tangent = normalize(mul(float4(input.tangent, 0.0f), world).xyz);
	output.texCoord = input.texCoord;
	
	return output;
}
