/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#pragma once

#include "common/game_state_manager_base.h"

#include "common/halfling_sys.h"
#include "common/vector.h"
#include "common/camera.h"

#include "DirectXMath.h"


namespace CrateDemo {

struct MatrixBufferType {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};

class GameStateManager : public Common::GameStateManagerBase {
public:
	GameStateManager();

private:
	Common::Vector2 m_mouseLastPos;
	Common::Camera m_camera;

public:
	MatrixBufferType WorldViewProj;

public:
	bool Initialize(HWND hwnd);
	void Shutdown();
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
	inline double GetUpdatePeriod() { return 30.0; }
	/**
	 * Called every time the game logic should be updated. The frequency
	 * of this being called is determined by getUpdatePeriod()
	 */
	void Update();

	void OnResize(int newClientWidth, int newClientHeight);

	void GamePaused();
	void GameUnpaused();

	void MouseDown(WPARAM buttonState, int x, int y);
	void MouseUp(WPARAM buttonState, int x, int y);
	void MouseMove(WPARAM buttonState, int x, int y);
	void MouseWheel(int zDelta);
};

} // End of namespace CrateDemo
