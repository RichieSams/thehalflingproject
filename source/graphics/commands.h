/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/typedefs.h"

#include "graphics/device_states.h"
#include "graphics/d3d_util.h"
#include "graphics/graphics_state.h"

#include <d3d11.h>

#include <cassert>
#include <map>


struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11InputLayout;
struct ID3D11Buffer;


namespace Graphics {

namespace Commands {

template <typename Derived>
class CommandBase {
public:
	static void Execute(ID3D11Device *device, ID3D11DeviceContext *context, 
	                    BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, 
	                    GraphicsState *currentGraphicsState, 
	                    const void *data) {
		static_cast<Derived *>(this)->Execute(device, context, data);
	}

	static void Dispose(const void *data) {
		static_cast<Derived *>(this)->Dispose(data);
	}
};

class DrawCommandBase {
public:
	DrawCommandBase()
			: m_materialShader(nullptr),
			  m_numVertexBuffers(0u),
			  m_indexBuffer(nullptr),
			  m_indexBufferFormat(DXGI_FORMAT_R32_UINT),
			  m_blendState(BlendState::BLEND_DISABLED),
			  m_sampleMask(0xFFFFFFFF),
			  m_rasterizerState(RasterizerState::CULL_BACKFACES),
			  m_depthStencilState(DepthStencilState::REVERSE_DEPTH_WRITE_ENABLED) {
		// WORKAROUND: We have to manually initialize because VS 2013 compiler doesn't support array initialization in the class initializer list
		m_vertexBuffers[0] = nullptr;
		m_vertexBuffers[1] = nullptr;

		m_vertexBufferStrides[0] = 0u;
		m_vertexBufferStrides[1] = 1u;

		m_blendFactor[0] = 1.0f;
		m_blendFactor[1] = 1.0f;
		m_blendFactor[2] = 1.0f;
		m_blendFactor[3] = 1.0f;
	}

protected:
	MaterialShader *m_materialShader;

	ID3D11Buffer *m_vertexBuffers[2];
	uint m_vertexBufferStrides[2];
	uint m_numVertexBuffers;
	ID3D11Buffer *m_indexBuffer;
	DXGI_FORMAT m_indexBufferFormat;

	std::map<uint, ID3D11ShaderResourceView *> m_textureSRVs;
	std::map<uint, ID3D11SamplerState *> m_textureSamplers;

	BlendState m_blendState;
	float m_blendFactor[4];
	uint m_sampleMask;
	RasterizerState m_rasterizerState;
	DepthStencilState m_depthStencilState;

public:
	inline void SetMaterialShader(MaterialShader *materialShader) {
		m_materialShader = materialShader;
	}

	inline void SetVertexBuffer(ID3D11Buffer *vertexBuffer, uint vertexStride) {
		assert(vertexBuffer);

		m_vertexBuffers[0] = vertexBuffer;

		m_numVertexBuffers = 1u;
		m_vertexBufferStrides[0] = vertexStride;
	}
	inline void SetVertexBuffers(ID3D11Buffer *vertexBuffer, uint vertexStride, ID3D11Buffer *instanceBuffer, uint instanceStride) {
		// We don't want to have to hit the heap for every single instanced draw call
		// So we keep an internal buffer than can hold up to 2 vertex buffer pointers
		// We don't support more than 2 vertex buffers
		// This isn't a big problem because most instance data is passed though a StructuredBuffer
		// rather than through instance vertex buffers
		assert(vertexBuffer);
		assert(vertexStride > 0u);
		assert(instanceBuffer);
		assert(instanceStride > 0u);

		m_vertexBuffers[0] = vertexBuffer;
		m_vertexBuffers[1] = instanceBuffer;

		m_numVertexBuffers = 2u;
		m_vertexBufferStrides[0] = vertexStride;
		m_vertexBufferStrides[1] = instanceStride;
	}
	inline void SetIndexBuffer(ID3D11Buffer *indexBuffer, DXGI_FORMAT format) {
		m_indexBuffer = indexBuffer;
		m_indexBufferFormat = format;
	}

	inline void SetTextureSRV(ID3D11ShaderResourceView *srv, uint slot) { m_textureSRVs[slot] = srv; }
	inline void SetTextureSampler(ID3D11SamplerState *sampler, uint slot) { m_textureSamplers[slot] = sampler; }

	inline void SetBlendState(BlendState blendState, float *blendFactor, uint sampleMask) {
		m_blendState = blendState;

		m_blendFactor[0] = blendFactor[0];
		m_blendFactor[1] = blendFactor[1];
		m_blendFactor[2] = blendFactor[2];
		m_blendFactor[3] = blendFactor[3];

		m_sampleMask = sampleMask;
	}
	inline void SetRasterizerState(RasterizerState rasterizerState) { m_rasterizerState = rasterizerState; }
	inline void SetDepthStencilState(DepthStencilState depthStencilState) { m_depthStencilState = depthStencilState; }

protected:
	void CheckAndSubmitChangedState(ID3D11Device *device, ID3D11DeviceContext *context, 
	                                BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, 
	                                GraphicsState *currentGraphicsState) const;
};


class Draw : public CommandBase<Draw>, public DrawCommandBase {
public:
	Draw()
		: m_vertexCount(0u),
		  m_vertexStart(0u) {
	}

private:
	uint m_vertexCount;
	uint m_vertexStart;

public:
	inline void SetVertexCount(uint vertexCount) { m_vertexCount = vertexCount; }
	inline void SetVertexStart(uint vertexStart) { m_vertexStart = vertexStart; }

