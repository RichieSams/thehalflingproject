/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef LIGHT_FUNCTIONS_SHADER_H
#define LIGHT_FUNCTIONS_SHADER_H

#include "common/shaders/lights.hlsli"

void AccumulateDirectionalLight(Material mat, DirectionalLight light, float3 posView, float3 normalView, inout float4 ambient, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = -light.DirectionView;

	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = saturate(dot(lightVector, normalView));

	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(-lightVector, normalView);
		float specFactor = pow(saturate(dot(v, -posView)), mat.Specular.w);
		
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		spec = specFactor * mat.Specular * light.Specular;
	}
}

void AccumulatePointLight(Material mat, PointLight light, float3 posView, float3 normalView, inout float4 ambient, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = light.PositionView - posView;
	float distance = length(lightVector);

	if (distance > light.Range)
		return;

	// Normalize the light vector
	lightVector /= distance;

	// Ambient
	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = saturate(dot(lightVector, normalView));

	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(-lightVector, normalView);
		float specFactor = pow(saturate(dot(v, -posView)), mat.Specular.w);
		
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		spec = specFactor * mat.Specular * light.Specular;
	}

	// Attenuate
	float attenuation = 1.0f / dot(light.Attenuation, float3(1.0f, distance, distance * distance));

	diffuse *= attenuation;
	spec *= attenuation;
}

void AccumulateSpotLight(Material mat, SpotLight light, float3 posView, float3 normalView, inout float4 ambient, inout float4 diffuse, inout float4 spec) {
	// The vector from the surface to the light
	float3 lightVector = light.PositionView - posView;
	float distance = length(lightVector);

	if (distance > light.Range)
		return;

	// Normalize the light vector
	lightVector /= distance;

	// Ambient
	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = saturate(dot(lightVector, normalView));

	[flatten]
	if (diffuseFactor > 0.0f) {
		float3 v = reflect(-lightVector, normalView);
		float specFactor = pow(saturate(dot(v, -posView)), mat.Specular.w);
		
		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		spec = specFactor * mat.Specular * light.Specular;
	}

	// Scale by spotlight factor and attenuate
	float spot = pow(max(dot(-lightVector, light.DirectionView), 0.0f), light.Spot);

	float attenuation = spot / dot(light.Attenuation, float3(1.0f, distance, distance * distance));

	ambient *= spot;
	diffuse *= attenuation;
	spec *= attenuation;
}

#endif