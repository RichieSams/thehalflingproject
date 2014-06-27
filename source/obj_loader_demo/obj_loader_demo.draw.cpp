/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_loader_demo/obj_loader_demo.h"

#include "obj_loader_demo/shader_constants.h"
#include "obj_loader_demo/shader_defines.h"

#include <DirectXColors.h>
#include <sstream>


namespace ObjLoaderDemo {

void ObjLoaderDemo::DrawFrame(double deltaTime) {
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
		RenderDebugGeometry();
		PostProcess();
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
	case ShadingType::TiledCullDeferred:
		DeferredRenderingPass();
		break;
	}
}

void ObjLoaderDemo::ForwardRenderingPass() {
	// Set the HDR output as the main render target
	ID3D11RenderTargetView *target = m_hdrOutput->GetRenderTarget();
	m_immediateContext->OMSetRenderTargets(1, &target, m_depthStencilBuffer->GetDepthStencil());

	// Clear the render target view and depth buffer
	m_immediateContext->ClearRenderTargetView(target, DirectX::Colors::LightGray);
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

	// Set the pixel shader
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

	m_immediateContext->IASetInputLayout(m_defaultInputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
		m_instancedForwardVertexShader->BindToPipeline(m_immediateContext);
		ID3D11ShaderResourceView *srv = m_instanceBuffer->GetShaderResource();
		m_immediateContext->VSSetShaderResources(0, 1, &srv);

		// Set the vertex shader frame constants
		SetInstancedForwardVertexShaderFrameConstants(DirectX::XMMatrixTranspose(viewProj));

		for (uint i = 0; i < m_instancedModels.size(); ++i) {
			SetInstancedForwardVertexShaderObjectConstants(offsets[i]);

			for (uint j = 0; j < m_instancedModels[i].first->GetSubsetCount(); ++j) {
				SetForwardPixelShaderObjectConstants(m_instancedModels[i].first->GetSubsetMaterial(j), m_instancedModels[i].first->GetSubsetTextureFlags(j));

				// Draw the models
				m_instancedModels[i].first->DrawInstancedSubset(m_immediateContext, m_instancedModels[i].second->size(), j);
			}
		}
	}

	// Draw non-instanced models
	if (m_models.size() > 0) {
		m_forwardVertexShader->BindToPipeline(m_immediateContext);

		for (auto iter = m_models.begin(); iter != m_models.end(); ++iter) {
			DirectX::XMMATRIX combinedWorld = iter->second * m_globalWorldTransform;
			DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(combinedWorld);

			DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(combinedWorld * viewProj);

			for (uint j = 0; j < iter->first->GetSubsetCount(); ++j) {
				SetForwardVertexShaderObjectConstants(worldMatrix, worldViewProjection);
				SetForwardPixelShaderObjectConstants(iter->first->GetSubsetMaterial(j), iter->first->GetSubsetTextureFlags(j));

				// Draw the models
				iter->first->DrawSubset(m_immediateContext, j);
			}
		}
	}
}

