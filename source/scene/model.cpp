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
	VertexStride = vertexBufferDesc.ByteWidth / vertexCount;
	
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices;

	HR(device->CreateBuffer(&vertexBufferDesc, &vInitData, &VertexBuffer));

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
	
	HR(device->CreateBuffer(&indexBufferDesc, &iInitData, &IndexBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] indices;
	}
}

void Model::CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse disposeAfterUse) {
	Subsets = subsetArray;
	SubsetCount = subsetCount;
	m_disposeSubsetArray = disposeAfterUse;

	DirectX::XMVECTOR tempAABB_min = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR tempAABB_max = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	for (uint i = 0; i < subsetCount; ++i) {
		tempAABB_min = DirectX::XMVectorMin(tempAABB_min, DirectX::XMLoadFloat3(&subsetArray[i].AABB_min));
		tempAABB_max = DirectX::XMVectorMax(tempAABB_max, DirectX::XMLoadFloat3(&subsetArray[i].AABB_max));
	}

	DirectX::XMStoreFloat3(&AABB_min, tempAABB_min);
	DirectX::XMStoreFloat3(&AABB_max, tempAABB_max);
}

void InstancedModel::CreateInstanceBuffer(ID3D11Device *device, size_t instanceStride, uint maxInstanceCount, void *instanceData, DisposeAfterUse disposeAfterUse) {
	InstanceStride = static_cast<uint>(instanceStride);
	MaxInstanceCount = maxInstanceCount;

	D3D11_BUFFER_DESC instbd;
	instbd.Usage = D3D11_USAGE_DYNAMIC;
	instbd.ByteWidth = InstanceStride * maxInstanceCount;
	instbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //needed for Map/Unmap
	instbd.MiscFlags = 0;
	instbd.StructureByteStride = 0;

	CreateInstanceBuffer(device, instbd, instanceData, disposeAfterUse);
}

void InstancedModel::CreateInstanceBuffer(ID3D11Device *device, D3D11_BUFFER_DESC instanceBufferDesc, void *instanceData, DisposeAfterUse disposeAfterUse) {
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = instanceData;

	HR(device->CreateBuffer(&instanceBufferDesc, instanceData ? &iInitData : nullptr, &InstanceBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] instanceData;
	}
}

void *InstancedModel::MapInstanceBuffer(ID3D11DeviceContext *deviceContext, uint *out_maxNumInstances) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	*out_maxNumInstances = MaxInstanceCount;
	return mappedResource.pData;
}

void InstancedModel::UnMapInstanceBuffer(ID3D11DeviceContext *deviceContext) {
	deviceContext->Unmap(InstanceBuffer, 0);
}

} // End of namespace Scene
