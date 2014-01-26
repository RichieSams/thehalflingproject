/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef LIGHT_FUNCTIONS_SHADER_H
#define LIGHT_FUNCTIONS_SHADER_H

#include "common/shaders/lights.hlsli"
#include "common/shaders/materials.hlsli"


void AccumulateBlinnPhongDirectionalLight(BlinnPhongMaterial mat, DirectionalLight light, float3 normal, float3 toEye, inout float4 ambient, inout float4 diffuse, inout float4 spec) {
	// Use explicit add instead of += so we can use MAD
	ambient = mat.Ambient * light.Ambient + ambient;

	float diffuseFactor = dot(-light.Direction, normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		// Use explicit add instead of += so we can use MAD
		diffuse = diffuseFactor * (mat.Diffuse * light.Diffuse) + diffuse;

		[flatten]
		// The alpha channel of the Ambient term is specular intensity
		float specularIntensity = mat.Ambient.w;
		if (specularIntensity > 0.0f) {
			float3 v = reflect(light.Direction, normal);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

			// Use explicit add instead of += so we can use MAD
			spec = (specularIntensity * specFactor) * (mat.Specular * light.Specular) + spec;
		}
	}
}

void AccumulateBlinnPhongPointLight(BlinnPhongMaterial mat, PointLight light, float3 position, float3 normal, float3 toEye, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = light.Position - position;
	float distance = length(lightVector);

	if (distance > light.Range)
		return;

	// Normalize the light vector
	lightVector /= distance;

	float diffuseFactor = dot(lightVector, normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		float attenuation = 1.0f - smoothstep(light.Range * light.AttenuationDistanceUNorm, light.Range, distance);

		// Use explicit add instead of += so we can use MAD
		float4 diffuseAdd = (attenuation * diffuseFactor) * (mat.Diffuse * light.Diffuse);
		diffuse = diffuseAdd + diffuse;

		[flatten]
		// The alpha channel of the Ambient term is specular intensity
		float specularIntensity = mat.Ambient.w;
		if (specularIntensity > 0.0f) {
			float3 v = reflect(-lightVector, normal);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

			// Use explicit add instead of += so we can use MAD
			spec = (attenuation * specularIntensity * specFactor) * (mat.Specular * light.Specular) + spec;
		}
	}
}

void AccumulateBlinnPhongSpotLight(BlinnPhongMaterial mat, SpotLight light, float3 position, float3 normal, float3 toEye, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = light.Position - position;
	float distance = length(lightVector);

	if (distance > light.Range)
		return;

	// Normalize the light vector
	lightVector /= distance;

	float spot = pow(max(dot(-lightVector, light.Direction), 0.0f), light.Spot);

	float diffuseFactor = dot(lightVector, normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		float attenuation = spot / dot(light.Attenuation, float3(1.0f, distance, distance * distance));

		// Use explicit add instead of += so we can use MAD
		diffuse = (attenuation * diffuseFactor) * (mat.Diffuse * light.Diffuse) + diffuse;

		[flatten]
		// The alpha channel of the Ambient term is specular intensity
		float specularIntensity = mat.Ambient.w;
		if (specularIntensity > 0.0f) {
			float3 v = reflect(-lightVector, normal);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
			
			// Use explicit add instead of += so we can use MAD
			spec = (attenuation * specularIntensity * specFactor) * (mat.Specular * light.Specular) + spec;
		}
	}
}

#endif