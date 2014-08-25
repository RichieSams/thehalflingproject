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
 * Copyright Adrian Astley 2013 - 2014
 */

#include "graphics/device_states.h"

#include "graphics/d3d_util.h"


namespace Graphics {

void BlendStateManager::Initialize(ID3D11Device *device) {
	HR(device->CreateBlendState(&BlendDisabledDesc(), &m_blendDisabled));
	HR(device->CreateBlendState(&AdditiveBlendDesc(), &m_additiveBlend));
	HR(device->CreateBlendState(&AlphaBlendDesc(), &m_alphaBlend));
	HR(device->CreateBlendState(&PreMultipliedAlphaBlendDesc(), &m_pmAlphaBlend));
	HR(device->CreateBlendState(&ColorWriteDisabledDesc(), &m_noColor));
	HR(device->CreateBlendState(&AlphaToCoverageDesc(), &m_alphaToCoverage));
	HR(device->CreateBlendState(&OpacityBlendDesc(), &m_opacityBlend));
}

BlendStateManager::~BlendStateManager() {
	ReleaseCOM(m_blendDisabled);
	ReleaseCOM(m_additiveBlend);
	ReleaseCOM(m_alphaBlend);
	ReleaseCOM(m_pmAlphaBlend);
	ReleaseCOM(m_noColor);
	ReleaseCOM(m_alphaToCoverage);
	ReleaseCOM(m_opacityBlend);
}

D3D11_BLEND_DESC BlendStateManager::BlendDisabledDesc() {
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	for (UINT i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = false;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}

	return blendDesc;
}

D3D11_BLEND_DESC BlendStateManager::AdditiveBlendDesc() {
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	for (uint32 i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = true;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}

	return blendDesc;
}

D3D11_BLEND_DESC BlendStateManager::AlphaBlendDesc() {
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = true;
	for (uint32 i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = true;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}

	blendDesc.RenderTarget[0].BlendEnable = true;

	return blendDesc;
}

D3D11_BLEND_DESC BlendStateManager::PreMultipliedAlphaBlendDesc() {
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	for (uint32 i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = false;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}

	return blendDesc;
}

D3D11_BLEND_DESC BlendStateManager::ColorWriteDisabledDesc() {
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	for (uint32 i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = false;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = 0;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}

	return blendDesc;
}

D3D11_BLEND_DESC BlendStateManager::AlphaToCoverageDesc() {
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = true;
	blendDesc.IndependentBlendEnable = false;
	for (uint32 i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = false;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}

	return blendDesc;
}

D3D11_BLEND_DESC BlendStateManager::OpacityBlendDesc() {
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	for (uint32 i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].BlendEnable = true;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC1_COLOR;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}

	return blendDesc;
}

void RasterizerStateManager::Initialize(ID3D11Device *device) {
	HR(device->CreateRasterizerState(&NoCullDesc(), &m_noCull));
	HR(device->CreateRasterizerState(&FrontFaceCullDesc(), &m_cullFrontFaces));
	HR(device->CreateRasterizerState(&FrontFaceCullScissorDesc(), &m_cullFrontFacesScissor));
	HR(device->CreateRasterizerState(&BackFaceCullDesc(), &m_cullBackFaces));
	HR(device->CreateRasterizerState(&BackFaceCullScissorDesc(), &m_cullBackFacesScissor));
	HR(device->CreateRasterizerState(&NoCullNoMSDesc(), &m_noCullNoMS));
	HR(device->CreateRasterizerState(&NoCullScissorDesc(), &m_noCullScissor));
	HR(device->CreateRasterizerState(&WireframeDesc(), &m_wireframe));
}

RasterizerStateManager::~RasterizerStateManager() {
	ReleaseCOM(m_noCull);
	ReleaseCOM(m_cullFrontFaces);
	ReleaseCOM(m_cullFrontFacesScissor);
	ReleaseCOM(m_cullBackFaces);
	ReleaseCOM(m_cullBackFacesScissor);
	ReleaseCOM(m_noCullNoMS);
	ReleaseCOM(m_noCullScissor);
	ReleaseCOM(m_wireframe);
}