	static void Execute(ID3D11Device *device, ID3D11DeviceContext *context, 
	                    BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, 
	                    GraphicsState *currentGraphicsState, 
	                    const void *data);

	static void Dispose(const void *data);
};


class DrawIndexed : public CommandBase<DrawIndexed>, public DrawCommandBase {
public:
	DrawIndexed()
		: m_indexCount(0u),
		  m_indexStart(0u),
		  m_vertexStart(0u) {
	}

private:
	uint m_indexCount;
	uint m_indexStart;
	uint m_vertexStart;

public:
	inline void SetIndexCount(uint indexCount) { m_indexCount = indexCount; }
	inline void SetIndexStart(uint indexStart) { m_indexStart = indexStart; }
	inline void SetVertexStart(uint vertexStart) { m_vertexStart = vertexStart; }

	static void Execute(ID3D11Device *device, ID3D11DeviceContext *context,
	                    BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager,
	                    GraphicsState *currentGraphicsState,
	                    const void *data);

	static void Dispose(const void *data);
};


class DrawIndexedInstanced : public CommandBase<DrawIndexed>, public DrawCommandBase {
public:
	DrawIndexedInstanced()
		: m_indexCountPerInstance(0u),
		  m_instanceCount(0u),
		  m_instanceStart(0u),
		  m_indexCount(0u),
		  m_indexStart(0u),
		  m_vertexStart(0u) {
	}

private:
	uint m_indexCountPerInstance;
	uint m_instanceCount;
	uint m_instanceStart;
	uint m_indexCount;
	uint m_indexStart;
	uint m_vertexStart;

public:
	inline void SetIndexCountPerInstance(uint indexCountPerInstance) { m_indexCountPerInstance = indexCountPerInstance; }
	inline void SetInstanceCount(uint instanceCount) { m_instanceCount = instanceCount; }
	inline void SetInstanceStart(uint instanceStart) { m_instanceStart = instanceStart; }
	inline void SetIndexCount(uint indexCount) { m_indexCount = indexCount; }
	inline void SetIndexStart(uint indexStart) { m_indexStart = indexStart; }
	inline void SetVertexStart(uint vertexStart) { m_vertexStart = vertexStart; }

	static void Execute(ID3D11Device *device, ID3D11DeviceContext *context,
	                    BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager,
	                    GraphicsState *currentGraphicsState,
	                    const void *data);

	static void Dispose(const void *data);
};

template <typename T>
class MapDataToConstantBuffer : public CommandBase<MapDataToConstantBuffer<T>> {
public:
	MapDataToConstantBuffer()
		: m_constantBuffer(nullptr) {
	}

private:
	ID3D11Buffer *m_constantBuffer;
	T m_constantBufferData;

public:
	inline void SetConstantBuffer(ID3D11Buffer *buffer) { m_constantBuffer = buffer; }
	inline void SetData(T &data) { m_constantBufferData = data; }

	static void Execute(ID3D11Device *device, ID3D11DeviceContext *context,
	                    BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager,
	                    GraphicsState *currentGraphicsState,
	                    const void *data);

	static void Dispose(const void *data);
};

template <typename T>
void Graphics::Commands::MapDataToConstantBuffer<T>::Execute(ID3D11Device *device, ID3D11DeviceContext *context, BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, GraphicsState *currentGraphicsState, const void *data) {
	const MapDataToConstantBuffer *command = reinterpret_cast<const MapDataToConstantBuffer *>(data);

	// Make sure the buffer even exists
	assert(command->m_constantBuffer != nullptr);

	// Map the data
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(context->Map(command->m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, &command->m_constantBufferData, sizeof(command->m_constantBufferData));
	context->Unmap(command->m_constantBuffer, 0);
}

template <typename T>
void Graphics::Commands::MapDataToConstantBuffer<T>::Dispose(const void *data) {
	// No Op since class is a POS
}


class BindConstantBufferToVS : public CommandBase<BindConstantBufferToVS> {
public:
	BindConstantBufferToVS()
		: m_constantBuffer(nullptr) {
	}

private:
	ID3D11Buffer *m_constantBuffer;
	uint m_slot;

public:
	inline void SetConstantBuffer(ID3D11Buffer *buffer, uint slot) { m_constantBuffer = buffer; m_slot = slot; }

	static void Execute(ID3D11Device *device, ID3D11DeviceContext *context,
	                    BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager,
	                    GraphicsState *currentGraphicsState,
	                    const void *data);

	static void Dispose(const void *data);
};


class BindConstantBufferToPS : public CommandBase<BindConstantBufferToPS> {
public:
	BindConstantBufferToPS()
		: m_constantBuffer(nullptr) {
	}

private:
	ID3D11Buffer *m_constantBuffer;
	uint m_slot;

public:
	inline void SetConstantBuffer(ID3D11Buffer *buffer, uint slot) { m_constantBuffer = buffer; m_slot = slot; }

	static void Execute(ID3D11Device *device, ID3D11DeviceContext *context,
	                    BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager,
	                    GraphicsState *currentGraphicsState,
	                    const void *data);

	static void Dispose(const void *data);
};

} // End of namespace Commands

} // End of namespace Graphics