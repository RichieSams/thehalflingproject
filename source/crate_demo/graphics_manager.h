/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef CRATE_DEMO_GRAPHICS_MANAGER_H
#define CRATE_DEMO_GRAPHICS_MANAGER_H

#include "common/graphics_manager_interface.h"

#include <d3d11.h>

namespace CrateDemo {

class GraphicsManager : public Common::IGraphicsManager {
public:
	GraphicsManager();

private:
	ID3D11Device *m_d3dDevice;
	ID3D11DeviceContext *m_d3dImmediateContext;
	IDXGISwapChain *m_swapChain;
	ID3D11Texture2D *m_depthStencilBuffer;
	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11DepthStencilView *m_depthStencilView;
	D3D11_VIEWPORT m_screenViewport;

	// Screen dimensions
	int m_clientWidth;
	int m_clientHeight;

	// MSAA
	bool m_enable4xMSAA;
	uint m_4xMSAAQuality;

	bool m_d3dInitialized;

public:
	bool Initialize(int clientWidth, int clientHeight, HWND hwnd);
	void Shutdown();
	void DrawFrame();
	void OnResize(int newClientWidth, int newClientHeight);
	void GamePaused();
	void GameUnpaused();
};

} // End of namespace CrateDemo

#endif
