/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef HALFLING_ENGINE_H
#define HALFLING_ENGINE_H

#include "common/halfling_sys.h"
#include "common/timer.h"

#include "d3d11.h"


namespace Halfling {

class HalflingEngine {
public:
	HalflingEngine(HINSTANCE hinstance);
	~HalflingEngine();

private:
	LPCTSTR m_mainWndCaption;
	HINSTANCE m_hinstance;

	bool m_appPaused;
	bool m_isMinOrMaximized;
	bool m_resizing;

	Common::Timer m_timer;

protected:
	HWND m_hwnd;
	bool m_fullscreen;
	uint32 m_clientWidth;
	uint32 m_clientHeight;

	double m_updatePeriod;

	uint m_fps;
	float m_frameTime;

	ID3D11Device *m_device;
	ID3D11DeviceContext *m_immediateContext;
	IDXGISwapChain *m_swapChain;

	bool m_d3dInitialized;

	uint m_msaaCount;
	bool m_stencil;

public:
	virtual bool Initialize(LPCTSTR mainWndCaption, uint32 screenWidth, uint32 screenHeight, bool fullscreen);
	virtual void Shutdown();
	void Run();

	virtual LRESULT MsgProc(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void OnResize() {}
	virtual void Update() {}
	virtual void DrawFrame(double deltaTime) {}

	virtual void PauseGame();
	virtual void UnPauseGame();

	inline float AspectRatio() const { return static_cast<float>(m_clientWidth) / m_clientHeight; }

	virtual void MouseDown(WPARAM buttonState, int x, int y) {}
	virtual void MouseUp(WPARAM buttonState, int x, int y) {}
	virtual void MouseMove(WPARAM buttonState, int x, int y) {}
	virtual void MouseWheel(int zDelta) {}

private:
	void InitializeWindow();
	void ShutdownWindow();

	void CalculateFrameStats(double deltaTime);
};

} // End of namespace Halfling

// This is used to forward Windows messages from a global window
// procedure to our member function window procedure because we cannot
// assign a member function to WNDCLASS::lpfnWndProc.
static Halfling::HalflingEngine *g_engine = NULL;

#endif
