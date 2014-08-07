/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef LIGHTS_SHADER_H
#define LIGHTS_SHADER_H

struct DirectionalLight {
	float3 Irradiance;
	float pad;

	float3 Direction;
	float pad2;
};

struct PointLight { 
	float3 Irradiance;
	float Range;

	float3 Position;
	float InvRange;
};

struct SpotLight {
	float3 Irradiance;
	float Range;

	float3 Position;
	float InvRange;

	float3 Direction;

	float CosOuterConeAngle;
	float InvCosConeDifference;

	float3 pad;
};

#endif
