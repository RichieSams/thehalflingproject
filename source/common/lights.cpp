/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/lights.h"


namespace Common {

DirectX::XMFLOAT3 ConvertPhotometricToIrradiance(const DirectX::XMFLOAT3 &color, float lumens) {
	DirectX::XMVECTOR temp = DirectX::XMLoadFloat3(&color);
	temp = DirectX::XMVectorScale(temp, lumens / 179);
	temp = DirectX::XMVectorMultiply(temp, DirectX::XMVectorSet(37.735849f, 1.492537f, 15.384615f, 1.0f));

	DirectX::XMFLOAT3 returnValue;
	DirectX::XMStoreFloat3(&returnValue, temp);

	return returnValue;
}

const ShaderDirectionalLight &DirectionalLight::GetShaderPackedLight() {
	if (!m_shaderPackedIsOutOfDate) {
		return m_shaderPackedLight;
	}

	DirectX::XMStoreFloat3(&m_shaderPackedLight.Irradiance, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&m_color), m_intensity));
	m_shaderPackedLight.Direction = m_direction;

	m_shaderPackedIsOutOfDate = false;
	return m_shaderPackedLight;
}

const ShaderPointLight &PointLight::GetShaderPackedLight() {
	if (!m_shaderPackedIsOutOfDate) {
		return m_shaderPackedLight;
	}

	m_shaderPackedLight.Irradiance = ConvertPhotometricToIrradiance(m_color, m_lumens);
	m_shaderPackedLight.Position = m_position;
	m_shaderPackedLight.Range = m_range;
	m_shaderPackedLight.InvRange = 1 / m_range;

	m_shaderPackedIsOutOfDate = false;
	return m_shaderPackedLight;
}


const ShaderSpotLight &SpotLight::GetShaderPackedLight() {
	if (!m_shaderPackedIsOutOfDate) {
		return m_shaderPackedLight;
	}

	m_shaderPackedLight.Irradiance = ConvertPhotometricToIrradiance(m_color, m_lumens);
	m_shaderPackedLight.Position = m_position;
	m_shaderPackedLight.Range = m_range;
	m_shaderPackedLight.InvRange = 1 / m_range;
	m_shaderPackedLight.Direction = m_direction;
	m_shaderPackedLight.CosOuterConeAngle = std::cos(m_outerConeAngle);
	m_shaderPackedLight.InvCosConeDifference = std::acos(m_coneDifference);

	m_shaderPackedIsOutOfDate = false;
	return m_shaderPackedLight;
}

} // End of namespace Common
