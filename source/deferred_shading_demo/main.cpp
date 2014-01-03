/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "halfling/halfling_engine.h"

#include "deferred_shading_demo/graphics_manager.h"
#include "deferred_shading_demo/game_state_manager.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	DeferredShadingDemo::GameStateManager gameStateManager;
	DeferredShadingDemo::GraphicsManager graphicsManager(&gameStateManager);
	
	Halfling::HalflingEngine engine(hInstance, &graphicsManager, &gameStateManager);

	engine.Initialize(L"Lighting Demo", 800, 600, false);
	engine.Run();
	
	engine.Shutdown();

	return 0;
}
