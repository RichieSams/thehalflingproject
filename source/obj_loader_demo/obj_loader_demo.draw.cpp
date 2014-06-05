/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_loader_demo/obj_loader_demo.h"

#include "obj_loader_demo/shader_constants.h"

#include <DirectXColors.h>
#include <sstream>


namespace ObjLoaderDemo {

void ObjLoaderDemo::DrawFrame(double deltaTime) {
	if (m_sceneLoaded.load(std::memory_order_relaxed)) {
		if (!m_sceneIsSetup) {
			// Clean-up the thread
			m_sceneLoaderThread.join();

			m_sceneIsSetup = true;
		}
		RenderMainPass();
		RenderDebugGeometry();
	} else {
		// Set the backbuffer as the main render target
		m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// Clear the render target view
		m_immediateContext->ClearRenderTargetView(m_renderTargetView, DirectX::Colors::LightGray);

		m_spriteRenderer.Begin(m_immediateContext, Common::SpriteRenderer::Point);
		DirectX::XMFLOAT4X4 transform {2.0f, 0.0f, 0.0f, 0.0f,
		                               0.0f, 2.0f, 0.0f, 0.0f,
		                               0.0f, 0.0f, 2.0f, 0.0f,
		                               m_clientWidth / 2.0f - 90.0f, m_clientHeight / 2.0f - 30.0f, 0.0f, 1.0f};
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Scene is loading....", transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
		m_spriteRenderer.End();

		Sleep(50);
	}
	RenderHUD();

	if (m_showConsole) {
		m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
		m_console.Render(m_immediateContext, deltaTime);
	}

	uint syncInterval = m_vsync ? 1 : 0;
	m_swapChain->Present(syncInterval, 0);
}

void ObjLoaderDemo::RenderMainPass() {
	switch (m_shadingType) {
	case ShadingType::Forward:
		ForwardRenderingPass();
		break;
	case ShadingType::NoCullDeferred:
		NoCullDeferredRenderingPass();
		break;
	}
}

void ObjLoaderDemo::ForwardRenderingPass() {
	// Set the backbuffer as the main render target
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilBuffer->GetDepthStencil());

	// Clear the render target view and depth buffer
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, DirectX::Colors::LightGray);
	m_immediateContext->ClearDepthStencilView(m_depthStencilBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH, 0.0f, 0);

	// Set States
	ID3D11SamplerState *samplerState[6] {m_samplerStates.Anisotropic(),  // Diffuse
	                                     m_samplerStates.Anisotropic(),  // Spec color
	                                     m_samplerStates.Linear(),       // Spec power
	                                     m_samplerStates.Linear(),       // Alpha
	                                     m_samplerStates.Linear(),       // Displacement
	                                     m_samplerStates.Linear()};      // Normal
	m_immediateContext->PSSetSamplers(0, 6, samplerState);

	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.ReverseDepthWriteEnabled(), 0);
	ID3D11RasterizerState *rasterState = m_wireframe ? m_rasterizerStates.Wireframe() : m_rasterizerStates.BackFaceCull();
	m_immediateContext->RSSetState(rasterState);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_forwardVertexShader->BindToPipeline(m_immediateContext);
	m_forwardPixelShader->BindToPipeline(m_immediateContext);

	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Fetch the transpose matricies
	DirectX::XMMATRIX viewMatrix = m_camera.GetView();
	DirectX::XMMATRIX projectionMatrix = m_camera.GetProj();

	// Cache the matrix multiplication
	DirectX::XMMATRIX viewProj = viewMatrix * projectionMatrix;

	SetForwardPixelShaderFrameConstants();

	// Set light buffers
	SetLightBuffers();

	if (m_pointLights.size() > 0) {
		ID3D11ShaderResourceView *srv = m_pointLightBuffer->GetShaderResource();
		m_immediateContext->PSSetShaderResources(8, 1, &srv);
	}
	if (m_spotLights.size() > 0) {
		ID3D11ShaderResourceView *srv = m_spotLightBuffer->GetShaderResource();
		m_immediateContext->PSSetShaderResources(9, 1, &srv);
	}


	m_immediateContext->IASetInputLayout(m_gBufferInputLayout);

	for (auto iter = m_models.begin(); iter != m_models.end(); ++iter) {
		DirectX::XMMATRIX combinedWorld = iter->second * m_globalWorldTransform;
		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(combinedWorld);

		DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(combinedWorld * viewProj);

		// GBuffer pass and Forward pass share the same Vertex cbPerFrame signature
		SetGBufferVertexShaderConstants(worldMatrix, worldViewProjection);

		for (uint j = 0; j < iter->first->GetSubsetCount(); ++j) {
			SetForwardPixelShaderObjectConstants(iter->first->GetSubsetMaterial(j), iter->first->GetSubsetTextureFlags(j));

			// Draw the models
			iter->first->DrawSubset(m_immediateContext, j);
		}
	}
}

