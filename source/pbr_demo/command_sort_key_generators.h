/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "graphics/shader.h"

#include "scene/materials.h"

#include <unordered_map>


namespace PBRDemo {

// Graphics State changes in order from most costly to least costly
//
// Render target
// Shader
// Rasterizer / blend state
// Texture binding
// Vertex Format
// Vertex / index buffer bind
// cbuffer change

class GBufferSortKeyGenerator {
private:
	std::unordered_map<Graphics::MaterialShader *, uint16> m_materialShaderMap;
	std::unordered_map<const Scene::Material *, uint16> m_materialMap;
	std::unordered_map<ID3D11Buffer *, uint16> m_vertexBufferMap;
	std::unordered_map<ID3D11Buffer *, uint16> m_indexBufferMap;

public:
	// From MSB to LSB
	// 12 bits - 4098 values - Material Shader
	// 12 bits - 4098 values - Material (representing the textures)
	// 12 bits - 4098 values - Vertex buffer
	// 12 bits - 4098 values - Index buffer
	// 
	// Total - 48 bits
	// Material
	uint64 GenerateKey(Graphics::MaterialShader *materialShader, const Scene::Material *material, ID3D11Buffer *vertexBuffer, ID3D11Buffer *indexBuffer);
};

} // End of namespace PBRDemo
