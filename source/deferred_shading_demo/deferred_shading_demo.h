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
#include "common/light_animator.h"

#include <vector>
#include <AntTweakBar.h>


namespace DeferredShadingDemo {

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoord;
};

struct DebugObjectVertex {
	DirectX::XMFLOAT3 pos;
};

struct DebugObjectInstance {
	DirectX::XMMATRIX worldViewProj;
	DirectX::XMFLOAT4 color;
};

struct FullScreenTriangleVertex {
	DirectX::XMFLOAT3 pos;
};

struct WorldViewProjection {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};

enum ShadingType {
	Forward,
	NoCullDeferred
};

class DeferredShadingDemo : public Halfling::HalflingEngine {
public:
	DeferredShadingDemo(HINSTANCE hinstance);

private:
	static const uint kMaxMaterialsPerFrame = 20;

	Common::Vector2 m_mouseLastPos;
	Common::Camera m_camera;
	Common::TextureManager m_textureManager;

	WorldViewProjection m_worldViewProj;
	std::vector<Common::Model<Vertex> > m_models;
	std::vector<Common::BlinnPhongMaterial> m_frameMaterialList;

	Common::Model<DebugObjectVertex, DebugObjectInstance> m_debugSphere;
	uint m_debugSphereNumIndices;

	Common::DirectionalLight m_directionalLight;
	std::vector<Common::PointLight> m_pointLights;
	std::vector<Common::PointLightAnimator> m_pointLightAnimators;
	std::vector<Common::SpotLight> m_spotLights;

	bool m_pointLightBufferNeedsRebuild;
	bool m_spotLightBufferNeedsRebuild;

	bool m_vsync;
	bool m_wireframe;
	ShadingType m_shadingType;
	bool m_showLightLocations;
	bool m_showGBuffers;

	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11InputLayout *m_gBufferInputLayout;
	ID3D11InputLayout *m_debugObjectInputLayout;

	Common::Depth2D *m_depthStencilBuffer;
	D3D11_VIEWPORT m_screenViewport;

	std::vector<Common::Texture2D *> m_gBuffers;
	std::vector<ID3D11ShaderResourceView *> m_gBufferSRVs;
	std::vector<ID3D11RenderTargetView *> m_gBufferRTVs;

	TwBar *m_settingsBar;

	// Shaders
	ID3D11VertexShader *m_gbufferVertexShader;
	ID3D11PixelShader *m_gbufferPixelShader;
	ID3D11VertexShader *m_fullscreenTriangleVertexShader;
	ID3D11PixelShader *m_noCullFinalGatherPixelShader;
	ID3D11VertexShader *m_debugObjectVertexShader;
	ID3D11PixelShader *m_debugObjectPixelShader;
	ID3D11VertexShader *m_transformedFullscreenTriangleVertexShader;
	ID3D11PixelShader *m_renderGbuffersPixelShader;

	ID3D11Buffer *m_gBufferVertexShaderObjectConstantsBuffer;
	ID3D11Buffer *m_gBufferPixelShaderObjectConstantsBuffer;
	ID3D11Buffer *m_noCullFinalGatherPixelShaderConstantsBuffer;
	ID3D11Buffer *m_transformedFullscreenTriangleVertexShaderConstantsBuffer;
	ID3D11Buffer *m_renderGbuffersPixelShaderConstantsBuffer;

	// We assume there is only one directional light. Therefore, it is stored in a cbuffer
	Common::StructuredBuffer<Common::PointLight> *m_pointLightBuffer;
	Common::StructuredBuffer<Common::SpotLight> *m_spotLightBuffer;

	Common::StructuredBuffer<Common::BlinnPhongMaterial> *m_frameMaterialListBuffer;

	ID3D11SamplerState *m_diffuseSampleState;

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
	void ForwardRenderingPass();
	void NoCullDeferredRenderingPass();
	void RenderDebugGeometry();
	void RenderHUD();
	void SetGBufferVertexShaderConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix);
	void SetGBufferPixelShaderConstants(uint materialIndex);
	void SetNoCullFinalGatherShaderConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix);
	void SetLightBuffers();
	void SetMaterialList();
};

} // End of namespace DeferredShadingDemo

#endif
