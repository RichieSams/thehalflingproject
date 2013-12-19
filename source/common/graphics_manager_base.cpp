/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/graphics_manager_base.h"

#include "common/d3d_util.h"


namespace Common {

GraphicsManagerBase::GraphicsManagerBase() 
	: m_device(nullptr),
	  m_immediateContext(nullptr),
	  m_swapChain(nullptr),
	  m_depthStencilBuffer(nullptr),
	  m_depthStencilView(nullptr),
	  m_d3dInitialized(false),
	  m_enable4xMSAA(true) {
}

bool GraphicsManagerBase::Initialize(int clientWidth, int clientHeight, HWND hwnd) {
	m_clientWidth = clientWidth;
	m_clientHeight = clientHeight;

	// Create the device and device context.
	UINT createDeviceFlags = 0;
	#if defined(DEBUG) || defined(_DEBUG)  
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
	
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(NULL,
								   D3D_DRIVER_TYPE_HARDWARE,
								   NULL,
								   createDeviceFlags,
								   NULL, 0,
								   D3D11_SDK_VERSION,
								   &m_device,
								   &featureLevel,
								   &m_immediateContext);

	

	// Describe the swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_clientWidth;
	sd.BufferDesc.Height = m_clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if (FAILED(hr)) {
		DXTRACE_ERR_MSGBOX(L"D3D11CreateDevice Failed.", hr);
		return false;
	}

	HR(m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMSAAQuality));
	assert(m_4xMSAAQuality > 0);

	// Use 4X MSAA? 
	if (m_enable4xMSAA) {
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMSAAQuality - 1;
	} else {
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hwnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	
	IDXGIDevice* dxgiDevice;
	HR(m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**) &dxgiAdapter));

	IDXGIFactory* dxgiFactory;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain((IUnknown *)m_device, &sd, &m_swapChain));
	
	// Cleanup
	ReleaseCOM(dxgiFactory);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiDevice);

	m_d3dInitialized = true;

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
	OnResize(clientWidth, clientHeight);

	return true;
}

void GraphicsManagerBase::Shutdown() {
	ReleaseCOM(m_depthStencilView);
	ReleaseCOM(m_swapChain);
	ReleaseCOM(m_depthStencilBuffer);

	// Restore all default settings.
	if (m_immediateContext)
		m_immediateContext->ClearState();

	ReleaseCOM(m_immediateContext);
	ReleaseCOM(m_device);
}

void GraphicsManagerBase::OnResize(int newClientWidth, int newClientHeight) {
	m_clientWidth = newClientWidth;
	m_clientHeight = newClientHeight;

	// Release the old views and the old depth/stencil buffer.
	ReleaseCOM(m_depthStencilView);
	ReleaseCOM(m_depthStencilBuffer);

	// Resize the swap chain and recreate the render target view.
	HR(m_swapChain->ResizeBuffers(1, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_clientWidth;
	depthStencilDesc.Height = m_clientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (m_enable4xMSAA) {
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMSAAQuality - 1;
	} else {
	// No MSAA
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_device->CreateTexture2D(&depthStencilDesc, 0, &m_depthStencilBuffer));
	HR(m_device->CreateDepthStencilView(m_depthStencilBuffer, 0, &m_depthStencilView));

	// Set the viewport transform.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(m_clientWidth);
	m_screenViewport.Height = static_cast<float>(m_clientHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_immediateContext->RSSetViewports(1, &m_screenViewport);
}

} // End of namespace Common
