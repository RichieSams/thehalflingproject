/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "deferred_shading_demo/game_state_manager.h"

#include "common/math.h"
#include "common/camera.h"
#include "common/d3d_util.h"
#include "common/geometry_generator.h"


namespace DeferredShadingDemo {

GameStateManager::GameStateManager() 
	: GameStateManagerBase(),
	  m_camera(1.5f * DirectX::XM_PI, 0.25f * DirectX::XM_PI, 50.0f) {
}

bool GameStateManager::Initialize(HWND hwnd, ID3D11Device **device) {
	GameStateManagerBase::Initialize(hwnd, device);

	BuildGeometryBuffers();
	CreateLights();

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
	// Make each wheel dedent correspond to 0.01 units
	m_camera.MoveCamera(0.0f, 0.0f, -0.01f * (float)zDelta);
}

void GameStateManager::BuildGeometryBuffers() {
	Models.push_back(Common::Model<Vertex>());
	Common::Model<Vertex> *model = &Models.back();

	Common::GeometryGenerator::MeshData meshData;
	Common::GeometryGenerator::CreateGrid(160.0f, 160.0f, 50, 50, &meshData);

	uint vertexCount = meshData.Vertices.size();
	uint indexCount = meshData.Indices.size();

	Vertex *vertices = new Vertex[vertexCount];
	for (uint i = 0; i < vertexCount; ++i) {
		vertices[i].pos = meshData.Vertices[i].Position;
		vertices[i].pos.y = GetHillHeight(vertices[i].pos.x, vertices[i].pos.z);
		vertices[i].normal = GetHillNormal(vertices[i].pos.x, vertices[i].pos.z);
	}
	model->SetVertices(*m_device, vertices, vertexCount);

	uint *indices = new uint[indexCount];
	for (uint i = 0; i < indexCount; ++i) {
		indices[i] = meshData.Indices[i];
	}
	model->SetIndices(*m_device, indices, indexCount);

	// Create subsets
	Common::ModelSubset *subsets = new Common::ModelSubset[1] {
		{0, vertexCount, 0, indexCount / 3, {DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f),
		                                     DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f),
		                                     DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f)},
											 m_textureManager.GetSRVFromDDSFile(*m_device, "grass.dds", D3D11_USAGE_IMMUTABLE)
		}
	};
	model->SetSubsets(subsets, 1);
}

void GameStateManager::CreateLights() {
	Common::DirectionalLight *directionalLight = LightManager.GetDirectionalLight();
	directionalLight->Ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	directionalLight->Diffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	directionalLight->Specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	directionalLight->Direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
}

float GameStateManager::GetHillHeight(float x, float z) const {
	return 0.3f * ((z * sinf(0.1f * x)) + (x * cosf(0.1f * z)));
}

DirectX::XMFLOAT3 GameStateManager::GetHillNormal(float x, float z) const {
	DirectX::XMFLOAT3 normal(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	DirectX::XMVECTOR unitNormal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal));
	DirectX::XMStoreFloat3(&normal, unitNormal);

	return normal;
}

} // End of namespace DeferredShadingDemo
