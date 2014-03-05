/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_MODEL_H
#define COMMON_MODEL_H

#include "common/typedefs.h"
#include "common/d3d_util.h"
#include "common/materials.h"

#include <DirectXMath.h>


namespace Common {

namespace TextureFlags {

enum Flags {
	DIFFUSE = 0x01,
	SPEC_COLOR = 0x02,
	SPEC_HIGHLIGHT = 0x04,
	ALPHA_MAP = 0x08,
	BUMP_MAP = 0x10
};

} // End of namespace TextureFlags

struct ModelSubset {
	uint VertexStart;
	uint VertexCount;

	uint IndexStart;
	uint IndexCount;

	Common::BlinnPhongMaterial Material;

	ID3D11ShaderResourceView *DiffuseSRV;
	ID3D11ShaderResourceView *SpecularColorSRV;
	ID3D11ShaderResourceView *SpecularHighlightSRV;
	ID3D11ShaderResourceView *AlphaSRV;
	ID3D11ShaderResourceView *BumpSRV;

	uint TextureFlags;
};

struct DefaultInstanceType {
	DirectX::XMFLOAT4X4 worldTransform;
};

template <typename Vertex, typename InstanceType = DefaultInstanceType>
class Model {
public:
	Model()
		: m_vertexBuffer(nullptr),
		  m_indexBuffer(nullptr),
		  m_instanceBuffer(nullptr),
		  m_vertexStride(0),
		  m_subsets(nullptr),
		  m_subsetCount(0),
		  m_maxInstanceCount(0),
		  m_worldTransform(DirectX::XMMatrixIdentity()),
		  m_disposeSubsetArray(DisposeAfterUse::YES) {
	}

	~Model() {
		ReleaseCOM(m_vertexBuffer);
		ReleaseCOM(m_indexBuffer);
		ReleaseCOM(m_instanceBuffer);
		if (m_disposeSubsetArray == DisposeAfterUse::YES) {
			delete[] m_subsets;
		}
	}

private:
	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;
	ID3D11Buffer *m_instanceBuffer;

	uint m_vertexStride;

	ModelSubset *m_subsets;
	uint m_subsetCount;

	uint m_maxInstanceCount;

	DirectX::XMMATRIX m_worldTransform;

	DisposeAfterUse::Flag m_disposeSubsetArray;

public:
	void *operator new(size_t size);
	void operator delete(void *memory);

	inline uint GetSubsetCount() const { return m_subsetCount; }
	const Common::BlinnPhongMaterial &GetSubsetMaterial(uint subsetIndex) const;
	uint GetSubsetTextureFlags(uint subsetIndex) const;

	void SetWorldTransform(DirectX::XMMATRIX &worldTransform) { m_worldTransform = worldTransform; }
	const DirectX::XMMATRIX &GetWorldTransform() { return m_worldTransform; }

	void CreateVertexBuffer(ID3D11Device *device, Vertex *vertices, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateInstanceBuffer(ID3D11Device *device, uint maxInstanceCount);
	void CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	InstanceType *MapInstanceBuffer(ID3D11DeviceContext *deviceContext, uint *out_maxNumInstances);
	void UnMapInstanceBuffer(ID3D11DeviceContext *deviceContext);

	void DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId = -1);
	void DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint indexCountPerInstance, uint instanceCount, uint subsetId = -1);
};

template <typename Vertex, typename InstanceType>
void *Model<Vertex, InstanceType>::operator new(size_t size) {
	return _aligned_malloc(size, 16);
}

template <typename Vertex, typename InstanceType>
void Model<Vertex, InstanceType>::operator delete(void *memory) {
	_aligned_free(memory);
}

template <typename Vertex, typename InstanceType>
const Common::BlinnPhongMaterial &Model<Vertex, InstanceType>::GetSubsetMaterial(uint subsetIndex) const {
	if (subsetIndex < m_subsetCount) {
		return m_subsets[subsetIndex].Material;
	}

	throw std::out_of_range("subsetIndex out of range");
}

template <typename Vertex, typename InstanceType>
uint Model<Vertex, InstanceType>::GetSubsetTextureFlags(uint subsetIndex) const {
	if (subsetIndex < m_subsetCount) {
		return m_subsets[subsetIndex].TextureFlags;
	}

	throw std::out_of_range("subsetIndex out of range");
}

