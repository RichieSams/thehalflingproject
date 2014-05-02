/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef COMMON_LIGHTS_H
#define COMMON_LIGHTS_H

#include "common/halfling_sys.h"

#include "DirectXMath.h"


namespace Common {

struct DirectionalLight {
public:
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT3 Direction;

private:
	float pad; // To pad the last float3 to a float4
};

struct PointLight {
public:
	PointLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	DirectX::XMFLOAT3 Position;
	float Range;

	float AttenuationDistanceUNorm;

private:
	float pad[3]; // To pad the last float3 to a float4
};

struct SpotLight {
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Direction;
	float AttenuationDistanceUNorm;

	float CosInnerConeAngle;
	float CosOuterConeAngle;

	float SpotLightFactor;

private:
	float pad; // To pad the last float3 to a float4
};

} // End of namespace Common

#endif
