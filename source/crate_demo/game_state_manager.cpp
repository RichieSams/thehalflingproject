/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "crate_demo/game_state_manager.h"



namespace CrateDemo {

GameStateManager::GameStateManager() 
	: m_hwnd(nullptr),
}

bool CrateDemo::GameStateManager::Initialize(HWND hwnd) {
	m_hwnd = hwnd;

	// Set the view matrices to identity
	DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
	WorldViewProj.world = identity;
	WorldViewProj.view = identity;
	WorldViewProj.view = identity;

	return true;
}

void CrateDemo::GameStateManager::Shutdown() {

}

void CrateDemo::GameStateManager::Update() {
}

void GameStateManager::OnResize(int newClientWidth, int newClientHeight) {
	// Update the aspect ratio and the projection matrix
	WorldViewProj.projection = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, float(newClientWidth) / newClientHeight, 1.0f, 1000.0f);
}

void CrateDemo::GameStateManager::GamePaused() {

}

void CrateDemo::GameStateManager::GameUnpaused() {

}

} // End of namespace CrateDemo
