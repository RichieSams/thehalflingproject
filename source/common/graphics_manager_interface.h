/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef HALFLING_GRAPHICS_HANDLER_H
#define HALFLING_GRAPHICS_HANDLER_H

#include "common/halfling_sys.h"

namespace Common {

class IGraphicsManager
{
public:
	virtual bool Initialize(int clientWidth, int clientHeight, HWND hwnd) = 0;
	virtual void Shutdown() = 0;
	virtual void DrawFrame() = 0;
	virtual void OnResize(int newClientWidth, int newClientHeight) = 0;
	virtual void GamePaused() = 0;
	virtual void GameUnpaused() = 0;
};

} // End of namespace Halfling

#endif
