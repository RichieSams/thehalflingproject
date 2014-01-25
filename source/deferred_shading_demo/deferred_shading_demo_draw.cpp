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
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, DirectX::Colors::LightGray);
	for (auto gbufferRTV : m_gBufferRTVs) {
		m_immediateContext->ClearRenderTargetView(gbufferRTV, DirectX::Colors::Black);
	}
	m_immediateContext->ClearDepthStencilView(m_depthStencilBuffer->GetDepthStencil(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set States
	m_immediateContext->PSSetSamplers(0, 1, &m_diffuseSampleState);
	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_immediateContext->OMSetBlendState(m_blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	m_immediateContext->OMSetDepthStencilState(m_depthStencilStates.StencilTestEnabled(), 0);
	m_immediateContext->RSSetState(m_rasterizerStates.BackFaceCull());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_immediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_immediateContext->PSSetShader(m_gbufferPixelShader, NULL, 0);

	m_immediateContext->IASetInputLayout(m_inputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Fetch the transpose matricies
	DirectX::XMMATRIX worldMatrix = m_worldViewProj.world;
	DirectX::XMMATRIX viewMatrix = m_worldViewProj.view;
	DirectX::XMMATRIX projectionMatrix = m_worldViewProj.projection;

	uint materialIndex = 0;

	for (uint i = 0; i < m_models.size(); ++i) {
		m_frameMaterialList.push_back(m_models[i].GetSubsetMaterial(0));
		uint materialIndex = m_frameMaterialList.size();

		// Cache the matrix multiplications
		DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(worldMatrix * viewMatrix * projectionMatrix);

		SetGBufferShaderObjectConstants(DirectX::XMMatrixTranspose(worldMatrix), worldViewProjection, materialIndex++);
	
		// Draw the models
		m_models[i].DrawSubset(m_immediateContext);
	}

	//SetLightBuffers(viewMatrix);

	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

	// Draw the gbuffers to the frame
	m_spriteRenderer.Begin(m_immediateContext, Common::SpriteRenderer::Point);
	DirectX::XMFLOAT4X4 transform{0.5, 0, 0, 0,
		0, 0.5, 0, 0,
		0, 0, 0.5, 0,
		0, 0, 0, 1};
	m_spriteRenderer.Render(m_gBufferSRVs[0], transform);
	transform._41 = 0;
	transform._42 = 300;
	m_spriteRenderer.Render(m_gBufferSRVs[1], transform);
	transform._41 = 400;
	transform._42 = 0;
	m_spriteRenderer.Render(m_gBufferSRVs[2], transform);
	m_spriteRenderer.End();
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

void DeferredShadingDemo::SetLightBuffers(DirectX::XMMATRIX &viewMatrix) {
	uint numPointLights = m_pointLights.size();
	//uint numSpotLights = m_gameStateManager->SpotLights.size();

	assert(m_pointLightBuffer->NumElements() == numPointLights);
	//assert(m_spotLightBuffer->NumElements() == numSpotLights);

	Common::PointLight *pointLightArray = m_pointLightBuffer->MapDiscard(m_immediateContext);
	for (unsigned int i = 0; i < m_pointLights.size(); ++i) {
		pointLightArray[i] = m_pointLights[i];
	}
	m_pointLightBuffer->Unmap(m_immediateContext);

	//Common::SpotLight *spotLightArray = m_spotLightBuffer->MapDiscard(m_immediateContext);
	//for (unsigned int i = 0; i < m_gameStateManager->SpotLights.size(); ++i) {
	//	spotLightArray[i] = m_gameStateManager->SpotLights[i];
	//}
	//m_pointLightBuffer->Unmap(m_immediateContext);
}

void DeferredShadingDemo::RenderHUD() {
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
}

} // End of namespace DeferredShadingDemo
