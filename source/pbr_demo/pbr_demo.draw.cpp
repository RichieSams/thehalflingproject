/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "pbr_demo/pbr_demo.h"

#include "pbr_demo/shader_constants.h"
#include "pbr_demo/shader_defines.h"

#include <DirectXColors.h>

#include <fastformat/fastformat.hpp>
#include <fastformat/shims/conversion/filter_type/reals.hpp>


namespace PBRDemo {

void PBRDemo::DrawFrame(double deltaTime) {
	if (m_sceneLoaded.load(std::memory_order_relaxed)) {
		if (!m_sceneIsSetup) {
			// Clean-up the thread
			m_sceneLoaderThread.join();

			DirectX::XMVECTOR AABB_minXM = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			DirectX::XMVECTOR AABB_maxXM = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			// Get the AABB of the scene so we can calculate the scale factors for the camera movement
			for (auto iter = m_models.begin(); iter != m_models.end(); ++iter) {
				AABB_minXM = DirectX::XMVectorMin(AABB_minXM, DirectX::XMVectorScale(iter->first->GetAABBMin_XM(), m_sceneScaleFactor));
				AABB_maxXM = DirectX::XMVectorMax(AABB_maxXM, DirectX::XMVectorScale(iter->first->GetAABBMax_XM(), m_sceneScaleFactor));
			}
			for (auto iter = m_instancedModels.begin(); iter != m_instancedModels.end(); ++iter) {
				for (auto transformIter = iter->second->begin(); transformIter != iter->second->end(); ++transformIter) {
					AABB_minXM = DirectX::XMVectorMin(AABB_minXM, DirectX::XMVectorScale(DirectX::XMVector3Transform(iter->first->GetAABBMin_XM(), *transformIter), m_sceneScaleFactor));
					AABB_maxXM = DirectX::XMVectorMax(AABB_maxXM, DirectX::XMVectorScale(DirectX::XMVector3Transform(iter->first->GetAABBMax_XM(), *transformIter), m_sceneScaleFactor));
				}
			}

			DirectX::XMFLOAT3 AABB_min;
			DirectX::XMFLOAT3 AABB_max;
			DirectX::XMStoreFloat3(&AABB_min, AABB_minXM);
			DirectX::XMStoreFloat3(&AABB_max, AABB_maxXM);

			float min = std::min(std::min(AABB_min.x, AABB_min.y), AABB_min.z);
			float max = std::max(std::max(AABB_max.x, AABB_max.y), AABB_max.z);
			float range = max - min;

			m_cameraPanFactor = range * 0.0002857f;
			m_cameraScrollFactor = range * 0.0002857f;

			m_sceneIsSetup = true;
		}
		RenderMainPass();
		PostProcess();
	} else {
		// Set the backbuffer as the main render target
		m_immediateContext->OMSetRenderTargets(1, &m_backbufferRTV, nullptr);

		// Clear the render target view
		m_immediateContext->ClearRenderTargetView(m_backbufferRTV, DirectX::Colors::LightGray);

		m_spriteRenderer.Begin(m_immediateContext, Graphics::SpriteRenderer::Point);
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
		m_immediateContext->OMSetRenderTargets(1, &m_backbufferRTV, nullptr);
		m_console.Render(m_immediateContext, deltaTime);
	}

	uint syncInterval = m_vsync ? 1 : 0;
	m_swapChain->Present(syncInterval, 0);
}

void PBRDemo::RenderMainPass() {
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
	                                     m_samplerStates.PointWrap(),    // Alpha
	                                     m_samplerStates.Linear(),       // Displacement
	                                     m_samplerStates.Linear()};      // Normal
	m_immediateContext->PSSetSamplers(0, 6, samplerState);

	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.ReverseDepthWriteEnabled(), 0);
	ID3D11RasterizerState *rasterState = m_wireframe ? m_rasterizerStates.Wireframe() : m_rasterizerStates.BackFaceCull();
	m_immediateContext->RSSetState(rasterState);

	m_immediateContext->IASetInputLayout(m_defaultInputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Fetch the transpose matricies
	DirectX::XMMATRIX viewMatrix = m_camera.GetView();
	DirectX::XMMATRIX projectionMatrix = m_camera.GetProj();

	// Cache the matrix multiplication
	DirectX::XMMATRIX viewProj = viewMatrix * projectionMatrix;

	// Draw instanced models
	if (m_instancedModels.size() > 0) {
		DirectX::XMVECTOR *instanceBuffer = m_instanceBuffer->MapDiscard(m_immediateContext);
		std::vector<uint> offsets;
		uint bufferOffset = 0;
		for (auto iter = m_instancedModels.begin(); iter != m_instancedModels.end(); ++iter) {
			assert(bufferOffset < static_cast<uint>(m_instanceBuffer->NumElements()));

			// In the future when we support more complicated instancing, this could be something like:
			//
			// uint offset = bufferOffset;
			// std::vector<DirectX::XMVECTOR, Common::AllocatorAligned16<DirectX::XMVECTOR> > instanceVectors = iter->second.InstanceVectors();
			// for (auto vectorIter = instanceVectors.begin(); vectorIter != instanceVectors.end(); ++vectorIter) {
			//     instanceBuffer[bufferOffset++] = *vectorIter;
			// }
			// 
			// offsetAndLengths.emplace_back(offset);

			uint offset = bufferOffset;
			for (auto instanceIter = iter->second->begin(); instanceIter != iter->second->end(); ++instanceIter) {
				DirectX::XMMATRIX columnOrderMatrix = DirectX::XMMatrixTranspose(m_globalWorldTransform * (*instanceIter));
				instanceBuffer[bufferOffset++] = columnOrderMatrix.r[0];
				instanceBuffer[bufferOffset++] = columnOrderMatrix.r[1];
				instanceBuffer[bufferOffset++] = columnOrderMatrix.r[2];
			}

			offsets.emplace_back(offset);
		}

		m_instanceBuffer->Unmap(m_immediateContext);

		// Set the vertex shader and bind the instance buffer to it
		m_instancedGBufferVertexShader->BindToPipeline(m_immediateContext);
		ID3D11ShaderResourceView *srv = m_instanceBuffer->GetShaderResource();
		m_immediateContext->VSSetShaderResources(0, 1, &srv);

		// Set the vertex shader frame constants
		SetInstancedGBufferVertexShaderFrameConstants(DirectX::XMMatrixTranspose(viewProj));

		for (uint i = 0; i < m_instancedModels.size(); ++i) {
			SetInstancedGBufferVertexShaderObjectConstants(offsets[i]);

			m_instancedModels[i].first->DrawInstancedSubset(m_immediateContext, static_cast<uint>(m_instancedModels[i].second->size()), &m_materialShaderManager);
		}
	}

	// Draw non-instanced models
	if (m_models.size() > 0) {
		m_gbufferVertexShader->BindToPipeline(m_immediateContext);

		for (auto iter = m_models.begin(); iter != m_models.end(); ++iter) {
			DirectX::XMMATRIX combinedWorld = iter->second * m_globalWorldTransform;
			DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(combinedWorld);
			
			DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(combinedWorld * viewProj);

			// GBuffer pass and Forward pass share the same Vertex cbPerFrame signature
			SetGBufferVertexShaderObjectConstants(worldMatrix, worldViewProjection);

			// Draw the models
			iter->first->DrawSubset(m_immediateContext, &m_materialShaderManager);
		}
	}


	// Final gather pass

	// Full screen triangle setup
	m_immediateContext->IASetInputLayout(nullptr);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind the vertex shader
	m_fullscreenTriangleVertexShader->BindToPipeline(m_immediateContext);

	// Set light buffers
	SetLightBuffers();

	// Cache some matrix calculations
	DirectX::XMMATRIX transposedWorldViewMatrix = DirectX::XMMatrixTranspose(m_globalWorldTransform * viewMatrix);
	DirectX::XMMATRIX tranposedProjMatrix = DirectX::XMMatrixTranspose(projectionMatrix);
	DirectX::XMMATRIX transposedInvViewProj = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, viewProj));

	// Bind and clear the HDR output texture
	ID3D11UnorderedAccessView *hdrUAV = m_hdrOutput->GetUnorderedAccess();
	m_immediateContext->CSSetUnorderedAccessViews(0, 1, &hdrUAV, nullptr);
	m_immediateContext->ClearUnorderedAccessViewFloat(hdrUAV, DirectX::Colors::LightGray);

	// Free the gbuffers from the previous pixel shader so we can use them as SRVs
	ID3D11RenderTargetView *targets[3] = {nullptr, nullptr, nullptr};
	m_immediateContext->OMSetRenderTargets(3, targets, nullptr);

	// Bind the gbuffers to the compute shader
	m_immediateContext->CSSetShaderResources(0, 4, &m_gBufferSRVs.front());

	// Bind the shader and set the constant buffer variables
	m_tiledCullFinalGatherComputeShader->BindToPipeline(m_immediateContext);
	SetTiledCullFinalGatherShaderConstants(transposedWorldViewMatrix, tranposedProjMatrix, transposedInvViewProj);

	if (m_pointLights.size() > 0) {
		ID3D11ShaderResourceView *srv = m_pointLightBuffer->GetShaderResource();
		m_immediateContext->CSSetShaderResources(4, 1, &srv);
	}
	if (m_spotLights.size() > 0) {
		ID3D11ShaderResourceView *srv = m_spotLightBuffer->GetShaderResource();
		m_immediateContext->CSSetShaderResources(5, 1, &srv);
	}

	// Dispatch
	uint dispatchWidth = (m_clientWidth + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
	uint dispatchHeight = (m_clientHeight + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
	m_immediateContext->Dispatch(dispatchWidth, dispatchHeight, 1);

	// Clear gBuffer resource bindings so they can be used as render targets next frame
	ID3D11ShaderResourceView *views[4] = {nullptr, nullptr, nullptr, nullptr};
	m_immediateContext->CSSetShaderResources(0, 4, views);

	// Clear the HDR resource binding so it can be used in the PostProcessing
	ID3D11UnorderedAccessView *nullUAV = nullptr;
	m_immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
}

void PBRDemo::SetGBufferVertexShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix) {
	GBufferVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.World = worldMatrix;
	vertexShaderObjectConstants.WorldViewProj = worldViewProjMatrix;

	m_gbufferVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void PBRDemo::SetInstancedGBufferVertexShaderFrameConstants(DirectX::XMMATRIX &viewProjMatrix) {
	InstancedGBufferVertexShaderFrameConstants vertexShaderFrameConstants;
	vertexShaderFrameConstants.ViewProj = viewProjMatrix;

	m_instancedGBufferVertexShader->SetPerFrameConstants(m_immediateContext, &vertexShaderFrameConstants, 0u);
}

void PBRDemo::SetInstancedGBufferVertexShaderObjectConstants(uint startIndex) {
	InstancedGBufferVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.StartVector = startIndex;

	m_instancedGBufferVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void PBRDemo::SetTiledCullFinalGatherShaderConstants(DirectX::XMMATRIX &worldViewMatrix, DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix) {
	TiledCullFinalGatherComputeShaderFrameConstants computeShaderFrameConstants;
	computeShaderFrameConstants.WorldView = worldViewMatrix;
	computeShaderFrameConstants.Projection = projMatrix;
	computeShaderFrameConstants.InvViewProjection = invViewProjMatrix;
	computeShaderFrameConstants.DirectionalLight = m_directionalLight.GetShaderPackedLight();
	computeShaderFrameConstants.EyePosition = m_camera.GetCameraPosition();
	computeShaderFrameConstants.NumPointLightsToDraw = m_numPointLightsToDraw;
	computeShaderFrameConstants.CameraClipPlanes.x = m_nearClip;
	computeShaderFrameConstants.CameraClipPlanes.y = m_farClip;
	computeShaderFrameConstants.NumSpotLightsToDraw = m_numSpotLightsToDraw;
	
	m_tiledCullFinalGatherComputeShader->SetPerFrameConstants(m_immediateContext, &computeShaderFrameConstants, 0u);
}

void PBRDemo::SetLightBuffers() {
	if (m_numPointLightsToDraw > 0) {
		assert(m_pointLightBuffer->NumElements() >= (int)m_numPointLightsToDraw);

		Scene::ShaderPointLight *pointLightArray = m_pointLightBuffer->MapDiscard(m_immediateContext);
		for (unsigned int i = 0; i < m_numPointLightsToDraw; ++i) {
			pointLightArray[i] = m_pointLights[i].GetShaderPackedLight();
		}
		m_pointLightBuffer->Unmap(m_immediateContext);
	}

	if (m_numSpotLightsToDraw > 0) {
		assert(m_spotLightBuffer->NumElements() >= (int)m_numSpotLightsToDraw);

		Scene::ShaderSpotLight *spotLightArray = m_spotLightBuffer->MapDiscard(m_immediateContext);
		for (unsigned int i = 0; i < m_numSpotLightsToDraw; ++i) {
			spotLightArray[i] = m_spotLights[i].GetShaderPackedLight();
		}
		m_spotLightBuffer->Unmap(m_immediateContext);
	}
}

void PBRDemo::PostProcess() {
	m_immediateContext->OMSetRenderTargets(1, &m_backbufferRTV, nullptr);

	// No need to clear the backbuffer because we're writing to the entire thing

	// Full screen triangle setup
	m_immediateContext->IASetInputLayout(nullptr);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_fullscreenTriangleVertexShader->BindToPipeline(m_immediateContext);
	m_postProcessPixelShader->BindToPipeline(m_immediateContext);

	m_immediateContext->RSSetState(m_rasterizerStates.NoCull());
	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);

	m_immediateContext->IASetVertexBuffers(0, 0, 0, 0, 0);
	m_immediateContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Bind the HDR texture to the pixel shader
	ID3D11ShaderResourceView *hdrSRV = m_hdrOutput->GetShaderResource();
	m_immediateContext->PSSetShaderResources(0, 1, &hdrSRV);

	m_immediateContext->Draw(3, 0);

	// Clear hdr texture binding so it can be used as a render target next frame
	ID3D11ShaderResourceView *nullSRV[1] = {nullptr};
	m_immediateContext->PSSetShaderResources(0, 1, nullSRV);
}

void PBRDemo::RenderHUD() {
	m_immediateContext->OMSetRenderTargets(1, &m_backbufferRTV, nullptr);

	m_spriteRenderer.Begin(m_immediateContext, Graphics::SpriteRenderer::Point);
	std::wstring output;
	fastformat::write(output, L"FPS: ", m_fps, L"\nFrame Time: ", m_frameTime, L" (ms)");
	
	DirectX::XMFLOAT4X4 transform {1, 0, 0, 0,
	                               0, 1, 0, 0,
	                               0, 0, 1, 0,
	                               25, 25, 0, 1};
	m_spriteRenderer.RenderText(m_timesNewRoman12Font, output.c_str(), transform, 0U, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);
	m_spriteRenderer.End();

	TwDraw();
}

} // End of namespace PBRDemo
