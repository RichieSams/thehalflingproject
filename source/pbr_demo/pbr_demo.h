/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "halfling/halfling_engine.h"

#include "pbr_demo/shader_constants.h"

#include "common/vector.h"
#include "common/camera.h"
#include "common/texture_manager.h"
#include "common/model_manager.h"
#include "common/material_shader_manager.h"
#include "common/console.h"
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


namespace Common {
class Model;
class ModelToLoad;
}

namespace PBRDemo {

class PBRDemo : public Halfling::HalflingEngine {
public:
	PBRDemo(HINSTANCE hinstance);

private:
	static const uint kMaxInstanceVectorsPerFrame = 5000;

	float m_nearClip;
	float m_farClip;

	Common::Vector2 m_mouseLastPos;
	Common::Camera m_camera;
	float m_cameraPanFactor;
	float m_cameraScrollFactor;

	Common::TextureManager m_textureManager;
	Common::ModelManager m_modelManager;
	Common::MaterialShaderManager m_materialShaderManager;
	Common::Console m_console;
	bool m_showConsole;

	DirectX::XMMATRIX m_globalWorldTransform;

	std::vector<std::pair<Common::Model *, DirectX::XMMATRIX>, Common::Allocator16ByteAligned<std::pair<Common::Model *, DirectX::XMMATRIX> > > m_models;
	std::vector<std::pair<Common::Model *, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *> > m_instancedModels;

	Common::StructuredBuffer<DirectX::XMVECTOR> *m_instanceBuffer;

	std::vector<Common::ModelToLoad *> m_modelsToLoad;
	std::atomic<bool> m_sceneLoaded;
	bool m_sceneIsSetup;
	std::thread m_sceneLoaderThread;

	float m_sceneScaleFactor;
	uint m_modelInstanceThreshold;

	Common::DirectionalLight m_directionalLight;
	std::vector<Common::PointLight> m_pointLights;
	std::vector<Common::PointLightAnimator> m_pointLightAnimators;
	std::vector<Common::SpotLight> m_spotLights;
	std::vector<Common::SpotLightAnimator> m_spotLightAnimators;

	bool m_vsync;
	bool m_wireframe;
	bool m_animateLights;
	uint32 m_numSpotLightsToDraw;
	uint32 m_numPointLightsToDraw;

	ID3D11RenderTargetView *m_backbufferRTV;
	ID3D11InputLayout *m_defaultInputLayout;
	ID3D11InputLayout *m_debugObjectInputLayout;

	Common::Depth2D *m_depthStencilBuffer;
	D3D11_VIEWPORT m_screenViewport;

	Common::Texture2D *m_hdrOutput;

	std::vector<Common::Texture2D *> m_gBuffers;
	std::vector<ID3D11ShaderResourceView *> m_gBufferSRVs;
	std::vector<ID3D11RenderTargetView *> m_gBufferRTVs;

	TwBar *m_settingsBar;

	// Shaders
	Common::VertexShader<Common::DefaultShaderConstantType, GBufferVertexShaderObjectConstants> *m_gbufferVertexShader;
	Common::VertexShader<InstancedGBufferVertexShaderFrameConstants, InstancedGBufferVertexShaderObjectConstants> *m_instancedGBufferVertexShader;

	Common::VertexShader<> *m_fullscreenTriangleVertexShader;
	Common::ComputeShader<TiledCullFinalGatherComputeShaderFrameConstants, Common::DefaultShaderConstantType> *m_tiledCullFinalGatherComputeShader;

	Common::PixelShader<> *m_postProcessPixelShader;

	// Shader Buffers
	// We assume there is only one directional light. Therefore, it is stored in a cbuffer
	Common::StructuredBuffer<Common::PointLight> *m_pointLightBuffer;
	Common::StructuredBuffer<Common::SpotLight> *m_spotLightBuffer;

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

	void SetForwardVertexShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix);
	void SetInstancedForwardVertexShaderFrameConstants(DirectX::XMMATRIX &viewProjMatrix);
	void SetInstancedForwardVertexShaderObjectConstants(uint startIndex);
	void SetForwardPixelShaderFrameConstants();
	void SetForwardPixelShaderObjectConstants(const Common::BlinnPhongMaterial &material, uint textureFlags);

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
