/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef CRATE_DEMO_GRAPHICS_MANAGER_H
#define CRATE_DEMO_GRAPHICS_MANAGER_H

#include "common/graphics_manager_interface.h"


namespace CrateDemo {

class GraphicsManager : public Common::IGraphicsManager {
public:
	GraphicsManager();

private:


public:
	bool Initialize(int screenWidth, int screenHeight, HWND hwnd);
	void Shutdown();
	void DrawFrame();
	void OnResize();
	void GamePaused();
	void GameUnpaused();
};

} // End of namespace CrateDemo

#endif
