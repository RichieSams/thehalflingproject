/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "scene/model.h"

#include "engine/material_shader_manager.h"


namespace Scene {

void Model::CreateVertexBuffer(ID3D11Device *device, void *vertices, size_t vertexStride, uint vertexCount, DisposeAfterUse disposeAfterUse) {
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = static_cast<uint>(vertexStride) * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	CreateVertexBuffer(device, vertices, vertexCount, vbd, disposeAfterUse);
}

void Model::CreateVertexBuffer(ID3D11Device *device, void *vertices, uint vertexCount, D3D11_BUFFER_DESC vertexBufferDesc, DisposeAfterUse disposeAfterUse) {
	m_vertexStride = vertexBufferDesc.ByteWidth / vertexCount;
	
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices;

	HR(device->CreateBuffer(&vertexBufferDesc, &vInitData, &m_vertexBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] vertices;
	}
}

void Model::CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse disposeAfterUse) {
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(uint) * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	
	CreateIndexBuffer(device, indices, indexCount, ibd, disposeAfterUse);
}

void Model::CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, D3D11_BUFFER_DESC indexBufferDesc, DisposeAfterUse disposeAfterUse) {
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = indices;
	
	HR(device->CreateBuffer(&indexBufferDesc, &iInitData, &m_indexBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] indices;
	}
}

void Model::CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse disposeAfterUse) {
	m_subsets = subsetArray;
	m_subsetCount = subsetCount;
	m_disposeSubsetArray = disposeAfterUse;

	DirectX::XMVECTOR AABB_min = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR AABB_max = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	for (uint i = 0; i < subsetCount; ++i) {
		AABB_min = DirectX::XMVectorMin(AABB_min, DirectX::XMLoadFloat3(&subsetArray[i].AABB_min));
		AABB_max = DirectX::XMVectorMax(AABB_max, DirectX::XMLoadFloat3(&subsetArray[i].AABB_max));
	}

	DirectX::XMStoreFloat3(&m_AABB_min, AABB_min);
	DirectX::XMStoreFloat3(&m_AABB_max, AABB_max);
}

void DrawIndexed_Helper(ID3D11DeviceContext *deviceContext, const ModelSubset &subset) {
	subset.Material->Shader->BindToPipeline(deviceContext);
	
	size_t numTextures = subset.Material->TextureSRVs.size();
	if (numTextures > 0) {
		deviceContext->PSSetShaderResources(0, static_cast<uint>(numTextures), &subset.Material->TextureSRVs[0]);
		deviceContext->PSSetSamplers(0, static_cast<uint>(numTextures), &subset.Material->TextureSamplers[0]);
	}

	deviceContext->DrawIndexed(subset.IndexCount, subset.IndexStart, subset.VertexStart);
}

void DrawIndexedInstanced_Helper(ID3D11DeviceContext *deviceContext, uint indexCountPerInstance, uint instanceCount, const ModelSubset &subset) {
	subset.Material->Shader->BindToPipeline(deviceContext);

	size_t numTextures = subset.Material->TextureSRVs.size();
	if (numTextures > 0) {
		deviceContext->PSSetShaderResources(0, static_cast<uint>(numTextures), &subset.Material->TextureSRVs[0]);
		deviceContext->PSSetSamplers(0, static_cast<uint>(numTextures), &subset.Material->TextureSamplers[0]);
	}

	deviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, subset.IndexStart, subset.VertexStart, 0);
}

void Model::DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId) {
	assert(subsetId >= -1 && subsetId < (int)m_subsetCount);

	uint offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexStride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			DrawIndexed_Helper(deviceContext, m_subsets[i]);
		}
	} else {
		DrawIndexed_Helper(deviceContext, m_subsets[subsetId]);
	}
}

void Model::DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint instanceCount, uint indexCountPerInstance, int subsetId) {
	uint offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexStride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			DrawIndexedInstanced_Helper(deviceContext, indexCountPerInstance == 0 ? m_subsets[i].IndexCount : indexCountPerInstance, instanceCount, m_subsets[i]);
		}
	} else {
		DrawIndexedInstanced_Helper(deviceContext, indexCountPerInstance == 0 ? m_subsets[subsetId].IndexCount : indexCountPerInstance, instanceCount, m_subsets[subsetId]);
	}
}


void InstancedModel::CreateInstanceBuffer(ID3D11Device *device, size_t instanceStride, uint maxInstanceCount, void *instanceData, DisposeAfterUse disposeAfterUse) {
	m_instanceStride = static_cast<uint>(instanceStride);
	m_maxInstanceCount = maxInstanceCount;

	D3D11_BUFFER_DESC instbd;
	instbd.Usage = D3D11_USAGE_DYNAMIC;
	instbd.ByteWidth = m_instanceStride * maxInstanceCount;
	instbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //needed for Map/Unmap
	instbd.MiscFlags = 0;
	instbd.StructureByteStride = 0;

	CreateInstanceBuffer(device, instbd, instanceData, disposeAfterUse);
}

void InstancedModel::CreateInstanceBuffer(ID3D11Device *device, D3D11_BUFFER_DESC instanceBufferDesc, void *instanceData, DisposeAfterUse disposeAfterUse) {
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = instanceData;

	HR(device->CreateBuffer(&instanceBufferDesc, instanceData ? &iInitData : nullptr, &m_instanceBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] instanceData;
	}
}

void *InstancedModel::MapInstanceBuffer(ID3D11DeviceContext *deviceContext, uint *out_maxNumInstances) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	*out_maxNumInstances = m_maxInstanceCount;
	return mappedResource.pData;
}

void InstancedModel::UnMapInstanceBuffer(ID3D11DeviceContext *deviceContext) {
	deviceContext->Unmap(m_instanceBuffer, 0);
}

void InstancedModel::DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint instanceCount, uint indexCountPerInstance, int subsetId) {
	assert(m_instanceBuffer);
	assert(instanceCount <= m_maxInstanceCount);

	ID3D11Buffer *vbs[] = {m_vertexBuffer, m_instanceBuffer};
	uint strides[] = {m_vertexStride, m_instanceStride};
	uint offsets[] = {0, 0};
	deviceContext->IASetVertexBuffers(0, 2u, vbs, strides, offsets);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			DrawIndexedInstanced_Helper(deviceContext, indexCountPerInstance == 0 ? m_subsets[i].IndexCount : indexCountPerInstance, instanceCount, m_subsets[i]);
		}
	} else {
		DrawIndexedInstanced_Helper(deviceContext, indexCountPerInstance == 0 ? m_subsets[subsetId].IndexCount : indexCountPerInstance, instanceCount, m_subsets[subsetId]);
	}
}

} // End of namespace Scene
