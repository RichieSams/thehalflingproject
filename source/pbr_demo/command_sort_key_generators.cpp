/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "pbr_demo/command_sort_key_generators.h"


namespace PBRDemo {

uint64 GBufferSortKeyGenerator::GenerateKey(Graphics::MaterialShader *materialShader, const Scene::Material *material, ID3D11Buffer *vertexBuffer, ID3D11Buffer *indexBuffer) {
	uint16 materialShaderKey;
	uint16 materialKey;
	uint16 vertexBufferKey;
	uint16 indexBufferKey;

	{
		auto iter = m_materialShaderMap.find(materialShader);
		if (iter != m_materialShaderMap.end()) {
			materialShaderKey = iter->second;
		} else {
			materialShaderKey = m_materialShaderMap.size();
			m_materialShaderMap[materialShader] = materialShaderKey;
		}
	}

	{
		auto iter = m_materialMap.find(material);
		if (iter != m_materialMap.end()) {
			materialKey = iter->second;
		} else {
			materialKey = m_materialMap.size();
			m_materialMap[material] = materialKey;
		}
	}

	{
		auto iter = m_vertexBufferMap.find(vertexBuffer);
		if (iter != m_vertexBufferMap.end()) {
			vertexBufferKey = iter->second;
		} else {
			vertexBufferKey = m_vertexBufferMap.size();
			m_vertexBufferMap[vertexBuffer] = vertexBufferKey;
		}
	}

	{
		auto iter = m_indexBufferMap.find(indexBuffer);
		if (iter != m_indexBufferMap.end()) {
			indexBufferKey = iter->second;
		} else {
			indexBufferKey = m_indexBufferMap.size();
			m_indexBufferMap[indexBuffer] = indexBufferKey;
		}
	}

	return ((uint64)materialShaderKey << 52) |
	       ((uint64)materialKey << 40) |
	       ((uint64)vertexBufferKey << 28) |
	       ((uint64)indexBufferKey << 16);
}

} // End of namespace PBRDemo
