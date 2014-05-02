/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef LIGHTS_SHADER_H
#define LIGHTS_SHADER_H

struct DirectionalLight {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight { 
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float AttenuationDistanceUNorm;
	float3 pad;
};

struct SpotLight {
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float AttenuationDistanceUNorm;

	float CosInnerConeAngle;
	float CosOuterConeAngle;
	
	float SpotLightFactor;

	float pad;
};

#endif
