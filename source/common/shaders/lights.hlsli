/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef LIGHTS_SHADER_H
#define LIGHTS_SHADER_H

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 DirectionView;
	float pad;
};

struct PointLight
{ 
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 PositionView;
	float Range;

	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 PositionView;
	float Range;

	float3 DirectionView;
	float Spot;

	float3 Att;
	float pad;
};

#endif
