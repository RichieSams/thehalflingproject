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

#include "common/sprite_font.h"

#include "common/halfling_sys.h"
#include "common/d3d_util.h"

#include <d3d11.h>
#include <algorithm>

namespace Common {

SpriteFont::SpriteFont()
	: m_size(0),
	  m_texHeight(0),
	  m_spaceWidth(0),
	  m_charHeight(0) {
}

SpriteFont::~SpriteFont() {
	ReleaseCOM(m_SRV);
	ReleaseCOM(m_texture);
}

void SpriteFont::Initialize(const wchar *fontName, float fontSize, UINT fontStyle, bool antiAliased, ID3D11Device *device) {
	m_size = fontSize;

	Gdiplus::TextRenderingHint hint = antiAliased ? Gdiplus::TextRenderingHintAntiAliasGridFit : Gdiplus::TextRenderingHintSingleBitPerPixelGridFit;

	// Init GDI+
	ULONG_PTR token = NULL;
	Gdiplus::GdiplusStartupInput startupInput(NULL, true, true);
	Gdiplus::GdiplusStartupOutput startupOutput;
	HR(GdiplusStartup(&token, &startupInput, &startupOutput));

	// Create the font
	Gdiplus::Font font(fontName, fontSize, fontStyle, Gdiplus::UnitPixel, NULL);

	// Check for error during construction
	HR(font.GetLastStatus());

	// Create a temporary Bitmap and Graphics for figuring out the rough size required
	// for drawing all of the characters
	int size = static_cast<int>(fontSize * NumChars * 2) + 1;
	Gdiplus::Bitmap sizeBitmap(size, size, PixelFormat32bppARGB);
	HR(sizeBitmap.GetLastStatus());

	Gdiplus::Graphics sizeGraphics(&sizeBitmap);
	HR(sizeGraphics.GetLastStatus());
	HR(sizeGraphics.SetTextRenderingHint(hint));

	m_charHeight = font.GetHeight(&sizeGraphics) * 1.5f;

	wchar allChars[NumChars + 1];
	for (wchar i = 0; i < NumChars; ++i) {
		allChars[i] = i + StartChar;
	}
	allChars[NumChars] = 0;

	Gdiplus::RectF sizeRect;
	HR(sizeGraphics.MeasureString(allChars, NumChars, &font, Gdiplus::PointF(0, 0), &sizeRect));
	int numRows = static_cast<int>(sizeRect.Width / TexWidth) + 1;
	int texHeight = static_cast<int>(numRows * m_charHeight) + 1;

	// Create a temporary Bitmap and Graphics for drawing the characters one by one
	int tempSize = static_cast<int>(fontSize * 2);
	Gdiplus::Bitmap drawBitmap(tempSize, tempSize, PixelFormat32bppARGB);
	HR(drawBitmap.GetLastStatus());

	Gdiplus::Graphics drawGraphics(&drawBitmap);
	HR(drawGraphics.GetLastStatus());
	HR(drawGraphics.SetTextRenderingHint(hint));

	// Create a temporary Bitmap + Graphics for creating a full character set
	Gdiplus::Bitmap textBitmap(TexWidth, texHeight, PixelFormat32bppARGB);
	HR(textBitmap.GetLastStatus());

	Gdiplus::Graphics textGraphics(&textBitmap);
	HR(textGraphics.GetLastStatus());
	HR(textGraphics.Clear(Gdiplus::Color(0, 255, 255, 255)));
	HR(textGraphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy));

