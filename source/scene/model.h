/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/typedefs.h"
#include "graphics/d3d_util.h"
#include "graphics/shader.h"
#include "scene/model_loading.h"

#include <DirectXMath.h>
#include <vector>


namespace Scene {

/** A struct to hold all the data needed to describe a subset of the model */
struct ModelSubset {
	uint VertexStart;
	uint VertexCount;

	uint IndexStart;
	uint IndexCount;

	DirectX::XMFLOAT3 AABB_min;
	DirectX::XMFLOAT3 AABB_max;

	size_t ShaderHandle;

	std::vector<ID3D11ShaderResourceView *> TextureSRVs;
	std::vector<ID3D11SamplerState *> TextureSamplers;
};

/** 
 * A class to represent a single model and its subsets
 *
 * Provides methods to render the whole model or a specific subset. Use
 * these instead of ID3D11Context::Draw*()
 */
class Model {
public:
	Model()
		: m_vertexBuffer(nullptr),
		  m_indexBuffer(nullptr),
		  m_vertexStride(0u),
		  m_subsets(nullptr),
		  m_subsetCount(0u),
		  m_AABB_min(0.0f, 0.0f, 0.0f),
		  m_AABB_max(0.0f, 0.0f, 0.0f),
		  m_disposeSubsetArray(DisposeAfterUse::YES) {
	}

	virtual ~Model() {
		ReleaseCOM(m_vertexBuffer);
		ReleaseCOM(m_indexBuffer);
		if (m_disposeSubsetArray == DisposeAfterUse::YES) {
			delete[] m_subsets;
		}
	}

protected:
	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;

	uint m_vertexStride;

	ModelSubset *m_subsets;
	uint m_subsetCount;

	DirectX::XMFLOAT3 m_AABB_min;
	DirectX::XMFLOAT3 m_AABB_max;

	DisposeAfterUse::Flag m_disposeSubsetArray;

public:
	/* Returns the number of subsets the model has */
	inline uint GetSubsetCount() const { return m_subsetCount; }
	/**
	 * Returns the minimum X, Y, Z components of the axis-aligned bounding box
	 * surrounding the whole model
     */
	inline DirectX::XMFLOAT3 GetAABBMin() { return m_AABB_min; }
	/**
	 * Returns the maximum X, Y, Z components of the axis-aligned bounding box
	 * surrounding the whole model
     */
	inline DirectX::XMFLOAT3 GetAABBMax() { return m_AABB_max; }
	/**
	 * Returns the minimum X, Y, Z components of the axis-aligned bounding box
	 * surrounding the whole model
     */
	inline DirectX::XMVECTOR GetAABBMin_XM() { return DirectX::XMLoadFloat3(&m_AABB_min); }
	/**
	 * Returns the maximum X, Y, Z components of the axis-aligned bounding box
	 * surrounding the whole model
     */
	inline DirectX::XMVECTOR GetAABBMax_XM() { return DirectX::XMLoadFloat3(&m_AABB_max); }

