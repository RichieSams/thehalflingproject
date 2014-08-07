/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#define TEXTURE_COUNT 0

#include "common/shaders/hlsl_util.hlsli"


void GetMaterialInfo(in float3 surfaceNormal, float3 surfaceTangent,
					 inout float3 baseColor, inout float specular, inout float3 normal, inout float metallic, inout float roughness) {
	baseColor = float3(0.5f, 0.5f, 0.5f);
	normal = surfaceNormal;

	// We'll leave the rest as default
}


#include "pbr_demo/shaders/gbuffer_ps.hlsl"