void ObjLoaderDemo::SetForwardPixelShaderFrameConstants() {
	ForwardPixelShaderFrameConstants pixelShaderFrameConstants;
	pixelShaderFrameConstants.gDirectionalLight = m_directionalLight;
	pixelShaderFrameConstants.gEyePosition = m_camera.GetCameraPosition();
	pixelShaderFrameConstants.gNumPointLightsToDraw = m_numPointLightsToDraw;
	pixelShaderFrameConstants.gNumSpotLightsToDraw = m_numSpotLightsToDraw;

	m_forwardPixelShader->SetPerFrameConstants(m_immediateContext, &pixelShaderFrameConstants, 0u);
}

void ObjLoaderDemo::SetForwardPixelShaderObjectConstants(const Common::BlinnPhongMaterial &material, uint textureFlags) {
	ForwardPixelShaderObjectConstants pixelShaderObjectConstants;
	pixelShaderObjectConstants.Material = material;
	pixelShaderObjectConstants.TextureFlags = textureFlags;

	m_forwardPixelShader->SetPerObjectConstants(m_immediateContext, &pixelShaderObjectConstants, 1u);
}

void ObjLoaderDemo::NoCullDeferredRenderingPass() {
	// Bind the gbufferRTVs and depth/stencil view to the pipeline.
	m_immediateContext->OMSetRenderTargets(3, &m_gBufferRTVs[0], m_depthStencilBuffer->GetDepthStencil());

	// Clear the Render Targets and DepthStencil
	for (auto iter = m_gBufferRTVs.begin(); iter != m_gBufferRTVs.end(); ++iter) {
		m_immediateContext->ClearRenderTargetView((*iter), DirectX::Colors::Black);
	}
	m_immediateContext->ClearDepthStencilView(m_depthStencilBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH, 0.0f, 0);

	// Set States
	ID3D11SamplerState *samplerState[6] {m_samplerStates.Anisotropic(),  // Diffuse
	                                     m_samplerStates.Anisotropic(),  // Spec color
	                                     m_samplerStates.Linear(),       // Spec power
	                                     m_samplerStates.Linear(),       // Alpha
	                                     m_samplerStates.Linear(),       // Displacement
	                                     m_samplerStates.Linear()};      // Normal
	m_immediateContext->PSSetSamplers(0, 6, samplerState);

	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.ReverseDepthWriteEnabled(), 0);
	ID3D11RasterizerState *rasterState = m_wireframe ? m_rasterizerStates.Wireframe() : m_rasterizerStates.BackFaceCull();
	m_immediateContext->RSSetState(rasterState);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_gbufferVertexShader->BindToPipeline(m_immediateContext);
	m_gbufferPixelShader->BindToPipeline(m_immediateContext);

	m_immediateContext->IASetInputLayout(m_gBufferInputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Fetch the transpose matricies
	DirectX::XMMATRIX viewMatrix = m_camera.GetView();
	DirectX::XMMATRIX projectionMatrix = m_camera.GetProj();

	// Cache the matrix multiplication
	DirectX::XMMATRIX viewProj = viewMatrix * projectionMatrix;

	for (auto iter = m_models.begin(); iter != m_models.end(); ++iter) {
		DirectX::XMMATRIX combinedWorld = iter->second * m_globalWorldTransform;
		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(combinedWorld);
			
		DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(combinedWorld * viewProj);

		// GBuffer pass and Forward pass share the same Vertex cbPerFrame signature
		SetGBufferVertexShaderConstants(worldMatrix, worldViewProjection);

		for (uint j = 0; j < iter->first->GetSubsetCount(); ++j) {
			SetGBufferPixelShaderConstants(iter->first->GetSubsetMaterial(j), iter->first->GetSubsetTextureFlags(j));

			// Draw the models
			iter->first->DrawSubset(m_immediateContext, j);
		}
	}

	// Final gather pass
	ID3D11RenderTargetView *targets[3] = {m_renderTargetView, nullptr, nullptr};
	m_immediateContext->OMSetRenderTargets(3, targets, nullptr);
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, DirectX::Colors::LightGray);

	// Full screen triangle setup
	m_immediateContext->IASetInputLayout(nullptr);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_fullscreenTriangleVertexShader->BindToPipeline(m_immediateContext);
	m_immediateContext->GSSetShader(0, 0, 0);

	m_immediateContext->PSSetShaderResources(0, 4, &m_gBufferSRVs.front());

	DirectX::XMMATRIX projMatrix = DirectX::XMMatrixTranspose(projectionMatrix);
	DirectX::XMMATRIX invViewProj = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, viewProj));

	if (m_gbufferSelector == None) {
		m_noCullFinalGatherPixelShader->BindToPipeline(m_immediateContext);

		SetNoCullFinalGatherShaderConstants(projMatrix, invViewProj);

		// Set light buffers
		SetLightBuffers();

		if (m_pointLights.size() > 0) {
			ID3D11ShaderResourceView *srv = m_pointLightBuffer->GetShaderResource();
			m_immediateContext->PSSetShaderResources(4, 1, &srv);
		}
		if (m_spotLights.size() > 0) {
			ID3D11ShaderResourceView *srv = m_spotLightBuffer->GetShaderResource();
			m_immediateContext->PSSetShaderResources(5, 1, &srv);
		}
	} else {
		m_renderGbuffersPixelShader->BindToPipeline(m_immediateContext);

		SetRenderGBuffersPixelShaderConstants(projMatrix, invViewProj, m_gbufferSelector);
	}

	m_immediateContext->RSSetState(m_rasterizerStates.NoCull());

	m_immediateContext->IASetVertexBuffers(0, 0, 0, 0, 0);
	m_immediateContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	m_immediateContext->Draw(3, 0);

	// Clear gBuffer resource bindings so they can be used as render targets next frame
	ID3D11ShaderResourceView *views[4] = {nullptr, nullptr, nullptr, nullptr};
	m_immediateContext->PSSetShaderResources(0, 4, views);
}

