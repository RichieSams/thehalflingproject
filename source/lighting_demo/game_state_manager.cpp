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
	  m_camera(1.5f * DirectX::XM_PI, 0.25f * DirectX::XM_PI, 5.0f),
	  m_accumulatedWaveSimTime(0) {
}

	m_waveSimulator.Init(100, 100, 0.8f, 0.03f, 3.25f, 1.0f);
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

	m_accumulatedWaveSimTime += kUpdatePeriod;
	if ((m_accumulatedWaveSimTime) >= 250.0f) {
		m_accumulatedWaveSimTime -= 250.0f;

		uint i = 5 + rand() % 90;
		uint j = 5 + rand() % 90;

		float r = Common::RandF(-1.0f, -2.0f);

		m_waveSimulator.Disturb(i, j, r);
	}

	m_waveSimulator.Update(kUpdatePeriod);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	ModelManager.MapDynamicVertexBuffer(0, *m_immediateContext, &mappedData);

	Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	for (UINT i = 0; i < m_waveSimulator.VertexCount(); ++i) {
		v[i].pos = m_waveSimulator[i];
		v[i].color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	ModelManager.UnMapDynamicVertexBuffer(0, *m_immediateContext);
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
	Common::Model<Vertex> model;

	// Iterate over each quad.
	uint m = m_waveSimulator.RowCount();
	uint n = m_waveSimulator.ColumnCount();

	model.Indices.resize(3 * m_waveSimulator.TriangleCount());

	int k = 0;
	for (uint i = 0; i < m - 1; ++i) {
		for (uint j = 0; j < n - 1; ++j) {
			model.Indices[k] = (i * n) + j;
			model.Indices[k + 1] = (i * n) + j + 1;
			model.Indices[k + 2] = ((i + 1) * n) + j;

			model.Indices[k + 3] = ((i + 1) * n) + j;
			model.Indices[k + 4] = (i * n) + j + 1;
			model.Indices[k + 5] = ((i + 1) * n) + j + 1;

			k += 6; // next quad
		}
	}

	ModelManager.AddModel(model, m_waveSimulator.VertexCount(), model.Indices.size(), false);

	ModelManager.InitStaticBuffers();
}

} // End of namespace CrateDemo
