/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_TEXTURE_MANAGER_H
#define COMMON_TEXTURE_MANAGER_H

#include "common/typedefs.h"

#include <unordered_map>
#include <string>
#include <d3d11.h>

#include <DDSTextureLoader.h>

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
	ID3D11ShaderResourceView *GetSRVFromDDSFile(ID3D11Device *device, const std::string filePath, D3D11_USAGE usage, uint bindFlags = D3D11_BIND_SHADER_RESOURCE, uint cpuAccessFlags = 0, uint miscFlags = 0, bool forceSRGB = false);

private:
	std::unordered_map<std::string, std::vector<std::pair<TextureParams, ID3D11ShaderResourceView *> > > m_textureCache;
};

} // End of namespace Common

#endif
