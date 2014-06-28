/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "DirectXMath.h"


namespace Common {

/** 
 * A class for emulating a camera looking into the scene. 
 * It stores the view matrix and projection matrix for use by the renderer
 * and provides methods for moving the camera around the scene
 */
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
	/**
	 * Rotate the camera about a point in front of it (m_target). Theta is a rotation 
	 * that tilts the camera forward and backward. Phi tilts the camera side to side. 
	 *
	 * @param dTheta    The number of radians to rotate in the theta direction
	 * @param dPhi      The number of radians to rotate in the phi direction
	 */
	void Rotate(float dTheta, float dPhi);
	/**
	 * Move the camera down the look vector, closer to m_target. If we overtake m_target,
	 * it is reprojected 30 units down the look vector
	 *
	 * TODO: Find a way to *not* hard-code the reprojection distance. Perhaps base it on the 
	 *       scene size? Or maybe have it defined in an settings.ini file
	 *
	 * @param distance    The distance to zoom. Negative distance will move the camera away from the target, positive will move towards
	 */
	void Zoom(float distance);
	/**
	 * Moves the camera within its local X-Y plane
	 *
	 * @param dx    The amount to move the camera right or left
	 * @param dy    The amount to move the camera up or down
	 */
	void Pan(float dx, float dy);

	/**
	 * Re-creates the internal projection matrix based on the input parameters
	 *
	 * @param clientWidth     The width of the client window
	 * @param clientHeight    The height of the client window
	 * @param nearClip        The distance to the near clip plane
	 * @param farClip         The distance to the far clip plane
	 */
	void UpdateProjectionMatrix(float clientWidth, float clientHeight, float nearClip, float farClip);

	/**
	 * Returns the position of the camera in Cartesian coordinates
	 *
	 * @return    The position of the camera
	 */
	inline DirectX::XMFLOAT3 GetCameraPosition() const { 
		DirectX::XMFLOAT3 temp;
		DirectX::XMStoreFloat3(&temp, GetCameraPositionXM());

		return temp;
	}
	/**
	 * Returns the position of the camera in Cartesian coordinates
	 *
	 * @return    The position of the camera
	 */
	inline DirectX::XMVECTOR GetCameraPositionXM() const {
		return DirectX::XMVectorAdd(m_target, ToCartesian());
	}

	/**
	 * Returns the view matrix represented by the camera
	 *
	 * @return    The view matrix
	 */
	inline DirectX::XMMATRIX GetView() {
		if (m_viewNeedsUpdate) {
			UpdateViewMatrix();
			m_viewNeedsUpdate = false;
		}

		return m_view;
	}
	/**
	 * Returns the internal projection matrix
	 *
	 * @return    The projection matrix
	 */
	inline DirectX::XMMATRIX GetProj() { return m_proj; }

private:
	/**
	 * Re-creates the view matrix. Don't call this directly. Lazy load
	 * the view matrix with GetView().       
	 */
	void UpdateViewMatrix();
	/**
	 * A helper function for converting the camera's location parameters
	 * into Cartesian coordinates relative to m_target. To get the absolute location,
	 * add this to m_target;
	 *
	 * @return    The camera's location relative to m_target in cartesian coordinates
	 */
	inline DirectX::XMVECTOR ToCartesian() const {
		float x = m_radius * sinf(m_phi) * sinf(m_theta);
		float y = m_radius * cosf(m_phi);
		float z = m_radius * sinf(m_phi) * cosf(m_theta);
		float w = 1.0f;

		return DirectX::XMVectorSet(x, y, z, w);
	}
};

} // End of namespace Common
