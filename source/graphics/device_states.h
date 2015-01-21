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

enum class BlendState {
	BLEND_DISABLED,
	ADDITIVE_BLEND,
	ALPHA_BLEND,
	PM_ALPHA_BLEND,
	NO_COLOR,
	ALPHA_TO_COVERAGE,
	OPACITY_BLEND
};

class BlendStateManager {
public:
	~BlendStateManager();

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

	inline ID3D11BlendState *GetD3DState(BlendState state) {
		switch (state) {
		case BlendState::BLEND_DISABLED:
			return m_blendDisabled;
		case BlendState::ADDITIVE_BLEND:
			return m_additiveBlend;
		case BlendState::ALPHA_BLEND:
			return m_alphaBlend;
		case BlendState::PM_ALPHA_BLEND:
			return m_pmAlphaBlend;
		case BlendState::NO_COLOR:
			return m_noColor;
		case BlendState::ALPHA_TO_COVERAGE:
			return m_alphaToCoverage;
		case BlendState::OPACITY_BLEND:
			return m_opacityBlend;
		default:
			return nullptr;
		}
	}

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


enum class  RasterizerState {
	NO_CULL,
	CULL_BACKFACES,
	CULL_BACKFACES_SCISSOR,
	CULL_FRONTFACES,
	CULL_FRONTFACES_SCISSOR,
	NO_CULL_NO_MS,
	NO_CULL_SCISSOR,
	WIREFRAME
};

class RasterizerStateManager {
public:
	~RasterizerStateManager();

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

	inline ID3D11RasterizerState *GetD3DState(RasterizerState state) {
		switch (state) {
		case Graphics::RasterizerState::NO_CULL:
			return m_noCull;
		case Graphics::RasterizerState::CULL_BACKFACES:
			return m_cullBackFaces;
		case Graphics::RasterizerState::CULL_BACKFACES_SCISSOR:
			return m_cullBackFacesScissor;
		case Graphics::RasterizerState::CULL_FRONTFACES:
			return m_cullFrontFaces;
		case Graphics::RasterizerState::CULL_FRONTFACES_SCISSOR:
			return m_cullBackFacesScissor;
		case Graphics::RasterizerState::NO_CULL_NO_MS:
			return m_noCullNoMS;
		case Graphics::RasterizerState::NO_CULL_SCISSOR:
			return m_noCullScissor;
		case Graphics::RasterizerState::WIREFRAME:
			return m_wireframe;
		default:
			return nullptr;
		}
	}

	inline ID3D11RasterizerState *NoCull() { return m_noCull; };
	inline ID3D11RasterizerState *BackFaceCull() { return m_cullBackFaces; };
	inline ID3D11RasterizerState *BackFaceCullScissor() { return m_cullBackFacesScissor; };
	inline ID3D11RasterizerState *FrontFaceCull() { return m_cullFrontFaces; };
	inline ID3D11RasterizerState *FrontFaceCullScissor() { return m_cullFrontFacesScissor; };
	inline ID3D11RasterizerState *NoCullNoMS() { return m_noCullNoMS; };
	inline ID3D11RasterizerState *NoCullScissor() { return m_noCullScissor; };
	inline ID3D11RasterizerState *Wireframe() { return m_wireframe; };

	static D3D11_RASTERIZER_DESC NoCullDesc();
	static D3D11_RASTERIZER_DESC FrontFaceCullDesc();
	static D3D11_RASTERIZER_DESC FrontFaceCullScissorDesc();
	static D3D11_RASTERIZER_DESC BackFaceCullDesc();
	static D3D11_RASTERIZER_DESC BackFaceCullScissorDesc();
	static D3D11_RASTERIZER_DESC NoCullNoMSDesc();
	static D3D11_RASTERIZER_DESC NoCullScissorDesc();
	static D3D11_RASTERIZER_DESC WireframeDesc();
};


enum class DepthStencilState {
	DEPTH_DISABLED,
	DEPTH_ENABLED,
	REVERSE_DEPTH_ENABLED,
	DEPTH_WRITE_ENABLED,
	REVERSE_DEPTH_WRITE_ENABLED,
	DEPTH_STENCIL_WRITE_ENABLED,
	STENCIL_ENABLED
};

class DepthStencilStateManager {
public:
	~DepthStencilStateManager();

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

	inline ID3D11DepthStencilState *GetD3DState(DepthStencilState state) {
		switch (state) {
		case Graphics::DepthStencilState::DEPTH_DISABLED:
			return m_depthDisabled;
		case Graphics::DepthStencilState::DEPTH_ENABLED:
			return m_depthEnabled;
		case Graphics::DepthStencilState::REVERSE_DEPTH_ENABLED:
			return m_revDepthEnabled;
		case Graphics::DepthStencilState::DEPTH_WRITE_ENABLED:
			return m_depthWriteEnabled;
		case Graphics::DepthStencilState::REVERSE_DEPTH_WRITE_ENABLED:
			return m_revDepthWriteEnabled;
		case Graphics::DepthStencilState::DEPTH_STENCIL_WRITE_ENABLED:
			return m_depthStencilWriteEnabled;
		case Graphics::DepthStencilState::STENCIL_ENABLED:
			return m_stencilEnabled;
		default:
			return nullptr;
		}
	}

	inline ID3D11DepthStencilState *DepthDisabled() { return m_depthDisabled; };
	inline ID3D11DepthStencilState *DepthEnabled() { return m_depthEnabled; };
	inline ID3D11DepthStencilState *ReverseDepthEnabled() { return m_revDepthEnabled; };
	inline ID3D11DepthStencilState *DepthWriteEnabled() { return m_depthWriteEnabled; };
	inline ID3D11DepthStencilState *ReverseDepthWriteEnabled() { return m_revDepthWriteEnabled; };
	inline ID3D11DepthStencilState *DepthStencilWriteEnabled() { return m_depthStencilWriteEnabled; };
	inline ID3D11DepthStencilState *StencilTestEnabled() { return m_depthStencilWriteEnabled; };

	static D3D11_DEPTH_STENCIL_DESC DepthDisabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC ReverseDepthEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC ReverseDepthWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthStencilWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC StencilEnabledDesc();
};


enum class SamplerState {
	LINEAR,
	LINEAR_CLAMP,
	LINEAR_BORDER,
	POINT,
	POINT_WRAP,
	ANISOTROPIC,
	SHADOW_MAP,
	SHADOW_MAP_PCF,
};

class SamplerStateManager {
public:
	~SamplerStateManager();

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

	inline ID3D11SamplerState *Linear() { return m_linear; };
	inline ID3D11SamplerState *LinearClamp() { return m_linearClamp; };
	inline ID3D11SamplerState *LinearBorder() { return m_linearBorder; };
	inline ID3D11SamplerState *Point() { return m_point; };
	inline ID3D11SamplerState *PointWrap() { return m_pointWrap; }
	inline ID3D11SamplerState *Anisotropic() { return m_anisotropic; };
	inline ID3D11SamplerState *ShadowMap() { return m_shadowMap; };
	inline ID3D11SamplerState *ShadowMapPCF() { return m_shadowMapPCF; };

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
