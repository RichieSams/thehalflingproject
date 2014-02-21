//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

/**
 * Modified for use in The Halfling Project - A Graphics Engine and Projects
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/sprite_renderer.h"

#include "common/sprite_font.h"
#include "common/d3d_util.h"

#include <algorithm>


namespace Common {

SpriteRenderer::SpriteRenderer()
	: m_context(nullptr),
	  m_initialized(false) {
}

SpriteRenderer::~SpriteRenderer() {
	// Release in the opposite order we initialized in
	ReleaseCOM(m_pointSamplerState);
	ReleaseCOM(m_linearSamplerState);
	ReleaseCOM(m_depthStencilState);
	ReleaseCOM(m_alphaBlendState);
	ReleaseCOM(m_rastState);
	ReleaseCOM(m_vsPerInstanceConstBuffer);
	ReleaseCOM(m_vsPerBatchConstBuffer);
	ReleaseCOM(m_indexBuffer);
	ReleaseCOM(m_instanceDataBuffer);
	ReleaseCOM(m_vertexBuffer);
	ReleaseCOM(m_pixelShader);
	ReleaseCOM(m_vertexShaderInstanced);
	ReleaseCOM(m_inputLayoutInstanced);
	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_inputLayout);
}

void SpriteRenderer::Initialize(ID3D11Device *device) {
	// Define the input layouts
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3D11_INPUT_ELEMENT_DESC layoutInstanced[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"SOURCERECT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	// Load the shaders
	HR(Common::LoadVertexShader("sprite_vertex_shader.cso", device, &m_vertexShader, &m_inputLayout, layout, 2));
	HR(Common::LoadVertexShader("sprite_instanced_vertex_shader.cso", device, &m_vertexShaderInstanced, &m_inputLayoutInstanced, layoutInstanced, 8));
	HR(Common::LoadPixelShader("sprite_pixel_shader.cso", device, &m_pixelShader));
	HR(Common::LoadPixelShader("sprite_sampled_pixel_shader.cso", device, &m_sampledPixelShader));

	// Create the vertex buffer
	SpriteVertex verts[] = {
		{DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)},
		{DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)},
		{DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f)},
		{DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f)}
	};

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(SpriteVertex) * 4;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = verts;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	HR(device->CreateBuffer(&desc, &initData, &m_vertexBuffer));

	// Create the instance data buffer
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = sizeof(SpriteDrawData) * MaxBatchSize;
	HR(device->CreateBuffer(&desc, nullptr, &m_instanceDataBuffer));

	// Create the index buffer
	uint16 indices[] = {0, 1, 2, 3, 0, 2};
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(uint16) * 6;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	initData.pSysMem = indices;
	HR(device->CreateBuffer(&desc, &initData, &m_indexBuffer));

	// Create our constant buffers
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = CBSize(sizeof(VSPerBatchCB));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HR(device->CreateBuffer(&desc, nullptr, &m_vsPerBatchConstBuffer));

	desc.ByteWidth = CBSize(sizeof(SpriteDrawData));
	HR(device->CreateBuffer(&desc, nullptr, &m_vsPerInstanceConstBuffer));

	// Create our states
	D3D11_RASTERIZER_DESC rastDesc;
	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 1.0f;
	rastDesc.DepthClipEnable = false;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = true;
	rastDesc.ScissorEnable = false;
	rastDesc.SlopeScaledDepthBias = 0;
	HR(device->CreateRasterizerState(&rastDesc, &m_rastState));

	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	for (UINT i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = true;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}
	HR(device->CreateBlendState(&blendDesc, &m_alphaBlendState));

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace = dsDesc.FrontFace;
	HR(device->CreateDepthStencilState(&dsDesc, &m_depthStencilState));

	// linear filtering
	D3D11_SAMPLER_DESC sampDesc;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 0;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 0;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.MinLOD = 0;
	sampDesc.MipLODBias = 0;
	HR(device->CreateSamplerState(&sampDesc, &m_linearSamplerState));

	// point filtering
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	HR(device->CreateSamplerState(&sampDesc, &m_pointSamplerState));

	m_initialized = true;
}

void SpriteRenderer::Begin(ID3D11DeviceContext *deviceContext, FilterMode filterMode) {
	assert(m_initialized);
	assert(!m_context);
	m_context = deviceContext;

	//D3DPERF_BeginEvent(0xFFFFFFFF, L"SpriteRenderer Begin/End");

	// Set the index buffer
	m_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the states
	float blendFactor[4] = {1, 1, 1, 1};
	m_context->RSSetState(m_rastState);
	m_context->OMSetBlendState(m_alphaBlendState, blendFactor, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_depthStencilState, 0);

	if (filterMode == Linear) {
		m_context->PSSetSamplers(0, 1, &m_linearSamplerState);
	} else if (filterMode == Point) {
		m_context->PSSetSamplers(0, 1, &m_pointSamplerState);
	}

	// Set the shaders
	m_context->GSSetShader(nullptr, nullptr, 0);
	m_context->DSSetShader(nullptr, nullptr, 0);
	m_context->HSSetShader(nullptr, nullptr, 0);
}

D3D11_TEXTURE2D_DESC SpriteRenderer::SetPerBatchData(ID3D11ShaderResourceView *texture) {
	// Set per-batch constants
	VSPerBatchCB perBatch;

	// Get the viewport dimensions
	UINT numViewports = 1;
	D3D11_VIEWPORT vp;
	m_context->RSGetViewports(&numViewports, &vp);
	perBatch.ViewportSize = DirectX::XMFLOAT2(static_cast<float>(vp.Width), static_cast<float>(vp.Height));

	D3D11_TEXTURE2D_DESC desc;

	// Get the size of the texture
	if (texture) {
		ID3D11Resource *resource;
		ID3D11Texture2D *texResource;
		texture->GetResource(&resource);
		texResource = reinterpret_cast<ID3D11Texture2D *>(resource);
		texResource->GetDesc(&desc);
		perBatch.TextureSize = DirectX::XMFLOAT2(static_cast<float>(desc.Width), static_cast<float>(desc.Height));

		m_context->PSSetShader(m_sampledPixelShader, nullptr, 0);
	} else {
		perBatch.TextureSize = DirectX::XMFLOAT2(1.0f, 1.0f);
		desc.Width = 1;
		desc.Height = 1;

		m_context->PSSetShader(m_pixelShader, nullptr, 0);
	}

	// Copy it into the buffer
	D3D11_MAPPED_SUBRESOURCE mapped;
	HR(m_context->Map(m_vsPerBatchConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	CopyMemory(mapped.pData, &perBatch, sizeof(VSPerBatchCB));
	m_context->Unmap(m_vsPerBatchConstBuffer, 0);

	return desc;
}

void SpriteRenderer::Render(ID3D11ShaderResourceView *texture, const DirectX::XMFLOAT4X4 &transform, const DirectX::XMFLOAT4 &color, const DirectX::XMFLOAT4 *drawRect) {
	assert(m_context);
	assert(m_initialized);

	//D3DPERF_BeginEvent(0xFFFFFFFF, L"SpriteRenderer Render");

	// Set the vertex shader
	m_context->VSSetShader(m_vertexShader, nullptr, 0);

	// Set the input layout
	m_context->IASetInputLayout(m_inputLayout);

	// Set the vertex buffer
	UINT stride = sizeof(SpriteVertex);
	UINT offset = 0;
	m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set per-batch constants
	D3D11_TEXTURE2D_DESC desc = SetPerBatchData(texture);

	// Set per-instance data
	SpriteDrawData perInstance;
	perInstance.Transform;
	DirectX::XMStoreFloat4x4(&perInstance.Transform, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&transform)));
	perInstance.Color = color;

	// Draw rect
	if (drawRect == nullptr) {
		perInstance.DrawRect = DirectX::XMFLOAT4(0, 0, static_cast<float>(desc.Width), static_cast<float>(desc.Height));
	} else {
		assert(drawRect->x >= 0 && drawRect->x < desc.Width);
		assert(drawRect->y >= 0 && drawRect->y < desc.Height);
		assert(drawRect->z > 0 && drawRect->x + drawRect->z < desc.Width);
		assert(drawRect->w > 0 && drawRect->y + drawRect->w < desc.Height);
		perInstance.DrawRect = *drawRect;
	}

	// Copy in the buffer data
	D3D11_MAPPED_SUBRESOURCE mapped;
	HR(m_context->Map(m_vsPerInstanceConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	CopyMemory(mapped.pData, &perInstance, sizeof(SpriteDrawData));
	m_context->Unmap(m_vsPerInstanceConstBuffer, 0);

	ID3D11Buffer *buffers[2] = {m_vsPerBatchConstBuffer, m_vsPerInstanceConstBuffer};
	m_context->VSSetConstantBuffers(0, 2, buffers);

	// Set the texture
	if (texture) {
		m_context->PSSetShaderResources(0, 1, &texture);
	}

	m_context->DrawIndexed(6, 0, 0);

	//D3DPERF_EndEvent();
}

void SpriteRenderer::RenderBatch(ID3D11ShaderResourceView *texture, const SpriteDrawData *drawData, uint64 numSprites) {
	assert(m_context);
	assert(m_initialized);

	//D3DPERF_BeginEvent(0xFFFFFFFF, L"SpriteRenderer RenderBatch");

	// Set the vertex shader
	m_context->VSSetShader(m_vertexShaderInstanced, nullptr, 0);

	// Set the input layout
	m_context->IASetInputLayout(m_inputLayoutInstanced);

	// Set per-batch constants
	D3D11_TEXTURE2D_DESC desc = SetPerBatchData(texture);

	// Make sure the draw rects are all valid
	for (uint64 i = 0; i < numSprites; ++i) {
		DirectX::XMFLOAT4 drawRect = drawData[i].DrawRect;
		assert(drawRect.x >= 0 && drawRect.x < desc.Width);
		assert(drawRect.y >= 0 && drawRect.y < desc.Height);
		assert(drawRect.z > 0 && drawRect.x + drawRect.z <= desc.Width);
		assert(drawRect.w > 0 && drawRect.y + drawRect.w <= desc.Height);
	}

	uint64 numSpritesToDraw = std::min(numSprites, MaxBatchSize);

	// Copy in the instance data
	D3D11_MAPPED_SUBRESOURCE mapped;
	HR(m_context->Map(m_instanceDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	CopyMemory(mapped.pData, drawData, static_cast<size_t>(sizeof(SpriteDrawData)* numSpritesToDraw));
	m_context->Unmap(m_instanceDataBuffer, 0);

	// Set the constant buffer
	ID3D11Buffer *constantBuffers[1] = {m_vsPerBatchConstBuffer};
	m_context->VSSetConstantBuffers(0, 1, constantBuffers);

	// Set the vertex buffers
	UINT strides[2] = {sizeof(SpriteVertex), sizeof(SpriteDrawData)};
	UINT offsets[2] = {0, 0};
	ID3D11Buffer *vertexBuffers[2] = {m_vertexBuffer, m_instanceDataBuffer};
	m_context->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);

	// Set the texture
	m_context->PSSetShaderResources(0, 1, &texture);

	// Draw
	m_context->DrawIndexedInstanced(6, static_cast<UINT>(numSpritesToDraw), 0, 0, 0);

	//D3DPERF_EndEvent();

	// If there's any left to be rendered, do it recursively
	if (numSprites > numSpritesToDraw) {
		RenderBatch(texture, drawData + numSpritesToDraw, numSprites - numSpritesToDraw);
	}
}

const DirectX::XMFLOAT2 SpriteRenderer::RenderText(const SpriteFont &font, const wchar *text, const DirectX::XMFLOAT4X4 &transform, const uint maxWidth, const DirectX::XMFLOAT4 &color) {
	//D3DPERF_BeginEvent(0xFFFFFFFF, L"SpriteRenderer RenderText");

	uint64 length = wcslen(text);

	DirectX::XMFLOAT4X4 textTransform {1.0f, 0.0f, 0.0f, 0.0f,
	                                   0.0f, 1.0f, 0.0f, 0.0f,
	                                   0.0f, 0.0f, 1.0f, 0.0f,
	                                   0.0f, 0.0f, 0.0f, 1.0f};

	uint64 numCharsToDraw = std::min(length, MaxBatchSize);
	uint64 currentDraw = 0;
	float spaceWidth = font.SpaceWidth();
	for (uint64 i = 0; i < numCharsToDraw; ++i) {
		wchar character = text[i];
		if (character == ' ') {
			// Check for wrapping
			if (maxWidth != 0U && textTransform._41 + spaceWidth > maxWidth) {
				textTransform._42 += font.CharHeight();
				textTransform._41 = 0;
			}

			textTransform._41 += spaceWidth;
		} else if (character == '\t') {
			// Check for wrapping
			if (maxWidth != 0U && textTransform._41 + 4.0f * spaceWidth > maxWidth) {
				textTransform._42 += font.CharHeight();
				textTransform._41 = 0;
			}

			textTransform._41 += 4.0f * spaceWidth;
		} else if (character == '\n' || character == '\r') {
			textTransform._42 += font.CharHeight();
			textTransform._41 = 0;
		} else {
			SpriteFont::CharDesc desc = font.GetCharDescriptor(character);

			// Check for wrapping
			if (maxWidth != 0U && textTransform._41 + desc.Width + 1 > maxWidth) {
				textTransform._42 += font.CharHeight();
				textTransform._41 = 0;
			}

			DirectX::XMMATRIX transformMatrix = DirectX::XMLoadFloat4x4(&textTransform) * DirectX::XMLoadFloat4x4(&transform);
			DirectX::XMStoreFloat4x4(&textDrawData[currentDraw].Transform, transformMatrix);
			textDrawData[currentDraw].Color = color;
			textDrawData[currentDraw].DrawRect.x = desc.X;
			textDrawData[currentDraw].DrawRect.y = desc.Y;
			textDrawData[currentDraw].DrawRect.z = desc.Width;
			textDrawData[currentDraw].DrawRect.w = desc.Height;
			currentDraw++;

			textTransform._41 += desc.Width + 1;
		}
	}

	// Submit a batch
	RenderBatch(font.SRView(), textDrawData, currentDraw);

	//D3DPERF_EndEvent();

	if (length > numCharsToDraw) {
		RenderText(font, text + numCharsToDraw, textTransform, 0U, color);
	}

	return DirectX::XMFLOAT2(textTransform._41 + transform._41, textTransform._42 + transform._42);
}

void SpriteRenderer::End() {
	_ASSERT(m_context);
	_ASSERT(m_initialized);
	m_context = nullptr;

	//D3DPERF_EndEvent();
}

} // End of namespace Common