/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_LIGHT_ANIMATOR_H
#define COMMON_LIGHT_ANIMATOR_H

#include "common/lights.h"

#include "DirectXMath.h"


namespace Common {

class LightAnimatorBase {
protected:
	LightAnimatorBase(DirectX::XMFLOAT3 velocity, DirectX::XMFLOAT3 negativeBounds, DirectX::XMFLOAT3 positiveBounds)
		: m_velocity(velocity), m_negativeBounds(negativeBounds), m_positiveBounds(positiveBounds) {}

protected:
	DirectX::XMFLOAT3 m_velocity;

	DirectX::XMFLOAT3 m_positiveBounds;
	DirectX::XMFLOAT3 m_negativeBounds;
};


class PointLightAnimator : public LightAnimatorBase {
public:
	PointLightAnimator(DirectX::XMFLOAT3 velocity, DirectX::XMFLOAT3 negativeBounds, DirectX::XMFLOAT3 positiveBounds)
		: LightAnimatorBase(velocity, negativeBounds, positiveBounds) {}

public:
	void MoveLight(Common::PointLight *light, double deltaTime);
};

} // End of namespace Common

#endif