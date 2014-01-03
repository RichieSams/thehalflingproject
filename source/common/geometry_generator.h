/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_GEOMETRY_GENERATOR_H
#define COMMON_GEOMETRY_GENERATOR_H

#include "common/typedefs.h"

#include "DirectXMath.h"
#include <vector>


namespace Common {

class GeometryGenerator {
public:
	struct Vertex {
	public:
		Vertex() {}
		Vertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT2& uv)
			: Position(pos), Normal(normal), TexCoord(uv) {}
		Vertex(float px, float py, float pz,
			   float nx, float ny, float nz,
			   float u, float v)
			   : Position(px, py, pz), Normal(nx, ny, nz), TexCoord(u, v) {}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexCoord;
	};

	struct MeshData {
		std::vector<Vertex> Vertices;
		std::vector<uint> Indices;
	};

	static void CreateBox(float width, float height, float depth, MeshData* meshData);
	static void CreateSphere(float radius, uint sliceCount, uint stackCount, MeshData* meshData);
	static void CreateGeosphere(float radius, uint numSubdivisions, MeshData* meshData) {}
	static void CreateCylinder(float bottomRadius, float topRadius, float height, uint sliceCount, uint stackCount, MeshData* meshData) {}
	static void CreateGrid(float width, float depth, uint m, uint n, MeshData* meshData);
	static void CreateFullscreenQuad(MeshData& meshData) {}
};

} // End of namespace Common

#endif
