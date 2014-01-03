/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef HALFLING_GRAPHICS_HANDLER_H
#define HALFLING_GRAPHICS_HANDLER_H

#include "common/halfling_sys.h"

#include "d3d11.h"
#include "DirectXMath.h"


namespace Common {

class GraphicsManagerBase
{
public:
	GraphicsManagerBase();

	virtual bool Initialize(int clientWidth, int clientHeight, HWND hwnd, bool fullscreen);
	virtual void Shutdown();
	virtual void DrawFrame() = 0;
	virtual void OnResize(int newClientWidth, int newClientHeight);
	virtual void GamePaused() {}
	virtual void GameUnpaused() {}

protected:
	uint m_fps;
	float m_frameTime;

	ID3D11Device *m_device;
	ID3D11DeviceContext *m_immediateContext;
	IDXGISwapChain *m_swapChain;
	ID3D11Texture2D *m_depthStencilBuffer;
	ID3D11DepthStencilView *m_depthStencilView;
	D3D11_VIEWPORT m_screenViewport;

	bool m_d3dInitialized;

	// Screen dimensions
	int m_clientWidth;
	int m_clientHeight;

	// MSAA
	bool m_enable4xMSAA;
	uint m_4xMSAAQuality;

protected:
	void CalculateFrameStats(float deltaTime);
};

} // End of namespace Halfling

#endif
