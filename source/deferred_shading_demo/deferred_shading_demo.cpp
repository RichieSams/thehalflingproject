/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "deferred_shading_demo/deferred_shading_demo.h"


namespace DeferredShadingDemo {

LRESULT DeferredShadingDemo::MsgProc(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam) {
	// Send event message to AntTweakBar
	if (TwEventWin(hwnd, msg, wParam, lParam))
		return 0; // Event has been handled by AntTweakBar

	// Let the base engine handle any events that we don't handle
	return Halfling::HalflingEngine::MsgProc(hwnd, msg, wParam, lParam);
}

DeferredShadingDemo::DeferredShadingDemo(HINSTANCE hinstance)
	: Halfling::HalflingEngine(hinstance),
	  m_camera(1.5f * DirectX::XM_PI, 0.25f * DirectX::XM_PI, 50.0f),
	  m_pointLightBufferNeedsRebuild(false),
	  m_spotLightBufferNeedsRebuild(false),
	  m_vsync(false),
	  m_wireframe(false),
	  m_renderTargetView(nullptr),
	  m_gBufferInputLayout(nullptr),
	  m_fullscreenTriangleInputLayout(nullptr),
	  m_gBufferVertexShaderObjectConstantsBuffer(nullptr),
	  m_gBufferPixelShaderObjectConstantsBuffer(nullptr),
	  m_noCullFinalGatherPixelShaderConstantsBuffer(nullptr),
	  m_pointLightBuffer(nullptr),
	  m_spotLightBuffer(nullptr),
	  m_gbufferVertexShader(nullptr),
	  m_gbufferPixelShader(nullptr),
	  m_fullscreenTriangleVertexShader(nullptr),
	  m_noCullFinalGatherPixelShader(nullptr),
	  m_diffuseSampleState(nullptr),
	  m_wireframeRS(nullptr),
	  m_solidRS(nullptr) {
}

void DeferredShadingDemo::Shutdown() {
	// Release in the opposite order we initialized in
	ReleaseCOM(m_gBufferVertexShaderObjectConstantsBuffer);
	ReleaseCOM(m_gBufferPixelShaderObjectConstantsBuffer);
	ReleaseCOM(m_noCullFinalGatherPixelShaderConstantsBuffer);
	delete m_pointLightBuffer;
	delete m_spotLightBuffer;
	ReleaseCOM(m_diffuseSampleState);
	ReleaseCOM(m_wireframeRS);
	ReleaseCOM(m_solidRS);
	ReleaseCOM(m_gbufferVertexShader);
	ReleaseCOM(m_gbufferPixelShader);
	ReleaseCOM(m_fullscreenTriangleVertexShader);
	ReleaseCOM(m_noCullFinalGatherPixelShader);
	ReleaseCOM(m_gBufferInputLayout);
	ReleaseCOM(m_fullscreenTriangleInputLayout);
	ReleaseCOM(m_renderTargetView);

	TwTerminate();

	Halfling::HalflingEngine::Shutdown();
}

void DeferredShadingDemo::OnResize() {
	if (!m_d3dInitialized) {
		return;
	}

	// Update the aspect ratio and the projection matrix
	m_worldViewProj.projection = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, float(m_clientWidth) / m_clientHeight, 1000.0f, 1.0f);

	// Release the gBuffers
	for (auto gbuffer : m_gBuffers) {
		delete gbuffer;
	}
	m_gBuffers.clear();
	m_gBufferRTVs.clear();
	m_gBufferSRVs.clear();

	// Release the old views and the old depth/stencil buffer.
	ReleaseCOM(m_renderTargetView);
	delete m_depthStencilBuffer;
	m_depthStencilBuffer = nullptr;

	// Resize the swap chain and recreate the render target view.
	HR(m_swapChain->ResizeBuffers(2, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	// Recreate the render target view.
	ID3D11Texture2D* backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_device->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
	ReleaseCOM(backBuffer);

	// Create the depth/stencil buffer and view.
	DXGI_SAMPLE_DESC sampleDesc;

	// Use MSAA? 
	if (m_msaaCount > 1) {
		uint msaaQuality;
		HR(m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, m_msaaCount, &msaaQuality));
		assert(msaaQuality > 0);

		sampleDesc.Count = m_msaaCount;
		sampleDesc.Quality = msaaQuality - 1;
	} else {
		sampleDesc.Count = 1;
		sampleDesc.Quality = 0;
	}

	m_depthStencilBuffer = new Common::Depth2D(m_device, m_clientWidth, m_clientHeight,
											   D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
											   sampleDesc, m_stencil);

	// Create the gBuffers
	// Albedo and Material id
	m_gBuffers.push_back(new Common::Texture2D(m_device, m_clientWidth, m_clientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		sampleDesc));

	// Normal
	m_gBuffers.push_back(new Common::Texture2D(m_device, m_clientWidth, m_clientHeight,
		DXGI_FORMAT_R16G16_FLOAT,
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

void DeferredShadingDemo::MouseDown(WPARAM buttonState, int x, int y) {
	m_mouseLastPos.x = x;
	m_mouseLastPos.y = y;

	SetCapture(m_hwnd);
}

void DeferredShadingDemo::MouseUp(WPARAM buttonState, int x, int y) {
	ReleaseCapture();
}

void DeferredShadingDemo::MouseMove(WPARAM buttonState, int x, int y) {
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

void DeferredShadingDemo::MouseWheel(int zDelta) {
	// Make each wheel dedent correspond to 0.01 units
	m_camera.MoveCamera(0.0f, 0.0f, -0.01f * (float)zDelta);
}

void TW_CALL DeferredShadingDemo::SetWireframeRSCallback(const void *value, void *clientData) {
	DeferredShadingDemo *graphicsManager = ((DeferredShadingDemo *)clientData);
	graphicsManager->m_wireframe = *((bool *)value);

	if (graphicsManager->m_wireframe) {
		graphicsManager->m_immediateContext->RSSetState(graphicsManager->m_wireframeRS);
	} else {
		graphicsManager->m_immediateContext->RSSetState(graphicsManager->m_solidRS);
	}
}

void TW_CALL DeferredShadingDemo::GetWireframeTSCallback(void *value, void *clientData) {
	*((bool *)value) = ((DeferredShadingDemo *)clientData)->m_wireframe;
}

} // End of namespace DeferredShadingDemo
