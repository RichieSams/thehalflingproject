/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/model.h"


namespace Common {

void Model::CreateVertexBuffer(ID3D11Device *device, void *vertices, size_t vertexStride, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse) {
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = vertexStride * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	CreateVertexBuffer(device, vertices, vertexCount, vbd, disposeAfterUse);
}

void Model::CreateVertexBuffer(ID3D11Device *device, void *vertices, uint vertexCount, D3D11_BUFFER_DESC vertexBufferDesc, DisposeAfterUse::Flag disposeAfterUse) {
	m_vertexStride = vertexBufferDesc.ByteWidth / vertexCount;
	
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices;

	HR(device->CreateBuffer(&vertexBufferDesc, &vInitData, &m_vertexBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] vertices;
	}
}

void Model::CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, DisposeAfterUse::Flag disposeAfterUse) {
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(uint) * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	
	CreateIndexBuffer(device, indices, indexCount, ibd, disposeAfterUse);
}

void Model::CreateIndexBuffer(ID3D11Device *device, uint *indices, uint indexCount, D3D11_BUFFER_DESC indexBufferDesc, DisposeAfterUse::Flag disposeAfterUse) {
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = indices;
	
	HR(device->CreateBuffer(&indexBufferDesc, &iInitData, &m_indexBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] indices;
	}
}

void Model::CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse) {
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

void Model::DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId) {
	assert(subsetId >= -1 && subsetId < (int)m_subsetCount);

	uint offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexStride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			deviceContext->PSSetShaderResources(0, m_subsets[i].TextureSRVs.size(), &m_subsets[i].TextureSRVs[0]);
			deviceContext->DrawIndexed(m_subsets[i].IndexCount, m_subsets[i].IndexStart, m_subsets[i].VertexStart);
		}
	} else {
		deviceContext->PSSetShaderResources(0, m_subsets[subsetId].TextureSRVs.size(), &m_subsets[subsetId].TextureSRVs[0]);
		deviceContext->DrawIndexed(m_subsets[subsetId].IndexCount, m_subsets[subsetId].IndexStart, m_subsets[subsetId].VertexStart);
	}
}

void Model::DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint instanceCount, uint indexCountPerInstance, int subsetId) {
	uint offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexStride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			deviceContext->PSSetShaderResources(0, m_subsets[i].TextureSRVs.size(), &m_subsets[i].TextureSRVs[0]);
			deviceContext->DrawIndexedInstanced(indexCountPerInstance == 0 ? m_subsets[i].IndexCount : indexCountPerInstance, instanceCount, m_subsets[i].IndexStart, m_subsets[i].VertexStart, 0);
		}
	} else {
		deviceContext->PSSetShaderResources(0, m_subsets[subsetId].TextureSRVs.size(), &m_subsets[subsetId].TextureSRVs[0]);
		deviceContext->DrawIndexedInstanced(indexCountPerInstance == 0 ? m_subsets[subsetId].IndexCount : indexCountPerInstance, instanceCount, m_subsets[subsetId].IndexStart, m_subsets[subsetId].VertexStart, 0);
	}
}


void InstancedModel::CreateInstanceBuffer(ID3D11Device *device, size_t instanceStride, uint maxInstanceCount, void *instanceData, DisposeAfterUse::Flag disposeAfterUse) {
	m_instanceStride = instanceStride;
	m_maxInstanceCount = maxInstanceCount;

	D3D11_BUFFER_DESC instbd;
	instbd.Usage = D3D11_USAGE_DYNAMIC;
	instbd.ByteWidth = instanceStride * maxInstanceCount;
	instbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //needed for Map/Unmap
	instbd.MiscFlags = 0;
	instbd.StructureByteStride = 0;

	CreateInstanceBuffer(device, instbd, instanceData, disposeAfterUse);
}

void InstancedModel::CreateInstanceBuffer(ID3D11Device *device, D3D11_BUFFER_DESC instanceBufferDesc, void *instanceData, DisposeAfterUse::Flag disposeAfterUse) {
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
	deviceContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			deviceContext->PSSetShaderResources(0, m_subsets[i].TextureSRVs.size(), &m_subsets[i].TextureSRVs[0]);
			deviceContext->DrawIndexedInstanced(indexCountPerInstance == 0 ? m_subsets[i].IndexCount : indexCountPerInstance, instanceCount, m_subsets[i].IndexStart, m_subsets[i].VertexStart, 0);
		}
	} else {
		deviceContext->PSSetShaderResources(0, m_subsets[subsetId].TextureSRVs.size(), &m_subsets[subsetId].TextureSRVs[0]);
		deviceContext->DrawIndexedInstanced(indexCountPerInstance == 0 ? m_subsets[subsetId].IndexCount : indexCountPerInstance, instanceCount, m_subsets[subsetId].IndexStart, m_subsets[subsetId].VertexStart, 0);
	}
}

} // End of namespace Common
