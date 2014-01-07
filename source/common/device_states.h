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
 * Copyright Adrian Astley 2013
 */

#pragma once

#include <d3d11.h>


namespace Common {

class BlendStates {
protected:
	ID3D11BlendState *blendDisabled;
	ID3D11BlendState *additiveBlend;
	ID3D11BlendState *alphaBlend;
	ID3D11BlendState *pmAlphaBlend;
	ID3D11BlendState *noColor;
	ID3D11BlendState *alphaToCoverage;
	ID3D11BlendState *opacityBlend;

public:
	void Initialize(ID3D11Device *device);

	inline ID3D11BlendState *BlendDisabled() { return blendDisabled; };
	inline ID3D11BlendState *AdditiveBlend() { return additiveBlend; };
	inline ID3D11BlendState *AlphaBlend() { return alphaBlend; };
	inline ID3D11BlendState *PreMultipliedAlphaBlend() { return pmAlphaBlend; };
	inline ID3D11BlendState *ColorWriteDisabled() { return noColor; };
	inline ID3D11BlendState *AlphaToCoverage() { return alphaToCoverage; };
	inline ID3D11BlendState *OpacityBlend() { return opacityBlend; };

	static D3D11_BLEND_DESC BlendDisabledDesc();
	static D3D11_BLEND_DESC AdditiveBlendDesc();
	static D3D11_BLEND_DESC AlphaBlendDesc();
	static D3D11_BLEND_DESC PreMultipliedAlphaBlendDesc();
	static D3D11_BLEND_DESC ColorWriteDisabledDesc();
	static D3D11_BLEND_DESC AlphaToCoverageDesc();
	static D3D11_BLEND_DESC OpacityBlendDesc();
};


class RasterizerStates {
protected:

	ID3D11RasterizerState *noCull;
	ID3D11RasterizerState *cullBackFaces;
	ID3D11RasterizerState *cullBackFacesScissor;
	ID3D11RasterizerState *cullFrontFaces;
	ID3D11RasterizerState *cullFrontFacesScissor;
	ID3D11RasterizerState *noCullNoMS;
	ID3D11RasterizerState *noCullScissor;
	ID3D11RasterizerState *wireframe;

public:

	void Initialize(ID3D11Device *device);

	ID3D11RasterizerState *NoCull() { return noCull; };
	ID3D11RasterizerState *BackFaceCull() { return cullBackFaces; };
	ID3D11RasterizerState *BackFaceCullScissor() { return cullBackFacesScissor; };
	ID3D11RasterizerState *FrontFaceCull() { return cullFrontFaces; };
	ID3D11RasterizerState *FrontFaceCullScissor() { return cullFrontFacesScissor; };
	ID3D11RasterizerState *NoCullNoMS() { return noCullNoMS; };
	ID3D11RasterizerState *NoCullScissor() { return noCullScissor; };
	ID3D11RasterizerState *Wireframe() { return wireframe; };

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
	ID3D11DepthStencilState *depthDisabled;
	ID3D11DepthStencilState *depthEnabled;
	ID3D11DepthStencilState *revDepthEnabled;
	ID3D11DepthStencilState *depthWriteEnabled;
	ID3D11DepthStencilState *revDepthWriteEnabled;
	ID3D11DepthStencilState *depthStencilWriteEnabled;
	ID3D11DepthStencilState *stencilEnabled;

public:

	void Initialize(ID3D11Device *device);

	ID3D11DepthStencilState *DepthDisabled() { return depthDisabled; };
	ID3D11DepthStencilState *DepthEnabled() { return depthEnabled; };
	ID3D11DepthStencilState *ReverseDepthEnabled() { return revDepthEnabled; };
	ID3D11DepthStencilState *DepthWriteEnabled() { return depthWriteEnabled; };
	ID3D11DepthStencilState *ReverseDepthWriteEnabled() { return revDepthWriteEnabled; };
	ID3D11DepthStencilState *DepthStencilWriteEnabled() { return depthStencilWriteEnabled; };
	ID3D11DepthStencilState *StencilTestEnabled() { return depthStencilWriteEnabled; };

	static D3D11_DEPTH_STENCIL_DESC DepthDisabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC ReverseDepthEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC ReverseDepthWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC DepthStencilWriteEnabledDesc();
	static D3D11_DEPTH_STENCIL_DESC StencilEnabledDesc();
};


class SamplerStates {

	ID3D11SamplerState *linear;
	ID3D11SamplerState *linearClamp;
	ID3D11SamplerState *linearBorder;
	ID3D11SamplerState *point;
	ID3D11SamplerState *anisotropic;
	ID3D11SamplerState *shadowMap;
	ID3D11SamplerState *shadowMapPCF;
public:

	void Initialize(ID3D11Device *device);

	ID3D11SamplerState *Linear() { return linear; };
	ID3D11SamplerState *LinearClamp() { return linearClamp; };
	ID3D11SamplerState *LinearBorder() { return linearBorder; };
	ID3D11SamplerState *Point() { return point; };
	ID3D11SamplerState *Anisotropic() { return anisotropic; };
	ID3D11SamplerState *ShadowMap() { return shadowMap; };
	ID3D11SamplerState *ShadowMapPCF() { return shadowMapPCF; };

	static D3D11_SAMPLER_DESC LinearDesc();
	static D3D11_SAMPLER_DESC LinearClampDesc();
	static D3D11_SAMPLER_DESC LinearBorderDesc();
	static D3D11_SAMPLER_DESC PointDesc();
	static D3D11_SAMPLER_DESC AnisotropicDesc();
	static D3D11_SAMPLER_DESC ShadowMapDesc();
	static D3D11_SAMPLER_DESC ShadowMapPCFDesc();
};

} // End of namespace Common