	// Solid brush for text rendering
	Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 255, 255));
	HR(brush.GetLastStatus());

	// Draw all of the characters, and copy them to the full character set
	wchar charString [2];
	charString[1] = 0;
	int currentX = 0;
	int currentY = 0;
	for (uint64 i = 0; i < NumChars; ++i) {
		charString[0] = static_cast<wchar>(i + StartChar);

		// Draw the character
		HR(drawGraphics.Clear(Gdiplus::Color(0, 255, 255, 255)));
		HR(drawGraphics.DrawString(charString, 1, &font, Gdiplus::PointF(0, 0), &brush));

		// Figure out the amount of blank space before the character
		int minX = 0;
		for (int x = 0; x < tempSize; ++x) {
			for (int y = 0; y < tempSize; ++y) {
				Gdiplus::Color color;
				HR(drawBitmap.GetPixel(x, y, &color));
				if (color.GetAlpha() > 0) {
					minX = x;
					x = tempSize;
					break;
				}
			}
		}

		// Figure out the amount of blank space after the character
		int maxX = tempSize - 1;
		for (int x = tempSize - 1; x >= 0; --x) {
			for (int y = 0; y < tempSize; ++y) {
				Gdiplus::Color color;
				HR(drawBitmap.GetPixel(x, y, &color));
				if (color.GetAlpha() > 0) {
					maxX = x;
					x = -1;
					break;
				}
			}
		}

		int charWidth = maxX - minX + 1;

		// Figure out if we need to move to the next row
		if (currentX + charWidth >= TexWidth) {
			currentX = 0;
			currentY += static_cast<int>(m_charHeight) + 1;
		}

		// Fill out the structure describing the character position
		m_charDescs[i].X = static_cast<float>(currentX);
		m_charDescs[i].Y = static_cast<float>(currentY);
		m_charDescs[i].Width = static_cast<float>(charWidth);
		m_charDescs[i].Height = static_cast<float>(m_charHeight);

		// Copy the character over
		int height = static_cast<int>(m_charHeight + 1);
		HR(textGraphics.DrawImage(&drawBitmap, currentX, currentY, minX, 0, charWidth, height, Gdiplus::UnitPixel));

		currentX += charWidth + 1;
	}

	// Figure out the width of a space character
	charString[0] = ' ';
	charString[1] = 0;
	HR(drawGraphics.MeasureString(charString, 1, &font, Gdiplus::PointF(0, 0), &sizeRect));
	m_spaceWidth = sizeRect.Width;

	// Lock the bitmap for direct memory access
	Gdiplus::BitmapData bmData;
	HR(textBitmap.LockBits(&Gdiplus::Rect(0, 0, TexWidth, texHeight), Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmData));

	// Create a D3D texture, initalized with the bitmap data
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = TexWidth;
	texDesc.Height = texHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = bmData.Scan0;
	data.SysMemPitch = TexWidth * 4;
	data.SysMemSlicePitch = 0;

	HR(device->CreateTexture2D(&texDesc, &data, &m_texture));

	HR(textBitmap.UnlockBits(&bmData));

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	srDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MipLevels = 1;
	srDesc.Texture2D.MostDetailedMip = 0;

	HR(device->CreateShaderResourceView(m_texture, &srDesc, &m_SRV));

	// Shutdown GDI+
	//Gdiplus::GdiplusShutdown(token);
	// TODO: Figure out why this throws exceptions
}

ID3D11ShaderResourceView *SpriteFont::SRView() const {
	return m_SRV;
}

const SpriteFont::CharDesc *SpriteFont::CharDescriptors() const {
	return m_charDescs;
}

const SpriteFont::CharDesc &SpriteFont::GetCharDescriptor(wchar character) const {
	assert(character >= StartChar && character <= EndChar);
	return m_charDescs[character - StartChar];
}

float SpriteFont::Size() const {
	return m_size;
}

uint SpriteFont::TextureWidth() const {
	return TexWidth;
}

uint SpriteFont::TextureHeight() const {
	return m_texHeight;
}

float SpriteFont::SpaceWidth() const {
	return m_spaceWidth;
}

float SpriteFont::CharHeight() const {
	return m_charHeight;
}

ID3D11Texture2D *SpriteFont::Texture() const {
	return m_texture;
}

DirectX::XMFLOAT2 SpriteFont::MeasureText(const wchar *text, uint maxWidth) const {
	DirectX::XMFLOAT2 size(0.0f, 0.0f);
	DirectX::XMFLOAT2 curPos(0.0f, 0.0f);;

	size_t length = wcslen(text);

	for (uint64 i = 0; i < length; ++i) {
		wchar character = text[i];
		if (character == ' ') {
			// Check for wrapping
			if (maxWidth != 0U && curPos.x + m_spaceWidth > maxWidth) {
				size.x = std::max(size.x, curPos.x);
				curPos.y += m_charHeight;
				curPos.x = 0;
			}

			curPos.x += m_spaceWidth;
		} else if (character == '\t') {
			// Check for wrapping
			if (maxWidth != 0U && curPos.x + 4.0f * m_spaceWidth > maxWidth) {
				curPos.y += m_charHeight;
				curPos.x = 0;
			}

			curPos.x += 4.0f * m_spaceWidth;
		} else if (character == '\n' || character == '\r') {
			size.x = std::max(size.x, curPos.x);
			curPos.y += m_charHeight;
			curPos.x = 0;
		} else {
			SpriteFont::CharDesc desc = GetCharDescriptor(character);

			// Check for wrapping
			if (maxWidth != 0U && curPos.x + desc.Width + 1 > maxWidth) {
				size.x = std::max(size.x, curPos.x);
				curPos.y += m_charHeight;
				curPos.x = 0;
			}

			curPos.x += desc.Width + 1;
		}

		size.x = std::max(curPos.x, size.x);
		size.y = curPos.y + m_charHeight;
	}

	return size;
}

}