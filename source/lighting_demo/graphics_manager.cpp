/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "lighting_demo/graphics_manager.h"

#include "common/d3d_util.h"

#include "game_state_manager.h"

#include "assert.h"

namespace LightingDemo {

GraphicsManager::GraphicsManager(GameStateManager *gameStateManager)
	: Common::GraphicsManagerBase(),
	  m_gameStateManager(gameStateManager),
	  m_renderTargetView(nullptr),
	  m_inputLayout(nullptr),
	  m_matrixBuffer(nullptr),
	  m_vertexShader(nullptr),
	  m_pixelShader(nullptr),
	  m_wireframeRS(nullptr) {
}

bool GraphicsManager::Initialize(int clientWidth, int clientHeight, HWND hwnd, bool fullscreen) {
	if (!Common::GraphicsManagerBase::Initialize(clientWidth, clientHeight, hwnd, fullscreen))
		return false;

	LoadShaders();

	if (!m_gameStateManager->Initialize(hwnd, &m_device))
		return false;
	
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
	ReleaseCOM(m_matrixBuffer);
	ReleaseCOM(m_wireframeRS);
	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);
	ReleaseCOM(m_inputLayout);
	ReleaseCOM(m_renderTargetView);

	Common::GraphicsManagerBase::Shutdown();
}

void GraphicsManager::DrawFrame() {
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	m_immediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	SetWorldViewProj();

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_immediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_immediateContext->PSSetShader(m_pixelShader, NULL, 0);

	m_gameStateManager->Models[0].DrawSubset(m_immediateContext);

	m_swapChain->Present(0, 0);
}

void GraphicsManager::SetWorldViewProj() {
	// Write the wold, view, projection matrices to the constant shader buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(m_gameStateManager->WorldViewProj.world);
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixTranspose(m_gameStateManager->WorldViewProj.view);
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixTranspose(m_gameStateManager->WorldViewProj.projection);

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	m_immediateContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finally, set the constant buffer in the vertex shader with the updated values.
	m_immediateContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
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

void GraphicsManager::LoadShaders() {
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(Common::LoadVertexShader("vertex_shader.cso", m_device, vertexDesc, 2, &m_vertexShader, &m_inputLayout));
	HR(Common::LoadPixelShader("pixel_shader.cso", m_device, &m_pixelShader));

	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer
	m_device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
}

} // End of namespace CrateDemo
