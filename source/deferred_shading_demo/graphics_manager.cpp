/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "deferred_shading_demo/graphics_manager.h"

#include "common/d3d_util.h"

#include "deferred_shading_demo/game_state_manager.h"

#include "assert.h"
#include <AntTweakBar.h>
#include <DirectXColors.h>
#include <sstream>


namespace DeferredShadingDemo {

GraphicsManager::GraphicsManager(GameStateManager *gameStateManager)
	: Common::GraphicsManagerBase(),
	  m_gameStateManager(gameStateManager),
	  m_vsync(false),
	  m_wireframe(false),
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
	  m_diffuseSampleState(nullptr),
	  m_wireframeRS(nullptr),
	  m_solidRS(nullptr) {
}

bool GraphicsManager::Initialize(int clientWidth, int clientHeight, HWND hwnd, bool fullscreen) {
	m_enable4xMSAA = false;
	
	if (!Common::GraphicsManagerBase::Initialize(clientWidth, clientHeight, hwnd, fullscreen))
		return false;

	InitTweakBar();

	if (!m_gameStateManager->Initialize(hwnd, &m_device))
		return false;

	LoadShaders();
	
	m_spriteRenderer.Initialize(m_device);
	m_timesNewRoman12Font.Initialize(L"Times New Roman", 12, Common::SpriteFont::Regular, true, m_device);

	m_blendStates.Initialize(m_device);
	m_depthStencilStates.Initialize(m_device);
	m_rasterizerStates.Initialize(m_device);
	m_samplerStates.Initialize(m_device);

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
	ReleaseCOM(m_diffuseSampleState);
	ReleaseCOM(m_wireframeRS);
	ReleaseCOM(m_solidRS);
	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);
	ReleaseCOM(m_inputLayout);
	ReleaseCOM(m_renderTargetView);

	TwTerminate();

	Common::GraphicsManagerBase::Shutdown();
}

void GraphicsManager::DrawFrame(float deltaTime) {
	CalculateFrameStats(deltaTime);

	RenderMainPass();
	RenderHUD();

	uint syncInterval = m_vsync ? 1 : 0;
	m_swapChain->Present(syncInterval, 0);
}

void GraphicsManager::RenderMainPass() {
	// Bind the gbufferRTVs and depth/stencil view to the pipeline.
	m_immediateContext->OMSetRenderTargets(2, &m_gBufferRTVs[0], m_depthStencilBuffer->GetDepthStencil());

	m_immediateContext->ClearRenderTargetView(m_renderTargetView, DirectX::Colors::LightGray);
	for (auto gbufferRTV : m_gBufferRTVs) {
		m_immediateContext->ClearRenderTargetView(gbufferRTV, DirectX::Colors::Black);
	}
	m_immediateContext->ClearDepthStencilView(m_depthStencilBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set States
	m_immediateContext->PSSetSamplers(0, 1, &m_diffuseSampleState);
	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.StencilTestEnabled(), 0);
	m_immediateContext->RSSetState(m_rasterizerStates.BackFaceCull());
	

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
}

