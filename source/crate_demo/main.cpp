/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "halfling/halfling_engine.h"

#include "crate_demo/graphics_manager.h"
#include "crate_demo/game_state_manager.h"


namespace CrateDemo {

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	GraphicsManager graphicsManager;
	GameStateManager gameStateManager;
	
	Halfling::HalflingEngine engine(&graphicsManager, &gameStateManager);

	engine.Initialize(L"Crate Demo", 800, 600, false);
	engine.Run();
	
	engine.Shutdown();
}

} // End of namespace CrateDemo
