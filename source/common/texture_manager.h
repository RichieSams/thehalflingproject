/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/typedefs.h"

#include <unordered_map>
#include <string>
#include <d3d11.h>


namespace Common {

class TextureManager {
private:
	struct TextureParams {
		D3D11_USAGE Usage;
		uint BindFlags;
		uint CpuAccessFlags;
		uint MiscFlags;
		bool ForceSRGB;
	};

public:
	~TextureManager();

private:
	std::unordered_map<std::wstring, std::vector<std::pair<TextureParams, ID3D11ShaderResourceView *> > > m_textureCache;

public:
	ID3D11ShaderResourceView *GetSRVFromFile(ID3D11Device *device, const std::wstring filePath, D3D11_USAGE usage, uint bindFlags = D3D11_BIND_SHADER_RESOURCE, uint cpuAccessFlags = 0, uint miscFlags = 0, bool forceSRGB = false);

private:
	ID3D11ShaderResourceView *GetSRVFromDDSFile(ID3D11Device *device, const std::wstring filePath, D3D11_USAGE usage, uint bindFlags, uint cpuAccessFlags, uint miscFlags, bool forceSRGB);
};

} // End of namespace Common
