/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_CAMERA_H
#define COMMON_CAMERA_H

#include "DirectXMath.h"


namespace Common {

class Camera {
public:
	Camera() : m_theta(0.0f), m_phi(0.0f), m_radius(0.0f), m_up(1.0f) {}
	Camera(float theta, float phi, float radius) : m_theta(theta), m_phi(phi), m_radius(radius), m_up(1.0f) {}

private:
	float m_theta;
	float m_phi;
	float m_radius;
	float m_up;

public:
	void MoveCamera(float dTheta, float dPhi, float dRadius);
	inline DirectX::XMMATRIX Camera::CreateViewMatrix(DirectX::XMVECTOR target) const {
		DirectX::XMVECTOR pos = ToCartesianVector();
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, m_up, 0.0f, 0.0f);

		return DirectX::XMMatrixLookAtLH(pos, target, up);
	}

private:
	inline DirectX::XMVECTOR Camera::ToCartesianVector() const {
		float x = m_radius * sinf(m_phi) * sinf(m_theta);
		float y = m_radius * cosf(m_phi);
		float z = m_radius * sinf(m_phi) * cosf(m_theta);
		float w = 1.0f;

		return DirectX::XMVectorSet(x, y, z, w);
	}
};

} // End of namespace Common

#endif