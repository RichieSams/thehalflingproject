/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef LIGHT_FUNCTIONS_SHADER_H
#define LIGHT_FUNCTIONS_SHADER_H

#include "common/shaders/lights.hlsli"

void AccumulateDirectionalLight(Material mat, DirectionalLight light, float3 normal, float3 toEye, inout float4 ambient, inout float4 diffuse, inout float4 spec) {
	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot(-light.Direction, normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(light.Direction, normal);
		float w = dot(v, toEye);
		float specFactor = pow(max(w, 0.0f), mat.Specular.w);
		
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		spec = specFactor * mat.Specular * light.Specular;
	}
}

void AccumulatePointLight(Material mat, PointLight light, float3 position, float3 normal, float3 toEye, inout float4 ambient, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = light.Position - position;
	float distance = length(lightVector);

	if (distance > light.Range)
		return;

	// Normalize the light vector
	lightVector /= distance;

	// Ambient
	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot(lightVector, normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(-lightVector, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
		
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		spec = specFactor * mat.Specular * light.Specular;
	}

	// Attenuate
	float attenuation = 1.0f / dot(light.Attenuation, float3(1.0f, distance, distance * distance));

	diffuse *= attenuation;
	spec *= attenuation;
}

void AccumulateSpotLight(Material mat, SpotLight light, float3 position, float3 normal, float3 toEye, inout float4 ambient, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = light.Position - position;
	float distance = length(lightVector);

	if (distance > light.Range)
		return;

	// Normalize the light vector
	lightVector /= distance;

	// Ambient
	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot(lightVector, normal);

	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(-lightVector, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
		
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		spec = specFactor * mat.Specular * light.Specular;
	}

	// Scale by spotlight factor and attenuate
	float spot = pow(max(dot(-lightVector, light.Direction), 0.0f), light.Spot);

	float attenuation = spot / dot(light.Attenuation, float3(1.0f, distance, distance * distance));

	ambient *= spot;
	diffuse *= attenuation;
	spec *= attenuation;
}

#endif