/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
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

			SetupScene();
		}
		RenderMainPass();
		RenderDebugGeometry();
	} else {
		// Set the backbuffer as the main render target
		m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// Clear the render target view
		m_immediateContext->ClearRenderTargetView(m_renderTargetView, DirectX::Colors::LightGray);

		m_spriteRenderer.Begin(m_immediateContext);
		DirectX::XMFLOAT4X4 transform{2.0f, 0.0f, 0.0f, 0.0f,
		                              0.0f, 2.0f, 0.0f, 0.0f,
		                              0.0f, 0.0f, 2.0f, 0.0f,
		                              m_clientWidth / 2.0f - 90.0f, m_clientHeight / 2.0f - 30.0f, 0.0f, 1.0f};
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Scene is loading....", transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
		m_spriteRenderer.End();
	}
	RenderHUD();

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
	m_immediateContext->PSSetSamplers(0, 1, &m_diffuseSampleState);
	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.ReverseDepthWriteEnabled(), 0);
	ID3D11RasterizerState *rasterState = m_wireframe ? m_rasterizerStates.Wireframe() : m_rasterizerStates.BackFaceCull();
	m_immediateContext->RSSetState(rasterState);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_immediateContext->VSSetShader(m_forwardVertexShader, nullptr, 0);
	m_immediateContext->PSSetShader(m_forwardPixelShader, nullptr, 0);

	m_immediateContext->IASetInputLayout(m_gBufferInputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Fetch the transpose matricies
	DirectX::XMMATRIX worldMatrix = m_worldViewProj.world;
	DirectX::XMMATRIX viewMatrix = m_worldViewProj.view;
	DirectX::XMMATRIX projectionMatrix = m_worldViewProj.projection;

	// Cache the matrix multiplication
	DirectX::XMMATRIX viewProj = viewMatrix * projectionMatrix;

	SetForwardPixelShaderFrameConstants();

	// Set light buffers
	SetLightBuffers();

	if (m_pointLights.size() > 0) {
		ID3D11ShaderResourceView *srv = m_pointLightBuffer->GetShaderResource();
		m_immediateContext->PSSetShaderResources(3, 1, &srv);
	}
	if (m_spotLights.size() > 0) {
		ID3D11ShaderResourceView *srv = m_spotLightBuffer->GetShaderResource();
		m_immediateContext->PSSetShaderResources(4, 1, &srv);
	}

	for (uint i = 0; i < m_models.size(); ++i) {
		m_frameMaterialList.push_back(m_models[i].GetSubsetMaterial(0));

		DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(worldMatrix * viewProj);

		// GBuffer pass and Forward pass share the same Vertex cbPerFrame signature
		SetGBufferVertexShaderConstants(DirectX::XMMatrixTranspose(worldMatrix), worldViewProjection);

		SetForwardPixelShaderObjectConstants(m_models[i].GetSubsetMaterial(0));

		// Draw the models
		m_models[i].DrawSubset(m_immediateContext);
	}

	// Cleanup (aka make the runtime happy)
	m_immediateContext->VSSetShader(0, 0, 0);
	m_immediateContext->GSSetShader(0, 0, 0);
	m_immediateContext->PSSetShader(0, 0, 0);
	m_immediateContext->OMSetRenderTargets(0, 0, 0);
	ID3D11ShaderResourceView* nullSRV[6] = {0, 0, 0, 0, 0, 0};
	m_immediateContext->VSSetShaderResources(0, 6, nullSRV);
	m_immediateContext->PSSetShaderResources(0, 6, nullSRV);
}

