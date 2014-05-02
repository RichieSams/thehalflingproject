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

#pragma once

#include "common/typedefs.h"

#include <d3d11.h>
#include <DirectXMath.h>


namespace Common {

class SpriteFont {
public:
	enum FontStyle {
		Regular = 0,
		Bold = 1 << 0,
		Italic = 1 << 1,
		BoldItalic = Bold | Italic,
		Underline = 1 << 2,
		Strikeout = 1 << 3
	};

	struct CharDesc {
		float X;
		float Y;
		float Width;
		float Height;
	};

	static const wchar StartChar = '!';
	static const wchar EndChar = 127;
	static const uint64 NumChars = EndChar - StartChar;
	static const uint TexWidth = 1024;

	// Lifetime
	SpriteFont();
	~SpriteFont();

protected:
	ID3D11Texture2D *m_texture;
	ID3D11ShaderResourceView *m_SRV;
	CharDesc m_charDescs[NumChars];
	float m_size;
	UINT m_texHeight;
	float m_spaceWidth;
	float m_charHeight;

public:
	void Initialize(const wchar *fontName, float fontSize, uint fontStyle, bool antiAliased, ID3D11Device *device);
	DirectX::XMFLOAT2 MeasureText(const wchar *text, uint maxWidth = 0U) const;

	// Accessors
	ID3D11ShaderResourceView *SRView() const;
	const CharDesc *CharDescriptors() const;
	const CharDesc &GetCharDescriptor(wchar character) const;
	float Size() const;
	ID3D11Texture2D *Texture() const;
	uint TextureWidth() const;
	uint TextureHeight() const;
	float SpaceWidth() const;
	float CharHeight() const;
};

} // End of namespace Common