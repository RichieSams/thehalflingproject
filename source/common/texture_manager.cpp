/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/texture_manager.h"

#include "common/halfling_sys.h"
#include "common/d3d_util.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"


namespace Common {

TextureManager::~TextureManager() {
	for (auto bucket = m_textureCache.begin(); bucket != m_textureCache.end(); ++bucket) {
		for (auto bucketIter = bucket->second.begin(); bucketIter != bucket->second.end(); ++bucketIter) {
			ReleaseCOM(bucketIter->second);
		}
	}
}

ID3D11ShaderResourceView * TextureManager::GetSRVFromFile(ID3D11Device *device, ID3D11DeviceContext *context, const std::wstring filePath, D3D11_USAGE usage, uint bindFlags, uint cpuAccessFlags, uint miscFlags, bool forceSRGB) {
	if (filePath.compare(filePath.length() - 4, std::string::npos, L".dds") == 0 || filePath.compare(filePath.length() - 4, std::string::npos, L".DDS") == 0) {
		return GetSRVFromDDSFile(device, filePath, usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB);
	} else {
		return GetSRVFromWICFile(device, context, filePath, usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB);
	}
}

ID3D11ShaderResourceView *TextureManager::GetSRVFromDDSFile(ID3D11Device *device, const std::wstring filePath, D3D11_USAGE usage, uint bindFlags, uint cpuAccessFlags, uint miscFlags, bool forceSRGB) {
	// First check the cache
	auto bucket = m_textureCache.find(filePath);
	if (bucket != m_textureCache.end()) {
		for (auto iter = bucket->second.begin(); iter != bucket->second.end(); ++iter) {
			if (usage == iter->first.Usage &&
				bindFlags == iter->first.BindFlags &&
				cpuAccessFlags == iter->first.CpuAccessFlags &&
				miscFlags == iter->first.MiscFlags &&
				forceSRGB == iter->first.ForceSRGB) {

				return iter->second;
			}
		}
	}

	// Else create it from scratch
	ID3D11ShaderResourceView *newSRV;
	HR(DirectX::CreateDDSTextureFromFileEx(device, filePath.c_str(), 0, usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB, nullptr, &newSRV));

	// Store the new SRV in cache
	TextureParams newParams {usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB};
	m_textureCache[filePath].push_back(std::pair<TextureParams, ID3D11ShaderResourceView *>(newParams, newSRV));

	// Finally return the SRV
	return newSRV;
}

ID3D11ShaderResourceView * TextureManager::GetSRVFromWICFile(ID3D11Device *device, ID3D11DeviceContext *context, const std::wstring filePath, D3D11_USAGE usage, uint bindFlags /*= D3D11_BIND_SHADER_RESOURCE*/, uint cpuAccessFlags /*= 0*/, uint miscFlags /*= 0*/, bool forceSRGB /*= false*/) {
	// First check the cache
	auto bucket = m_textureCache.find(filePath);
	if (bucket != m_textureCache.end()) {
		for (auto iter = bucket->second.begin(); iter != bucket->second.end(); ++iter) {
			if (usage == iter->first.Usage &&
				bindFlags == iter->first.BindFlags &&
				cpuAccessFlags == iter->first.CpuAccessFlags &&
				miscFlags == iter->first.MiscFlags &&
				forceSRGB == iter->first.ForceSRGB) {

				return iter->second;
			}
		}
	}

	ID3D11ShaderResourceView *newSRV = nullptr;
	HR(DirectX::CreateWICTextureFromFile(device, context, filePath.c_str(), nullptr, &newSRV));

	// Store the new SRV in cache
	TextureParams newParams{usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB};
	m_textureCache[filePath].push_back(std::pair<TextureParams, ID3D11ShaderResourceView *>(newParams, newSRV));

	// Finally return the SRV
	return newSRV;
}

} // End of namespace Common
