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
	DirectionalLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction)
		: Color(color),
		  Direction(direction) {
	}

	DirectX::XMFLOAT3 Color;
private:
	float pad;

public:
	DirectX::XMFLOAT3 Direction;

private:
	float pad2; // To pad the last float3 to a float4
};

struct PointLight {
public:
	PointLight() { ZeroMemory(this, sizeof(this)); }
	PointLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 position, float range, float invRange)
		: Color(color),
		  Range(range),
		  Position(position),
		  InvRange(invRange) {
	}

	DirectX::XMFLOAT3 Color;
	float Range;

	DirectX::XMFLOAT3 Position;	
	float InvRange;
};

struct SpotLight {
	SpotLight() { ZeroMemory(this, sizeof(this)); }
	SpotLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 position, float range, float invRange, DirectX::XMFLOAT3 direction, float cosOuterConeAngle, float invCosConeDifference)
		: Color(color),
		  Range(range),
		  Position(position),
		  InvRange(invRange),
		  Direction(direction),
		  CosOuterConeAngle(cosOuterConeAngle),
		  InvCosConeDifference(invCosConeDifference) {
	}

	DirectX::XMFLOAT3 Color;
	float Range;

	DirectX::XMFLOAT3 Position;
	float InvRange;

	DirectX::XMFLOAT3 Direction;

	float CosOuterConeAngle;
	float InvCosConeDifference;

private:
	float pad[3];
};

} // End of namespace Common
