/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/typedefs.h"

#include "graphics/d3d_util.h"

#include <d3d11.h>


namespace Graphics {

void SetConstants(ID3D11DeviceContext *context, ID3D11Buffer *buffer, void *data, size_t dataSize, uint slotNumber);

struct DefaultShaderConstantType {};

template <typename ShaderType, typename PerFrameType, typename PerObjectType>
class BaseShader {
protected:
	BaseShader(ID3D11Device *device, bool hasPerFrameBuffer, bool hasPerObjectBuffer)
			: m_d3dShader(nullptr),
			  m_perFrameConstantBuffer(nullptr), 
			  m_perObjectConstantBuffer(nullptr) {
		// Create the two buffers
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		if (hasPerFrameBuffer) {
			assert(sizeof(PerFrameType) > 0);
			bufferDesc.ByteWidth = static_cast<uint>(Graphics::CBSize(sizeof(PerFrameType)));
			device->CreateBuffer(&bufferDesc, nullptr, &m_perFrameConstantBuffer);
		}
		if (hasPerObjectBuffer) {
			assert(sizeof(PerObjectType) > 0);
			bufferDesc.ByteWidth = static_cast<uint>(Graphics::CBSize(sizeof(PerObjectType)));
			device->CreateBuffer(&bufferDesc, nullptr, &m_perObjectConstantBuffer);
		}
	}

public:
	virtual ~BaseShader() {
		ReleaseCOM(m_perObjectConstantBuffer);
		ReleaseCOM(m_perFrameConstantBuffer);
		ReleaseCOM(m_d3dShader);
	}

protected:
	ShaderType *m_d3dShader;
	ID3D11Buffer *m_perFrameConstantBuffer;
	ID3D11Buffer *m_perObjectConstantBuffer;

public:
	inline ID3D11Buffer *GetPerFrameConstantBuffer() { return m_perFrameConstantBuffer; }
	inline ID3D11Buffer *GetPerObjectConstantBuffer() { return m_perObjectConstantBuffer; }
	virtual inline void BindToPipeline(ID3D11DeviceContext *context) = 0;
	
};

template <typename PerFrameType = DefaultShaderConstantType, typename PerObjectType = DefaultShaderConstantType>
class VertexShader : public BaseShader<ID3D11VertexShader, PerFrameType, PerObjectType> {
public:
	VertexShader(const wchar *fileName, ID3D11Device *device, bool hasPerFrameBuffer, bool hasPerObjectBuffer, ID3D11InputLayout **inputLayout = nullptr, D3D11_INPUT_ELEMENT_DESC *vertexDesc = nullptr, uint numElements = 0) 
			: BaseShader(device, hasPerFrameBuffer, hasPerObjectBuffer) {
		LoadVertexShader(fileName, device, &m_d3dShader, inputLayout, vertexDesc, numElements);
	}

	inline virtual void BindToPipeline(ID3D11DeviceContext *context) {
		context->VSSetShader(m_d3dShader, nullptr, 0);
	}
};


template <typename PerFrameType = DefaultShaderConstantType, typename PerObjectType = DefaultShaderConstantType>
class PixelShader : public BaseShader<ID3D11PixelShader, PerFrameType, PerObjectType> {
public:
	PixelShader(const wchar *fileName, ID3D11Device *device, bool hasPerFrameBuffer, bool hasPerObjectBuffer)
			: BaseShader(device, hasPerFrameBuffer, hasPerObjectBuffer) {
		LoadPixelShader(fileName, device, &m_d3dShader);
	}

	inline virtual void BindToPipeline(ID3D11DeviceContext *context) {
		context->PSSetShader(m_d3dShader, nullptr, 0);
	}

	void SetPerFrameConstants(ID3D11DeviceContext *context, PerFrameType *perFrameData, uint slotNumber) {
		SetConstants(context, m_perFrameConstantBuffer, perFrameData, sizeof(PerFrameType), slotNumber);

		// Bind it to the shader
		context->PSSetConstantBuffers(slotNumber, 1u, &m_perFrameConstantBuffer);
	}

	void SetPerObjectConstants(ID3D11DeviceContext *context, PerObjectType *perObjectData, uint slotNumber) {
		SetConstants(context, m_perObjectConstantBuffer, perObjectData, sizeof(PerObjectType), slotNumber);

		// Bind it to the shader
		context->PSSetConstantBuffers(slotNumber, 1u, &m_perObjectConstantBuffer);
	}
};

template <typename PerFrameType = DefaultShaderConstantType, typename PerObjectType = DefaultShaderConstantType>
class ComputeShader : public BaseShader<ID3D11ComputeShader, PerFrameType, PerObjectType> {
public:
	ComputeShader(const wchar *fileName, ID3D11Device *device, bool hasPerFrameBuffer, bool hasPerObjectBuffer)
			: BaseShader(device, hasPerFrameBuffer, hasPerObjectBuffer) {
		LoadComputeShader(fileName, device, &m_d3dShader);
	}

	inline virtual void BindToPipeline(ID3D11DeviceContext *context) {
		context->CSSetShader(m_d3dShader, nullptr, 0);
	}

	void SetPerFrameConstants(ID3D11DeviceContext *context, PerFrameType *perFrameData, uint slotNumber) {
		SetConstants(context, m_perFrameConstantBuffer, perFrameData, sizeof(PerFrameType), slotNumber);

		// Bind it to the shader
		context->CSSetConstantBuffers(slotNumber, 1u, &m_perFrameConstantBuffer);
	}

	void SetPerObjectConstants(ID3D11DeviceContext *context, PerObjectType *perObjectData, uint slotNumber) {
		SetConstants(context, m_perObjectConstantBuffer, perObjectData, sizeof(PerObjectType), slotNumber);

		// Bind it to the shader
		context->CSSetConstantBuffers(slotNumber, 1u, &m_perObjectConstantBuffer);
	}
};

typedef PixelShader<> MaterialShader;

} // End of namespace Graphics
