/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/model.h"
#include "common/texture_manager.h"


namespace Common {

class HalflingModelFile {
private:
	HalflingModelFile();

private:
	enum Flags {
		HAS_STRING_TABLE = 0x0001
	};

public:
	struct Subset {
		Subset()
			: VertexStart(0),
			  VertexCount(0),
			  IndexStart(0),
			  IndexCount(0),
			  AABB_min(0.0f, 0.0f, 0.0f),
			  AABB_max(0.0f, 0.0f, 0.0f),
			  MatDiffuseColor(0.0f, 0.0f, 0.0f, 0.0f),
			  MatSpecColor(0.0f, 0.0f, 0.0f),
			  MatSpecPower(0.0f),
			  DiffuseColorMapIndex(-1),
			  SpecColorMapIndex(-1),
			  SpecPowerMapIndex(-1),
			  AlphaMapIndex(-1),
			  DisplacementMapIndex(-1),
			  NormalMapIndex(-1) {
		}

		uint32 VertexStart;
		uint32 VertexCount;
		uint32 IndexStart;
		uint32 IndexCount;

		DirectX::XMFLOAT3 AABB_min;
		DirectX::XMFLOAT3 AABB_max;

		DirectX::XMFLOAT4 MatDiffuseColor;
		DirectX::XMFLOAT3 MatSpecColor;
		float MatSpecPower;

		int32 DiffuseColorMapIndex;
		int32 SpecColorMapIndex;
		int32 SpecPowerMapIndex;
		int32 AlphaMapIndex;
		int32 DisplacementMapIndex;
		int32 NormalMapIndex;
	};

private:
	static const byte kFileFormatVersion = 2;

public:
	static Common::Model *Load(ID3D11Device *device, Common::TextureManager *textureManager, const wchar *filePath);
	static void Write(const wchar *filepath, 
	                  uint numVertices, uint numIndices, 
	                  D3D11_BUFFER_DESC *vertexBufferDesc,
	                  D3D11_BUFFER_DESC *indexBufferDesc,
	                  D3D11_BUFFER_DESC *instanceBufferDesc,
	                  void *vertexData,
	                  void *indexData,
	                  void *instanceData,
	                  std::vector<Subset> &subsets, std::vector<std::string> &stringTable);
};

} // End of namespace Common
