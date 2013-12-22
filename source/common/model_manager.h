/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_MODEL_MANAGER_H
#define COMMON_MODEL_MANAGER_H

#include "common/typedefs.h"
#include "common/d3d_util.h"

#include "DirectXMath.h"
#include <d3d11.h>

#include <vector>
#include <unordered_map>


namespace Common {

template <class Vertex>
struct Model {
	std::vector<Vertex> Vertices;
	std::vector<uint> Indices;

	uint VertexOffset;
	uint IndexOffset;
};

template <class Vertex>
class ModelManager {
public:
	ModelManager()
		: m_staticVertexBuffer(nullptr),
		  m_staticIndexBuffer(nullptr),
		  m_staticKey(0),
		  m_dynamicKey(0) {
	}
	~ModelManager() {
		ReleaseCOM(m_staticVertexBuffer);
		ReleaseCOM(m_staticIndexBuffer);
		for (std::unordered_map<uint, ID3D11Buffer *>::iterator iter = m_dynamicVertexBuffers.begin(); iter != m_dynamicVertexBuffers.end(); ++iter) {
			ReleaseCOM((*iter).second);
		}
		for (std::unordered_map<uint, ID3D11Buffer *>::iterator iter = m_dynamicIndexBuffers.begin(); iter != m_dynamicIndexBuffers.end(); ++iter) {
			ReleaseCOM((*iter).second);
		}
	}

private:
	ID3D11Device **m_device;

	// Vertex and Index buffers
	ID3D11Buffer *m_staticVertexBuffer;
	ID3D11Buffer *m_staticIndexBuffer;
	std::unordered_map<uint, ID3D11Buffer *> m_dynamicVertexBuffers;
	std::unordered_map<uint, ID3D11Buffer *> m_dynamicIndexBuffers;

	std::unordered_map<uint, Model<Vertex> > m_staticModels;
	std::unordered_map<uint, Model<Vertex> > m_dynamicModels;

	uint m_staticKey;
	uint m_dynamicKey;

public:
	bool Initialize(ID3D11Device **graphicsDevice) {
		m_device = graphicsDevice;
		return true;
	}

	uint AddModel(const Model<Vertex> &model, uint verticesSize, uint indicesSize, bool isStatic = true) {
		if (isStatic) {
			m_staticModels[m_staticKey] = model;

			return m_staticKey++;
		} else {
			m_dynamicModels[m_dynamicKey] = model;

			D3D11_BUFFER_DESC vbd;
			vbd.Usage = D3D11_USAGE_DYNAMIC;
			vbd.ByteWidth = sizeof(Vertex) * verticesSize;
			vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			vbd.MiscFlags = 0;
			vbd.StructureByteStride = 0;

			if (model.Vertices.size() > 0) {
				D3D11_SUBRESOURCE_DATA vInitData;
				vInitData.pSysMem = &model.Vertices[0];

				HR((*m_device)->CreateBuffer(&vbd, &vInitData, &m_dynamicVertexBuffers[m_dynamicKey]));
			} else {
				HR((*m_device)->CreateBuffer(&vbd, 0, &m_dynamicVertexBuffers[m_dynamicKey]));
			}

			D3D11_BUFFER_DESC ibd;
			ibd.Usage = D3D11_USAGE_DYNAMIC;
			ibd.ByteWidth = sizeof(uint) * indicesSize;
			ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			ibd.MiscFlags = 0;
			ibd.StructureByteStride = 0;

			if (model.Indices.size() > 0) {
				D3D11_SUBRESOURCE_DATA iInitData;
				iInitData.pSysMem = &model.Indices[0];

				HR((*m_device)->CreateBuffer(&ibd, &iInitData, &m_dynamicIndexBuffers[m_dynamicKey]));
			} else {
				HR((*m_device)->CreateBuffer(&ibd, 0, &m_dynamicIndexBuffers[m_dynamicKey]));
			}

			return m_dynamicKey++;
		}
	}

	void InitStaticBuffers() {
		if (m_staticModels.size() == 0)
			return;

		std::vector<Vertex> vertices;
		std::vector<uint> indices;
		for (std::unordered_map<uint, Model<Vertex> >::iterator iter = m_staticModels.begin(); iter != m_staticModels.end(); ++iter) {
			iter->second.VertexOffset = vertices.size();
			vertices.insert(vertices.end(), iter->second.Vertices.begin(), iter->second.Vertices.end());
			iter->second.IndexOffset = indices.size();
			indices.insert(indices.end(), iter->second.Indices.begin(), iter->second.Indices.end());
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;
			
		D3D11_SUBRESOURCE_DATA vInitData;
		vInitData.pSysMem = &vertices[0];

		HR((*m_device)->CreateBuffer(&vbd, &vInitData, &m_staticVertexBuffer));

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(uint) * indices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA iInitData;
		iInitData.pSysMem = &indices[0];

		HR((*m_device)->CreateBuffer(&ibd, &iInitData, &m_staticIndexBuffer));
	}

	void MapDynamicVertexBuffer(uint modelId, ID3D11DeviceContext *deviceContext, D3D11_MAPPED_SUBRESOURCE *mappedData) {
		HR(deviceContext->Map(m_dynamicVertexBuffers[modelId], 0, D3D11_MAP_WRITE_DISCARD, 0, mappedData));
	}

	void UnMapDynamicVertexBuffer(uint modelId, ID3D11DeviceContext *deviceContext) {
		deviceContext->Unmap(m_dynamicVertexBuffers[modelId], 0);
	}

	void MapDynamicIndexBuffer(uint modelId, ID3D11DeviceContext *deviceContext, D3D11_MAPPED_SUBRESOURCE *mappedData) {
		HR(deviceContext->Map(m_dynamicIndexBuffers[modelId], 0, D3D11_MAP_WRITE_DISCARD, 0, mappedData));
	}

	void UnMapDynamicIndexBuffer(uint modelId, ID3D11DeviceContext *deviceContext) {
		deviceContext->Unmap(m_dynamicIndexBuffers[modelId], 0);
	}

	void StaticModelsPreRender(ID3D11DeviceContext *deviceContext, ID3D11InputLayout *inputLayout) {
		deviceContext->IASetInputLayout(inputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		uint stride = sizeof(Vertex);
		uint offset = 0;

		deviceContext->IASetVertexBuffers(0, 1, &m_staticVertexBuffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(m_staticIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}

	void RenderStaticModel(uint modelId, ID3D11DeviceContext *deviceContext) {
		Model<Vertex> *model = &m_staticModels.at(modelId);
		deviceContext->DrawIndexed(model->Indices.size(), model->IndexOffset, model->VertexOffset);
	}

	void RenderDynamicModel(uint modelId, ID3D11DeviceContext *deviceContext, ID3D11InputLayout *inputLayout) {
		deviceContext->IASetInputLayout(inputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		uint stride = sizeof(Vertex);
		uint offset = 0;

		deviceContext->IASetVertexBuffers(0, 1, &m_dynamicVertexBuffers[modelId], &stride, &offset);
		deviceContext->IASetIndexBuffer(m_dynamicIndexBuffers[modelId], DXGI_FORMAT_R32_UINT, 0);

		deviceContext->DrawIndexed(m_dynamicModels[modelId].Indices.size(), 0, 0);
	}
};

} // End of namespace Common

#endif
