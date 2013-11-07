/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef HALFLING_ENGINE_H
#define HALFLING_ENGINE_H

#include "common/halfling_sys.h"

#include "common/graphics_manager_base.h"
#include "common/game_state_manager_base.h"

#include "common/timer.h"


namespace Halfling {

class HalflingEngine {
public:
	HalflingEngine(HINSTANCE hinstance, Common::GraphicsManagerBase *graphicsManager, Common::GameStateManagerBase *gameStateManager);
	~HalflingEngine();

private:
	LPCTSTR m_mainWndCaption;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	Common::GraphicsManagerBase *m_graphicsManager;
	Common::GameStateManagerBase *m_gameStateManager;
	Common::Timer m_timer;

	bool m_fullscreen;
	uint32 m_clientWidth;
	uint32 m_clientHeight;
	bool m_appPaused;
	bool m_isMinOrMaximized;
	bool m_resizing;

public:
	HINSTANCE AppInst() const;
	HWND MainWnd() const;
	float AspectRatio() const;

	bool Initialize(LPCTSTR mainWndCaption, uint32 screenWidth, uint32 screenHeight, bool fullscreen);
	void Shutdown();
	void Run();

	LRESULT MsgProc(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);

private:
	void CalculateFrameStats();
	void OnResize();

	void InitializeWindow();
	void ShutdownWindow();

	void PauseGame();
	void UnPauseGame();
};

} // End of namespace Halfling

// This is used to forward Windows messages from a global window
// procedure to our member function window procedure because we cannot
// assign a member function to WNDCLASS::lpfnWndProc.
static Halfling::HalflingEngine *g_engine = NULL;

#endif
