/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "crate_demo/graphics_manager.h"

#include "common/d3d_util.h"

#include "game_state_manager.h"

#include "assert.h"

namespace CrateDemo {

GraphicsManager::GraphicsManager(GameStateManager *gameStateManager)
	: Common::GraphicsManagerBase(),
	  m_gameStateManager(gameStateManager),
	  m_renderTargetView(nullptr),
	  m_inputLayout(nullptr),
	  m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr),
	  m_vertexShader(nullptr),
	  m_pixelShader(nullptr) {
}

bool CrateDemo::GraphicsManager::Initialize(int clientWidth, int clientHeight, HWND hwnd) {
	if (!Common::GraphicsManagerBase::Initialize(clientWidth, clientHeight, hwnd))
		return false;

	LoadShaders();
	BuildGeometryBuffers();

	return true;
}

void GraphicsManager::Shutdown() {
	// Release in the opposite order we initialized in
	ReleaseCOM(m_matrixBuffer);
	ReleaseCOM(m_vertexBuffer);
	ReleaseCOM(m_indexBuffer);
	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);
	ReleaseCOM(m_inputLayout);
	ReleaseCOM(m_renderTargetView);

	Common::GraphicsManagerBase::Shutdown();
}

void GraphicsManager::DrawFrame() {
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	m_immediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

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

	m_immediateContext->IASetInputLayout(m_inputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	uint stride = sizeof(Vertex);
	uint offset = 0;
	m_immediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_immediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_immediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_immediateContext->PSSetShader(m_pixelShader, NULL, 0);

	// Render the triangle.
	m_immediateContext->DrawIndexed(36, 0, 0);

	m_swapChain->Present(0, 0);
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
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Common::LoadVertexShader("vertex_shader.cso", m_device, vertexDesc, 2, &m_vertexShader, &m_inputLayout);
	Common::LoadPixelShader("pixel_shader.cso", m_device, &m_pixelShader);

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

void GraphicsManager::BuildGeometryBuffers() {
	// Create the vertex buffer
	Vertex verticies[] = {
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), Colors::White },
		{ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), Colors::Black },
		{ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), Colors::Red },
		{ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), Colors::Green },
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), Colors::Blue },
		{ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), Colors::Yellow },
		{ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), Colors::Cyan },
		{ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), Colors::Magenta },
	};

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = verticies;

	HR(m_device->CreateBuffer(&vbd, &vInitData, &m_vertexBuffer));

	// Create the index buffer
	uint indicies[] = {
		// Front face
		0, 1, 2,
		0, 2, 3,
		// Back face
		4, 6, 5,
		4, 7, 6,
		// Left face
		4, 5, 1,
		4, 1, 0,
		// Right face
		3, 2, 6,
		3, 6, 7,
		// Top face
		1, 5, 6,
		1, 6, 2,
		// Bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(uint) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = indicies;

	HR(m_device->CreateBuffer(&ibd, &iInitData, &m_indexBuffer));
}

} // End of namespace CrateDemo
