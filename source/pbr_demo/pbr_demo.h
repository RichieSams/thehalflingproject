/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "engine/halfling_engine.h"

#include "pbr_demo/shader_constants.h"

#include "common/vector.h"
#include "scene/camera.h"
#include "engine/texture_manager.h"
#include "engine/model_manager.h"
#include "engine/material_shader_manager.h"
#include "engine/console.h"
#include "graphics/texture2d.h"
#include "graphics/structured_buffer.h"
#include "graphics/device_states.h"
#include "graphics/sprite_renderer.h"
#include "graphics/sprite_font.h"
#include "scene/lights.h"
#include "scene/light_animator.h"
#include "common/allocator_16_byte_aligned.h"
#include "graphics/shader.h"

#include <vector>
#include <AntTweakBar.h>
#include <atomic>
#include <thread>


namespace Scene {
class Model;
class ModelToLoad;
}

namespace PBRDemo {

class PBRDemo : public Engine::HalflingEngine {
public:
	PBRDemo(HINSTANCE hinstance);

private:
	static const uint kMaxInstanceVectorsPerFrame = 5000;

	float m_nearClip;
	float m_farClip;

	Common::Vector2 m_mouseLastPos;
	Scene::Camera m_camera;
	float m_cameraPanFactor;
	float m_cameraScrollFactor;

	Engine::TextureManager m_textureManager;
	Engine::ModelManager m_modelManager;
	Engine::MaterialShaderManager m_materialShaderManager;
	Engine::Console m_console;
	bool m_showConsole;

	DirectX::XMMATRIX m_globalWorldTransform;

	std::vector<std::pair<Scene::Model *, DirectX::XMMATRIX>, Common::Allocator16ByteAligned<std::pair<Scene::Model *, DirectX::XMMATRIX> > > m_models;
	std::vector<std::pair<Scene::Model *, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *> > m_instancedModels;

	Graphics::StructuredBuffer<DirectX::XMVECTOR> *m_instanceBuffer;

	std::vector<Scene::ModelToLoad *> m_modelsToLoad;
	std::atomic<bool> m_sceneLoaded;
	bool m_sceneIsSetup;
	std::thread m_sceneLoaderThread;

	float m_sceneScaleFactor;
	uint m_modelInstanceThreshold;

	Scene::DirectionalLight m_directionalLight;
	std::vector<Scene::PointLight> m_pointLights;
	std::vector<Scene::PointLightAnimator> m_pointLightAnimators;
	std::vector<Scene::SpotLight> m_spotLights;
	std::vector<Scene::SpotLightAnimator> m_spotLightAnimators;

	bool m_vsync;
	bool m_wireframe;
	bool m_animateLights;
	uint32 m_numSpotLightsToDraw;
	uint32 m_numPointLightsToDraw;

	ID3D11RenderTargetView *m_backbufferRTV;
	ID3D11InputLayout *m_defaultInputLayout;
	ID3D11InputLayout *m_debugObjectInputLayout;

	Graphics::Depth2D *m_depthStencilBuffer;
	D3D11_VIEWPORT m_screenViewport;

	Graphics::Texture2D *m_hdrOutput;

	std::vector<Graphics::Texture2D *> m_gBuffers;
	std::vector<ID3D11ShaderResourceView *> m_gBufferSRVs;
	std::vector<ID3D11RenderTargetView *> m_gBufferRTVs;

	TwBar *m_settingsBar;

	// Shaders
	Graphics::VertexShader<Graphics::DefaultShaderConstantType, GBufferVertexShaderObjectConstants> *m_gbufferVertexShader;
	Graphics::VertexShader<InstancedGBufferVertexShaderFrameConstants, InstancedGBufferVertexShaderObjectConstants> *m_instancedGBufferVertexShader;

	Graphics::VertexShader<> *m_fullscreenTriangleVertexShader;
	Graphics::ComputeShader<TiledCullFinalGatherComputeShaderFrameConstants, Graphics::DefaultShaderConstantType> *m_tiledCullFinalGatherComputeShader;

	Graphics::PixelShader<> *m_postProcessPixelShader;

	// Shader Buffers
	// We assume there is only one directional light. Therefore, it is stored in a cbuffer
	Graphics::StructuredBuffer<Scene::ShaderPointLight> *m_pointLightBuffer;
	Graphics::StructuredBuffer<Scene::ShaderSpotLight> *m_spotLightBuffer;

	Graphics::BlendStates m_blendStates;
	Graphics::DepthStencilStates m_depthStencilStates;
	Graphics::RasterizerStates m_rasterizerStates;
	Graphics::SamplerStates m_samplerStates;

	Graphics::SpriteRenderer m_spriteRenderer;
	Graphics::SpriteFont m_timesNewRoman12Font;
	Graphics::SpriteFont m_courierNew10Font;

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
	void CreateLights(const DirectX::XMFLOAT3 &sceneSizeMin, const DirectX::XMFLOAT3 &sceneSizeMax);

	// Rendering methods
	/** Renders the geometry */
	void RenderMainPass();
	/** Renders the geometry using Deferred Shading */
	void DeferredRenderingPass();
	/** Does the post processing for the frame */
	void PostProcess();
	/** Renders the frame statistics and the settings bar */
	void RenderHUD();

	void SetGBufferVertexShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix);
	void SetInstancedGBufferVertexShaderFrameConstants(DirectX::XMMATRIX &viewProjMatrix);
	void SetInstancedGBufferVertexShaderObjectConstants(uint startIndex);

	void SetNoCullFinalGatherShaderConstants(DirectX::XMMATRIX &invViewProjMatrix);
	void SetTiledCullFinalGatherShaderConstants(DirectX::XMMATRIX &viewMatrix, DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix);

	void SetRenderGBuffersPixelShaderConstants(DirectX::XMMATRIX &invViewProjMatrix, uint gBufferId);

	/** Maps the point light StructuredBuffer and the spot light Structured buffer to the pixel shader */
	void SetLightBuffers();
};

} // End of namespace PBRDemo
