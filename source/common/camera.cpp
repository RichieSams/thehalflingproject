/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/camera.h"


namespace Common {

void Camera::MoveCamera(float dTheta, float dPhi, float dRadius) {
	if (m_up > 0.0f) {
		m_theta += dTheta;
	} else {
		m_theta -= dTheta;
	}

	m_phi += dPhi;
	m_radius += dRadius;

	// Keep phi within -2PI to +2PI for easy 'up' comparison
	if (m_phi > DirectX::XM_2PI) {
		m_phi -= DirectX::XM_2PI;
	} else if (m_phi < -DirectX::XM_2PI) {
		m_phi += DirectX::XM_2PI;
	}

	// If phi is between 0 to PI or -PI to -2PI, make 'up' be positive Y, other wise make it negative Y
	if ((m_phi > 0 && m_phi < DirectX::XM_PI) || (m_phi < -DirectX::XM_PI && m_phi > -DirectX::XM_2PI)) {
		m_up = 1;
	} else {
		m_up = -1;
	}
}

} // End of namespace Common
