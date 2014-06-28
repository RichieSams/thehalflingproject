/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/halfling_sys.h"

#include "DirectXMath.h"


namespace Common {

struct DirectionalLight {
public:
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT3 Direction;

private:
	float pad; // To pad the last float3 to a float4
};

struct PointLight {
public:
	PointLight() { ZeroMemory(this, sizeof(this)); }
	PointLight(DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular, DirectX::XMFLOAT3 position, float range, float attenuationDistanceUNorm)
		: Diffuse(diffuse),
		  Specular(specular),
		  Position(position),
		  Range(range),
		  AttenuationDistanceUNorm(attenuationDistanceUNorm) {
	}

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
	SpotLight(DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular, DirectX::XMFLOAT3 position, float range, DirectX::XMFLOAT3 direction, float attenuationDistanceUNorm, float cosInnerConeAngle, float cosOuterConeAngle)
		: Diffuse(diffuse),
		  Specular(specular),
		  Position(position),
		  Range(range),
		  Direction(direction),
		  AttenuationDistanceUNorm(attenuationDistanceUNorm),
		  CosInnerConeAngle(cosInnerConeAngle),
		  CosOuterConeAngle(cosOuterConeAngle) {
	}

	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Direction;
	float AttenuationDistanceUNorm;

	float CosInnerConeAngle;
	float CosOuterConeAngle;

private:
	float pad[2]; // To pad the last float3 to a float4
};

} // End of namespace Common
