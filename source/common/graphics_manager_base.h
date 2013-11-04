/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef HALFLING_GRAPHICS_HANDLER_H
#define HALFLING_GRAPHICS_HANDLER_H

#include "common/halfling_sys.h"

#include "d3d11.h"
#include "DirectXMath.h"


namespace Common {

class GraphicsManagerBase
{
public:
	GraphicsManagerBase();

	virtual bool Initialize(int clientWidth, int clientHeight, HWND hwnd);
	virtual void Shutdown();
	virtual void DrawFrame() = 0;
	virtual void OnResize(int newClientWidth, int newClientHeight);
	virtual void GamePaused() {}
	virtual void GameUnpaused() {}

};

} // End of namespace Halfling

#endif
