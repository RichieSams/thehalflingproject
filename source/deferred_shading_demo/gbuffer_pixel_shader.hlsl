/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"
#include "common/shaders/materials.hlsli"
#include "deferred_shading_demo/hlsl_util.hlsli"


struct GBuffer {
	float4 albedo : SV_Target0;
    float3 normal_specular : SV_Target1;
};

cbuffer cbPerObject : register(b3) {
	Material gMaterial;
};

Texture2D gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

void DeferredPS(GBufferShaderPixelIn input, out GBuffer gbuffer) {
	gbuffer.normal_specular = float3(CartesianToSpherical(input.normal), 1.0f);
	gbuffer.albedo = gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord);
	gbuffer.albedo.w = gMaterial.Specular.w;
}