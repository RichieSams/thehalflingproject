/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/texture_manager.h"

#include "common/halfling_sys.h"
#include "common/d3d_util.h"


namespace Common {

ID3D11ShaderResourceView * TextureManager::GetSRVFromDDSFile(ID3D11Device *device, const std::string filePath, D3D11_USAGE usage, uint bindFlags, uint cpuAccessFlags, uint miscFlags, bool forceSRGB) {
	// First check the cache
	auto bucket = m_textureCache.find(filePath);
	if (bucket != m_textureCache.end()) {
		for (auto element : bucket->second) {
			if (usage == element.first.Usage &&
				bindFlags == element.first.BindFlags &&
				cpuAccessFlags == element.first.CpuAccessFlags &&
				miscFlags == element.first.MiscFlags &&
				forceSRGB == element.first.ForceSRGB) {

				return element.second;
			}
		}
	}
	    
	// Else create it from scratch
	std::wstring wideFilePath( filePath.begin(), filePath.end() );

	ID3D11ShaderResourceView *newSRV;
	HR(DirectX::CreateDDSTextureFromFileEx(device, wideFilePath.c_str(), 0, usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB, nullptr, &newSRV));

	// Store the new SRV in cache
	TextureParams newParams{usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB};
	m_textureCache[filePath].push_back(std::pair<TextureParams, ID3D11ShaderResourceView *>(newParams, newSRV));

	// Finally return the SRV
	return newSRV;
}

} // End of namespace Common
