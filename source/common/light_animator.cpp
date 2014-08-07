/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/light_animator.h"


namespace Common {

void PointLightAnimator::AnimateLight(double deltaTime) {
	DirectX::XMFLOAT3 position((*m_lightList)[m_index].GetPosition());

	float xPosition = m_velocity.x * static_cast<float>(deltaTime)+position.x;
	float yPosition = m_velocity.y * static_cast<float>(deltaTime)+position.y;
	float zPosition = m_velocity.z * static_cast<float>(deltaTime)+position.z;

	// Check x for out of bounds
	if (xPosition > m_positiveBounds.x) {
		xPosition = m_positiveBounds.x - (xPosition - m_positiveBounds.x);
		m_velocity.x = -m_velocity.x;
	} else if (xPosition < m_negativeBounds.x) {
		xPosition = m_negativeBounds.x + (m_negativeBounds.x - xPosition);
		m_velocity.x = -m_velocity.x;
	}

	// Check y for out of bounds
	if (yPosition > m_positiveBounds.y) {
		yPosition = m_positiveBounds.y - (yPosition - m_positiveBounds.y);
		m_velocity.y = -m_velocity.y;
	} else if (yPosition < m_negativeBounds.y) {
		yPosition = m_negativeBounds.y + (m_negativeBounds.y - yPosition);
		m_velocity.y = -m_velocity.y;
	}

	// Check z for out of bounds
	if (zPosition > m_positiveBounds.z) {
		zPosition = m_positiveBounds.z - (zPosition - m_positiveBounds.z);
		m_velocity.z = -m_velocity.z;
	} else if (zPosition < m_negativeBounds.z) {
		zPosition = m_negativeBounds.z + (m_negativeBounds.z - zPosition);
		m_velocity.z = -m_velocity.z;
	}

	(*m_lightList)[m_index].SetPosition(DirectX::XMFLOAT3(xPosition, yPosition, zPosition));
}


void SpotLightAnimator::AnimateLight(double deltaTime) {
	// Move the light
	if (m_velocity.x != 0.0f || m_velocity.y != 0.0f || m_velocity.z == 0.0f) {
		DirectX::XMFLOAT3 position((*m_lightList)[m_index].GetPosition());

		float xPosition = m_velocity.x * static_cast<float>(deltaTime) + position.x;
		float yPosition = m_velocity.y * static_cast<float>(deltaTime) + position.y;
		float zPosition = m_velocity.z * static_cast<float>(deltaTime) + position.z;

		// Check x for out of bounds
		if (xPosition > m_positiveBounds.x) {
			xPosition = m_positiveBounds.x - (xPosition - m_positiveBounds.x);
			m_velocity.x = -m_velocity.x;
		} else if (xPosition < m_negativeBounds.x) {
			xPosition = m_negativeBounds.x + (m_negativeBounds.x - xPosition);
			m_velocity.x = -m_velocity.x;
		}

		// Check y for out of bounds
		if (yPosition > m_positiveBounds.y) {
			yPosition = m_positiveBounds.y - (yPosition - m_positiveBounds.y);
			m_velocity.y = -m_velocity.y;
		} else if (yPosition < m_negativeBounds.y) {
			yPosition = m_negativeBounds.y + (m_negativeBounds.y - yPosition);
			m_velocity.y = -m_velocity.y;
		}

		// Check z for out of bounds
		if (zPosition > m_positiveBounds.z) {
			zPosition = m_positiveBounds.z - (zPosition - m_positiveBounds.z);
			m_velocity.z = -m_velocity.z;
		} else if (zPosition < m_negativeBounds.z) {
			zPosition = m_negativeBounds.z + (m_negativeBounds.z - zPosition);
			m_velocity.z = -m_velocity.z;
		}

		(*m_lightList)[m_index].SetPosition(DirectX::XMFLOAT3(xPosition, yPosition, zPosition));
	}

	// Rotate the light
	if (m_angularVelocity.x != 0.0f || m_angularVelocity.y != 0.0f || m_angularVelocity.z == 0.0f) {
		DirectX::XMFLOAT3 direction((*m_lightList)[m_index].GetDirection());

		DirectX::XMVECTOR temp = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&direction), DirectX::XMMatrixRotationRollPitchYaw(m_angularVelocity.x, m_angularVelocity.y, m_angularVelocity.z));
		DirectX::XMStoreFloat3(&direction, temp);

		(*m_lightList)[m_index].SetDirection(direction);
	}
}

} // End of namespace Common