void ObjLoaderDemo::SetGBufferVertexShaderConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix) {
	GBufferVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.World = worldMatrix;
	vertexShaderObjectConstants.WorldViewProj = worldViewProjMatrix;

	m_gbufferVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void ObjLoaderDemo::SetGBufferPixelShaderConstants(const Common::BlinnPhongMaterial &material, uint textureFlags) {
	GBufferPixelShaderObjectConstants pixelShaderObjectConstants;
	pixelShaderObjectConstants.Material = material;
	pixelShaderObjectConstants.TextureFlags = textureFlags;

	m_gbufferPixelShader->SetPerObjectConstants(m_immediateContext, &pixelShaderObjectConstants, 1u);
}

void ObjLoaderDemo::SetNoCullFinalGatherShaderConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix) {
	NoCullFinalGatherPixelShaderFrameConstants pixelShaderFrameConstants;
	pixelShaderFrameConstants.gProjection = projMatrix;
	pixelShaderFrameConstants.gInvViewProjection = invViewProjMatrix;
	pixelShaderFrameConstants.gDirectionalLight = m_directionalLight;
	pixelShaderFrameConstants.gEyePosition = m_camera.GetCameraPosition();
	pixelShaderFrameConstants.gNumPointLightsToDraw = m_numPointLightsToDraw;
	pixelShaderFrameConstants.gNumSpotLightsToDraw = m_numSpotLightsToDraw;

	m_noCullFinalGatherPixelShader->SetPerFrameConstants(m_immediateContext, &pixelShaderFrameConstants, 0u);
}

