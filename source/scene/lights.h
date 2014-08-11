/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/halfling_sys.h"

#include <DirectXMath.h>


namespace Scene {

DirectX::XMFLOAT3 ConvertPhotometricToIrradiance(const DirectX::XMFLOAT3 &color, float lumens);


struct ShaderDirectionalLight {
public:
	ShaderDirectionalLight() { ZeroMemory(this, sizeof(this)); }
	ShaderDirectionalLight(DirectX::XMFLOAT3 irradiance, DirectX::XMFLOAT3 direction)
		: Irradiance(irradiance),
		  Direction(direction) {
	}

	DirectX::XMFLOAT3 Irradiance;
private:
	float pad;

public:
	DirectX::XMFLOAT3 Direction;

private:
	float pad2; // To pad the last float3 to a float4
};

struct ShaderPointLight {
public:
	ShaderPointLight() { ZeroMemory(this, sizeof(this)); }
	ShaderPointLight(DirectX::XMFLOAT3 irradiance, DirectX::XMFLOAT3 position, float range, float invRange)
		: Irradiance(irradiance),
		  Range(range),
		  Position(position),
		  InvRange(invRange) {
	}

	DirectX::XMFLOAT3 Irradiance;
	float Range;

	DirectX::XMFLOAT3 Position;	
	float InvRange;
};

struct ShaderSpotLight {
	ShaderSpotLight() { ZeroMemory(this, sizeof(this)); }
	ShaderSpotLight(DirectX::XMFLOAT3 irradiance, DirectX::XMFLOAT3 position, float range, float invRange, DirectX::XMFLOAT3 direction, float cosOuterConeAngle, float invCosConeDifference)
		: Irradiance(irradiance),
		  Range(range),
		  Position(position),
		  InvRange(invRange),
		  Direction(direction),
		  CosOuterConeAngle(cosOuterConeAngle),
		  InvCosConeDifference(invCosConeDifference) {
	}

	DirectX::XMFLOAT3 Irradiance;
	float Range;

	DirectX::XMFLOAT3 Position;
	float InvRange;

	DirectX::XMFLOAT3 Direction;

	float CosOuterConeAngle;
	float InvCosConeDifference;

private:
	float pad[3];
};


class DirectionalLight {
public:
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }
	DirectionalLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction, float intensity)
		: m_color(color),
		  m_direction(direction),
		  m_intensity(intensity),
		  m_shaderPackedIsOutOfDate(false),
		  m_shaderPackedLight(DirectX::XMFLOAT3(color.x * intensity, color.y * intensity, color.z * intensity), direction) {
	}

private:
	DirectX::XMFLOAT3 m_color;
	float m_intensity;

	DirectX::XMFLOAT3 m_direction;
	
	bool m_shaderPackedIsOutOfDate;
	ShaderDirectionalLight m_shaderPackedLight;

public:
	inline const DirectX::XMFLOAT3 &GetColor() const { return m_color; }
	inline void SetColor(const DirectX::XMFLOAT3 &color) { m_color = color; m_shaderPackedIsOutOfDate = true; }

	inline const float GetIntensity() const { return m_intensity; }
	inline void SetIntensity(float intenisty) { m_intensity = intenisty; m_shaderPackedIsOutOfDate = true; }

	inline const DirectX::XMFLOAT3 &GetDirection() const { return m_direction; }
	inline void SetDirection(const DirectX::XMFLOAT3 &direction) { m_direction = direction; m_shaderPackedIsOutOfDate = true; }

	const ShaderDirectionalLight &GetShaderPackedLight();
};

class PointLight {
public:
	PointLight() { ZeroMemory(this, sizeof(this)); }
	PointLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 position, float lumens, float range)
		: m_color(color),
		  m_lumens(lumens),
		  m_position(position),
		  m_range(range),
		  m_shaderPackedIsOutOfDate(true),
		  m_shaderPackedLight(ConvertPhotometricToIrradiance(color, lumens), position, range, 1 / range) {
	}

private:
	DirectX::XMFLOAT3 m_color;
	float m_lumens;

	DirectX::XMFLOAT3 m_position;
	float m_range;

	bool m_shaderPackedIsOutOfDate;
	ShaderPointLight m_shaderPackedLight;

public:
	inline const DirectX::XMFLOAT3 &GetColor() const { return m_color; }
	inline void SetColor(const DirectX::XMFLOAT3 &color) { m_color = color; m_shaderPackedIsOutOfDate = true; }

	inline float GetLumens() const { return m_lumens; }
	inline void SetLumens(float lumens) { m_lumens = lumens; m_shaderPackedIsOutOfDate = true; }

	inline const DirectX::XMFLOAT3 &GetPosition() const { return m_position; }
	inline void SetPosition(const DirectX::XMFLOAT3 &position) { m_position = position; m_shaderPackedIsOutOfDate = true; }

	inline float GetRange() const { return m_range; }
	inline void SetRange(float range) { m_range = range; m_shaderPackedIsOutOfDate = true; }

	const ShaderPointLight &GetShaderPackedLight();
};

class SpotLight {
public:
	SpotLight() { ZeroMemory(this, sizeof(this)); }
	SpotLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 position, float lumens, float range, DirectX::XMFLOAT3 direction, float outerConeAngle, float coneDifference)
		: m_color(color),
		  m_lumens(lumens),
		  m_position(position),
		  m_range(range),
		  m_direction(direction),
		  m_outerConeAngle(outerConeAngle),
		  m_coneDifference(coneDifference) {
	}

private:
	DirectX::XMFLOAT3 m_color;
	float m_lumens;

	DirectX::XMFLOAT3 m_position;
	float m_range;

	DirectX::XMFLOAT3 m_direction;

	float m_outerConeAngle;
	float m_coneDifference;

	bool m_shaderPackedIsOutOfDate;
	ShaderSpotLight m_shaderPackedLight;

public:
	inline const DirectX::XMFLOAT3 &GetColor() const { return m_color; }
	inline void SetColor(const DirectX::XMFLOAT3 &color) { m_color = color; m_shaderPackedIsOutOfDate = true; }

	inline float GetLumens() const { return m_lumens; }
	inline void SetLumens(float lumens) { m_lumens = lumens; m_shaderPackedIsOutOfDate = true; }

	inline const DirectX::XMFLOAT3 &GetPosition() const { return m_position; }
	inline void SetPosition(const DirectX::XMFLOAT3 &position) { m_position = position; m_shaderPackedIsOutOfDate = true; }

	inline float GetRange() const { return m_range; }
	inline void SetRange(float range) { m_range = range; m_shaderPackedIsOutOfDate = true; }

	inline const DirectX::XMFLOAT3 &GetDirection() const { return m_direction; }
	inline void SetDirection(const DirectX::XMFLOAT3 &direction) { m_direction = direction; m_shaderPackedIsOutOfDate = true; }

	inline float GetOuterConeAngle() const { return m_outerConeAngle; }
	inline void SetOuterConeAngle(float outerConeAngle) { m_outerConeAngle = outerConeAngle; m_shaderPackedIsOutOfDate = true; }

	inline float GetConeDifference() const { return m_coneDifference; }
	inline void SetConeDifference(float coneDifference) { m_coneDifference = coneDifference; m_shaderPackedIsOutOfDate = true; }

	const ShaderSpotLight &GetShaderPackedLight();
};

} // End of namespace Scene