void ObjLoaderDemo::SetForwardPixelShaderFrameConstants() {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_forwardPixelShaderFrameConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	ForwardPixelShaderFrameConstants *pixelShaderFrameConstants = static_cast<ForwardPixelShaderFrameConstants *>(mappedResource.pData);
	pixelShaderFrameConstants->gDirectionalLight = m_directionalLight;
	pixelShaderFrameConstants->gEyePosition = m_camera.GetCameraPosition();
	pixelShaderFrameConstants->gNumPointLightsToDraw = m_numPointLightsToDraw;
	pixelShaderFrameConstants->gNumSpotLightsToDraw = m_numSpotLightsToDraw;

	m_immediateContext->Unmap(m_forwardPixelShaderFrameConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(0, 1, &m_forwardPixelShaderFrameConstantsBuffer);
}

void ObjLoaderDemo::SetForwardPixelShaderObjectConstants(const Common::BlinnPhongMaterial &material) {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_forwardPixelShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	ForwardPixelShaderObjectConstants *pixelShaderObjectConstants = static_cast<ForwardPixelShaderObjectConstants *>(mappedResource.pData);
	pixelShaderObjectConstants->Material = material;

	m_immediateContext->Unmap(m_forwardPixelShaderObjectConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(1, 1, &m_forwardPixelShaderObjectConstantsBuffer);
}

void ObjLoaderDemo::NoCullDeferredRenderingPass() {
	// Clear the material list
	m_frameMaterialList.clear();

	// Bind the gbufferRTVs and depth/stencil view to the pipeline.
	m_immediateContext->OMSetRenderTargets(2, &m_gBufferRTVs[0], m_depthStencilBuffer->GetDepthStencil());

	// Clear the Render Targets and DepthStencil
	for (auto gbufferRTV : m_gBufferRTVs) {
		m_immediateContext->ClearRenderTargetView(gbufferRTV, DirectX::Colors::Black);
	}
	m_immediateContext->ClearDepthStencilView(m_depthStencilBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH, 0.0f, 0);

	// Set States
	m_immediateContext->PSSetSamplers(0, 1, &m_diffuseSampleState);
	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.ReverseDepthWriteEnabled(), 0);
	ID3D11RasterizerState *rasterState = m_wireframe ? m_rasterizerStates.Wireframe() : m_rasterizerStates.BackFaceCull();
	m_immediateContext->RSSetState(rasterState);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_immediateContext->VSSetShader(m_gbufferVertexShader, nullptr, 0);
	m_immediateContext->PSSetShader(m_gbufferPixelShader, nullptr, 0);

	m_immediateContext->IASetInputLayout(m_gBufferInputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Fetch the transpose matricies
	DirectX::XMMATRIX worldMatrix = m_worldViewProj.world;
	DirectX::XMMATRIX viewMatrix = m_worldViewProj.view;
	DirectX::XMMATRIX projectionMatrix = m_worldViewProj.projection;

	uint materialIndex = 0;

	// Cache the matrix multiplication
	DirectX::XMMATRIX viewProj = viewMatrix * projectionMatrix;

	for (uint i = 0; i < m_models.size(); ++i) {
		m_frameMaterialList.push_back(m_models[i].GetSubsetMaterial(0));

		DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(worldMatrix * viewProj);

		SetGBufferVertexShaderConstants(DirectX::XMMatrixTranspose(worldMatrix), worldViewProjection);
		SetGBufferPixelShaderConstants(m_frameMaterialList.size() - 1);

		// Draw the models
		m_models[i].DrawSubset(m_immediateContext);
	}

	// Cleanup (aka make the runtime happy)
	m_immediateContext->OMSetRenderTargets(0, 0, 0);

	
	// Final gather pass
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, DirectX::Colors::LightGray);

	// Full screen triangle setup
	m_immediateContext->IASetInputLayout(nullptr);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_immediateContext->VSSetShader(m_fullscreenTriangleVertexShader, nullptr, 0);
	m_immediateContext->GSSetShader(0, 0, 0);
	m_immediateContext->PSSetShader(m_noCullFinalGatherPixelShader, nullptr, 0);

	m_immediateContext->RSSetState(m_rasterizerStates.NoCull());

	DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj);
	SetNoCullFinalGatherShaderConstants(DirectX::XMMatrixTranspose(projectionMatrix), DirectX::XMMatrixTranspose(invViewProj));

	m_immediateContext->PSSetShaderResources(0, 3, &m_gBufferSRVs.front());

	// Set light buffers
	SetLightBuffers();

	if (m_pointLights.size() > 0) {
		ID3D11ShaderResourceView *srv = m_pointLightBuffer->GetShaderResource();
		m_immediateContext->PSSetShaderResources(3, 1, &srv);
	}
	if (m_spotLights.size() > 0) {
		ID3D11ShaderResourceView *srv = m_spotLightBuffer->GetShaderResource();
		m_immediateContext->PSSetShaderResources(4, 1, &srv);
	}

	// Set material list
	SetMaterialList();

	m_immediateContext->IASetVertexBuffers(0, 0, 0, 0, 0);
	m_immediateContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
	
	m_immediateContext->Draw(3, 0);


	// Cleanup (aka make the runtime happy)
	m_immediateContext->VSSetShader(0, 0, 0);
	m_immediateContext->GSSetShader(0, 0, 0);
	m_immediateContext->PSSetShader(0, 0, 0);
	m_immediateContext->OMSetRenderTargets(0, 0, 0);
	ID3D11ShaderResourceView* nullSRV[6] = {0, 0, 0, 0, 0, 0};
	m_immediateContext->VSSetShaderResources(0, 6, nullSRV);
	m_immediateContext->PSSetShaderResources(0, 6, nullSRV);
}

void ObjLoaderDemo::SetGBufferVertexShaderConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix) {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_gBufferVertexShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	GBufferVertexShaderObjectConstants *vertexShaderObjectConstants = static_cast<GBufferVertexShaderObjectConstants *>(mappedResource.pData);
	vertexShaderObjectConstants->World = worldMatrix;
	vertexShaderObjectConstants->WorldViewProj = worldViewProjMatrix;

	m_immediateContext->Unmap(m_gBufferVertexShaderObjectConstantsBuffer, 0);
	m_immediateContext->VSSetConstantBuffers(1, 1, &m_gBufferVertexShaderObjectConstantsBuffer);
}

