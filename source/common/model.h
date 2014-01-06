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


namespace Common {

struct ModelSubset {
	uint VertexStart;
	uint VertexCount;

	uint FaceStart;
	uint FaceCount;

	Common::Material Material;

	ID3D11ShaderResourceView *SRV;
};

template <typename Vertex>
class Model {
public:
	Model()
		: m_vertexBuffer(nullptr),
		  m_indexBuffer(nullptr),
		  m_vertexStride(0),
		  m_vertices(nullptr),
		  m_indices(nullptr),
		  m_subsets(nullptr),
		  m_vertexCount(0),
		  m_indexCount(0),
		  m_subsetCount(0),
		  m_disposeVertices(DisposeAfterUse::NO),
		  m_disposeIndices(DisposeAfterUse::NO),
		  m_disposeSubsetArray(DisposeAfterUse::NO) {
	}

	~Model() {
		ReleaseCOM(m_vertexBuffer);
		ReleaseCOM(m_indexBuffer);
		if (m_disposeVertices == DisposeAfterUse::YES)
			delete[] m_vertices;
		if (m_disposeIndices == DisposeAfterUse::YES)
			delete[] m_indices;
		if (m_disposeSubsetArray)
			delete[] m_subsets;
	}

private:
	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;

	uint m_vertexStride;

	// Local copy of vertices and indices
	Vertex *m_vertices;
	uint *m_indices;
	ModelSubset *m_subsets;

	uint m_vertexCount;
	uint m_indexCount;
	uint m_subsetCount;

	DisposeAfterUse::Flag m_disposeVertices;
	DisposeAfterUse::Flag m_disposeIndices;
	DisposeAfterUse::Flag m_disposeSubsetArray;

public:
	const Common::Material &GetSubsetMaterial(uint subsetIndex) const;
	void SetVertices(ID3D11Device *device, Vertex *vertices, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void SetIndices(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void SetSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	void DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId = -1);
};

template <typename Vertex>
const Common::Material &Model<Vertex>::GetSubsetMaterial(uint subsetIndex) const {
	if (subsetIndex < m_subsetCount)
		return m_subsets[subsetIndex].Material;
	
	throw std::out_of_range("subsetIndex out of range");
}

template <typename Vertex>
void Common::Model<Vertex>::SetVertices(ID3D11Device *device, Vertex *vertices, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse) {
	m_vertices = vertices;
	m_vertexCount = vertexCount;
	m_disposeVertices = disposeAfterUse;

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
}

template <typename Vertex>
void Common::Model<Vertex>::SetIndices(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse::Flag disposeAfterUse) {
	m_indices = indices;
	m_indexCount = indexCount;
	m_disposeIndices = disposeAfterUse;

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
}

template <typename Vertex>
void Common::Model<Vertex>::SetSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse) {
	m_subsets = subsetArray;
	m_subsetCount = subsetCount;
	m_disposeSubsetArray = disposeAfterUse;
}

template <typename Vertex>
void Common::Model<Vertex>::DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId) {
	assert(subsetId >= -1 && subsetId < (int)m_subsetCount);

	uint stride = sizeof(Vertex);
	uint offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			deviceContext->PSSetShaderResources(0, 1, &m_subsets[i].SRV);
			deviceContext->DrawIndexed(m_subsets[i].FaceCount * 3, m_subsets[i].FaceStart * 3, 0);
		}
	} else {
		deviceContext->PSSetShaderResources(0, 1, &m_subsets[subsetId].SRV);
		deviceContext->DrawIndexed(m_subsets[subsetId].FaceCount * 3, m_subsets[subsetId].FaceStart * 3, 0);
	}
}

} // End of namespace Common

#endif