template <typename Vertex, typename InstanceType>
void Model<Vertex, InstanceType>::CreateVertexBuffer(ID3D11Device *device, Vertex *vertices, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse) {
	m_vertexStride = sizeof(Vertex);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices;

	HR(device->CreateBuffer(&vbd, &vInitData, &m_vertexBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] vertices;
	}
}

template <typename Vertex, typename InstanceType>
void Model<Vertex, InstanceType>::CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse::Flag disposeAfterUse) {
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(uint) * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = indices;

	HR(device->CreateBuffer(&ibd, &iInitData, &m_indexBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] indices;
	}
}

template <typename Vertex, typename InstanceType>
void Model<Vertex, InstanceType>::CreateInstanceBuffer(ID3D11Device *device, uint maxInstanceCount) {
	m_maxInstanceCount = maxInstanceCount;

	D3D11_BUFFER_DESC instbd;
	instbd.Usage = D3D11_USAGE_DYNAMIC;
	instbd.ByteWidth = sizeof(InstanceType) * maxInstanceCount;
	instbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //needed for Map/Unmap
	instbd.MiscFlags = 0;
	instbd.StructureByteStride = 0;

	HR(device->CreateBuffer(&instbd, nullptr, &m_instanceBuffer));
}

template <typename Vertex, typename InstanceType>
void Model<Vertex, InstanceType>::CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse) {
	m_subsets = subsetArray;
	m_subsetCount = subsetCount;
	m_disposeSubsetArray = disposeAfterUse;
}

template <typename Vertex, typename InstanceType>
InstanceType *Model<Vertex, InstanceType>::MapInstanceBuffer(ID3D11DeviceContext *deviceContext, uint *out_maxNumInstances) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	*out_maxNumInstances = m_maxInstanceCount;
	return static_cast<InstanceType *>(mappedResource.pData);
}

template <typename Vertex, typename InstanceType>
void Model<Vertex, InstanceType>::UnMapInstanceBuffer(ID3D11DeviceContext *deviceContext) {
	deviceContext->Unmap(m_instanceBuffer, 0);
}

template <typename Vertex, typename InstanceType>
void Common::Model<Vertex, InstanceType>::DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId) {
	assert(subsetId >= -1 && subsetId < (int)m_subsetCount);

	uint stride = sizeof(Vertex);
	uint offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			if ((m_subsets[i].TextureFlags & TextureFlags::DIFFUSE) == TextureFlags::DIFFUSE) {
				deviceContext->PSSetShaderResources(0, 1, &m_subsets[i].DiffuseSRV);
			}
			deviceContext->DrawIndexed(m_subsets[i].IndexCount, m_subsets[i].IndexStart, 0);
		}
	} else {
		if ((m_subsets[subsetId].TextureFlags & TextureFlags::DIFFUSE) == TextureFlags::DIFFUSE) {
			deviceContext->PSSetShaderResources(0, 1, &m_subsets[subsetId].DiffuseSRV);
		}
		deviceContext->DrawIndexed(m_subsets[subsetId].IndexCount, m_subsets[subsetId].IndexStart, 0);
	}
}

template <typename Vertex, typename InstanceType>
void Model<Vertex, InstanceType>::DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint indexCountPerInstance, uint instanceCount, uint subsetId) {
	assert(m_instanceBuffer);
	assert(instanceCount <= m_maxInstanceCount);

	ID3D11Buffer *vbs[] = {m_vertexBuffer, m_instanceBuffer};
	uint strides[] = {sizeof(Vertex), sizeof(InstanceType)};
	uint offsets[] = {0, 0};
	deviceContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			if ((m_subsets[i].TextureFlags & TextureFlags::DIFFUSE) == TextureFlags::DIFFUSE) {
				deviceContext->PSSetShaderResources(0, 1, &m_subsets[i].DiffuseSRV);
			}
			deviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, m_subsets[i].IndexStart, 0, 0);
		}
	} else {
		if ((m_subsets[subsetId].TextureFlags & TextureFlags::DIFFUSE) == TextureFlags::DIFFUSE) {
			deviceContext->PSSetShaderResources(0, 1, &m_subsets[subsetId].DiffuseSRV);
		}
		deviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, m_subsets[subsetId].IndexStart, 0, 0);
	}
}

} // End of namespace Common

#endif