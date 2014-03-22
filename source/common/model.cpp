/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/model.h"


namespace Common {

void *Model::operator new(size_t size) {
	return _aligned_malloc(size, 16);
}

void Model::operator delete(void *memory) {
	_aligned_free(memory);
}

const Common::BlinnPhongMaterial &Model::GetSubsetMaterial(uint subsetIndex) const {
	if (subsetIndex < m_subsetCount) {
		return m_subsets[subsetIndex].Material;
	}

	throw std::out_of_range("subsetIndex out of range");
}

uint Model::GetSubsetTextureFlags(uint subsetIndex) const {
	if (subsetIndex < m_subsetCount) {
		return m_subsets[subsetIndex].TextureFlags;
	}

	throw std::out_of_range("subsetIndex out of range");
}

void Model::CreateVertexBuffer(ID3D11Device *device, void *vertices, size_t vertexStride, uint vertexCount, DisposeAfterUse::Flag disposeAfterUse) {
	m_vertexStride = vertexStride;

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

void Model::CreateInstanceBuffer(ID3D11Device *device, size_t instanceStride, uint maxInstanceCount, void *instanceData, DisposeAfterUse::Flag disposeAfterUse) {
	m_instanceStride = instanceStride;
	m_maxInstanceCount = maxInstanceCount;

	D3D11_BUFFER_DESC instbd;
	instbd.Usage = D3D11_USAGE_DYNAMIC;
	instbd.ByteWidth = instanceStride * maxInstanceCount;
	instbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //needed for Map/Unmap
	instbd.MiscFlags = 0;
	instbd.StructureByteStride = 0;

	CreateInstanceBuffer(device, maxInstanceCount, instbd, instanceData, disposeAfterUse);
}

void Model::CreateInstanceBuffer(ID3D11Device *device, uint maxInstanceCount, D3D11_BUFFER_DESC instanceBufferDesc, void *instanceData, DisposeAfterUse::Flag disposeAfterUse) {
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = instanceData;

	HR(device->CreateBuffer(&instanceBufferDesc, instanceData ? &iInitData : nullptr, &m_instanceBuffer));

	if (disposeAfterUse == DisposeAfterUse::YES) {
		delete[] instanceData;
	}
}

void Model::CreateSubsets(ModelSubset *subsetArray, uint subsetCount, DisposeAfterUse::Flag disposeAfterUse) {
	m_subsets = subsetArray;
	m_subsetCount = subsetCount;
	m_disposeSubsetArray = disposeAfterUse;
}

void *Model::MapInstanceBuffer(ID3D11DeviceContext *deviceContext, uint *out_maxNumInstances) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	*out_maxNumInstances = m_maxInstanceCount;
	return mappedResource.pData;
}

void Model::UnMapInstanceBuffer(ID3D11DeviceContext *deviceContext) {
	deviceContext->Unmap(m_instanceBuffer, 0);
}

void Common::Model::DrawSubset(ID3D11DeviceContext *deviceContext, int subsetId) {
	assert(subsetId >= -1 && subsetId < (int)m_subsetCount);

	uint offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexStride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			if ((m_subsets[i].TextureFlags & TextureFlags::DIFFUSE_COLOR) == TextureFlags::DIFFUSE_COLOR) {
				deviceContext->PSSetShaderResources(0, 1, &m_subsets[i].DiffuseColorSRV);
			}
			deviceContext->DrawIndexed(m_subsets[i].IndexCount, m_subsets[i].IndexStart, 0);
		}
	} else {
		if ((m_subsets[subsetId].TextureFlags & TextureFlags::DIFFUSE_COLOR) == TextureFlags::DIFFUSE_COLOR) {
			deviceContext->PSSetShaderResources(0, 1, &m_subsets[subsetId].DiffuseColorSRV);
		}
		deviceContext->DrawIndexed(m_subsets[subsetId].IndexCount, m_subsets[subsetId].IndexStart, 0);
	}
}

void Model::DrawInstancedSubset(ID3D11DeviceContext *deviceContext, uint indexCountPerInstance, uint instanceCount, uint subsetId) {
	assert(m_instanceBuffer);
	assert(instanceCount <= m_maxInstanceCount);

	ID3D11Buffer *vbs[] = {m_vertexBuffer, m_instanceBuffer};
	uint strides[] = {m_vertexStride, sizeof(m_instanceStride)};
	uint offsets[] = {0, 0};
	deviceContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (subsetId == -1) {
		for (uint i = 0; i < m_subsetCount; ++i) {
			if ((m_subsets[i].TextureFlags & TextureFlags::DIFFUSE_COLOR) == TextureFlags::DIFFUSE_COLOR) {
				deviceContext->PSSetShaderResources(0, 1, &m_subsets[i].DiffuseColorSRV);
			}
			deviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, m_subsets[i].IndexStart, 0, 0);
		}
	} else {
		if ((m_subsets[subsetId].TextureFlags & TextureFlags::DIFFUSE_COLOR) == TextureFlags::DIFFUSE_COLOR) {
			deviceContext->PSSetShaderResources(0, 1, &m_subsets[subsetId].DiffuseColorSRV);
		}
		deviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, m_subsets[subsetId].IndexStart, 0, 0);
	}
}

} // End of namespace Common
