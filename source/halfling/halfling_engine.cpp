/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "halfling_engine.h"

#include <sstream>
#include <windowsx.h>


namespace Halfling {

#define WINDOW_CLASS_NAME = L"HalflingEngineWindow";

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return g_engine->MsgProc(hwnd, msg, wParam, lParam);
}

HalflingEngine::HalflingEngine(Common::IGraphicsManager *graphicsManager, Common::IGameStateManager *gameStateManager)
		: m_graphicsManager(graphicsManager),
		  m_gameStateManager(gameStateManager),
		  m_timer(Common::Timer()),
		  m_appPaused(false),
		  m_isMinOrMaximized(false),
		  m_resizing(false) {
	// Get a pointer to the application object so we can forward Windows messages to 
	// the object's window procedure through the global window procedure.
	g_engine = this;
}

HalflingEngine::~HalflingEngine() {
	g_engine = nullptr;
}

HINSTANCE HalflingEngine::AppInst() const {
	return m_hinstance;
}

HWND HalflingEngine::MainWnd() const {
	return m_hwnd;
}

float HalflingEngine::AspectRatio() const {
	return static_cast<float>(m_screenWidth) / m_screenHeight;
}

bool HalflingEngine::Initialize(LPCTSTR mainWndCaption, uint32 screenWidth, uint32 screenHeight, bool fullscreen) {
	m_mainWndCaption = mainWndCaption;
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_fullscreen = fullscreen;

	// Initialize the window
	InitializeWindow();

	// Initialize the managers
	if (!m_graphicsManager->Initialize(m_screenWidth, m_screenHeight, m_hwnd))
		return false;
	if (!m_gameStateManager->Initialize())
		return false;	

	return true;
}

void HalflingEngine::Shutdown() {
	// Shutdown in reverse order
	m_gameStateManager->Shutdown();
	m_graphicsManager->Shutdown();
	
	// Shutdown the window.
	ShutdownWindow();

	g_engine = nullptr;

	return;
}

void HalflingEngine::Run() {
	MSG msg;
	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	
	bool done = false;
	double accumulatedTime = 0.0;
	double deltaTime = 0.0;
	const double updatePeriod = m_gameStateManager->GetUpdatePeriod();

	// Loop until there is a quit message from the window or the user.
	while (!done) {
		// Handle the windows messages
		// *ALL* messages must be handled before game logic is done
		// This allows for the highest response time, but lags the game
		// during high message count.
		// TODO: Add logic to only allow a certain amount of low priority messages through each frame and buffer the rest
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			// If windows signals to end the application then exit out.
			if (msg.message == WM_QUIT) {
				done = true;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Otherwise do the frame processing
			m_timer.Tick();
			deltaTime = m_timer.DeltaTime();
			// Avoid spiral of death
			if (deltaTime > 250.0) {
				deltaTime = 250.0;
			}
			accumulatedTime += deltaTime;

			while (accumulatedTime >= updatePeriod) {
				m_gameStateManager->Update();
			}
			
			m_graphicsManager->DrawFrame();

			CalculateFrameStats();
		}
	}

	return;
}

void HalflingEngine::PauseGame() {
	m_appPaused = true;
	m_timer.Stop();
	m_graphicsManager->GamePaused();
	m_gameStateManager->GamePaused();
}

void HalflingEngine::UnPauseGame() {
	m_appPaused = false;
	m_timer.Start();
	m_graphicsManager->GameUnpaused();
	m_gameStateManager->GameUnpaused();
}

void HalflingEngine::CalculateFrameStats() {
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static double timeElapsed = 0.0;

	frameCnt++;
	timeElapsed += m_timer.DeltaTime();

	// Compute averages over one second period.
	if (timeElapsed >= 1000.0f) {
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;   
		outs.precision(6);
		outs << m_mainWndCaption << L"    "
		     << L"FPS: " << fps << L"    " 
		     << L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hwnd, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed = 0;
	}
}

void HalflingEngine::OnResize() {
	m_graphicsManager->OnResize();
}

void HalflingEngine::InitializeWindow() {
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"HalflingEngineWindow";
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (m_fullscreen) {
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));

		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)m_screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)m_screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	} else {
		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - m_screenWidth)  / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - m_screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, 
	                        L"HalflingEngineWindow", 
	                        m_mainWndCaption, 
	                        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
	                        posX, posY, 
	                        m_screenWidth, m_screenHeight, 
	                        NULL, NULL, 
	                        m_hinstance, 
	                        NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	//ShowCursor(false);

	return;
}

void HalflingEngine::ShutdownWindow() {
	// Show the mouse cursor.
	//ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(m_fullscreen) {
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(L"HalflingEngineWindow", m_hinstance);
	m_hinstance = NULL;

	return;
}

} // End of namespace Halfling
