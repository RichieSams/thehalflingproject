/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef HALFLING_ENGINE_H
#define HALFLING_ENGINE_H

#include "common/halfling_sys.h"
#include "common/clock.h"

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

	Common::Clock m_mainClock;

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
	/**
	 * Initializes the Engine.
	 * This will create and register the window, and create and initialize D3D
	 *
	 * Note: If you would like to use MSAA, set m_msaaCount before calling this method
	 *
	 * @param mainWndCaption
	 * @param screenWidth
	 * @param screenHeight
	 * @param fullscreen
	 * @return
	 */
	virtual bool Initialize(LPCTSTR mainWndCaption, uint32 screenWidth, uint32 screenHeight, bool fullscreen);
	/**
	 * Shuts down the Engine
	 * This will shut down D3D, release any COM devices, and then shutdown the window.
	 */
	virtual void Shutdown();
	/**
	 * The main window loop
	 * It will loop until it receives a WM_QUIT message
	 *
	 * The loop is as follows:
	 * 1. Process all windows messages
	 * 2. Accumulate the deltaTime from last loop
	 * 3. while (accumulatedTime >= m_updatePeriod) {
	 *        accumulatedTime -= m_updatePeriod;
	 *        Update()
	 *    }
	 * 4. CalculateFrameStats()
	 * 5. DrawFrame()
	 */
	void Run();

	/**
	 * The main window Message Handler.
	 * Override if you need to handle special messages. However, any messages
	 * that you don't handle should be passed to this base method.
	 *
	 * @param hwnd      The handle of the main window
	 * @param msg       The message code
	 * @param wParam    The wParam of the message
	 * @param lParam    The lParam of the message
	 * @return          A LRESULT code signaling whether a message was handled or not
	 */
	virtual LRESULT MsgProc(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);

protected:
	/**
	 * Called every time the window is resized
	 */
	virtual void OnResize() {}
	/**
	 * Called when at least m_updatePeriod time has passed since the last call to Update()
	 */
	virtual void Update() {}
	/**
	 * Called once every loop
	 *
	 * @param deltaTime    The deltaTime passed since the last loop
	 */
	virtual void DrawFrame(double deltaTime) {}

	/**
	 * Called every time the game is paused. You can also call this function to
	 * pause the game.
	 */
	virtual void PauseGame();
	/**
	 * Called every time the game is unpaused. You can also called this function to
	 * unpause the game.
	 *
	 * @return
	 */
	virtual void UnPauseGame();

	/** Returns the current window aspect ratio */
	inline float AspectRatio() const {
		return static_cast<float>(m_clientWidth) / m_clientHeight;
	}

	/**
	 * Called every time one of the mouse buttons is pressed down
	 *
	 * @param buttonState    The wParam describing the button state
	 * @param x              The x position of the cursor (In window coordinates)
	 * @param y              The y position of the cursor (In window coordinates)
	 */
	virtual void MouseDown(WPARAM buttonState, int x, int y) {}
	/**
	 * Called every time one of the mouse buttons is let go
	 *
	 * @param buttonState    The wParam describing the button state
	 * @param x              The x position of the cursor (In window coordinates)
	 * @param y              The y position of the cursor (In window coordinates)
	 */
	virtual void MouseUp(WPARAM buttonState, int x, int y) {}
	/**
	 * Called every time the mouse moves
	 *
	 * @param buttonState    The wParam describing the button state
	 * @param x              The x position of the cursor (In window coordinates)
	 * @param y              The y position of the cursor (In window coordinates)
	 */
	virtual void MouseMove(WPARAM buttonState, int x, int y) {}
	/**
	 * Called every time the middle mouse wheel is scrolled
	 *
	 * @param zDelta    The number of units scrolled. The number of units per full circle of the wheel depends on the mouse vendor
	 */
	virtual void MouseWheel(int zDelta) {}

private:
	/** Creates the window and registers it */
	void InitializeWindow();
	/** Un-registers the window and destroys it*/
	void ShutdownWindow();

	/**
	 * Calculates m_fps and m_frameTime.
	 * It assumes that it will be called once per frame.
	 *
	 * @param deltaTime    The deltaTime (in ms) since the last call to CalculateFrameStats
	 */
	void CalculateFrameStats(double deltaTime);
};

} // End of namespace Halfling

// This is used to forward Windows messages from a global window
// procedure to our member function window procedure because we cannot
// assign a member function to WNDCLASS::lpfnWndProc.
static Halfling::HalflingEngine *g_engine = NULL;

#endif