void ObjLoaderDemo::SetLightBuffers() {
	if (m_numPointLightsToDraw > 0) {
		assert(m_pointLightBuffer->NumElements() >= (int)m_numPointLightsToDraw);

		Common::PointLight *pointLightArray = m_pointLightBuffer->MapDiscard(m_immediateContext);
		for (unsigned int i = 0; i < m_numPointLightsToDraw; ++i) {
			pointLightArray[i] = m_pointLights[i];
		}
		m_pointLightBuffer->Unmap(m_immediateContext);
	}

	if (m_numSpotLightsToDraw > 0) {
		assert(m_spotLightBuffer->NumElements() >= (int)m_numSpotLightsToDraw);

		Common::SpotLight *spotLightArray = m_spotLightBuffer->MapDiscard(m_immediateContext);
		for (unsigned int i = 0; i < m_numSpotLightsToDraw; ++i) {
			spotLightArray[i] = m_spotLights[i];
		}
		m_spotLightBuffer->Unmap(m_immediateContext);
	}
}

void ObjLoaderDemo::RenderDebugGeometry() {
	DirectX::XMMATRIX viewMatrix = m_camera.GetView();
	DirectX::XMMATRIX projectionMatrix = m_camera.GetProj();

	// Cache the matrix multiplication
	DirectX::XMMATRIX viewProj = viewMatrix * projectionMatrix;
	DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj);

	if (m_showLightLocations) {
		uint maxInstances;
		DebugObjectInstance *instances = (DebugObjectInstance *)m_debugSphere.MapInstanceBuffer(m_immediateContext, &maxInstances);

		for (uint i = 0; i < m_numPointLightsToDraw; ++i) {
			DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(m_pointLights[i].Position.x, m_pointLights[i].Position.y, m_pointLights[i].Position.z);
			instances[i].worldViewProj = DirectX::XMMatrixTranspose(translation * viewProj);
			instances[i].color = m_pointLights[i].Diffuse;
		}

		m_debugSphere.UnMapInstanceBuffer(m_immediateContext);

		instances = (DebugObjectInstance *)m_debugCone.MapInstanceBuffer(m_immediateContext, &maxInstances);

		DirectX::XMVECTOR yAxis = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		for (uint i = 0; i < m_numSpotLightsToDraw; ++i) {
			DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(m_spotLights[i].Position.x, m_spotLights[i].Position.y, m_spotLights[i].Position.z);

			DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&m_spotLights[i].Direction);

			DirectX::XMVECTOR crossProduct = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(direction, yAxis));
			float angle = acos(DirectX::XMVector3Dot(direction, yAxis).m128_f32[0]);
			DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationNormal(crossProduct, -angle);

			instances[i].worldViewProj = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(quaternion) * translation * viewProj);
			instances[i].color = m_spotLights[i].Diffuse;
		}

		m_debugCone.UnMapInstanceBuffer(m_immediateContext);

		// Use the backbuffer render target and the original depth buffer
		m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilBuffer->GetDepthStencil());

		// Set States
		m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.ReverseDepthEnabled(), 0);
		m_immediateContext->RSSetState(m_rasterizerStates.BackFaceCull());

		// Set the vertex and pixel shaders that will be used to render this triangle.
		m_debugObjectVertexShader->BindToPipeline(m_immediateContext);
		m_debugObjectPixelShader->BindToPipeline(m_immediateContext);

		m_immediateContext->IASetInputLayout(m_debugObjectInputLayout);
		m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_debugSphere.DrawInstancedSubset(m_immediateContext, m_numPointLightsToDraw);
		m_debugCone.DrawInstancedSubset(m_immediateContext, m_numSpotLightsToDraw);
	}

	if (m_showGBuffers && m_shadingType == ShadingType::NoCullDeferred) {
		// Use the backbuffer render target
		m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// Set States
		m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.DepthDisabled(), 0);
		m_immediateContext->RSSetState(m_rasterizerStates.NoCull());

		m_immediateContext->IASetInputLayout(nullptr);
		m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_transformedFullscreenTriangleVertexShader->BindToPipeline(m_immediateContext);
		m_renderGbuffersPixelShader->BindToPipeline(m_immediateContext);

		m_immediateContext->IASetVertexBuffers(0, 0, 0, 0, 0);
		m_immediateContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		m_immediateContext->PSSetShaderResources(0, 4, &m_gBufferSRVs.front());

		DirectX::XMFLOAT2 translations[6] = {DirectX::XMFLOAT2(-1.0f, -1.0f),
		                                     DirectX::XMFLOAT2(-0.5f, -1.0f),
		                                     DirectX::XMFLOAT2(0.0f, -1.0f),
		                                     DirectX::XMFLOAT2(0.5f, -1.0f),
		                                     DirectX::XMFLOAT2(0.5f, -0.5f),
		                                     DirectX::XMFLOAT2(0.5f, 0.0f)};

		for (uint i = 0; i < 6; ++i) {
			TransformedFullScreenTriangleVertexShaderConstants vertexShaderConstantsBuffer;
			vertexShaderConstantsBuffer.gClipTranslation = translations[i];
			vertexShaderConstantsBuffer.gClipScale = 0.25f;

			m_transformedFullscreenTriangleVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderConstantsBuffer, 1u);

			SetRenderGBuffersPixelShaderConstants(projectionMatrix, invViewProj, i);

			m_immediateContext->Draw(6, 0);
		}

		// Render labels for each GBuffer part
		m_spriteRenderer.Begin(m_immediateContext, Common::SpriteRenderer::Point);

		float quarterWidth = (float)m_clientWidth / 4.0f;
		float quarterHeight = (float)m_clientHeight / 4.0f;
		DirectX::XMFLOAT4X4 transform {1, 0, 0, 0,
		                               0, 1, 0, 0,
		                               0, 0, 1, 0,
		                               0, m_clientHeight - quarterHeight, 0, 1};
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Diffuse Albedo", transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._41 = quarterWidth;
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Specular Albedo", transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._41 = 2.0f * quarterWidth;
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Spherical Coord Normal", transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._41 = 3.0f * quarterWidth;
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Cartesian Coord Normal", transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._42 = m_clientHeight - (2.0f * quarterHeight);
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Z/W Depth", transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._42 = m_clientHeight - (3.0f * quarterHeight);
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Position", transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		m_spriteRenderer.End();
	}
}

