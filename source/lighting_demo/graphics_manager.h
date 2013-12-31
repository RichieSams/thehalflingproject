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

struct Material {
	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; // w = SpecPower
	DirectX::XMFLOAT4 Reflect;
};

struct VertexShaderFrameConstants {
	DirectX::XMMATRIX viewProj;
	DirectX::XMMATRIX proj;
};

struct VertexShaderObjectConstants {
	DirectX::XMMATRIX worldViewProj;
	DirectX::XMMATRIX worldView;
};

struct PixelShaderObjectConstants {
	Common::DirectionalLight directionalLight;
	Material material;
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

	ID3D11Buffer *m_vertexShaderFrameConstantsBuffer;
	ID3D11Buffer *m_vertexShaderObjectConstantsBuffer;
	ID3D11Buffer *m_pixelShaderObjectConstantsBuffer;

	// We assume there is only one directional light. Therefore, it is stored in a cbuffer
	Common::StructuredBuffer<Common::PointLight> *m_pointLightBuffer;
	Common::StructuredBuffer<Common::SpotLight> *m_spotLightBuffer;

	ID3D11RasterizerState *m_wireframeRS;

public:
	bool Initialize(int clientWidth, int clientHeight, HWND hwnd, bool fullscreen);
	void Shutdown();
	void DrawFrame();
	void SetFrameConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &viewProjMatrix);
	void SetObjectConstants(DirectX::XMMATRIX &worldViewMatrix, DirectX::XMMATRIX &worldViewProjMatrix, Material &material);
	void OnResize(int newClientWidth, int newClientHeight);

private:
	void LoadShaders();
};

} // End of namespace CrateDemo

#endif
