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
	// Bind the gbufferRTVs and depth/stencil view to the pipeline.
	m_immediateContext->OMSetRenderTargets(2, &m_gBufferRTVs[0], m_depthStencilBuffer->GetDepthStencil());

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


	// Transpose the matrices to prepare them for the shader.
	DirectX::XMMATRIX worldMatrix = m_worldViewProj.world;
	DirectX::XMMATRIX viewMatrix = m_worldViewProj.view;
	DirectX::XMMATRIX projectionMatrix = m_worldViewProj.projection;

	// Cache the matrix multiplications
	DirectX::XMMATRIX viewProj = DirectX::XMMatrixTranspose(viewMatrix * projectionMatrix);
	DirectX::XMMATRIX worldViewProjection = DirectX::XMMatrixTranspose(worldMatrix * viewMatrix * projectionMatrix);

	SetFrameConstants(DirectX::XMMatrixTranspose(projectionMatrix), viewProj);

	SetObjectConstants(DirectX::XMMatrixTranspose(worldMatrix), worldViewProjection, m_models[0].GetSubsetMaterial(0));
	SetLightBuffers(viewMatrix);

	m_immediateContext->IASetInputLayout(m_inputLayout);
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	m_immediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_immediateContext->PSSetShader(m_gbufferPixelShader, NULL, 0);

	m_models[0].DrawSubset(m_immediateContext);


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

void DeferredShadingDemo::SetFrameConstants(DirectX::XMMATRIX &projMatrix, DirectX::XMMATRIX &viewProjMatrix) {
	// Fill in frame constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_vertexShaderFrameConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	VertexShaderFrameConstants *vertexShaderFrameConstants = static_cast<VertexShaderFrameConstants *>(mappedResource.pData);
	vertexShaderFrameConstants->proj = projMatrix;
	vertexShaderFrameConstants->viewProj = viewProjMatrix;

	m_immediateContext->Unmap(m_vertexShaderFrameConstantsBuffer, 0);
	m_immediateContext->VSSetConstantBuffers(0, 1, &m_vertexShaderFrameConstantsBuffer);
}

void DeferredShadingDemo::SetObjectConstants(DirectX::XMMATRIX &worldMatrix, DirectX::XMMATRIX &worldViewProjMatrix, const Common::Material &material) {
	// Fill in object constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_vertexShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	VertexShaderObjectConstants *vertexShaderObjectConstants = static_cast<VertexShaderObjectConstants *>(mappedResource.pData);
	vertexShaderObjectConstants->world = worldMatrix;
	vertexShaderObjectConstants->worldViewProj = worldViewProjMatrix;

	m_immediateContext->Unmap(m_vertexShaderObjectConstantsBuffer, 0);
	m_immediateContext->VSSetConstantBuffers(1, 1, &m_vertexShaderObjectConstantsBuffer);

	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_pixelShaderObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PixelShaderObjectConstants *pixelShaderObjectConstants = static_cast<PixelShaderObjectConstants *>(mappedResource.pData);
	pixelShaderObjectConstants->material = material;

	m_immediateContext->Unmap(m_pixelShaderObjectConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(3, 1, &m_pixelShaderObjectConstantsBuffer);


	// Lock the constant buffer so it can be written to.
	HR(m_immediateContext->Map(m_pixelShaderFrameConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PixelShaderFrameConstants *pixelShaderFrameConstants = static_cast<PixelShaderFrameConstants *>(mappedResource.pData);
	pixelShaderFrameConstants->directionalLight = m_directionalLight;
	pixelShaderFrameConstants->eyePosition = m_camera.GetCameraPosition();

	m_immediateContext->Unmap(m_pixelShaderFrameConstantsBuffer, 0);
	m_immediateContext->PSSetConstantBuffers(2, 1, &m_pixelShaderFrameConstantsBuffer);
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
