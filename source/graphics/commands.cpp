/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "graphics/commands.h"

#include "graphics/d3d_util.h"
#include "graphics/graphics_state.h"

#include <d3d11.h>


namespace Graphics {

namespace Commands {

void DrawCommandBase::CheckAndSubmitChangedState(ID3D11Device *device, ID3D11DeviceContext *context, BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, GraphicsState *currentGraphicsState) const {
	// Check material shader
	if (currentGraphicsState->MaterialShader != m_materialShader) {
		m_materialShader->BindToPipeline(context);

		// Update the current graphics state
		currentGraphicsState->MaterialShader = m_materialShader;
	}
	
	// Check vertex buffers
	if (m_numVertexBuffers == 1 && currentGraphicsState->VertexBuffers[0] != m_vertexBuffers[0]) {
		uint offsets = 0;
		context->IASetVertexBuffers(0, 1u, m_vertexBuffers, m_vertexBufferStrides, &offsets);

		// Update the current graphics state
		currentGraphicsState->VertexBuffers[0] = m_vertexBuffers[0];
	} else if (m_numVertexBuffers == 2 && (currentGraphicsState->VertexBuffers[0] != m_vertexBuffers[0] || currentGraphicsState->VertexBuffers[1] != m_vertexBuffers[1])) {
		uint offsets[] = {0, 0};
		context->IASetVertexBuffers(0, 2u, m_vertexBuffers, m_vertexBufferStrides, offsets);

		// Update the current graphics state
		memcpy(currentGraphicsState->VertexBuffers, m_vertexBuffers, sizeof(ID3D11Buffer *) * 2ull);
	}

	// Check index buffer
	if (m_indexBuffer != currentGraphicsState->IndexBuffer) {
		context->IASetIndexBuffer(m_indexBuffer, m_indexBufferFormat, 0u);

		// Update the current graphics state
		currentGraphicsState->IndexBuffer = m_indexBuffer;
	}

	// TODO: Do an element by element search and replace. Preferably optimizing to as few calls as possible (by grouping textures into an array)
	if (m_textureSRVs.begin()->second != currentGraphicsState->TextureSRVs.begin()->second) {
		for (auto iter = m_textureSRVs.begin(); iter != m_textureSRVs.end(); ++iter) {
			context->PSSetShaderResources(iter->first, 1u, &iter->second);
		}

		// Update the current graphics state
		currentGraphicsState->TextureSRVs = m_textureSRVs;
	}

	// TODO: Do an element by element search and replace. Preferably optimizing to as few calls as possible (by grouping samplers into an array)
	if (m_textureSamplers.begin()->second != currentGraphicsState->TextureSamplers.begin()->second) {
		for (auto iter = m_textureSamplers.begin(); iter != m_textureSamplers.end(); ++iter) {
			context->PSSetSamplers(iter->first, 1u, &iter->second);
		}

		// Update the current graphics state
		currentGraphicsState->TextureSamplers = m_textureSamplers;
	}

	// Check blend state
	if (m_blendState != currentGraphicsState->BlendState ||
		m_blendFactor[0] != currentGraphicsState->BlendFactor[0] ||
		m_blendFactor[1] != currentGraphicsState->BlendFactor[1] ||
		m_blendFactor[2] != currentGraphicsState->BlendFactor[2] ||
		m_blendFactor[3] != currentGraphicsState->BlendFactor[3] ||
		m_sampleMask != currentGraphicsState->SampleMask) {
		context->OMSetBlendState(blendStateManager->GetD3DState(m_blendState), m_blendFactor, m_sampleMask);

		// Update the current graphics state
		currentGraphicsState->BlendState = m_blendState;
		memcpy(currentGraphicsState->BlendFactor, m_blendFactor, sizeof(float) * 4ull);
		currentGraphicsState->SampleMask = m_sampleMask;
	}

	// Check rasterizer state
	if (m_rasterizerState != currentGraphicsState->RasterizerState) {
		context->RSSetState(rasterizerStateManager->GetD3DState(m_rasterizerState));

		// Update the current graphics state
		currentGraphicsState->RasterizerState = m_rasterizerState;
	}

	// Check depth stencil state
	if (m_depthStencilState != currentGraphicsState->DepthStencilState) {
		context->OMSetDepthStencilState(depthStencilStateManager->GetD3DState(m_depthStencilState), 0u);

		// Update the current graphics state
		currentGraphicsState->DepthStencilState = m_depthStencilState;
	}
}

void Draw::Execute(ID3D11Device *device, ID3D11DeviceContext *context, BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, GraphicsState *currentGraphicsState, const void *data) {
	const Draw *command = reinterpret_cast<const Draw *>(data);

	command->CheckAndSubmitChangedState(device, context, blendStateManager, rasterizerStateManager, depthStencilStateManager, currentGraphicsState);
	context->Draw(command->m_vertexCount, command->m_vertexStart);
}

void DrawIndexed::Execute(ID3D11Device *device, ID3D11DeviceContext *context, BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, GraphicsState *currentGraphicsState, const void *data) {
	const DrawIndexed *command = reinterpret_cast<const DrawIndexed *>(data);

	command->CheckAndSubmitChangedState(device, context, blendStateManager, rasterizerStateManager, depthStencilStateManager, currentGraphicsState);
	context->DrawIndexed(command->m_indexCount, command->m_indexStart, command->m_vertexStart);
}

void DrawIndexedInstanced::Execute(ID3D11Device *device, ID3D11DeviceContext *context, BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, GraphicsState *currentGraphicsState, const void *data) {
	const DrawIndexedInstanced *command = reinterpret_cast<const DrawIndexedInstanced *>(data);

	command->CheckAndSubmitChangedState(device, context, blendStateManager, rasterizerStateManager, depthStencilStateManager, currentGraphicsState);
	context->DrawIndexedInstanced(command->m_indexCountPerInstance, command->m_instanceCount, command->m_indexStart, command->m_vertexStart, command->m_instanceStart);
}

void MapDataToConstantBuffer::Execute(ID3D11Device *device, ID3D11DeviceContext *context, BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, GraphicsState *currentGraphicsState, const void *data) {
	const MapDataToConstantBuffer *command = reinterpret_cast<const MapDataToConstantBuffer *>(data);

	// Make sure the buffer even exists
	assert(command->m_constantBuffer != nullptr);

	// Map the data
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(context->Map(command->m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, command->m_data, command->m_dataSize);
	context->Unmap(command->m_constantBuffer, 0);
}

} // End of namespace Commands

} // End of namespace Graphics