void ObjLoaderDemo::SetRenderGBuffersPixelShaderConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix, uint gBufferId) {
	RenderGBuffersPixelShaderConstants pixelShaderConstantsBuffer;
	pixelShaderConstantsBuffer.gProj = DirectX::XMMatrixTranspose(projMatrix);
	pixelShaderConstantsBuffer.gInvViewProjection = DirectX::XMMatrixTranspose(invViewProjMatrix);
	pixelShaderConstantsBuffer.gGBufferIndex = gBufferId;

	m_renderGbuffersPixelShader->SetPerFrameConstants(m_immediateContext, &pixelShaderConstantsBuffer, 0u);
}

void ObjLoaderDemo::RenderHUD() {
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

	m_spriteRenderer.Begin(m_immediateContext, Common::SpriteRenderer::Point);
	std::wostringstream stream;
	stream << L"FPS: " << m_fps << L"\nFrame Time: " << m_frameTime << L" (ms)";
	
	DirectX::XMFLOAT4X4 transform {1, 0, 0, 0,
	                               0, 1, 0, 0,
	                               0, 0, 1, 0,
	                               25, 25, 0, 1};
	m_spriteRenderer.RenderText(m_timesNewRoman12Font, stream.str().c_str(), transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);
	m_spriteRenderer.End();

	TwDraw();
}

} // End of namespace ObjLoaderDemo
