/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "deferred_shading_demo/deferred_shading_demo.h"

#include "deferred_shading_demo/shader_constants.h"

#include <DirectXColors.h>
#include <sstream>


namespace DeferredShadingDemo {

void DeferredShadingDemo::DrawFrame(double deltaTime) {
	RenderMainPass();
	RenderHUD();

	uint syncInterval = m_vsync ? 1 : 0;
	m_swapChain->Present(syncInterval, 0);
}

void DeferredShadingDemo::RenderMainPass() {
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

		SetGBufferShaderObjectConstants(DirectX::XMMatrixTranspose(worldMatrix), worldViewProjection, m_frameMaterialList.size() - 1);
	
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
		m_immediateContext->PSGetShaderResources(4, 1, &srv);
	}

	// Set material list
	SetMaterialList();

	m_immediateContext->IASetVertexBuffers(0, 0, 0, 0, 0);
	m_immediateContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
	
	m_immediateContext->Draw(3, 0);


	// Debug pass

	uint maxInstances;
	DebugObjectInstance *instances = m_debugSphere.MapInstanceBuffer(m_immediateContext, &maxInstances);

	DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
	for (uint i = 0; i < m_pointLights.size(); ++i) {
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(m_pointLights[i].Position.x, m_pointLights[i].Position.y, m_pointLights[i].Position.z);
		instances[i].worldViewProj = DirectX::XMMatrixTranspose(translation * viewProj);
		instances[i].color = m_pointLights[i].Diffuse;
	}

	m_debugSphere.UnMapInstanceBuffer(m_immediateContext);

	// Use the backbuffer render target and the original depth buffer
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilBuffer->GetDepthStencil());

	// Set States
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.ReverseDepthWriteEnabled(), 0);
	m_immediateContext->RSSetState(m_rasterizerStates.BackFaceCull());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_immediateContext->VSSetShader(m_debugObjectVertexShader, nullptr, 0);
	m_immediateContext->PSSetShader(m_debugObjectPixelShader, nullptr, 0);

	m_immediateContext->IASetInputLayout(m_debugObjectInputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_debugSphere.DrawInstancedSubset(m_immediateContext, m_debugSphereNumIndices, m_pointLights.size());

	// Cleanup (aka make the runtime happy)
	m_immediateContext->VSSetShader(0, 0, 0);
	m_immediateContext->GSSetShader(0, 0, 0);
	m_immediateContext->PSSetShader(0, 0, 0);
	m_immediateContext->OMSetRenderTargets(0, 0, 0);
	ID3D11ShaderResourceView* nullSRV[6] = {0, 0, 0, 0, 0, 0};
	m_immediateContext->VSSetShaderResources(0, 6, nullSRV);
	m_immediateContext->PSSetShaderResources(0, 6, nullSRV);
}

void DeferredShadingDemo::SetGBufferShaderObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix, uint materialIndex) {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_gBufferVertexShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	GBufferVertexShaderObjectConstants *vertexShaderObjectConstants = static_cast<GBufferVertexShaderObjectConstants *>(mappedResource.pData);
	vertexShaderObjectConstants->World = worldMatrix;
	vertexShaderObjectConstants->WorldViewProj = worldViewProjMatrix;

	m_immediateContext->Unmap(m_gBufferVertexShaderObjectConstantsBuffer, 0);
	m_immediateContext->VSSetConstantBuffers(1, 1, &m_gBufferVertexShaderObjectConstantsBuffer);

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_gBufferPixelShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	GBufferPixelShaderObjectConstants *pixelShaderObjectConstants = static_cast<GBufferPixelShaderObjectConstants *>(mappedResource.pData);
	pixelShaderObjectConstants->MaterialIndex = materialIndex;

	m_immediateContext->Unmap(m_gBufferPixelShaderObjectConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(1, 1, &m_gBufferPixelShaderObjectConstantsBuffer);
}

void DeferredShadingDemo::SetNoCullFinalGatherShaderConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &invViewProjMatrix) {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_noCullFinalGatherPixelShaderConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	NoCullFinalGatherPixelShaderFrameConstants *pixelShaderFrameConstants = static_cast<NoCullFinalGatherPixelShaderFrameConstants *>(mappedResource.pData);
	pixelShaderFrameConstants->gProjection = projMatrix;
	pixelShaderFrameConstants->gInvViewProjection = invViewProjMatrix;
	pixelShaderFrameConstants->gDirectionalLight = m_directionalLight;
	pixelShaderFrameConstants->gEyePosition = m_camera.GetCameraPosition();

	m_immediateContext->Unmap(m_noCullFinalGatherPixelShaderConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(0, 1, &m_noCullFinalGatherPixelShaderConstantsBuffer);
}

void DeferredShadingDemo::SetLightBuffers() {
	uint numPointLights = m_pointLights.size();
	uint numSpotLights = m_spotLights.size();

	if (numPointLights > 0) {
		assert(m_pointLightBuffer->NumElements() == numPointLights);

		Common::PointLight *pointLightArray = m_pointLightBuffer->MapDiscard(m_immediateContext);
		for (unsigned int i = 0; i < m_pointLights.size(); ++i) {
			pointLightArray[i] = m_pointLights[i];
		}
		m_pointLightBuffer->Unmap(m_immediateContext);
	}
	
	if (numSpotLights > 0) {
		assert(m_spotLightBuffer->NumElements() == numSpotLights);

		Common::SpotLight *spotLightArray = m_spotLightBuffer->MapDiscard(m_immediateContext);
		for (unsigned int i = 0; i < numSpotLights; ++i) {
			spotLightArray[i] = m_spotLights[i];
		}
		m_pointLightBuffer->Unmap(m_immediateContext);
	}
}

void DeferredShadingDemo::SetMaterialList() {
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

void DeferredShadingDemo::RenderHUD() {
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

} // End of namespace DeferredShadingDemo
