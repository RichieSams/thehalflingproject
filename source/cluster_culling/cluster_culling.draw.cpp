/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "cluster_culling/cluster_culling.h"

#include "cluster_culling/shader_constants.h"
#include "cluster_culling/shader_defines.h"

#include <DirectXColors.h>

#include <fastformat/fastformat.hpp>
#include <fastformat/shims/conversion/filter_type/reals.hpp>


namespace ClusterCulling {

void ClusterCulling::DrawFrame(double deltaTime) {
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

void ClusterCulling::RenderMainPass() {
	// Bind the gbufferRTVs and depth/stencil view to the pipeline.
	m_immediateContext->OMSetRenderTargets(3, &m_gBufferRTVs[0], m_depthStencilBuffer->GetDepthStencil());

	// Clear the Render Targets and DepthStencil
	for (auto iter = m_gBufferRTVs.begin(); iter != m_gBufferRTVs.end(); ++iter) {
		m_immediateContext->ClearRenderTargetView((*iter), DirectX::Colors::Black);
	}
	m_immediateContext->ClearDepthStencilView(m_depthStencilBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH, 0.0f, 0);

	// Set States
	ID3D11SamplerState *samplerState[6] {m_samplerStateManager.Anisotropic(),  // Diffuse
	                                     m_samplerStateManager.Anisotropic(),  // Spec color
	                                     m_samplerStateManager.Linear(),       // Spec power
	                                     m_samplerStateManager.PointWrap(),    // Alpha
	                                     m_samplerStateManager.Linear(),       // Displacement
	                                     m_samplerStateManager.Linear()};      // Normal
	m_immediateContext->PSSetSamplers(0, 6, samplerState);

	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStateManager.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStateManager.ReverseDepthWriteEnabled(), 0);
	ID3D11RasterizerState *rasterState = m_wireframe ? m_rasterizerStateManager.Wireframe() : m_rasterizerStateManager.BackFaceCull();
	m_immediateContext->RSSetState(rasterState);

	m_immediateContext->IASetInputLayout(m_defaultInputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Fetch the transpose matricies
	DirectX::XMMATRIX viewMatrix = m_camera.GetView();
	DirectX::XMMATRIX projectionMatrix = m_camera.GetProj();

	// Cache the matrix multiplication
	DirectX::XMMATRIX viewProj = viewMatrix * projectionMatrix;

	DirectX::XMVECTOR temp = DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
	temp = DirectX::XMVector4Transform(temp, projectionMatrix);


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


	// Prepare the lights
	if (m_lightCullingPlanesNeedUpdate) {
		UpdateLightCullingPlanes();
		m_lightCullingPlanesNeedUpdate = false;
	}

	// Assign lights to clusters


	// Set light buffers
	SetLightBuffers();


	// Final gather pass

	// Full screen triangle setup
	m_immediateContext->IASetInputLayout(nullptr);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind the vertex shader
	m_fullscreenTriangleVertexShader->BindToPipeline(m_immediateContext);

	

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

void ClusterCulling::SetGBufferVertexShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix) {
	GBufferVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.World = worldMatrix;
	vertexShaderObjectConstants.WorldViewProj = worldViewProjMatrix;

	m_gbufferVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void ClusterCulling::SetInstancedGBufferVertexShaderFrameConstants(DirectX::XMMATRIX &viewProjMatrix) {
	InstancedGBufferVertexShaderFrameConstants vertexShaderFrameConstants;
	vertexShaderFrameConstants.ViewProj = viewProjMatrix;

	m_instancedGBufferVertexShader->SetPerFrameConstants(m_immediateContext, &vertexShaderFrameConstants, 0u);
}

void ClusterCulling::SetInstancedGBufferVertexShaderObjectConstants(uint startIndex) {
	InstancedGBufferVertexShaderObjectConstants vertexShaderObjectConstants;
	vertexShaderObjectConstants.StartVector = startIndex;

	m_instancedGBufferVertexShader->SetPerObjectConstants(m_immediateContext, &vertexShaderObjectConstants, 1u);
}

void ClusterCulling::SetTiledCullFinalGatherShaderConstants(DirectX::XMMATRIX &worldViewMatrix, DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix) {
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

void ClusterCulling::UpdateLightCullingPlanes() {
	const Common::float2 tileScale(float(m_clientWidth) / float(2u * COMPUTE_SHADER_TILE_GROUP_DIM), float(m_clientHeight) / float(2u * COMPUTE_SHADER_TILE_GROUP_DIM));
	
	DirectX::XMFLOAT4X4 projMatrix;
	DirectX::XMStoreFloat4x4(&projMatrix, m_camera.GetProj());

	const Common::float4 c4(0.0f, 0.0f, 1.0f, 0.0f);

	for (uint x = 0; x < m_lightCullingPlanes_X.size() - 1; x+=2) {
		float tileBiasX = tileScale.X - x;

		// Now work out composite projection matrix
		// Relevant matrix columns for this tile frusta
		Common::float4 c1(projMatrix._11 * tileScale.X, 0.0f, tileBiasX, 0.0f);

		m_lightCullingPlanes_X[x] = c4 + c1;
		m_lightCullingPlanes_X[x] /= Common::float3(m_lightCullingPlanes_X[x].X, m_lightCullingPlanes_X[x].Y, m_lightCullingPlanes_X[x].Z).Length();

		m_lightCullingPlanes_X[x + 1] = c4 - c1;
		m_lightCullingPlanes_X[x + 1] /= Common::float3(m_lightCullingPlanes_X[x + 1].X, m_lightCullingPlanes_X[x + 1].Y, m_lightCullingPlanes_X[x + 1].Z).Length();
	}

	for (uint y = 0; y < m_lightCullingPlanes_Y.size() - 1; y+=2) {
		float tileBiasY = tileScale.Y - y;

		// Now work out composite projection matrix
		// Relevant matrix columns for this tile frusta
		Common::float4 c2(0.0f, -projMatrix._22 * tileScale.Y, tileBiasY, 0.0f);

		m_lightCullingPlanes_Y[y] = c4 - c2;
		m_lightCullingPlanes_Y[y] /= Common::float3(m_lightCullingPlanes_Y[y].X, m_lightCullingPlanes_Y[y].Y, m_lightCullingPlanes_Y[y].Z).Length();

		m_lightCullingPlanes_Y[y + 1] = c4 + c2;
		m_lightCullingPlanes_Y[y + 1] /=  Common::float3(m_lightCullingPlanes_Y[y + 1].X, m_lightCullingPlanes_Y[y + 1].Y, m_lightCullingPlanes_Y[y + 1].Z).Length();
	}

	m_lightCullingPlanes_Z[0] = Common::float4(0.0f, 0.0f, 1.0f, 0.0f);
	for (uint z = 1; z < m_lightCullingPlanes_Z.size(); ++z) {
		m_lightCullingPlanes_Z[z] = Common::float4(0.0f, 0.0f, 1.0f, GetLinearDepthFromClusterId(z));
	}
}

#define DEPTH_CLUSTER_CALC_SCALE 8.740867046f
#define DEPTH_CLUSTER_CALC_BIAS -20.29566477f

uint ClusterCulling::GetDepthClusterId(float linearDepth) {
	return static_cast<uint>(std::max(std::log2(linearDepth) * DEPTH_CLUSTER_CALC_SCALE + DEPTH_CLUSTER_CALC_BIAS, 0.0f));
}

float ClusterCulling::GetLinearDepthFromClusterId(uint clusterId) {
	return std::pow(2, ((float)clusterId - DEPTH_CLUSTER_CALC_BIAS) / DEPTH_CLUSTER_CALC_SCALE);
}

void ClusterCulling::CalculateClusterLights() {
	// Iterate over each point light
	for (uint i = 0; i < m_pointLights.size(); ++i) {
		Scene::PointLight light = m_pointLights[i];

		// Get the 'cube' of clusters that surround the light
		DirectX::XMFLOAT3 position = light.GetPosition();
		DirectX::XMStoreFloat3(&position, DirectX::XMVector4Transform(DirectX::XMLoadFloat3(&position), m_camera.GetView()));

		// Iterate over z
		for (int z = 0; z < position.z - light.GetRange(); ++z) {

		}
	}

	//for (int z = z0; z <= z1; z++) {
	//    float4 z_light = light;
	//    if (z != center_z) {           // Use original in the middle, shrunken sphere otherwise
	//        const ZPlane &plane = (z < center_z)? z_planes[z + 1] : -z_planes[z];
	//        z_light = project_to_plane(z_light, plane);
	//    }
	//    for (int y = y0; y < y1; y++) {
	//        float3 y_light = z_light;
	//        if (y != center_y) {      // Use original in the middle, shrunken sphere otherwise
	//            const YPlane &plane = (y < center_y)? y_planes[y + 1] : -y_planes[y];
	//            y_light = project_to_plane(y_light, plane);
	//        }
	//        int x = x0;               // Scan from left until with hit the sphere
	//        do { ++x; } while (x < x1 && GetDistance(x_planes[x], y_light_pos) >= y_light_radius);
	//
	//        int xs = x1;              // Scan from right until with hit the sphere
	//        do { --xs; } while (xs >= x && -GetDistance(x_planes[xs], y_light_pos) >= y_light_radius);
	//
	//        for (--x; x <= xs; x++)   // Fill in the clusters in the range
	//            light_lists.AddPointLight(base_cluster + x, light_index);
	//    }
	//}
}

void ClusterCulling::SetLightBuffers() {
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

void ClusterCulling::PostProcess() {
	m_immediateContext->OMSetRenderTargets(1, &m_backbufferRTV, nullptr);

	// No need to clear the backbuffer because we're writing to the entire thing

	// Full screen triangle setup
	m_immediateContext->IASetInputLayout(nullptr);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_fullscreenTriangleVertexShader->BindToPipeline(m_immediateContext);
	m_postProcessPixelShader->BindToPipeline(m_immediateContext);

	m_immediateContext->RSSetState(m_rasterizerStateManager.NoCull());
	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStateManager.BlendDisabled(), blendFactor, 0xFFFFFFFF);

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

void ClusterCulling::RenderHUD() {
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

} // End of namespace ClusterCulling
