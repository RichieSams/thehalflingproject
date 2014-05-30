/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef COMMON_CAMERA_H
#define COMMON_CAMERA_H

#include "DirectXMath.h"


namespace Common {

class Camera {
public:
	Camera() 
		: m_theta(0.0f), 
		  m_phi(0.0f), 
		  m_radius(0.0f), 
		  m_up(1.0f),
		  m_target(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)),
		  m_view(DirectX::XMMatrixIdentity()),
		  m_proj(DirectX::XMMatrixIdentity()),
		  m_viewNeedsUpdate(true) {
	}
	Camera(float theta, float phi, float radius) 
		: m_theta(theta), 
		  m_phi(phi), 
		  m_radius(radius), 
		  m_up(1.0f),
		  m_target(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)),
		  m_view(DirectX::XMMatrixIdentity()),
		  m_proj(DirectX::XMMatrixIdentity()),
		  m_viewNeedsUpdate(true) {
	}

private:
	float m_theta;
	float m_phi;
	float m_radius;
	float m_up;

	DirectX::XMVECTOR m_target;

	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_proj;

	bool m_viewNeedsUpdate;

public:
	void Rotate(float dTheta, float dPhi);
	void Zoom(float distance);
	void Pan(float dx, float dy);

	void UpdateProjectionMatrix(float clientWidth, float clientHeight, float nearClip, float farClip);

	inline DirectX::XMFLOAT3 GetCameraPosition() const { 
		DirectX::XMFLOAT3 temp;
		DirectX::XMStoreFloat3(&temp, GetCameraPositionXM());

		return temp;
	}
	inline DirectX::XMVECTOR GetCameraPositionXM() const {
		return DirectX::XMVectorAdd(m_target, ToCartesian());
	}

	inline DirectX::XMMATRIX GetView() {
		if (m_viewNeedsUpdate) {
			UpdateViewMatrix();
			m_viewNeedsUpdate = false;
		}

		return m_view;
	}
	inline DirectX::XMMATRIX GetProj() { return m_proj; }

private:
	void UpdateViewMatrix();
	inline DirectX::XMVECTOR ToCartesian() const {
		float x = m_radius * sinf(m_phi) * sinf(m_theta);
		float y = m_radius * cosf(m_phi);
		float z = m_radius * sinf(m_phi) * cosf(m_theta);
		float w = 1.0f;

		return DirectX::XMVectorSet(x, y, z, w);
	}
};

} // End of namespace Common

#endif
