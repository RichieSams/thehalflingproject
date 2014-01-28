/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "halfling/halfling_engine.h"

#include "lighting_demo/graphics_manager.h"
#include "lighting_demo/game_state_manager.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	LightingDemo::GameStateManager gameStateManager;
	LightingDemo::GraphicsManager graphicsManager(&gameStateManager);
	
	Halfling::HalflingEngine engine(hInstance, &graphicsManager, &gameStateManager);

	engine.Initialize(L"Wave Simulation", 800, 600, false);
	engine.Run();
	
	engine.Shutdown();

	return 0;
}
