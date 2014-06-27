// Copyright 2010 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

/**
 * Modified for use in The Halfling Project - A Graphics Engine and Projects
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef COMMON_TEXTURE2D_H
#define COMMON_TEXTURE2D_H

#include "common/typedefs.h"

#include <d3d11.h>
#include <vector>


namespace Common {

class Texture2D {
public:
	// Construct a Texture2D
	Texture2D(ID3D11Device *d3dDevice, int width, int height, DXGI_FORMAT format, uint bindFlags, int mipLevels);
	// Construct a Texture2DMS
	Texture2D(ID3D11Device *d3dDevice, int width, int height, DXGI_FORMAT format, uint bindFlags, int mipLevels, const DXGI_SAMPLE_DESC &sampleDesc);
	// Construct a Texture2DArray
	Texture2D(ID3D11Device *d3dDevice, int width, int height, DXGI_FORMAT format, uint bindFlags, int mipLevels, int arraySize);
	// Construct a Texture2DMSArray
	Texture2D(ID3D11Device *d3dDevice, int width, int height, DXGI_FORMAT format, uint bindFlags, int mipLevels, int arraySize, const DXGI_SAMPLE_DESC &sampleDesc);

	~Texture2D();

private:
	ID3D11Texture2D *mTexture;
	ID3D11ShaderResourceView *mShaderResource;

	// One per array element
	std::vector<ID3D11RenderTargetView *> mRenderTargetElements;
	std::vector<ID3D11UnorderedAccessView *> mUnorderedAccessElements;
	std::vector<ID3D11ShaderResourceView *> mShaderResourceElements;

public:
	ID3D11Texture2D *GetTexture() { return mTexture; }
	ID3D11RenderTargetView *GetRenderTarget(std::size_t arrayIndex = 0) { return mRenderTargetElements[arrayIndex]; }
	// Treat these like render targets for now... i.e. only access a slice
	ID3D11UnorderedAccessView *GetUnorderedAccess(std::size_t arrayIndex = 0) { return mUnorderedAccessElements[arrayIndex]; }
	// Get a full view of the resource
	ID3D11ShaderResourceView *GetShaderResource() { return mShaderResource; }
	// Get a view to the top mip of a single array element
	ID3D11ShaderResourceView *GetShaderResource(std::size_t arrayIndex) { return mShaderResourceElements[arrayIndex]; }

private:
	void InternalConstructor(ID3D11Device *d3dDevice,
	                         int width, int height, DXGI_FORMAT format,
	                         UINT bindFlags, int mipLevels, int arraySize,
	                         int sampleCount, int sampleQuality,
	                         D3D11_RTV_DIMENSION rtvDimension,
	                         D3D11_UAV_DIMENSION uavDimension,
	                         D3D11_SRV_DIMENSION srvDimension);

	// Not implemented
	Texture2D(const Texture2D &);
	Texture2D &operator=(const Texture2D &);
};


// Currently always float 32 as this one works best with sampling
// Optionally supports adding 8-bit stencil, but SRVs will only reference the 32-bit float part
class Depth2D {
public:
	// Construct a Texture2D depth buffer
	Depth2D(ID3D11Device *d3dDevice, int width, int height, uint bindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, bool stencil = false);
	// Construct a Texture2DMS depth buffer
	Depth2D(ID3D11Device *d3dDevice, int width, int height, uint bindFlags, const DXGI_SAMPLE_DESC &sampleDesc, bool stencil = false);
	// Construct a Texture2DArray depth buffer
	Depth2D(ID3D11Device *d3dDevice, int width, int height, uint bindFlags, int arraySize, bool stencil = false);
	// Construct a Texture2DMSArray depth buffer
	Depth2D(ID3D11Device *d3dDevice, int width, int height, uint bindFlags, int arraySize, const DXGI_SAMPLE_DESC &sampleDesc, bool stencil = false);

	~Depth2D();

private:
	ID3D11Texture2D *mTexture;
	ID3D11ShaderResourceView *mShaderResource;
	// One per array element
	std::vector<ID3D11DepthStencilView *> mDepthStencilElements;

public:
	ID3D11Texture2D *GetTexture() { return mTexture; }
	ID3D11DepthStencilView *GetDepthStencil(std::size_t arrayIndex = 0) { return mDepthStencilElements[arrayIndex]; }
	ID3D11ShaderResourceView *GetShaderResource() { return mShaderResource; }

private:
	void InternalConstruct(ID3D11Device *d3dDevice,
	                       int width, int height,
	                       UINT bindFlags, int arraySize,
	                       int sampleCount, int sampleQuality,
	                       D3D11_DSV_DIMENSION dsvDimension,
	                       D3D11_SRV_DIMENSION srvDimension,
	                       bool stencil);

	// Not implemented
	Depth2D(const Depth2D &);
	Depth2D &operator=(const Depth2D &);
};

} // End of namespace Common

#endif
