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

class Model {
public:
	Model()
		: m_vertexBuffer(nullptr),
		  m_indexBuffer(nullptr),
		  m_instanceBuffer(nullptr),
		  m_vertexStride(0),
		  m_instanceStride(0),
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

	size_t m_vertexStride;
	size_t m_instanceStride;

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

	void CreateVertexBuffer(ID3D11Device *device, void *vertices, size_t vertexStride, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateVertexBuffer(ID3D11Device *device, void *vertices, uint vertexCount, D3D11_BUFFER_DESC vertexBufferDesc, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, D3D11_BUFFER_DESC indexBufferDesc, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateInstanceBuffer(ID3D11Device *device, size_t instanceStride, uint maxInstanceCount, void *instanceData = nullptr, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateInstanceBuffer(ID3D11Device *device, uint maxInstanceCount, D3D11_BUFFER_DESC instanceBufferDesc, void *instanceData = nullptr, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	void *MapInstanceBuffer(ID3D11DeviceContext *deviceContext, uint *out_maxNumInstances);
	void UnMapInstanceBuffer(ID3D11DeviceContext *deviceContext);

	void DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId = -1);
	void DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint indexCountPerInstance, uint instanceCount, uint subsetId = -1);
};

} // End of namespace Common

#endif