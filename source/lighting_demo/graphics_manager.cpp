/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "lighting_demo/graphics_manager.h"

#include "common/d3d_util.h"

#include "lighting_demo/game_state_manager.h"

#include "assert.h"
#include <AntTweakBar.h>

namespace LightingDemo {

GraphicsManager::GraphicsManager(GameStateManager *gameStateManager)
	: Common::GraphicsManagerBase(),
	  m_gameStateManager(gameStateManager),
	  m_vsync(false),
	  m_renderTargetView(nullptr),
	  m_inputLayout(nullptr),
	  m_vertexShaderFrameConstantsBuffer(nullptr),
	  m_vertexShaderObjectConstantsBuffer(nullptr),
	  m_pixelShaderFrameConstantsBuffer(nullptr),
	  m_pixelShaderObjectConstantsBuffer(nullptr),
	  m_pointLightBuffer(nullptr),
	  m_spotLightBuffer(nullptr),
	  m_vertexShader(nullptr),
	  m_pixelShader(nullptr),
	  m_wireframeRS(nullptr) {
}

bool GraphicsManager::Initialize(int clientWidth, int clientHeight, HWND hwnd, bool fullscreen) {
	if (!Common::GraphicsManagerBase::Initialize(clientWidth, clientHeight, hwnd, fullscreen))
		return false;

	InitTweakBar();

	if (!m_gameStateManager->Initialize(hwnd, &m_device))
		return false;

	LoadShaders();
	
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(m_device->CreateRasterizerState(&wireframeDesc, &m_wireframeRS));

	return true;
}

void GraphicsManager::Shutdown() {
	// Release in the opposite order we initialized in
	ReleaseCOM(m_vertexShaderFrameConstantsBuffer);
	ReleaseCOM(m_vertexShaderObjectConstantsBuffer);
	ReleaseCOM(m_pixelShaderFrameConstantsBuffer);
	ReleaseCOM(m_pixelShaderObjectConstantsBuffer);
	delete m_pointLightBuffer;
	delete m_spotLightBuffer;
	ReleaseCOM(m_wireframeRS);
	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);
	ReleaseCOM(m_inputLayout);
	ReleaseCOM(m_renderTargetView);

	TwTerminate();

	Common::GraphicsManagerBase::Shutdown();
}

void GraphicsManager::DrawFrame(float deltaTime) {
	CalculateFrameStats(deltaTime);

	m_immediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	m_immediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Transpose the matrices to prepare them for the shader.
	DirectX::XMMATRIX worldMatrix = m_gameStateManager->WorldViewProj.world;
	DirectX::XMMATRIX viewMatrix = m_gameStateManager->WorldViewProj.view;
	DirectX::XMMATRIX projectionMatrix = m_gameStateManager->WorldViewProj.projection;

	// Cache the matrix multiplications
	DirectX::XMMATRIX viewProj = DirectX::XMMatrixTranspose(viewMatrix * projectionMatrix);
	DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(worldMatrix * viewMatrix * projectionMatrix);

	SetFrameConstants(DirectX::XMMatrixTranspose(projectionMatrix), viewProj);

	SetObjectConstants(DirectX::XMMatrixTranspose(worldMatrix), worldViewProjection, m_gameStateManager->Models[0].GetSubsetMaterial(0));
	SetLightBuffers(viewMatrix);

	m_immediateContext->IASetInputLayout(m_inputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_immediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_immediateContext->PSSetShader(m_pixelShader, NULL, 0);

	m_gameStateManager->Models[0].DrawSubset(m_immediateContext);

	TwDraw();

	if (m_vsync)
		m_swapChain->Present(1, 0);
	else
		m_swapChain->Present(0, 0);
}

void GraphicsManager::SetFrameConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &viewProjMatrix) {
	// Fill in frame constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_vertexShaderFrameConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	
	VertexShaderFrameConstants *vertexShaderFrameConstants = static_cast<VertexShaderFrameConstants *>(mappedResource.pData);
	vertexShaderFrameConstants->proj = projMatrix;
	vertexShaderFrameConstants->viewProj = viewProjMatrix;

	m_immediateContext->Unmap(m_vertexShaderFrameConstantsBuffer, 0);
	m_immediateContext->VSSetConstantBuffers(0, 1, &m_vertexShaderFrameConstantsBuffer);
}

