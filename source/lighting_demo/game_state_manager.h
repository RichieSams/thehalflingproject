/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef LIGHTING_DEMO_GAME_STATE_MANAGER_H
#define LIGHTING_DEMO_GAME_STATE_MANAGER_H

#include "common/game_state_manager_base.h"

#include "lighting_demo/wave_simulator.h"

#include "common/halfling_sys.h"
#include "common/vector.h"
#include "common/camera.h"
#include "common/model.h"
#include "common/light_manager.h"

#include "DirectXMath.h"
#include <d3d11.h>
#include <vector>


namespace LightingDemo {

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
};

struct WorldViewProjection {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};

class GameStateManager : public Common::GameStateManagerBase {
public:
	GameStateManager();

private:
	const double kUpdatePeriod = 30.0f;

	Common::Vector2 m_mouseLastPos;
	Common::Camera m_camera;

public:
	WorldViewProjection WorldViewProj;
	std::vector<Common::Model<Vertex> > Models;
	Common::LightManager LightManager;

public:
	bool Initialize(HWND hwnd, ID3D11Device **device);
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
	inline double GetUpdatePeriod() { return kUpdatePeriod; }
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

	inline DirectX::XMFLOAT3 GetCameraPosition() { return m_camera.GetCameraPosition(); }

private:
	void BuildGeometryBuffers();
	void CreateLights();
};

} // End of namespace CrateDemo

#endif
