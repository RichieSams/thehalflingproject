/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef LIGHT_FUNCTIONS_SHADER_H
#define LIGHT_FUNCTIONS_SHADER_H

#include "common/shaders/lights.hlsli"
#include "common/shaders/materials.hlsli"


struct SurfaceProperties {
	float4 diffuseAlbedo;
	float4 specAlbedoAndPower;
	float3 position;
	float3 normal;
};

void AccumulateBlinnPhongDirectionalLight(DirectionalLight light, SurfaceProperties surfProps, float3 toEye, inout float4 diffuse, inout float4 spec) {
	float diffuseFactor = dot(-light.Direction, surfProps.normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		diffuse += diffuseFactor * (surfProps.diffuseAlbedo * light.Diffuse);

		float3 v = reflect(light.Direction, surfProps.normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), surfProps.specAlbedoAndPower.w);

		spec += specFactor * (float4(surfProps.specAlbedoAndPower.xyz, 1.0f) * light.Specular);
	}
}

void AccumulateBlinnPhongPointLight(PointLight light, SurfaceProperties surfProps, float3 toEye, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = light.Position - surfProps.position;
	float distance = length(lightVector);

	if (distance > light.Range)
		return;

	// Normalize the light vector
	lightVector /= distance;

	float diffuseFactor = dot(lightVector, surfProps.normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		float attenuation = 1.0f - smoothstep(light.Range * light.AttenuationDistanceUNorm, light.Range, distance);

		diffuse += (attenuation * diffuseFactor) * (surfProps.diffuseAlbedo * light.Diffuse);

		float3 v = reflect(-lightVector, surfProps.normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), surfProps.specAlbedoAndPower.w);

		spec += (attenuation * specFactor) * (float4(surfProps.specAlbedoAndPower.xyz, 1.0f) * light.Specular);
	}
}

void AccumulateBlinnPhongSpotLight(SpotLight light, SurfaceProperties surfProps, float3 toEye, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = light.Position - surfProps.position;
	float distance = length(lightVector);

	if (distance > light.Range)
		return;

	// Normalize the light vector
	lightVector /= distance;

	float diffuseFactor = dot(lightVector, surfProps.normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		float currentAngle = dot(-lightVector, light.Direction);
		float spot = 1.0f - smoothstep(light.CosInnerConeAngle, light.CosOuterConeAngle, currentAngle);
		float attenuation = spot * (1.0f - smoothstep(light.Range * light.AttenuationDistanceUNorm, light.Range, distance));

		diffuse += (attenuation * diffuseFactor) * (surfProps.diffuseAlbedo * light.Diffuse);

		float3 v = reflect(-lightVector, surfProps.normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), surfProps.specAlbedoAndPower.w);
		
		spec += (attenuation * specFactor) * (float4(surfProps.specAlbedoAndPower.xyz, 1.0f) * light.Specular);
	}
}

#endif