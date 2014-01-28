/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/light_animator.h"


namespace Common {

void PointLightAnimator::MoveLight(Common::PointLight *light, double deltaTime) {
	float xPosition = m_velocity.x * deltaTime + light->Position.x;
	float yPosition = m_velocity.y * deltaTime + light->Position.y;
	float zPosition = m_velocity.z * deltaTime + light->Position.z;

	// Check x for out of bounds
	if (xPosition > m_positiveBounds.x) {
		xPosition = xPosition - (m_positiveBounds.x - xPosition);
		m_velocity.x = -m_velocity.x;
	} else if (xPosition < m_negativeBounds.x) {
		xPosition = xPosition + (m_negativeBounds.x - xPosition);
		m_velocity.x = -m_velocity.x;
	}

	// Check y for out of bounds
	if (yPosition > m_positiveBounds.y) {
		yPosition = yPosition - (m_positiveBounds.y - yPosition);
		m_velocity.y = -m_velocity.y;
	} else if (yPosition < m_negativeBounds.y) {
		yPosition = yPosition + (m_negativeBounds.y - yPosition);
		m_velocity.y = -m_velocity.y;
	}

	// Check z for out of bounds
	if (zPosition > m_positiveBounds.z) {
		zPosition = zPosition - (m_positiveBounds.z - zPosition);
		m_velocity.z = -m_velocity.z;
	} else if (zPosition < m_negativeBounds.z) {
		zPosition = zPosition + (m_negativeBounds.z - zPosition);
		m_velocity.z = -m_velocity.z;
	}

	light->Position = {xPosition, yPosition, zPosition};
}

} // End of namespace Common
