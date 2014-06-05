/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
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
	DIFFUSE_COLOR = 0x01,
	SPEC_COLOR = 0x02,
	SPEC_POWER = 0x04,
	ALPHA_MAP = 0x08,
	DISPLACEMENT_MAP = 0x10,
	NORMAL_MAP = 0x20
};

} // End of namespace TextureFlags

struct ModelSubset {
	uint VertexStart;
	uint VertexCount;

	uint IndexStart;
	uint IndexCount;

	DirectX::XMFLOAT3 AABB_min;
	DirectX::XMFLOAT3 AABB_max;

	Common::BlinnPhongMaterial Material;

	ID3D11ShaderResourceView *DiffuseColorSRV;
	ID3D11ShaderResourceView *SpecularColorSRV;
	ID3D11ShaderResourceView *SpecularPowerSRV;
	ID3D11ShaderResourceView *AlphaMapSRV;
	ID3D11ShaderResourceView *DisplacementMapSRV;
	ID3D11ShaderResourceView *NormalMapSRV;

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
		  m_AABB_min(0.0f, 0.0f, 0.0f),
		  m_AABB_max(0.0f, 0.0f, 0.0f),
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
	uint m_instanceStride;

	ModelSubset *m_subsets;
	uint m_subsetCount;

	uint m_maxInstanceCount;

	DirectX::XMFLOAT3 m_AABB_min;
	DirectX::XMFLOAT3 m_AABB_max;

	DisposeAfterUse::Flag m_disposeSubsetArray;

private:
	static void SetTextureResources(ID3D11DeviceContext *context, ModelSubset &subset);

public:
	inline uint GetSubsetCount() const { return m_subsetCount; }
	const Common::BlinnPhongMaterial &GetSubsetMaterial(uint subsetIndex) const;
	uint GetSubsetTextureFlags(uint subsetIndex) const;

	inline DirectX::XMFLOAT3 GetAABBMin() { return m_AABB_min; }
	inline DirectX::XMFLOAT3 GetAABBMax() { return m_AABB_max; }
	inline DirectX::XMVECTOR GetAABBMin_XM() { return DirectX::XMLoadFloat3(&m_AABB_min); }
	inline DirectX::XMVECTOR GetAABBMax_XM() { return DirectX::XMLoadFloat3(&m_AABB_max); }

	void CreateVertexBuffer(ID3D11Device *device, void *vertices, size_t vertexStride, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateVertexBuffer(ID3D11Device *device, void *vertices, uint vertexCount, D3D11_BUFFER_DESC vertexBufferDesc, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, D3D11_BUFFER_DESC indexBufferDesc, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateInstanceBuffer(ID3D11Device *device, size_t instanceStride, uint maxInstanceCount, void *instanceData = nullptr, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateInstanceBuffer(ID3D11Device *device, D3D11_BUFFER_DESC instanceBufferDesc, void *instanceData = nullptr, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	void CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	void *MapInstanceBuffer(ID3D11DeviceContext *deviceContext, uint *out_maxNumInstances);
	void UnMapInstanceBuffer(ID3D11DeviceContext *deviceContext);

	void DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId = -1);
	void DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint instanceCount, uint indexCountPerInstance = 0,uint subsetId = -1);
	void DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint instanceCount, ID3D11Buffer *instanceBuffer, size_t instanceStride, uint indexCountPerInstance = 0, uint subsetId = -1);
};

} // End of namespace Common

#endif