/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef GAME_STATE_MANAGER_H
#define GAME_STATE_MANAGER_H

#include "common/halfling_sys.h"


namespace Common {

class GameStateManagerBase
{
public:
	GameStateManagerBase() : m_hwnd(nullptr) {}

protected:
	HWND m_hwnd;

public:
	virtual bool Initialize(HWND hwnd) = 0;
	virtual void Shutdown() = 0;
	/**
	 * Return the wanted period of time between update() calls.
	 *
	 * IE: If you want update to be called 20 times a second, this
	 * should return 50.
	 *
	 * NOTE: This should probably be inlined.
	 * NOTE: This will only be called at the beginning of HalflingEngine::Run()
	 * TODO: Contemplate the cost/benefit of calling this once per frame
	 *
	 * @return    The period in milliseconds
	 */
	virtual double GetUpdatePeriod() = 0;
	/**
	 * Called every time the game logic should be updated. The frequency
	 * of this being called is determined by getUpdatePeriod()
	 */
	virtual void Update() = 0;

	virtual void OnResize(int newClientWidth, int newClientHeight) = 0;

	virtual void GamePaused() = 0;
	virtual void GameUnpaused() = 0;

	virtual void MouseDown(WPARAM buttonState, int x, int y) = 0;
	virtual void MouseUp(WPARAM buttonState, int x, int y) = 0;
	virtual void MouseMove(WPARAM buttonState, int x, int y) = 0;
	virtual void MouseWheel(int zDelta) = 0;
};

} // End of namespace Common

#endif
