/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef DEFERRED_SHADING_DEMO_H
#define DEFERRED_SHADING_DEMO_H

#include "halfling/halfling_engine.h"

#include "common/vector.h"
#include "common/camera.h"
#include "common/texture_manager.h"
#include "common/model.h"
#include "common/texture2d.h"
#include "common/structured_buffer.h"
#include "common/device_states.h"
#include "common/sprite_renderer.h"
#include "common/sprite_font.h"
#include "common/lights.h"

#include <vector>
#include <AntTweakBar.h>


namespace DeferredShadingDemo {

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoord;
};

struct WorldViewProjection {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};

class DeferredShadingDemo : public Halfling::HalflingEngine {
public:
	DeferredShadingDemo(HINSTANCE hinstance);

private:
	Common::Vector2 m_mouseLastPos;
	Common::Camera m_camera;
	Common::TextureManager m_textureManager;

	WorldViewProjection m_worldViewProj;
	std::vector<Common::Model<Vertex> > m_models;

	Common::DirectionalLight m_directionalLight;
	std::vector<Common::PointLight> m_pointLights;
	std::vector<Common::SpotLight> m_spotLights;

	bool m_pointLightBufferNeedsRebuild;
	bool m_spotLightBufferNeedsRebuild;

	bool m_vsync;
	bool m_wireframe;

	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11InputLayout *m_inputLayout;

	Common::Depth2D *m_depthStencilBuffer;
	D3D11_VIEWPORT m_screenViewport;

	std::vector<Common::Texture2D *> m_gBuffers;
	std::vector<ID3D11ShaderResourceView *> m_gBufferSRVs;
	std::vector<ID3D11RenderTargetView *> m_gBufferRTVs;

	TwBar *m_settingsBar;

	// Shaders
	ID3D11VertexShader *m_vertexShader;
	ID3D11PixelShader *m_pixelShader;

	ID3D11Buffer *m_vertexShaderFrameConstantsBuffer;
	ID3D11Buffer *m_vertexShaderObjectConstantsBuffer;
	ID3D11Buffer *m_pixelShaderFrameConstantsBuffer;
	ID3D11Buffer *m_pixelShaderObjectConstantsBuffer;

	// We assume there is only one directional light. Therefore, it is stored in a cbuffer
	Common::StructuredBuffer<Common::PointLight> *m_pointLightBuffer;
	Common::StructuredBuffer<Common::SpotLight> *m_spotLightBuffer;

	ID3D11SamplerState *m_diffuseSampleState;

	ID3D11RasterizerState *m_wireframeRS;
	ID3D11RasterizerState *m_solidRS;

	Common::BlendStates m_blendStates;
	Common::DepthStencilStates m_depthStencilStates;
	Common::RasterizerStates m_rasterizerStates;
	Common::SamplerStates m_samplerStates;

	Common::SpriteRenderer m_spriteRenderer;
	Common::SpriteFont m_timesNewRoman12Font;

public:
	// Inherited methods
	bool Initialize(LPCTSTR mainWndCaption, uint32 screenWidth, uint32 screenHeight, bool fullscreen);
	void Shutdown();

	// Tweak bar callbacks
	static void TW_CALL SetWireframeRSCallback(const void *value, void *clientData);
	static void TW_CALL GetWireframeTSCallback(void *value, void *clientData);

private:
	// Inherited methods
	LRESULT MsgProc(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);

	void OnResize();
	void Update();
	void DrawFrame(double deltaTime);

	void MouseDown(WPARAM buttonState, int x, int y);
	void MouseUp(WPARAM buttonState, int x, int y);
	void MouseMove(WPARAM buttonState, int x, int y);
	void MouseWheel(int zDelta);

	// Initialization methods
	void InitTweakBar();
	void LoadShaders();
	void CreateShaderBuffers();
	void BuildGeometryBuffers();
	inline float GetHillHeight(float x, float z) const;
	inline DirectX::XMFLOAT3 GetHillNormal(float x, float z) const;
	void CreateLights();

	// Rendering methods
	void RenderMainPass();
	void RenderHUD();
	void SetFrameConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &viewProjMatrix);
	void SetObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix, const Common::Material &material);
	void SetLightBuffers(DirectX::XMMATRIX &viewMatrix);
};

} // End of namespace DeferredShadingDemo

#endif