D3D11_RASTERIZER_DESC RasterizerStateManager::NoCullDesc() {
	D3D11_RASTERIZER_DESC rastDesc;

	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = true;
	rastDesc.ScissorEnable = false;
	rastDesc.SlopeScaledDepthBias = 0;

	return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStateManager::FrontFaceCullDesc() {
	D3D11_RASTERIZER_DESC rastDesc;

	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = true;
	rastDesc.ScissorEnable = false;
	rastDesc.SlopeScaledDepthBias = 0;

	return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStateManager::FrontFaceCullScissorDesc() {
	D3D11_RASTERIZER_DESC rastDesc;

	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = true;
	rastDesc.MultisampleEnable = true;
	rastDesc.ScissorEnable = true;
	rastDesc.SlopeScaledDepthBias = 0;

	return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStateManager::BackFaceCullDesc() {
	D3D11_RASTERIZER_DESC rastDesc;

	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = true;
	rastDesc.ScissorEnable = false;
	rastDesc.SlopeScaledDepthBias = 0;

	return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStateManager::BackFaceCullScissorDesc() {
	D3D11_RASTERIZER_DESC rastDesc;

	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = true;
	rastDesc.ScissorEnable = true;
	rastDesc.SlopeScaledDepthBias = 0;

	return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStateManager::NoCullNoMSDesc() {
	D3D11_RASTERIZER_DESC rastDesc;

	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = false;
	rastDesc.ScissorEnable = false;
	rastDesc.SlopeScaledDepthBias = 0;

	return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStateManager::NoCullScissorDesc() {
	D3D11_RASTERIZER_DESC rastDesc;

	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = true;
	rastDesc.ScissorEnable = true;
	rastDesc.SlopeScaledDepthBias = 0;

	return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStateManager::WireframeDesc() {
	D3D11_RASTERIZER_DESC rastDesc;

	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_WIREFRAME;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = true;
	rastDesc.ScissorEnable = false;
	rastDesc.SlopeScaledDepthBias = 0;

	return rastDesc;
}

void DepthStencilStateManager::Initialize(ID3D11Device *device) {
	HR(device->CreateDepthStencilState(&DepthDisabledDesc(), &m_depthDisabled));
	HR(device->CreateDepthStencilState(&DepthEnabledDesc(), &m_depthEnabled));
	HR(device->CreateDepthStencilState(&ReverseDepthEnabledDesc(), &m_revDepthEnabled));
	HR(device->CreateDepthStencilState(&DepthWriteEnabledDesc(), &m_depthWriteEnabled));
	HR(device->CreateDepthStencilState(&ReverseDepthWriteEnabledDesc(), &m_revDepthWriteEnabled));
	HR(device->CreateDepthStencilState(&DepthStencilWriteEnabledDesc(), &m_depthStencilWriteEnabled));
	HR(device->CreateDepthStencilState(&StencilEnabledDesc(), &m_stencilEnabled));
}

DepthStencilStateManager::~DepthStencilStateManager() {
	ReleaseCOM(m_depthDisabled);
	ReleaseCOM(m_depthEnabled);
	ReleaseCOM(m_revDepthEnabled);
	ReleaseCOM(m_depthWriteEnabled);
	ReleaseCOM(m_revDepthWriteEnabled);
	ReleaseCOM(m_depthStencilWriteEnabled);
	ReleaseCOM(m_stencilEnabled);
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStateManager::DepthDisabledDesc() {
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace = dsDesc.FrontFace;

	return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStateManager::DepthEnabledDesc() {
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace = dsDesc.FrontFace;

	return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStateManager::ReverseDepthEnabledDesc() {
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace = dsDesc.FrontFace;

	return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStateManager::DepthWriteEnabledDesc() {
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace = dsDesc.FrontFace;

	return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStateManager::ReverseDepthWriteEnabledDesc() {
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace = dsDesc.FrontFace;

	return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStateManager::DepthStencilWriteEnabledDesc() {
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace = dsDesc.FrontFace;

	return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStateManager::StencilEnabledDesc() {
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = 0;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	dsDesc.BackFace = dsDesc.FrontFace;

	return dsDesc;
}

void SamplerStateManager::Initialize(ID3D11Device *device) {
	HR(device->CreateSamplerState(&LinearDesc(), &m_linear));
	HR(device->CreateSamplerState(&LinearClampDesc(), &m_linearClamp));
	HR(device->CreateSamplerState(&LinearBorderDesc(), &m_linearBorder));
	HR(device->CreateSamplerState(&PointDesc(), &m_point));
	HR(device->CreateSamplerState(&PointWrapDesc(), &m_pointWrap));
	HR(device->CreateSamplerState(&AnisotropicDesc(), &m_anisotropic));
	HR(device->CreateSamplerState(&ShadowMapDesc(), &m_shadowMap));
	HR(device->CreateSamplerState(&ShadowMapPCFDesc(), &m_shadowMapPCF));
}

SamplerStateManager::~SamplerStateManager() {
	ReleaseCOM(m_linear);
	ReleaseCOM(m_linearClamp);
	ReleaseCOM(m_linearBorder);
	ReleaseCOM(m_point);
	ReleaseCOM(m_pointWrap);
	ReleaseCOM(m_anisotropic);
	ReleaseCOM(m_shadowMap);
	ReleaseCOM(m_shadowMapPCF);
}

D3D11_SAMPLER_DESC SamplerStateManager::LinearDesc() {
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStateManager::LinearClampDesc() {
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStateManager::LinearBorderDesc() {
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStateManager::PointDesc() {
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStateManager::PointWrapDesc() {
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStateManager::AnisotropicDesc() {
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStateManager::ShadowMapDesc() {
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStateManager::ShadowMapPCFDesc() {
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	return sampDesc;
}

} // End of namespace Graphics