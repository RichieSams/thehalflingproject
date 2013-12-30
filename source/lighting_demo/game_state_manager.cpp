/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "lighting_demo/game_state_manager.h"

#include "common/math.h"
#include "common/camera.h"
#include "common/d3d_util.h"


namespace LightingDemo {

GameStateManager::GameStateManager() 
	: GameStateManagerBase(),
	  m_camera(1.5f * DirectX::XM_PI, 0.25f * DirectX::XM_PI, 5.0f) {
}

bool GameStateManager::Initialize(HWND hwnd, ID3D11Device **device) {
	GameStateManagerBase::Initialize(hwnd, device);

	BuildGeometryBuffers();

	// Set the view matrices to identity
	DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
	WorldViewProj.world = identity;
	WorldViewProj.view = identity;
	WorldViewProj.view = identity;

	return true;
}

void GameStateManager::Shutdown() {
}

void GameStateManager::Update() {
	WorldViewProj.view = m_camera.CreateViewMatrix(DirectX::XMVectorZero());
}

void GameStateManager::OnResize(int newClientWidth, int newClientHeight) {
	// Update the aspect ratio and the projection matrix
	WorldViewProj.projection = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, float(newClientWidth) / newClientHeight, 1.0f, 1000.0f);
}

void GameStateManager::GamePaused() {

}

void GameStateManager::GameUnpaused() {

}

void GameStateManager::MouseDown(WPARAM buttonState, int x, int y) {
	m_mouseLastPos.x = x;
	m_mouseLastPos.y = y;

	SetCapture(m_hwnd);
}

void GameStateManager::MouseUp(WPARAM buttonState, int x, int y) {
	ReleaseCapture();
}

void GameStateManager::MouseMove(WPARAM buttonState, int x, int y) {
	if ((buttonState & MK_LBUTTON) != 0) {
		// Calculate the new phi and theta based on mouse position relative to where the user clicked
		// Four mouse pixel movements is 1 degree
		float dPhi = ((float)(m_mouseLastPos.y - y) / 300);
		float dTheta = ((float)(x - m_mouseLastPos.x) / 300);

		m_camera.MoveCamera(dTheta, dPhi, 0.0f);
	}

	m_mouseLastPos.x = x;
	m_mouseLastPos.y = y;
}

void GameStateManager::MouseWheel(int zDelta) {
	// Make each wheel dedent correspond to 0.05 units
	m_camera.MoveCamera(0.0f, 0.0f, -0.005f * (float)zDelta);
}

void GameStateManager::BuildGeometryBuffers() {
}

} // End of namespace CrateDemo
