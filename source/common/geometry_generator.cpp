/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/geometry_generator.h"


namespace Common {

void GeometryGenerator::CreateGrid(float width, float depth, uint m, uint n, MeshData* meshData) {
	uint vertexCount = m * n;
	uint faceCount = (m - 1) * (n - 1) * 2;

	// Create the vertices
	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData->Vertices.resize(vertexCount);
	for (uint i = 0; i < m; ++i) {
		float z = halfDepth - (i * dz);
		for (uint j = 0; j < n; ++j) {
			float x = -halfWidth + (j * dx);

			meshData->Vertices[(i * n) + j].Position = DirectX::XMFLOAT3(x, 0.0f, z);
			meshData->Vertices[(i * n) + j].Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

			// Stretch texture over grid.
			meshData->Vertices[(i * n) + j].TexCoord.x = j * du;
			meshData->Vertices[(i * n) + j].TexCoord.y = i * dv;
		}
	}

	// Create the indices.
	meshData->Indices.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint k = 0;
	for (uint i = 0; i < m - 1; ++i) {
		for (uint j = 0; j < n - 1; ++j) {
			meshData->Indices[k] = (i * n) + j;
			meshData->Indices[k + 1] = (i * n) + j + 1;
			meshData->Indices[k + 2] = ((i + 1) * n) + j;

			meshData->Indices[k + 3] = ((i + 1) * n) + j;
			meshData->Indices[k + 4] = (i * n) + j + 1;
			meshData->Indices[k + 5] = ((i + 1) * n) + j + 1;

			k += 6; // next quad
		}
	}
}

} // End of namespace Common
