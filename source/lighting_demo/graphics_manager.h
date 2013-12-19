/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef LIGHTING_DEMO_GRAPHICS_MANAGER_H
#define LIGHTING_DEMO_GRAPHICS_MANAGER_H

#include "common/graphics_manager_base.h"

#include <d3d11.h>
#include "DirectXMath.h"

namespace LightingDemo {

class GameStateManager;

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

class GraphicsManager : public Common::GraphicsManagerBase {
public:
	GraphicsManager(GameStateManager *gameStateManager);

private:
	GameStateManager *m_gameStateManager;

	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11InputLayout *m_inputLayout;

	// Shaders
	ID3D11VertexShader *m_vertexShader;
	ID3D11PixelShader *m_pixelShader;

	// Vertex and Index buffers
	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;

	ID3D11Buffer *m_matrixBuffer;

public:
	bool Initialize(int clientWidth, int clientHeight, HWND hwnd);
	void Shutdown();
	void DrawFrame();
	void OnResize(int newClientWidth, int newClientHeight);

private:
	void LoadShaders();
	void BuildGeometryBuffers();
};

} // End of namespace CrateDemo

#endif
