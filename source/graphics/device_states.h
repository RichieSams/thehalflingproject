//=================================================================================================
//
//	MJP's DX11 Sample Framework
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

#pragma once

#include <d3d11.h>


namespace Graphics {

class BlendStates {
public:
	~BlendStates();

private:
	ID3D11BlendState *m_blendDisabled;
	ID3D11BlendState *m_additiveBlend;
	ID3D11BlendState *m_alphaBlend;
	ID3D11BlendState *m_pmAlphaBlend;
	ID3D11BlendState *m_noColor;
	ID3D11BlendState *m_alphaToCoverage;
	ID3D11BlendState *m_opacityBlend;

public:
	void Initialize(ID3D11Device *device);

	inline ID3D11BlendState *BlendDisabled() { return m_blendDisabled; };
	inline ID3D11BlendState *AdditiveBlend() { return m_additiveBlend; };
	inline ID3D11BlendState *AlphaBlend() { return m_alphaBlend; };
	inline ID3D11BlendState *PreMultipliedAlphaBlend() { return m_pmAlphaBlend; };
	inline ID3D11BlendState *ColorWriteDisabled() { return m_noColor; };
	inline ID3D11BlendState *AlphaToCoverage() { return m_alphaToCoverage; };
	inline ID3D11BlendState *OpacityBlend() { return m_opacityBlend; };

	static D3D11_BLEND_DESC BlendDisabledDesc();
	static D3D11_BLEND_DESC AdditiveBlendDesc();
	static D3D11_BLEND_DESC AlphaBlendDesc();
	static D3D11_BLEND_DESC PreMultipliedAlphaBlendDesc();
	static D3D11_BLEND_DESC ColorWriteDisabledDesc();
	static D3D11_BLEND_DESC AlphaToCoverageDesc();
	static D3D11_BLEND_DESC OpacityBlendDesc();
};


class RasterizerStates {
public:
	~RasterizerStates();

private:
	ID3D11RasterizerState *m_noCull;
	ID3D11RasterizerState *m_cullBackFaces;
	ID3D11RasterizerState *m_cullBackFacesScissor;
	ID3D11RasterizerState *m_cullFrontFaces;
	ID3D11RasterizerState *m_cullFrontFacesScissor;
	ID3D11RasterizerState *m_noCullNoMS;
	ID3D11RasterizerState *m_noCullScissor;
	ID3D11RasterizerState *m_wireframe;

public:
	void Initialize(ID3D11Device *device);

	ID3D11RasterizerState *NoCull() { return m_noCull; };
	ID3D11RasterizerState *BackFaceCull() { return m_cullBackFaces; };
	ID3D11RasterizerState *BackFaceCullScissor() { return m_cullBackFacesScissor; };
	ID3D11RasterizerState *FrontFaceCull() { return m_cullFrontFaces; };
	ID3D11RasterizerState *FrontFaceCullScissor() { return m_cullFrontFacesScissor; };
	ID3D11RasterizerState *NoCullNoMS() { return m_noCullNoMS; };
	ID3D11RasterizerState *NoCullScissor() { return m_noCullScissor; };
	ID3D11RasterizerState *Wireframe() { return m_wireframe; };

	static D3D11_RASTERIZER_DESC NoCullDesc();
	static D3D11_RASTERIZER_DESC FrontFaceCullDesc();
	static D3D11_RASTERIZER_DESC FrontFaceCullScissorDesc();
	static D3D11_RASTERIZER_DESC BackFaceCullDesc();
	static D3D11_RASTERIZER_DESC BackFaceCullScissorDesc();
	static D3D11_RASTERIZER_DESC NoCullNoMSDesc();
	static D3D11_RASTERIZER_DESC NoCullScissorDesc();
	static D3D11_RASTERIZER_DESC WireframeDesc();
};


class DepthStencilStates {
public:
	~DepthStencilStates();

private:
	ID3D11DepthStencilState *m_depthDisabled;
	ID3D11DepthStencilState *m_depthEnabled;
	ID3D11DepthStencilState *m_revDepthEnabled;
	ID3D11DepthStencilState *m_depthWriteEnabled;
	ID3D11DepthStencilState *m_revDepthWriteEnabled;
	ID3D11DepthStencilState *m_depthStencilWriteEnabled;
	ID3D11DepthStencilState *m_stencilEnabled;

public:
	void Initialize(ID3D11Device *device);

	ID3D11DepthStencilState *DepthDisabled() { return m_depthDisabled; };
	ID3D11DepthStencilState *DepthEnabled() { return m_depthEnabled; };
	ID3D11DepthStencilState *ReverseDepthEnabled() { return m_revDepthEnabled; };
	ID3D11DepthStencilState *DepthWriteEnabled() { return m_depthWriteEnabled; };
	ID3D11DepthStencilState *ReverseDepthWriteEnabled() { return m_revDepthWriteEnabled; };
	ID3D11DepthStencilState *DepthStencilWriteEnabled() { return m_depthStencilWriteEnabled; };
	ID3D11DepthStencilState *StencilTestEnabled() { return m_depthStencilWriteEnabled; };

	static D3D11_DEPTH_STENCIL_DESC DepthDisabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC ReverseDepthEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC ReverseDepthWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthStencilWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC StencilEnabledDesc();
};


class SamplerStates {
public:
	~SamplerStates();

private:
	ID3D11SamplerState *m_linear;
	ID3D11SamplerState *m_linearClamp;
	ID3D11SamplerState *m_linearBorder;
	ID3D11SamplerState *m_point;
	ID3D11SamplerState *m_pointWrap;
	ID3D11SamplerState *m_anisotropic;
	ID3D11SamplerState *m_shadowMap;
	ID3D11SamplerState *m_shadowMapPCF;

public:
	void Initialize(ID3D11Device *device);

	ID3D11SamplerState *Linear() { return m_linear; };
	ID3D11SamplerState *LinearClamp() { return m_linearClamp; };
	ID3D11SamplerState *LinearBorder() { return m_linearBorder; };
	ID3D11SamplerState *Point() { return m_point; };
	ID3D11SamplerState *PointWrap() { return m_pointWrap; }
	ID3D11SamplerState *Anisotropic() { return m_anisotropic; };
	ID3D11SamplerState *ShadowMap() { return m_shadowMap; };
	ID3D11SamplerState *ShadowMapPCF() { return m_shadowMapPCF; };

	static D3D11_SAMPLER_DESC LinearDesc();
	static D3D11_SAMPLER_DESC LinearClampDesc();
	static D3D11_SAMPLER_DESC LinearBorderDesc();
	static D3D11_SAMPLER_DESC PointDesc();
	static D3D11_SAMPLER_DESC PointWrapDesc();
	static D3D11_SAMPLER_DESC AnisotropicDesc();
	static D3D11_SAMPLER_DESC ShadowMapDesc();
	static D3D11_SAMPLER_DESC ShadowMapPCFDesc();
};

} // End of namespace Graphics