	/**
	 * Creates the vertex buffer for the model. All subsets share the same vertex buffer.
	 * Assumes D3D11_USAGE_IMMUTABLE with no cpu access flags and no misc flags.
	 *
	 * NOTE: CreateVertexBuffer(), CreateIndexBuffer(), and CreateSubsets() *MUST ALL* be called before
	 *       any Draw*Subset() calls
	 *
	 * @param device             The DirectX device
	 * @param vertices           An array holding the vertex data
	 * @param vertexStride       The stride of a single vertex
	 * @param vertexCount        The number of vertices
	 * @param disposeAfterUse    If YES, the function will call delete[] on 'vertices' when it finishes
	 */
	void CreateVertexBuffer(ID3D11Device *device, void *vertices, size_t vertexStride, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	/**
	 * Creates the vertex buffer for the model. All subsets share the same vertex buffer.
	 *
	 * NOTE: CreateVertexBuffer(), CreateIndexBuffer(), and CreateSubsets() *MUST ALL* be called before
	 *       any Draw*Subset() calls
	 *
	 * @param device              The DirectX device
	 * @param vertices            An array holding the vertex data
	 * @param vertexCount         The number of vertices
	 * @param vertexBufferDesc    The vertex buffer description
	 * @param disposeAfterUse     If YES, the function will call delete[] on 'vertices' when it finishes
	 */
	void CreateVertexBuffer(ID3D11Device *device, void *vertices, uint vertexCount, D3D11_BUFFER_DESC vertexBufferDesc, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	/**
	 * Creates the index buffer for the model. All subsets share the same index buffer.
	 * Assumes D3D11_USAGE_IMMUTABLE with no cpu access flags and no misc flags.
	 *
	 * NOTE: CreateVertexBuffer(), CreateIndexBuffer(), and CreateSubsets() *MUST ALL* be called before
	 *       any Draw*Subset() calls
	 *
	 * @param device             The DirectX device
	 * @param indices            An array holding the index data
	 * @param indexCount         The number of indices
	 * @param disposeAfterUse    If YES, the function will call delete[] on 'indices' when it finishes
	 */
	void CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	/**
	 * Creates the index buffer for the model. All subsets share the same index buffer.
	 *
	 * NOTE: CreateVertexBuffer(), CreateIndexBuffer(), and CreateSubsets() *MUST ALL* be called before
	 *       any Draw*Subset() calls
	 *
	 * @param device             The DirectX device
	 * @param indices            An array holding the index data
	 * @param indexCount         The number of indices
	 * @param indexBufferDesc    The index buffer description
	 * @param disposeAfterUse    If YES, the function will call delete[] on 'indices' when it finishes
	 */
	void CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, D3D11_BUFFER_DESC indexBufferDesc, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	/**
	 * Sets the subsets for the model
	 *
	 * @param subsetArray        An array of subsets
	 * @param subsetCount        The number of subsets
	 * @param disposeAfterUse    If YES, the function will call delete[] on 'indices' in the Model destructor
	 */
	void CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	/**
	 * Binds the model's vertex and index buffers, then binds the texture SRVS, before finally drawing the geometry
	 *
	 * NOTE: CreateVertexBuffer(), CreateIndexBuffer(), and CreateSubsets() *MUST ALL* be called before
	 *       any DrawSubset() calls
	 *
	 * @param deviceContext    A DirectX device context
	 * @param subsetId         The subset to draw. If the it is -1, all the subsets will be drawn.
	 */
	void DrawSubset(ID3D11DeviceContext *deviceContext, Engine::MaterialShaderManager *materialShaderManager, int subsetId = -1);
	/**
	 * Identical to DrawSubsets, except it uses ID3D11DeviceContext::DrawInstancedIndexed() instead of 
	 * ID3D11DeviceContext::DrawIndexed(). This function does not bind an extra buffer to the vertex shader
	 * (an instance buffer). If you wish to use an instance buffer, use the InstancedModel class instead
	 *
	 * NOTE: CreateVertexBuffer(), CreateIndexBuffer(), and CreateSubsets() *MUST ALL* be called before
	 *       any DrawInstancedSubset() calls
	 *
	 * @param deviceContext            A DirectX device context
	 * @param instanceCount            The number of instances to draw
	 * @param indexCountPerInstance    The number of indices per instance. If 0, the function will use the subset index count
	 * @param subsetId                 The subset to draw. If it is -1, all the subsets will be drawn.
	 */
	virtual void DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint instanceCount, Engine::MaterialShaderManager *materialShaderManager, uint indexCountPerInstance = 0, int subsetId = -1);
};


/**
 * A class to represent a model and its subsets that are to be instanced using 
 * an explicit instance buffer
 *
 * Provides methods to render the whole model or a specific subset. Use
 * these instead of ID3D11Context::Draw*()
 */
class InstancedModel : public Model {
public:
	InstancedModel()
		: Model(),
		m_instanceBuffer(nullptr),
		m_instanceStride(0u),
		m_maxInstanceCount(0u) {
	}
	~InstancedModel() {
		ReleaseCOM(m_instanceBuffer);
	}

private:
	ID3D11Buffer *m_instanceBuffer;
	uint m_instanceStride;
	uint m_maxInstanceCount;

public:
	/**
	 * Creates the instance buffer for the model
	 *
	 * @param device              The DirectX device
	 * @param instanceStride      The stride of the each instance
	 * @param maxInstanceCount    The maximum number of instances
	 * @param instanceData        [Optional] Data to fill the instance buffer with
	 * @param disposeAfterUse     If YES, the function will call delete[] on 'instanceData' when it finishes
	 */
	void CreateInstanceBuffer(ID3D11Device *device, size_t instanceStride, uint maxInstanceCount, void *instanceData = nullptr, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	/**
	 * Creates the instance buffer for the model
	 *
	 * @param device                The DirectX device
	 * @param instanceBufferDesc    The instance buffer description
	 * @param maxInstanceCount      The maximum number of instances
	 * @param instanceData          [Optional] Data to fill the instance buffer with
	 * @param disposeAfterUse       If YES, the function will call delete[] on 'instanceData' when it finishes
	 */
	void CreateInstanceBuffer(ID3D11Device *device, D3D11_BUFFER_DESC instanceBufferDesc, void *instanceData = nullptr, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	/**
	 * Maps the instance buffer to local memory so that it can be written to
	 *
	 * @param deviceContext          A DirectX device context
	 * @param out_maxNumInstances    Will be filled with the maximum number of instances the buffer can support
	 */
	void *MapInstanceBuffer(ID3D11DeviceContext *deviceContext, uint *out_maxNumInstances);
	/**
	 * Un-map the instance buffer, allowing it to be used by the GPU again
	 *
	 * @param deviceContext    A DirectX device context
	 */
	void UnMapInstanceBuffer(ID3D11DeviceContext *deviceContext);

	/**
	 * Identical to DrawSubsets, except it uses ID3D11DeviceContext::DrawInstancedIndexed() instead of 
	 * ID3D11DeviceContext::DrawIndexed(). This function will bind the internal instance buffer to the
	 * vertex shader.
	 *
	 * NOTE: CreateVertexBuffer(), CreateIndexBuffer(), CreateSubsets() and CreateInstanceBuffer()
	 *       *MUST ALL* be called before any DrawInstancedSubset() calls
	 *
	 * @param deviceContext            A DirectX device context
	 * @param instanceCount            The number of instances to draw
	 * @param indexCountPerInstance    The number of indices per instance. If 0, the function will use the subset index count
	 * @param subsetId                 The subset to draw. If it is -1, all the subsets will be drawn.
	 */
	void DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint instanceCount, Engine::MaterialShaderManager *materialShaderManager, uint indexCountPerInstance = 0, int subsetId = -1);
};

} // End of namespace Scene