void ObjLoaderDemo::SetGBufferPixelShaderConstants(uint materialIndex) {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_gBufferPixelShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	GBufferPixelShaderObjectConstants *pixelShaderObjectConstants = static_cast<GBufferPixelShaderObjectConstants *>(mappedResource.pData);
	pixelShaderObjectConstants->MaterialIndex = materialIndex;

	m_immediateContext->Unmap(m_gBufferPixelShaderObjectConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(1, 1, &m_gBufferPixelShaderObjectConstantsBuffer);
}

void ObjLoaderDemo::SetNoCullFinalGatherShaderConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix) {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_noCullFinalGatherPixelShaderConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	NoCullFinalGatherPixelShaderFrameConstants *pixelShaderFrameConstants = static_cast<NoCullFinalGatherPixelShaderFrameConstants *>(mappedResource.pData);
	pixelShaderFrameConstants->gProjection = projMatrix;
	pixelShaderFrameConstants->gInvViewProjection = invViewProjMatrix;
	pixelShaderFrameConstants->gDirectionalLight = m_directionalLight;
	pixelShaderFrameConstants->gEyePosition = m_camera.GetCameraPosition();
	pixelShaderFrameConstants->gNumPointLightsToDraw = m_numPointLightsToDraw;
	pixelShaderFrameConstants->gNumSpotLightsToDraw = m_numSpotLightsToDraw;

	m_immediateContext->Unmap(m_noCullFinalGatherPixelShaderConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(0, 1, &m_noCullFinalGatherPixelShaderConstantsBuffer);
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

void ObjLoaderDemo::SetMaterialList() {
	uint numMaterials = m_frameMaterialList.size();
	assert(numMaterials <= kMaxMaterialsPerFrame);
	
	Common::BlinnPhongMaterial *materialArray = m_frameMaterialListBuffer->MapDiscard(m_immediateContext);
	for (uint i = 0; i < numMaterials; ++i) {
		materialArray[i] = m_frameMaterialList[i];
	}
	m_frameMaterialListBuffer->Unmap(m_immediateContext);

	ID3D11ShaderResourceView *view = m_frameMaterialListBuffer->GetShaderResource();
	m_immediateContext->PSSetShaderResources(5, 1, &view);
}

void ObjLoaderDemo::RenderDebugGeometry() {
	DirectX::XMMATRIX viewMatrix = m_worldViewProj.view;
	DirectX::XMMATRIX projectionMatrix = m_worldViewProj.projection;

	// Cache the matrix multiplication
	DirectX::XMMATRIX viewProj = viewMatrix * projectionMatrix;
	DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj);	

	if (m_showLightLocations) {
		uint maxInstances;
		DebugObjectInstance *instances = m_debugSphere.MapInstanceBuffer(m_immediateContext, &maxInstances);

		for (uint i = 0; i < m_numPointLightsToDraw; ++i) {
			DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(m_pointLights[i].Position.x, m_pointLights[i].Position.y, m_pointLights[i].Position.z);
			instances[i].worldViewProj = DirectX::XMMatrixTranspose(translation * viewProj);
			instances[i].color = m_pointLights[i].Diffuse;
		}

		m_debugSphere.UnMapInstanceBuffer(m_immediateContext);

		instances = m_debugCone.MapInstanceBuffer(m_immediateContext, &maxInstances);

		DirectX::XMVECTOR xAxis = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR yAxis = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR zAxis = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
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
		m_immediateContext->VSSetShader(m_debugObjectVertexShader, nullptr, 0);
		m_immediateContext->PSSetShader(m_debugObjectPixelShader, nullptr, 0);

		m_immediateContext->IASetInputLayout(m_debugObjectInputLayout);
		m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_debugSphere.DrawInstancedSubset(m_immediateContext, m_debugSphereNumIndices, m_numPointLightsToDraw);
		m_debugCone.DrawInstancedSubset(m_immediateContext, m_debugConeNumIndices, m_numSpotLightsToDraw);
	}

	if (m_showGBuffers && m_shadingType == ShadingType::NoCullDeferred) {
		// Use the backbuffer render target
		m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// Set States
		m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.DepthDisabled(), 0);
		m_immediateContext->RSSetState(m_rasterizerStates.NoCull());

		m_immediateContext->IASetInputLayout(nullptr);
		m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_immediateContext->VSSetShader(m_transformedFullscreenTriangleVertexShader, nullptr, 0);
		m_immediateContext->GSSetShader(0, 0, 0);
		m_immediateContext->PSSetShader(m_renderGbuffersPixelShader, nullptr, 0);

		m_immediateContext->IASetVertexBuffers(0, 0, 0, 0, 0);
		m_immediateContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		m_immediateContext->PSSetShaderResources(0, 3, &m_gBufferSRVs.front());

		DirectX::XMFLOAT2 translations[6] = {DirectX::XMFLOAT2(-1.0f, -1.0f),
			DirectX::XMFLOAT2(-0.5f, -1.0f),
			DirectX::XMFLOAT2(0.0f, -1.0f),
			DirectX::XMFLOAT2(0.5f, -1.0f),
			DirectX::XMFLOAT2(0.5f, -0.5f),
			DirectX::XMFLOAT2(0.5f, 0.0f)};

		for (uint i = 0; i < 6; ++i) {
			// Fill in object constants
			D3D11_MAPPED_SUBRESOURCE mappedResource;

			HR(m_immediateContext->Map(m_renderGbuffersPixelShaderConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

			RenderGBuffersPixelShaderConstants *pixelShaderConstantsBuffer = static_cast<RenderGBuffersPixelShaderConstants *>(mappedResource.pData);
			pixelShaderConstantsBuffer->gProj = DirectX::XMMatrixTranspose(projectionMatrix);
			pixelShaderConstantsBuffer->gInvViewProjection = DirectX::XMMatrixTranspose(invViewProj);
			pixelShaderConstantsBuffer->gGBufferIndex = i;

			m_immediateContext->Unmap(m_renderGbuffersPixelShaderConstantsBuffer, 0);
			m_immediateContext->PSSetConstantBuffers(0, 1, &m_renderGbuffersPixelShaderConstantsBuffer);

			// Lock the constant buffer so it can be written to.
			HR(m_immediateContext->Map(m_transformedFullscreenTriangleVertexShaderConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

			TransformedFullScreenTriangleVertexShaderConstants *vertexShaderConstantsBuffer = static_cast<TransformedFullScreenTriangleVertexShaderConstants *>(mappedResource.pData);
			vertexShaderConstantsBuffer->gClipTranslation = translations[i];
			vertexShaderConstantsBuffer->gClipScale = 0.25f;

			m_immediateContext->Unmap(m_transformedFullscreenTriangleVertexShaderConstantsBuffer, 0);
			m_immediateContext->VSSetConstantBuffers(1, 1, &m_transformedFullscreenTriangleVertexShaderConstantsBuffer);

			m_immediateContext->Draw(6, 0);
		}

		// Render labels for each GBuffer part
		m_spriteRenderer.Begin(m_immediateContext);

		float quarterWidth = (float)m_clientWidth / 4.0f;
		float quarterHeight = (float)m_clientHeight / 4.0f;
		DirectX::XMFLOAT4X4 transform{1, 0, 0, 0,
		                              0, 1, 0, 0,
		                              0, 0, 1, 0,
		                              0, m_clientHeight - quarterHeight, 0, 1};
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Diffuse Albedo", transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);
		
		transform._41 = quarterWidth;
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"MaterialId", transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._41 = 2.0f * quarterWidth;
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Spherical Coord Normal", transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._41 = 3.0f * quarterWidth;
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Cartesian Coord Normal", transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._42 = m_clientHeight - (2.0f * quarterHeight);
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Z/W Depth", transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		transform._42 = m_clientHeight - (3.0f * quarterHeight);
		m_spriteRenderer.RenderText(m_timesNewRoman12Font, L"Position", transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);

		m_spriteRenderer.End();
	}
	
	// Cleanup (aka make the runtime happy)
	m_immediateContext->VSSetShader(0, 0, 0);
	m_immediateContext->GSSetShader(0, 0, 0);
	m_immediateContext->PSSetShader(0, 0, 0);
	m_immediateContext->OMSetRenderTargets(0, 0, 0);
	ID3D11ShaderResourceView* nullSRV[6] = {0, 0, 0, 0, 0, 0};
	m_immediateContext->VSSetShaderResources(0, 6, nullSRV);
	m_immediateContext->PSSetShaderResources(0, 6, nullSRV);
}

void ObjLoaderDemo::RenderHUD() {
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

	m_spriteRenderer.Begin(m_immediateContext, Common::SpriteRenderer::Point);
	std::wostringstream stream;
	stream << L"FPS: " << m_fps << L"\nFrame Time: " << m_frameTime << L" (ms)";

	DirectX::XMFLOAT4X4 transform{1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		25, 25, 0, 1};
	m_spriteRenderer.RenderText(m_timesNewRoman12Font, stream.str().c_str(), transform, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) /* Yellow */);
	m_spriteRenderer.End();

	TwDraw();

	// Cleanup (aka make the runtime happy)
	m_immediateContext->OMSetRenderTargets(0, 0, 0);
}

} // End of namespace ObjLoaderDemo
