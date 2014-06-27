/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef OBJ_LOADER_DEMO_H
#define OBJ_LOADER_DEMO_H

#include "halfling/halfling_engine.h"

#include "obj_loader_demo/shader_constants.h"

#include "common/vector.h"
#include "common/camera.h"
#include "common/texture_manager.h"
#include "common/model_manager.h"
#include "common/console.h"
#include "common/model.h"
#include "common/texture2d.h"
#include "common/structured_buffer.h"
#include "common/device_states.h"
#include "common/sprite_renderer.h"
#include "common/sprite_font.h"
#include "common/lights.h"
#include "common/light_animator.h"
#include "common/allocator_16_byte_aligned.h"
#include "common/shader.h"

#include <vector>
#include <AntTweakBar.h>
#include <atomic>
#include <thread>


namespace ObjLoaderDemo {

struct DebugObjectVertex {
	DirectX::XMFLOAT3 pos;
};

struct DebugObjectInstance {
	DirectX::XMMATRIX worldViewProj;
	DirectX::XMFLOAT4 color;
};

enum ShadingType {
	Forward,
	NoCullDeferred
};

enum GBufferSelector {
	Diffuse = 0,
	Specular = 1,
	Normal_Spherical = 2,
	Normal_Cartesian = 3,
	Depth = 4,
	None = 5
};

struct ModelToLoad {
	ModelToLoad(std::string filePath, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *instances)
		: FilePath(filePath),
		  Instances(instances) {
	}

	std::string FilePath;
	std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *Instances;
};

class ObjLoaderDemo : public Halfling::HalflingEngine {
public:
	ObjLoaderDemo(HINSTANCE hinstance);

private:
	static const uint kMaxMaterialsPerFrame = 2000;
	static const uint kMaxInstanceVectorsPerFrame = 5000;

	float m_nearClip;
	float m_farClip;

	Common::Vector2 m_mouseLastPos;
	Common::Camera m_camera;
	float m_cameraPanFactor;
	float m_cameraScrollFactor;

	Common::TextureManager m_textureManager;
	Common::ModelManager m_modelManager;
	Common::Console m_console;
	bool m_showConsole;

	DirectX::XMMATRIX m_globalWorldTransform;

	std::vector<std::pair<Common::Model *, DirectX::XMMATRIX>, Common::Allocator16ByteAligned<std::pair<Common::Model *, DirectX::XMMATRIX> > > m_models;
	std::vector<std::pair<Common::Model *, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *> > m_instancedModels;

	Common::StructuredBuffer<DirectX::XMVECTOR> *m_instanceBuffer;

	std::vector<ModelToLoad> m_modelsToLoad;
	std::atomic<bool> m_sceneLoaded;
	bool m_sceneIsSetup;
	std::thread m_sceneLoaderThread;

	float m_sceneScaleFactor;
	uint m_modelInstanceThreshold;

	Common::InstancedModel m_debugSphere;
	Common::InstancedModel m_debugCone;

	Common::DirectionalLight m_directionalLight;
	std::vector<Common::PointLight> m_pointLights;
	std::vector<Common::PointLightAnimator> m_pointLightAnimators;
	std::vector<Common::SpotLight> m_spotLights;
	std::vector<Common::SpotLightAnimator> m_spotLightAnimators;

	bool m_pointLightBufferNeedsRebuild;
	bool m_spotLightBufferNeedsRebuild;

	bool m_vsync;
	bool m_wireframe;
	bool m_animateLights;
	ShadingType m_shadingType;
	GBufferSelector m_gbufferSelector;
	bool m_showLightLocations;
	bool m_showGBuffers;
	uint32 m_numSpotLightsToDraw;
	uint32 m_numPointLightsToDraw;

	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11InputLayout *m_defaultInputLayout;
	ID3D11InputLayout *m_debugObjectInputLayout;

	Common::Depth2D *m_depthStencilBuffer;
	D3D11_VIEWPORT m_screenViewport;

	std::vector<Common::Texture2D *> m_gBuffers;
	std::vector<ID3D11ShaderResourceView *> m_gBufferSRVs;
	std::vector<ID3D11RenderTargetView *> m_gBufferRTVs;

	TwBar *m_settingsBar;

	// Shaders
	Common::VertexShader<Common::DefaultShaderConstantType, ForwardVertexShaderObjectConstants> *m_forwardVertexShader;
	Common::VertexShader<InstancedForwardVertexShaderFrameConstants, InstancedForwardVertexShaderObjectConstants> *m_instancedForwardVertexShader;
	Common::PixelShader<ForwardPixelShaderFrameConstants, ForwardPixelShaderObjectConstants> *m_forwardPixelShader;
	Common::VertexShader<Common::DefaultShaderConstantType, GBufferVertexShaderObjectConstants> *m_gbufferVertexShader;
	Common::VertexShader<InstancedGBufferVertexShaderFrameConstants, InstancedGBufferVertexShaderObjectConstants> *m_instancedGBufferVertexShader;
	Common::PixelShader<Common::DefaultShaderConstantType, GBufferPixelShaderObjectConstants> *m_gbufferPixelShader;
	Common::VertexShader<> *m_fullscreenTriangleVertexShader;
	Common::PixelShader<NoCullFinalGatherPixelShaderFrameConstants, Common::DefaultShaderConstantType> *m_noCullFinalGatherPixelShader;
	Common::VertexShader<> *m_debugObjectVertexShader;
	Common::PixelShader<> *m_debugObjectPixelShader;
	Common::VertexShader<Common::DefaultShaderConstantType, TransformedFullScreenTriangleVertexShaderConstants> *m_transformedFullscreenTriangleVertexShader;
	Common::PixelShader<RenderGBuffersPixelShaderConstants, Common::DefaultShaderConstantType> *m_renderGbuffersPixelShader;

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
	void LoadSceneJson();
	void InitTweakBar();
	void LoadShaders();
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

	void SetForwardVertexShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix);
	void SetInstancedForwardVertexShaderFrameConstants(DirectX::XMMATRIX &viewProjMatrix);
	void SetInstancedForwardVertexShaderObjectConstants(uint startIndex);
	void SetForwardPixelShaderFrameConstants();
	void SetForwardPixelShaderObjectConstants(const Common::BlinnPhongMaterial &material, uint textureFlags);
	void SetGBufferVertexShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix);
	void SetInstancedGBufferVertexShaderFrameConstants(DirectX::XMMATRIX &viewProjMatrix);
	void SetInstancedGBufferVertexShaderObjectConstants(uint startIndex);
	void SetGBufferPixelShaderConstants(const Common::BlinnPhongMaterial &material, uint textureFlags);
	void SetNoCullFinalGatherShaderConstants(DirectX::XMMATRIX &invViewProjMatrix);
	void SetRenderGBuffersPixelShaderConstants(DirectX::XMMATRIX &invViewProjMatrix, uint gBufferId);

	/** Maps the point light StructuredBuffer and the spot light Structured buffer to the pixel shader */
	void SetLightBuffers();
};

} // End of namespace ObjLoaderDemo

#endif
