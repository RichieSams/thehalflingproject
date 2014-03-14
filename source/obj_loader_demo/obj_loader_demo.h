/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef OBJ_LOADER_DEMO_H
#define OBJ_LOADER_DEMO_H

#include "halfling/halfling_engine.h"

#include "common/vector.h"
#include "common/camera.h"
#include "common/texture_manager.h"
#include "common/console.h"
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
#include <atomic>
#include <thread>


namespace ObjLoaderDemo {

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

struct SceneLoaderModelSubset {
	uint VertexStart;
	uint VertexCount;

	uint IndexStart;
	uint IndexCount;

	DirectX::XMFLOAT4 Ambient; // w = SpecularIntensity
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; // w = SpecPower

	std::wstring DiffuseMapFile;
	std::wstring AmbientMapFile;
	std::wstring SpecularColorMapFile;
	std::wstring SpecularHighlightMapFile;
	std::wstring AlphaMapFile;
	std::wstring BumpMapFile;
	
	DirectX::XMFLOAT3 AABBMin;
	DirectX::XMFLOAT3 AABBMax;
};

struct SceneLoaderModel {
	Vertex *Vertices;
	uint *Indices;
	SceneLoaderModelSubset *Subsets;

	uint VertexCount;
	uint IndexCount;
	uint SubsetCount;
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

class ObjLoaderDemo : public Halfling::HalflingEngine {
public:
	ObjLoaderDemo(HINSTANCE hinstance);

private:
	static const uint kMaxMaterialsPerFrame = 2000;

	Common::Vector2 m_mouseLastPos;
	Common::Camera m_camera;
	Common::TextureManager m_textureManager;
	Common::Console m_console;
	bool m_showConsole;

	std::vector<Common::Model<Vertex> *> m_models;
	std::vector<Common::BlinnPhongMaterial> m_frameMaterialList;

	std::vector<SceneLoaderModel> m_sceneLoaderModels;
	std::atomic<bool> m_sceneLoaded;
	bool m_sceneIsSetup;
	std::thread m_sceneLoaderThread;

	float m_sceneScaleFactor;

	Common::Model<DebugObjectVertex, DebugObjectInstance> m_debugSphere;
	Common::Model<DebugObjectVertex, DebugObjectInstance> m_debugCone;
	uint m_debugSphereNumIndices;
	uint m_debugConeNumIndices;

	Common::DirectionalLight m_directionalLight;
	std::vector<Common::PointLight *> m_pointLights;
	std::vector<Common::PointLightAnimator *> m_pointLightAnimators;
	std::vector<Common::SpotLight *> m_spotLights;
	std::vector<Common::SpotLightAnimator *> m_spotLightAnimators;

	bool m_pointLightBufferNeedsRebuild;
	bool m_spotLightBufferNeedsRebuild;

	bool m_vsync;
	bool m_wireframe;
	bool m_animateLights;
	ShadingType m_shadingType;
	bool m_showLightLocations;
	bool m_showGBuffers;
	uint32 m_numSpotLightsToDraw;
	uint32 m_numPointLightsToDraw;

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
	ID3D11VertexShader *m_forwardVertexShader;
	ID3D11PixelShader *m_forwardPixelShader;
	ID3D11VertexShader *m_gbufferVertexShader;
	ID3D11PixelShader *m_gbufferPixelShader;
	ID3D11VertexShader *m_fullscreenTriangleVertexShader;
	ID3D11PixelShader *m_noCullFinalGatherPixelShader;
	ID3D11VertexShader *m_debugObjectVertexShader;
	ID3D11PixelShader *m_debugObjectPixelShader;
	ID3D11VertexShader *m_transformedFullscreenTriangleVertexShader;
	ID3D11PixelShader *m_renderGbuffersPixelShader;

	ID3D11Buffer *m_forwardPixelShaderFrameConstantsBuffer;
	ID3D11Buffer *m_forwardPixelShaderObjectConstantsBuffer;
	ID3D11Buffer *m_gBufferVertexShaderObjectConstantsBuffer;
	ID3D11Buffer *m_gBufferPixelShaderObjectConstantsBuffer;
	ID3D11Buffer *m_noCullFinalGatherPixelShaderConstantsBuffer;
	ID3D11Buffer *m_transformedFullscreenTriangleVertexShaderConstantsBuffer;
	ID3D11Buffer *m_renderGbuffersPixelShaderConstantsBuffer;

	// We assume there is only one directional light. Therefore, it is stored in a cbuffer
	Common::StructuredBuffer<Common::PointLight> *m_pointLightBuffer;
	Common::StructuredBuffer<Common::SpotLight> *m_spotLightBuffer;

	Common::StructuredBuffer<Common::BlinnPhongMaterial> *m_frameMaterialListBuffer;

	Common::BlendStates m_blendStates;
	Common::DepthStencilStates m_depthStencilStates;
	Common::RasterizerStates m_rasterizerStates;
	Common::SamplerStates m_samplerStates;

	Common::SpriteRenderer m_spriteRenderer;
	Common::SpriteFont m_timesNewRoman12Font;
	Common::SpriteFont m_courierNew10Font;

public:
	// Inherited methods
	bool Initialize(LPCTSTR mainWndCaption, uint32 screenWidth, uint32 screenHeight, bool fullscreen);
	void Shutdown();

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
	void CharacterInput(wchar character);

	// Initialization methods
	void InitTweakBar();
	void LoadShaders();
	void CreateShaderBuffers();
	void SetupScene();
	void BuildGeometryBuffers();
	void CreateLights(const DirectX::XMFLOAT3 &sceneSizeMin, const DirectX::XMFLOAT3 &sceneSizeMax);

	// Rendering methods
	/** Renders the geometry using the ShadingType in m_shadingType */
	void RenderMainPass();
	/** Renders the geometry using Forward Shading */
	void ForwardRenderingPass();
	/** Renders the geometry using Deferred Shading with no light culling */
	void NoCullDeferredRenderingPass();
	/** Renders any geometry used for visualizing effects, etc. (Light locations, etc) */
	void RenderDebugGeometry();
	/** Renders the frame statistics and the settings bar */
	void RenderHUD();

	void SetForwardPixelShaderFrameConstants();
	void SetForwardPixelShaderObjectConstants(const Common::BlinnPhongMaterial &material, uint textureFlags);
	void SetGBufferVertexShaderConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix);
	void SetGBufferPixelShaderConstants(uint materialIndex, uint textureFlags);
	void SetNoCullFinalGatherShaderConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix);

	/** Maps the point light StructuredBuffer and the spot light Structured buffer to the pixel shader */
	void SetLightBuffers();
	/** Maps a list of Materials used in the frame to the pixel shader. This is only used for deferred shading */
	void SetMaterialList();
};

} // End of namespace ObjLoaderDemo

#endif
