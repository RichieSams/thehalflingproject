/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_loader_demo/obj_loader_demo.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	#ifdef _DEBUG
	ID3D11Debug *debugInterface = nullptr;
	#endif
	
	// Force scope so we can guarantee the destructor is called before we try to use ReportLiveDeviceObjects
	{
		ObjLoaderDemo::ObjLoaderDemo app(hInstance);
		DBG_UNREFERENCED_PARAMETER(hPrevInstance);
		DBG_UNREFERENCED_PARAMETER(lpCmdLine);
		DBG_UNREFERENCED_PARAMETER(nCmdShow);

		app.Initialize(L"OBJ Loader Demo", 1280, 720, false);

		#ifdef _DEBUG
		app.CreateDebugInterface(&debugInterface);
		#endif

		app.Run();

		app.Shutdown();
	}

	#ifdef _DEBUG
	debugInterface->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	ReleaseCOM(debugInterface);
	#endif

	return 0;
}
