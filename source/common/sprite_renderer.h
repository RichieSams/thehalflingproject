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

#pragma once

#include "common/halfling_sys.h"

#include <d3d11.h>
#include <DirectXMath.h>


namespace Common {

class SpriteFont;

class SpriteRenderer {
public:
	enum FilterMode {
		DontSet = 0,
		Linear = 1,
		Point = 2
	};

	static const uint64 MaxBatchSize = 1000;

	struct SpriteDrawData {
		DirectX::XMFLOAT4X4 Transform;
		DirectX::XMFLOAT4 Color;
		DirectX::XMFLOAT4 DrawRect;
	};

protected:
	struct SpriteVertex {
		DirectX::XMFLOAT2 Position;
		DirectX::XMFLOAT2 TexCoord;
	};

	struct VSPerBatchCB {
		DirectX::XMFLOAT2 TextureSize;
		DirectX::XMFLOAT2 ViewportSize;
	};

public:
	SpriteRenderer();
	~SpriteRenderer();

private:
	bool m_initialized;
	SpriteDrawData textDrawData[MaxBatchSize];

protected:
	ID3D11VertexShader *m_vertexShader;
	ID3D11VertexShader *m_vertexShaderInstanced;
	ID3D11PixelShader *m_pixelShader;
	ID3D11PixelShader *m_sampledPixelShader;
	ID3D11Buffer *m_vertexBuffer;
	ID3D11Buffer *m_indexBuffer;
	ID3D11Buffer *m_vsPerBatchConstBuffer;
	ID3D11Buffer *m_vsPerInstanceConstBuffer;
	ID3D11Buffer *m_instanceDataBuffer;
	ID3D11InputLayout *m_inputLayout;
	ID3D11InputLayout *m_inputLayoutInstanced;
	ID3D11DeviceContext *m_context;

	ID3D11RasterizerState *m_rastState;
	ID3D11DepthStencilState *m_depthStencilState;
	ID3D11BlendState *m_alphaBlendState;
	ID3D11SamplerState *m_linearSamplerState;
	ID3D11SamplerState *m_pointSamplerState;

public:
	void Initialize(ID3D11Device *device);
	void Begin(ID3D11DeviceContext *deviceContext, FilterMode filterMode = DontSet);
	void Render(ID3D11ShaderResourceView *texture, const DirectX::XMFLOAT4X4 &transform, const DirectX::XMFLOAT4 &color = DirectX::XMFLOAT4(1, 1, 1, 1), const DirectX::XMFLOAT4 *drawRect = nullptr);
	void RenderBatch(ID3D11ShaderResourceView *texture, const SpriteDrawData *drawData, uint64 numSprites);
	const DirectX::XMFLOAT2 RenderText(const SpriteFont &font, const wchar *text, const DirectX::XMFLOAT4X4 &transform, const uint maxWidth = 0U, const DirectX::XMFLOAT4 &color = DirectX::XMFLOAT4(1, 1, 1, 1));
	void End();

protected:
	D3D11_TEXTURE2D_DESC SetPerBatchData(ID3D11ShaderResourceView *texture);
};

} // End of namespace Common