void GraphicsManager::SetObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix, const Common::Material &material) {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_vertexShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	VertexShaderObjectConstants *vertexShaderObjectConstants = static_cast<VertexShaderObjectConstants *>(mappedResource.pData);
	vertexShaderObjectConstants->world = worldMatrix;
	vertexShaderObjectConstants->worldViewProj = worldViewProjMatrix;

	m_immediateContext->Unmap(m_vertexShaderObjectConstantsBuffer, 0);
	m_immediateContext->VSSetConstantBuffers(1, 1, &m_vertexShaderObjectConstantsBuffer);

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_pixelShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PixelShaderObjectConstants *pixelShaderObjectConstants = static_cast<PixelShaderObjectConstants *>(mappedResource.pData);
	pixelShaderObjectConstants->material = material;

	m_immediateContext->Unmap(m_pixelShaderObjectConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(3, 1, &m_pixelShaderObjectConstantsBuffer);


	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_pixelShaderFrameConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PixelShaderFrameConstants *pixelShaderFrameConstants = static_cast<PixelShaderFrameConstants *>(mappedResource.pData);
	pixelShaderFrameConstants->directionalLight = *(m_gameStateManager->LightManager.GetDirectionalLight());
	pixelShaderFrameConstants->eyePosition = m_gameStateManager->GetCameraPosition();

	m_immediateContext->Unmap(m_pixelShaderFrameConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(2, 1, &m_pixelShaderFrameConstantsBuffer);
}

void GraphicsManager::SetLightBuffers(DirectX::XMMATRIX &viewMatrix) {
	//{
	//	Common::PointLight* light = m_pointLightBuffer->MapDiscard(m_immediateContext);
	//	for (unsigned int i = 0; i < mActiveLights; ++i) {
	//		light[i] = mPointLightParameters[i];
	//	}
	//	mLightBuffer->Unmap(d3dDeviceContext);
	//}
}

void GraphicsManager::OnResize(int newClientWidth, int newClientHeight) {
	if (!m_d3dInitialized) {
		return;
	}

	// Release the render target view
	ReleaseCOM(m_renderTargetView);

	Common::GraphicsManagerBase::OnResize(newClientWidth, newClientHeight);

	// Recreate the render target view.
	ID3D11Texture2D* backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_device->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
	ReleaseCOM(backBuffer);

	// Bind the render target view and depth/stencil view to the pipeline.
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
}

void GraphicsManager::InitTweakBar() {
	int success = TwInit(TW_DIRECT3D11, m_device);

	m_tweakBar = TwNewBar("RootMenu");
	TwAddVarRO(m_tweakBar, "FPS", TwType::TW_TYPE_UINT32, &m_fps, "");
	TwAddVarRO(m_tweakBar, "Frame Time (ms)", TwType::TW_TYPE_FLOAT, &m_frameTime, "");
	TwAddVarRW(m_tweakBar, "V-Sync", TwType::TW_TYPE_BOOLCPP, &m_vsync, "");
	TwDefine(" RootMenu movable=false resizable=false fontresizable=false contained=true ");
}

void GraphicsManager::LoadShaders() {
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(Common::LoadVertexShader("vertex_shader.cso", m_device, vertexDesc, 2, &m_vertexShader, &m_inputLayout));
	HR(Common::LoadPixelShader("pixel_shader.cso", m_device, &m_pixelShader));

	// Create the constant shader buffers
	D3D11_BUFFER_DESC vertexShaderFrameBufferDesc;
	vertexShaderFrameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexShaderFrameBufferDesc.ByteWidth = sizeof(VertexShaderFrameConstants);
	vertexShaderFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vertexShaderFrameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexShaderFrameBufferDesc.MiscFlags = 0;
	vertexShaderFrameBufferDesc.StructureByteStride = 0;
	
	m_device->CreateBuffer(&vertexShaderFrameBufferDesc, NULL, &m_vertexShaderFrameConstantsBuffer);

	D3D11_BUFFER_DESC vertexShaderObjectBufferDesc;
	vertexShaderObjectBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexShaderObjectBufferDesc.ByteWidth = sizeof(VertexShaderObjectConstants);
	vertexShaderObjectBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vertexShaderObjectBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexShaderObjectBufferDesc.MiscFlags = 0;
	vertexShaderObjectBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&vertexShaderObjectBufferDesc, NULL, &m_vertexShaderObjectConstantsBuffer);

	D3D11_BUFFER_DESC pixelShaderFrameBufferDesc;
	pixelShaderFrameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelShaderFrameBufferDesc.ByteWidth = sizeof(PixelShaderFrameConstants);
	pixelShaderFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelShaderFrameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelShaderFrameBufferDesc.MiscFlags = 0;
	pixelShaderFrameBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&pixelShaderFrameBufferDesc, NULL, &m_pixelShaderFrameConstantsBuffer);

	D3D11_BUFFER_DESC pixelShaderObjectBufferDesc;
	pixelShaderObjectBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelShaderObjectBufferDesc.ByteWidth = sizeof(PixelShaderObjectConstants);
	pixelShaderObjectBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelShaderObjectBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelShaderObjectBufferDesc.MiscFlags = 0;
	pixelShaderObjectBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&pixelShaderObjectBufferDesc, NULL, &m_pixelShaderObjectConstantsBuffer);

	m_pointLightBuffer = new Common::StructuredBuffer<Common::PointLight>(m_device, 1, D3D11_BIND_SHADER_RESOURCE, true);
	m_spotLightBuffer = new Common::StructuredBuffer<Common::SpotLight>(m_device, 1, D3D11_BIND_SHADER_RESOURCE, true);
}

} // End of namespace CrateDemo
