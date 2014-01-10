/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"
#include "common/shaders/materials.hlsli"


struct GBuffer {
	float4 albedo : SV_Target0;
    float3 normal_specular : SV_Target1;
};

cbuffer cbPerObject : register(b3) {
	Material gMaterial;
};

Texture2D gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

// Converts a normalized cartesian direction vector
// to spherical coordinates.
float2 CartesianToSpherical(float3 cartesian) {
  float2 spherical;

  spherical.x = atan2(cartesian.y, cartesian.x);
  spherical.y = cartesian.z;

  return spherical;
}

// Converts a spherical coordinate to a normalized
// cartesian direction vector.
float3 SphericalToCartesian(float2 spherical) {
  float2 sinCosTheta, sinCosPhi;

  sincos(spherical.x, sinCosTheta.x, sinCosTheta.y);
  sinCosPhi = float2(sqrt(1.0 - spherical.y * spherical.y), spherical.y);

  return float3(sinCosTheta.y * sinCosPhi.x, sinCosTheta.x * sinCosPhi.x, sinCosPhi.y);    
}

void DeferredPS(PixelIn input, out GBuffer gbuffer) {
	gbuffer.normal_specular = float3(CartesianToSpherical(input.normal), 1.0f);
	gbuffer.albedo = gDiffuseTexture.Sample(gDiffuseSampler, input.texCoord);
	gbuffer.albedo.w = gMaterial.Specular.w;
}