/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#define TEXTURE_COUNT 2

#include "common/shaders/hlsl_util.hlsli"


void GetMaterialInfo(in float3 surfaceNormal, float3 surfaceTangent, in float4 inputTextureSamples[TEXTURE_COUNT],
					 inout float3 baseColor, inout float specular, inout float3 normal, inout float metallic, inout float roughness) {
	// The first texture is base color and roughness
	baseColor = inputTextureSamples[0].rgb;
	roughness = lerp(0.4f, 0.8f, inputTextureSamples[0].a);

	// The second texture is the normal. There isn't an alpha channel
	normal = PerturbNormal(surfaceNormal, inputTextureSamples[1].rgb, surfaceTangent);

	// We'll leave the rest as default
}


#include "pbr_demo/shaders/gbuffer_ps.hlsl"
