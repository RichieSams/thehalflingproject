/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "obj_loader_demo/obj_loader_demo.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	ObjLoaderDemo::ObjLoaderDemo app(hInstance);
	DBG_UNREFERENCED_PARAMETER(hPrevInstance);
	DBG_UNREFERENCED_PARAMETER(lpCmdLine);
	DBG_UNREFERENCED_PARAMETER(nCmdShow);

	app.Initialize(L"OBJ Loader Demo", 1280, 720, false);
	app.Run();

	app.Shutdown();

	return 0;
}