void GraphicsManager::RenderHUD() {
	m_spriteRenderer.Begin(m_immediateContext, Common::SpriteRenderer::Point);
	std::wostringstream stream;
	stream << L"FPS: " << m_fps << L"\nFrame Time: " << m_frameTime << L" (ms)";

	DirectX::XMFLOAT4X4 transform{1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		25, 25, 0, 1};
	m_spriteRenderer.RenderText(m_timesNewRoman12Font, stream.str().c_str(), transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);
	m_spriteRenderer.End();

	TwDraw();
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

	m_clientWidth = newClientWidth;
	m_clientHeight = newClientHeight;

	// Release the gBuffers
	for (auto gbuffer : m_gBuffers) {
		delete gbuffer;
	}
	m_gBuffers.clear();
	m_gBufferRTVs.clear();
	m_gBufferSRVs.clear();

	// Release the old views and the old depth/stencil buffer.
	delete m_depthStencilBuffer;

	// Resize the swap chain and recreate the render target view.
	HR(m_swapChain->ResizeBuffers(1, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	// Recreate the render target view.
	ID3D11Texture2D* backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_device->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
	ReleaseCOM(backBuffer);

	// Create the depth/stencil buffer and view.
	DXGI_SAMPLE_DESC sampleDesc;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (m_enable4xMSAA) {
		sampleDesc.Count = 4;
		sampleDesc.Quality = m_4xMSAAQuality - 1;
	} else {
		// No MSAA
		sampleDesc.Count = 1;
		sampleDesc.Quality = 0;
	}

	m_depthStencilBuffer = new Common::Depth2D(m_device, newClientWidth, newClientHeight,
	                                           D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 
											   sampleDesc, m_enable4xMSAA);

	// Albedo and Specular Power
	m_gBuffers.push_back(new Common::Texture2D(m_device, newClientWidth, newClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		sampleDesc));

	// Create the gBuffers
	// Normal and Specular Intensity
	m_gBuffers.push_back(new Common::Texture2D(m_device, newClientWidth, newClientHeight, 
	                                           DXGI_FORMAT_R11G11B10_FLOAT,
	                                           D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
	                                           sampleDesc));

	// Set up GBuffer resource list
	size_t size = m_gBuffers.size();
	m_gBufferRTVs.resize(size, 0);
	m_gBufferSRVs.resize(size + 1, 0);
	for (std::size_t i = 0; i < size; ++i) {
		m_gBufferRTVs[i] = m_gBuffers[i]->GetRenderTarget();
		m_gBufferSRVs[i] = m_gBuffers[i]->GetShaderResource();
	}

	// Add the depth buffer as a SRV for the Compute Shader
	m_gBufferSRVs.back() = m_depthStencilBuffer->GetShaderResource();

	// Set the viewport transform.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(m_clientWidth);
	m_screenViewport.Height = static_cast<float>(m_clientHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_immediateContext->RSSetViewports(1, &m_screenViewport);
}

void GraphicsManager::InitTweakBar() {
	int success = TwInit(TW_DIRECT3D11, m_device);

	m_settingsBar = TwNewBar("settings");
	TwDefine(" settings label='Settings' size='200 300' movable=true resizable=false fontresizable=false contained=true iconified=true ");

	TwAddVarRW(m_settingsBar, "V-Sync", TwType::TW_TYPE_BOOLCPP, &m_vsync, "");
	TwAddVarCB(m_settingsBar, "Wireframe", TwType::TW_TYPE_BOOLCPP, GraphicsManager::SetWireframeRSCallback, GraphicsManager::GetWireframeTSCallback, this, "");
}

void GraphicsManager::LoadShaders() {
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(Common::LoadVertexShader("vertex_shader.cso", m_device, &m_vertexShader, &m_inputLayout, vertexDesc, 3));
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

	D3D11_SAMPLER_DESC diffuseSamplerDesc;
	memset(&diffuseSamplerDesc, 0, sizeof(D3D11_SAMPLER_DESC));
	diffuseSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	diffuseSamplerDesc.MaxAnisotropy = 4;
	diffuseSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	diffuseSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	diffuseSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	HR(m_device->CreateSamplerState(&diffuseSamplerDesc, &m_diffuseSampleState));
}

void TW_CALL GraphicsManager::SetWireframeRSCallback(const void *value, void *clientData) {
	GraphicsManager *graphicsManager = ((GraphicsManager *)clientData);
	graphicsManager->m_wireframe = *((bool *)value);

	if (graphicsManager->m_wireframe) {
		graphicsManager->m_immediateContext->RSSetState(graphicsManager->m_wireframeRS);
	} else {
		graphicsManager->m_immediateContext->RSSetState(graphicsManager->m_solidRS);
	}
}

void TW_CALL GraphicsManager::GetWireframeTSCallback(void *value, void *clientData) {
	*((bool *)value) = ((GraphicsManager *)clientData)->m_wireframe;
}

} // End of namespace DeferredShadingDemo