void ObjLoaderDemo::SetForwardVertexShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix) {
	ForwardVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.World = worldMatrix;
	vertexShaderObjectConstants.WorldViewProj = worldViewProjMatrix;

	m_forwardVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void ObjLoaderDemo::SetInstancedForwardVertexShaderFrameConstants(DirectX::XMMATRIX &viewProjMatrix) {
	InstancedForwardVertexShaderFrameConstants vertexShaderFrameConstants;
	vertexShaderFrameConstants.ViewProj = viewProjMatrix;

	m_instancedForwardVertexShader->SetPerFrameConstants(m_immediateContext, &vertexShaderFrameConstants, 0u);
}

void ObjLoaderDemo::SetInstancedForwardVertexShaderObjectConstants(uint startIndex) {
	InstancedForwardVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.StartVector = startIndex;

	m_instancedForwardVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void ObjLoaderDemo::SetForwardPixelShaderFrameConstants() {
	ForwardPixelShaderFrameConstants pixelShaderFrameConstants;
	pixelShaderFrameConstants.DirectionalLight = m_directionalLight;
	pixelShaderFrameConstants.EyePosition = m_camera.GetCameraPosition();
	pixelShaderFrameConstants.NumPointLightsToDraw = m_numPointLightsToDraw;
	pixelShaderFrameConstants.NumSpotLightsToDraw = m_numSpotLightsToDraw;

	m_forwardPixelShader->SetPerFrameConstants(m_immediateContext, &pixelShaderFrameConstants, 0u);
}

void ObjLoaderDemo::SetForwardPixelShaderObjectConstants(const Common::BlinnPhongMaterial &material, uint textureFlags) {
	ForwardPixelShaderObjectConstants pixelShaderObjectConstants;
	pixelShaderObjectConstants.Material = material;
	pixelShaderObjectConstants.TextureFlags = textureFlags;

	m_forwardPixelShader->SetPerObjectConstants(m_immediateContext, &pixelShaderObjectConstants, 1u);
}

void ObjLoaderDemo::DeferredRenderingPass() {
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

	// Set the pixel shader
	m_gbufferPixelShader->BindToPipeline(m_immediateContext);

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

			for (uint j = 0; j < m_instancedModels[i].first->GetSubsetCount(); ++j) {
				// Draw the models
				m_instancedModels[i].first->DrawInstancedSubset(m_immediateContext, m_instancedModels[i].second->size(), j);
			}
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

			for (uint j = 0; j < iter->first->GetSubsetCount(); ++j) {
				SetGBufferPixelShaderConstants(iter->first->GetSubsetMaterial(j), iter->first->GetSubsetTextureFlags(j));

				// Draw the models
				iter->first->DrawSubset(m_immediateContext, j);
			}
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

	if (m_shadingType == ShadingType::TiledCullDeferred) {
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
	} else if (m_shadingType == ShadingType::NoCullDeferred) {
		// Bind and clear the HDR output texture
		ID3D11RenderTargetView *targets[3] = {m_hdrOutput->GetRenderTarget(), nullptr, nullptr};
		m_immediateContext->OMSetRenderTargets(3, targets, nullptr);
		m_immediateContext->ClearRenderTargetView(targets[0], DirectX::Colors::LightGray);

		// Bind the gbuffers to the pixel shader
		m_immediateContext->PSSetShaderResources(0, 4, &m_gBufferSRVs.front());

		// Choose whether to actually calculate the lighting, or to render one of the GBuffers
		if (m_gbufferSelector == None) {
			// Bind the shader and set the constant buffer variables
			m_noCullFinalGatherPixelShader->BindToPipeline(m_immediateContext);
			SetNoCullFinalGatherShaderConstants(transposedInvViewProj);

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

			SetRenderGBuffersPixelShaderConstants(transposedInvViewProj, m_gbufferSelector);
		}

		m_immediateContext->RSSetState(m_rasterizerStates.NoCull());

		m_immediateContext->IASetVertexBuffers(0, 0, 0, 0, 0);
		m_immediateContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		m_immediateContext->Draw(3, 0);

		// Clear gBuffer resource bindings so they can be used as render targets next frame
		ID3D11ShaderResourceView *views[4] = {nullptr, nullptr, nullptr, nullptr};
		m_immediateContext->PSSetShaderResources(0, 4, views);

		// Clear the HDR resource binding so it can be used in the PostProcessing
		ID3D11RenderTargetView *nullRTV = nullptr;
		m_immediateContext->OMSetRenderTargets(1, &nullRTV, nullptr);
	}
}

void ObjLoaderDemo::SetGBufferVertexShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix) {
	GBufferVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.World = worldMatrix;
	vertexShaderObjectConstants.WorldViewProj = worldViewProjMatrix;

	m_gbufferVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void ObjLoaderDemo::SetInstancedGBufferVertexShaderFrameConstants(DirectX::XMMATRIX &viewProjMatrix) {
	InstancedGBufferVertexShaderFrameConstants vertexShaderFrameConstants;
	vertexShaderFrameConstants.ViewProj = viewProjMatrix;

	m_instancedGBufferVertexShader->SetPerFrameConstants(m_immediateContext, &vertexShaderFrameConstants, 0u);
}

void ObjLoaderDemo::SetInstancedGBufferVertexShaderObjectConstants(uint startIndex) {
	InstancedGBufferVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.StartVector = startIndex;

	m_instancedGBufferVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void ObjLoaderDemo::SetGBufferPixelShaderConstants(const Common::BlinnPhongMaterial &material, uint textureFlags) {
	GBufferPixelShaderObjectConstants pixelShaderObjectConstants;
	pixelShaderObjectConstants.Material = material;
	pixelShaderObjectConstants.TextureFlags = textureFlags;

	m_gbufferPixelShader->SetPerObjectConstants(m_immediateContext, &pixelShaderObjectConstants, 1u);
}

void ObjLoaderDemo::SetNoCullFinalGatherShaderConstants(DirectX::XMMATRIX &invViewProjMatrix) {
	NoCullFinalGatherPixelShaderFrameConstants pixelShaderFrameConstants;
	pixelShaderFrameConstants.InvViewProjection = invViewProjMatrix;
	pixelShaderFrameConstants.DirectionalLight = m_directionalLight;
	pixelShaderFrameConstants.EyePosition = m_camera.GetCameraPosition();
	pixelShaderFrameConstants.NumPointLightsToDraw = m_numPointLightsToDraw;
	pixelShaderFrameConstants.NumSpotLightsToDraw = m_numSpotLightsToDraw;

	m_noCullFinalGatherPixelShader->SetPerFrameConstants(m_immediateContext, &pixelShaderFrameConstants, 0u);
}

void ObjLoaderDemo::SetTiledCullFinalGatherShaderConstants(DirectX::XMMATRIX &worldViewMatrix, DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix) {
	TiledCullFinalGatherComputeShaderFrameConstants computeShaderFrameConstants;
	computeShaderFrameConstants.WorldView = worldViewMatrix;
	computeShaderFrameConstants.Projection = projMatrix;
	computeShaderFrameConstants.InvViewProjection = invViewProjMatrix;
	computeShaderFrameConstants.DirectionalLight = m_directionalLight;
	computeShaderFrameConstants.EyePosition = m_camera.GetCameraPosition();
	computeShaderFrameConstants.NumPointLightsToDraw = m_numPointLightsToDraw;
	computeShaderFrameConstants.CameraClipPlanes.x = m_nearClip;
	computeShaderFrameConstants.CameraClipPlanes.y = m_farClip;
	computeShaderFrameConstants.NumSpotLightsToDraw = m_numSpotLightsToDraw;
	
	m_tiledCullFinalGatherComputeShader->SetPerFrameConstants(m_immediateContext, &computeShaderFrameConstants, 0u);
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

		// Use the hdr render target and the original depth buffer
		ID3D11RenderTargetView *target = m_hdrOutput->GetRenderTarget();
		m_immediateContext->OMSetRenderTargets(1, &target, m_depthStencilBuffer->GetDepthStencil());

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
		ID3D11RenderTargetView *target = m_hdrOutput->GetRenderTarget();
		m_immediateContext->OMSetRenderTargets(1, &target, nullptr);

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

		DirectX::XMFLOAT2 translations[5] = {DirectX::XMFLOAT2(-1.0f, -1.0f),
		                                     DirectX::XMFLOAT2(-0.5f, -1.0f),
		                                     DirectX::XMFLOAT2(0.0f, -1.0f),
		                                     DirectX::XMFLOAT2(0.5f, -1.0f),
		                                     DirectX::XMFLOAT2(0.5f, -0.5f)};

		for (uint i = 0; i < 5; ++i) {
			TransformedFullScreenTriangleVertexShaderConstants vertexShaderConstantsBuffer;
			vertexShaderConstantsBuffer.gClipTranslation = translations[i];
			vertexShaderConstantsBuffer.gClipScale = 0.25f;

			m_transformedFullscreenTriangleVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderConstantsBuffer, 1u);

			SetRenderGBuffersPixelShaderConstants(invViewProj, i);

			m_immediateContext->Draw(6, 0);
		}

		// Clear gBuffer resource bindings so they can be used as render targets next frame
		ID3D11ShaderResourceView *views[4] = {nullptr, nullptr, nullptr, nullptr};
		m_immediateContext->PSSetShaderResources(0, 4, views);

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

		m_spriteRenderer.End();
	}
}

void ObjLoaderDemo::SetRenderGBuffersPixelShaderConstants(DirectX::XMMATRIX &invViewProjMatrix, uint gBufferId) {
	RenderGBuffersPixelShaderConstants pixelShaderConstantsBuffer;
	pixelShaderConstantsBuffer.gInvViewProjection = DirectX::XMMatrixTranspose(invViewProjMatrix);
	pixelShaderConstantsBuffer.gGBufferIndex = gBufferId;

	m_renderGbuffersPixelShader->SetPerFrameConstants(m_immediateContext, &pixelShaderConstantsBuffer, 0u);
}

void ObjLoaderDemo::PostProcess() {
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

	// No need to clear the backbuffer because we're writing to the entire thing

	// Full screen triangle setup
	m_immediateContext->IASetInputLayout(nullptr);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_fullscreenTriangleVertexShader->BindToPipeline(m_immediateContext);
	m_postProcessPixelShader->BindToPipeline(m_immediateContext);

	m_immediateContext->RSSetState(m_rasterizerStates.NoCull());

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
