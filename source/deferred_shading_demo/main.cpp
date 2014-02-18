/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "deferred_shading_demo/deferred_shading_demo.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	DeferredShadingDemo::DeferredShadingDemo app(hInstance);

	app.Initialize(L"Deferred Shading Demo", 800, 600, false);
	app.Run();

	app.Shutdown();

	return 0;
}
