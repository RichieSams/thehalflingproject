/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_loader_demo/obj_loader_demo.h"


namespace ObjLoaderDemo {

LRESULT ObjLoaderDemo::MsgProc(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam) {
	// Send event message to AntTweakBar
	if (TwEventWin(hwnd, msg, wParam, lParam)) {
		return 0;    // Event has been handled by AntTweakBar
	}

	// Let the base engine handle any events that we don't handle
	return Halfling::HalflingEngine::MsgProc(hwnd, msg, wParam, lParam);
}

ObjLoaderDemo::ObjLoaderDemo(HINSTANCE hinstance)
	: Halfling::HalflingEngine(hinstance),
	  m_nearClip(0.1f),
	  m_farClip(5000.0f),
	  m_globalWorldTransform(DirectX::XMMatrixIdentity()),
	  m_camera(0.5f * DirectX::XM_PI, 0.45f * DirectX::XM_PI, 100.0f),
	  m_showConsole(false),
	  m_sceneLoaded(false),
	  m_sceneIsSetup(false),
	  m_sceneScaleFactor(0.0f),
	  m_pointLightBufferNeedsRebuild(false),
	  m_spotLightBufferNeedsRebuild(false),
	  m_vsync(false),
	  m_wireframe(false),
	  m_animateLights(true),
	  m_shadingType(ShadingType::NoCullDeferred),
	  m_gbufferSelector(GBufferSelector::None),
	  m_showLightLocations(false),
	  m_showGBuffers(false),
	  m_numPointLightsToDraw(100),
	  m_numSpotLightsToDraw(100),
	  m_renderTargetView(nullptr),
	  m_depthStencilBuffer(nullptr),
	  m_gBufferInputLayout(nullptr),
	  m_debugObjectInputLayout(nullptr),
	  m_pointLightBuffer(nullptr),
	  m_spotLightBuffer(nullptr),
	  m_gbufferVertexShader(nullptr),
	  m_gbufferPixelShader(nullptr),
	  m_fullscreenTriangleVertexShader(nullptr),
	  m_noCullFinalGatherPixelShader(nullptr),
	  m_debugObjectVertexShader(nullptr),
	  m_debugObjectPixelShader(nullptr),
	  m_transformedFullscreenTriangleVertexShader(nullptr),
	  m_renderGbuffersPixelShader(nullptr) {
}

void ObjLoaderDemo::Shutdown() {
	// Release in the opposite order we initialized in
	delete m_pointLightBuffer;
	delete m_spotLightBuffer;
	delete m_frameMaterialListBuffer;
	delete(m_forwardVertexShader);
	delete(m_forwardPixelShader);
	delete(m_gbufferVertexShader);
	delete(m_gbufferPixelShader);
	delete(m_fullscreenTriangleVertexShader);
	delete(m_noCullFinalGatherPixelShader);
	delete(m_debugObjectVertexShader);
	delete(m_debugObjectPixelShader);
	delete(m_transformedFullscreenTriangleVertexShader);
	delete(m_renderGbuffersPixelShader);
	ReleaseCOM(m_gBufferInputLayout);
	ReleaseCOM(m_debugObjectInputLayout);

	for (auto iter = m_pointLightAnimators.begin(); iter != m_pointLightAnimators.end(); ++iter) {
		delete *iter;
	}
	for (auto iter = m_spotLightAnimators.begin(); iter != m_spotLightAnimators.end(); ++iter) {
		delete *iter;
	}
	for (auto iter = m_gBuffers.begin(); iter != m_gBuffers.end(); ++iter) {
		delete *iter;
	}

	delete m_depthStencilBuffer;
	ReleaseCOM(m_renderTargetView);

	if (m_sceneLoaderThread.joinable()) {
		m_sceneLoaderThread.detach();
	}

	TwTerminate();

	Halfling::HalflingEngine::Shutdown();
}

void ObjLoaderDemo::OnResize() {
	if (!m_d3dInitialized) {
		return;
	}

	// Update the projection matrix
	// We swap near and far clip because we are using 1 - depth in our depth buffer
	m_camera.UpdateProjectionMatrix((float)m_clientWidth, (float)m_clientHeight, m_farClip, m_nearClip);

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
	ID3D11Texture2D *backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backBuffer)));
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
	// Diffuse Albedo
	m_gBuffers.push_back(new Common::Texture2D(m_device, m_clientWidth, m_clientHeight,
		DXGI_FORMAT_R11G11B10_FLOAT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		sampleDesc));

	// Specular Albedo and Spec Power
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

void ObjLoaderDemo::MouseDown(WPARAM buttonState, int x, int y) {
	m_mouseLastPos.x = x;
	m_mouseLastPos.y = y;

	SetCapture(m_hwnd);
}

void ObjLoaderDemo::MouseUp(WPARAM buttonState, int x, int y) {
	ReleaseCapture();
}

void ObjLoaderDemo::MouseMove(WPARAM buttonState, int x, int y) {
	if ((buttonState & MK_LBUTTON) != 0) {
		if (GetKeyState(VK_MENU) & 0x8000) {
			// Calculate the new phi and theta based on mouse position relative to where the user clicked
			float dPhi = ((float)(m_mouseLastPos.y - y) / 300);
			float dTheta = ((float)(m_mouseLastPos.x - x) / 300);

			m_camera.Rotate(-dTheta, dPhi);
		}
	} else if ((buttonState & MK_MBUTTON) != 0) {
		if (GetKeyState(VK_MENU) & 0x8000) {
			float dx = ((float)(m_mouseLastPos.x - x) / 10);
			float dy = ((float)(m_mouseLastPos.y - y) / 10);

			m_camera.Pan(-dx, dy);
		}
	}

	m_mouseLastPos.x = x;
	m_mouseLastPos.y = y;
}

void ObjLoaderDemo::MouseWheel(int zDelta) {
	// Make each wheel dedent correspond to a size based on the scene
	m_camera.Zoom(0.1f * (float)zDelta);
}

void ObjLoaderDemo::CharacterInput(wchar character) {
	if (m_showConsole) {
		m_console.InputCharacter(character);
	}
}

} // End of namespace ObjLoaderDemo
