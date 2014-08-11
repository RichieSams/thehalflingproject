/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "scene/lights.h"

#include "DirectXMath.h"

#include <vector>


namespace Scene {

class PointLightAnimator {
public:
	PointLightAnimator(DirectX::XMFLOAT3 velocity, const DirectX::XMFLOAT3 negativeBounds, const DirectX::XMFLOAT3 positiveBounds, std::vector<PointLight> *lightList, uint index)
		: m_velocity(velocity),
		  m_negativeBounds(negativeBounds),
		  m_positiveBounds(positiveBounds),
		  m_lightList(lightList),
		  m_index(index) {
	}

private:
	DirectX::XMFLOAT3 m_velocity;

	DirectX::XMFLOAT3 m_positiveBounds;
	DirectX::XMFLOAT3 m_negativeBounds;

	std::vector<PointLight> *m_lightList;
	uint m_index;

public:
	void AnimateLight(double deltaTime);
};


class SpotLightAnimator {
public:
	SpotLightAnimator(DirectX::XMFLOAT3 velocity, const DirectX::XMFLOAT3 negativeBounds, const DirectX::XMFLOAT3 positiveBounds, DirectX::XMFLOAT3 angularVelocity, std::vector<SpotLight> *lightList, uint index)
		: m_velocity(velocity),
		  m_negativeBounds(negativeBounds),
		  m_positiveBounds(positiveBounds),
		  m_angularVelocity(angularVelocity),
		  m_lightList(lightList),
		  m_index(index) {
	}

private:
	DirectX::XMFLOAT3 m_velocity;

	DirectX::XMFLOAT3 m_positiveBounds;
	DirectX::XMFLOAT3 m_negativeBounds;

	DirectX::XMFLOAT3 m_angularVelocity;

	std::vector<SpotLight> *m_lightList;
	uint m_index;

public:
	void AnimateLight(double deltaTime);
};

} // End of namespace Scene
