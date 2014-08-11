/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "engine/texture_manager.h"

#include "common/halfling_sys.h"
#include "graphics/d3d_util.h"
#include "DDSTextureLoader.h"


namespace Engine {

TextureManager::~TextureManager() {
	for (auto bucket = m_textureCache.begin(); bucket != m_textureCache.end(); ++bucket) {
		for (auto bucketIter = bucket->second.begin(); bucketIter != bucket->second.end(); ++bucketIter) {
			ReleaseCOM(bucketIter->second);
		}
	}
}

ID3D11ShaderResourceView * TextureManager::GetSRVFromFile(ID3D11Device *device, const std::wstring filePath, D3D11_USAGE usage, uint bindFlags, uint cpuAccessFlags, uint miscFlags, bool forceSRGB) {
	size_t offset = filePath.find_last_of(L".");
	if (_wcsicmp(filePath.c_str() + offset, L".dds") == 0) {
		return GetSRVFromDDSFile(device, filePath, usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB);
	} else {
		return nullptr;
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

	// Lock the cache before writing
	std::lock_guard<std::mutex> guard(m_cacheLock);

	// Store the new SRV in cache
	TextureParams newParams {usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB};
	m_textureCache[filePath].push_back(std::pair<TextureParams, ID3D11ShaderResourceView *>(newParams, newSRV));

	// The mutex will be unlocked when 'guard' goes out of scope and is destructed

	// Finally return the SRV
	return newSRV;
}

} // End of namespace Engine
