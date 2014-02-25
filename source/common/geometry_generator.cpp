/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/geometry_generator.h"

#include "common/file_io_util.h"
#include "common/string_util.h"
#include "common/profiler.h"
#include "common/memory_stream.h"
#include "common/hash.h"

#include <unordered_map>
#include <tuple>

namespace Common {

void GeometryGenerator::CreateGrid(float width, float depth, uint m, uint n, MeshData *meshData, float textureTilingX, float textureTilingY) {
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
			meshData->Vertices[(i * n) + j].TexCoord.x = j * du * textureTilingX;
			meshData->Vertices[(i * n) + j].TexCoord.y = i * dv * textureTilingY;
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

void GeometryGenerator::CreateBox(float width, float height, float depth, MeshData *meshData) {
	//
	// Create the vertices.
	//

	Vertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	meshData->Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	uint i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData->Indices.assign(&i[0], &i[36]);
}

void GeometryGenerator::CreateSphere(float radius, uint sliceCount, uint stackCount, MeshData *meshData) {
	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);

	meshData->Vertices.push_back(topVertex);

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint i = 1; i <= stackCount - 1; ++i) {
		float phi = i * phiStep;

		// Vertices of ring.
		for (uint j = 0; j <= sliceCount; ++j) {
			float theta = j * thetaStep;

			Vertex v;

			// spherical to cartesian
			v.Position.x = radius * sinf(phi) * cosf(theta);
			v.Position.y = radius * cosf(phi);
			v.Position.z = radius * sinf(phi) * sinf(theta);

			DirectX::XMVECTOR p = XMLoadFloat3(&v.Position);
			XMStoreFloat3(&v.Normal, DirectX::XMVector3Normalize(p));

			v.TexCoord.x = theta / DirectX::XM_2PI;
			v.TexCoord.y = phi / DirectX::XM_2PI;

			meshData->Vertices.push_back(v);
		}
	}

	meshData->Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint i = 1; i <= sliceCount; ++i) {
		meshData->Indices.push_back(0);
		meshData->Indices.push_back(i + 1);
		meshData->Indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint baseIndex = 1;
	uint ringVertexCount = sliceCount + 1;
	for (uint i = 0; i < stackCount - 2; ++i) {
		for (uint j = 0; j < sliceCount; ++j) {
			meshData->Indices.push_back(baseIndex + i * ringVertexCount + j);
			meshData->Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData->Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

			meshData->Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
			meshData->Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData->Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint southPoleIndex = (uint)meshData->Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint i = 0; i < sliceCount; ++i) {
		meshData->Indices.push_back(southPoleIndex);
		meshData->Indices.push_back(baseIndex + i);
		meshData->Indices.push_back(baseIndex + i + 1);
	}
}

void GeometryGenerator::CreateFullscreenQuad(MeshData &meshData) {
	meshData.Vertices.resize(4);
	meshData.Indices.resize(6);

	// Position coordinates specified in NDC space.
	meshData.Vertices[0] = Vertex(
		-1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = Vertex(
		-1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = Vertex(
		+1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = Vertex(
		+1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 1.0f);

	meshData.Indices[0] = 0;
	meshData.Indices[1] = 1;
	meshData.Indices[2] = 2;

	meshData.Indices[3] = 0;
	meshData.Indices[4] = 2;
	meshData.Indices[5] = 3;
}

void GeometryGenerator::CreateCone(float angle, float height, uint sliceCount, MeshData *meshData, bool invert) {
	float bottomRadius = tanf(angle / 2.0f) * height;
	float phiHeight = tanf(angle / 2.0f) * bottomRadius;
	phiHeight = invert ? -phiHeight : phiHeight;

	float top = invert ? 0.0f : height;
	float bottom = invert ? height : 0.0f;

	// Vertices of ring
	float dTheta = DirectX::XM_2PI / sliceCount;
	float halfDTheta = dTheta / 2.0f;
	for (uint i = 0; i < sliceCount; ++i) {
		Vertex firstVertex;
		firstVertex.Position = DirectX::XMFLOAT3(bottomRadius * cosf(i * dTheta), bottom, bottomRadius * sinf(i * dTheta));
		firstVertex.Normal = DirectX::XMFLOAT3(firstVertex.Position.x, phiHeight, firstVertex.Position.z);
		// Normalize and store
		DirectX::XMVECTOR normalizedNormal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&firstVertex.Normal));
		DirectX::XMStoreFloat3(&firstVertex.Normal, normalizedNormal);
		meshData->Vertices.push_back(firstVertex);

		Vertex topVertex;
		topVertex.Position = DirectX::XMFLOAT3(0.0f, top, 0.0f);
		// Point halfway between 'firstVertex' and the next vertex
		topVertex.Normal = DirectX::XMFLOAT3(bottomRadius * cosf(i * dTheta + halfDTheta), phiHeight, bottomRadius * (sinf(i * dTheta + halfDTheta)));
		// Normalize and store
		normalizedNormal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&topVertex.Normal));
		DirectX::XMStoreFloat3(&topVertex.Normal, normalizedNormal);
		meshData->Vertices.push_back(topVertex);
	}

	for (uint i = 0; i < sliceCount * 2; i += 2) {
		meshData->Indices.push_back(i);
		meshData->Indices.push_back(i + 1);

		uint lastIndex = i + 2;
		if (lastIndex == sliceCount * 2) {
			lastIndex = 0;
		}
		meshData->Indices.push_back(lastIndex);
	}

	// Build the bottom cap
	uint baseIndex = meshData->Vertices.size();

	Vertex centerVertex;
	centerVertex.Position = DirectX::XMFLOAT3(0.0f, bottom, 0.0f);
	centerVertex.Normal = DirectX::XMFLOAT3(0.0f, invert ? 1.0f : -1.0f, 0.0f);
	meshData->Vertices.push_back(centerVertex);

	for (uint i = 0; i < sliceCount; ++i) {
		Vertex bottomVertex;
		bottomVertex.Position = DirectX::XMFLOAT3(bottomRadius * cosf(i * dTheta), bottom, bottomRadius * sinf(i * dTheta));
		bottomVertex.Normal = centerVertex.Normal;
		meshData->Vertices.push_back(bottomVertex);
	}

	for (uint i = 1; i <= sliceCount; ++i) {
		uint lastIndex = baseIndex + i + 1;
		if (lastIndex == meshData->Vertices.size()) {
			lastIndex = 0;
		}
		meshData->Indices.push_back(lastIndex);
		meshData->Indices.push_back(baseIndex);
		meshData->Indices.push_back(baseIndex + i);
	}
}

typedef std::tuple<uint, uint, uint> TupleUInt3;
typedef std::tuple<TupleUInt3, TupleUInt3, TupleUInt3> FaceTuple;

struct ObjMaterial {
	DirectX::XMFLOAT4 Ambient; // w = SpecularIntensity
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; // w = SpecPower

	std::wstring DiffuseMapFile;
	std::wstring AmbientMapFile;
	std::wstring SpecularColorMapFile;
	std::wstring SpecularHighlightMapFile;
	std::wstring AlphaMapFile;
	std::wstring BumpMapFile;
};

bool GeometryGenerator::LoadFromOBJ(const wchar *fileName, MeshData *meshData, std::vector<MeshSubset> *meshSubsets, bool calculateAABB, bool fileIsRightHanded, bool flipFaces) {
	// String to hold our obj material library filename (model.mtl)
	std::vector<std::wstring> meshMatLibs;

	//Arrays to store our model's information
	std::unordered_map<TupleUInt3, uint> vertexMap;

	std::vector<DirectX::XMFLOAT3> vertPos;
	std::vector<DirectX::XMFLOAT3> vertNorm;
	std::vector<DirectX::XMFLOAT2> vertTexCoord;
	std::vector<std::string> subsetMaterialNames;

	bool hasNormals = false;

	std::string line;
	char nextChar;
	
	DWORD bytesRead;
	char *fileBuffer = ReadWholeFile(fileName, &bytesRead);
	if (fileBuffer == NULL) {
		return false;
	}

	Common::MemoryInputStream fin(fileBuffer, bytesRead);

	uint lineNumber = 0;
	while (SafeGetLine(fin, line)) {
		++lineNumber;
		Trim(line);

		if (line.empty()) {
			continue;
		}

		switch (line[0]) {
		// A comment. Skip rest of the line
		case '#':
			break;

		// Get Vertex Descriptions;
		case 'v':
			nextChar = line[1];
			// v - vert position
			if (nextChar == ' ') {
				float vx, vy, vz;
				sscanf_s(line.c_str(), "%*s %f %f %f", &vx, &vy, &vz);    //Store the next three types

				if (fileIsRightHanded) { //If model is from an RH Coord System
					vertPos.push_back(DirectX::XMFLOAT3(vx, vy, vz * -1.0f));    //Invert the Z axis
				} else {
					vertPos.push_back(DirectX::XMFLOAT3(vx, vy, vz));
				}
			} else if (nextChar == 't') {
				// vt - vert tex coords
				float vtcu, vtcv;
				sscanf_s(line.c_str(), "%*s %f %f", &vtcu, &vtcv);    //Store the next two types

				if (fileIsRightHanded) { //If model is from an RH Coord System
					vertTexCoord.push_back(DirectX::XMFLOAT2(vtcu, 1.0f - vtcv));    //Reverse the "v" axis
				} else {
					vertTexCoord.push_back(DirectX::XMFLOAT2(vtcu, vtcv));
				}
			} else if (nextChar == 'n') {
				// vn - vert normal
				hasNormals = true;

				float vnx, vny, vnz;
				sscanf_s(line.c_str(), "%*s %f %f %f", &vnx, &vny, &vnz); //Store the next three types

				if (fileIsRightHanded) { //If model is from an RH Coord System
					vertNorm.push_back(DirectX::XMFLOAT3(vnx, vny, vnz * -1.0f));    //Invert the Z axis
				} else {
					vertNorm.push_back(DirectX::XMFLOAT3(vnx, vny, vnz));
				}
			}
			break;

		// New group (Subset)
		case 'g': // g - defines a group
			nextChar = line[1];
			if (nextChar == ' ') {
				// TODO: Contemplate saving the group names and being able to render them by name as well as by index

				// Do nothing, AKA ignore groups
			}
			break;

		// Get Face Index
		case 'f': // f - defines the faces
			nextChar = line[1];
			if (nextChar == ' ') {
				std::vector<std::string> indexGroups;
				Tokenize(line.substr(2), indexGroups, " ", true);

				// Process the first two vertices. 
				// Then treat any following vertices as triangles off the first two vertices

				// Holds the first and last vertex indices for the polygon
				uint firstIndex, lastIndex;

				// First vertex
				std::vector<std::string> indexStrings;
				for (uint i = 0; i < 2; ++i) {
					indexStrings.clear();
					Tokenize(indexGroups[i], indexStrings, "/", false);
					int posIndex = std::stoul(indexStrings[0]);
					int texCoordIndex = indexStrings.size() > 1 && indexStrings[1].length() > 0 ? std::stoul(indexStrings[1]) : 0;
					int normalIndex = indexStrings.size() > 2 ? std::stoul(indexStrings[2]) : 0;

					if (posIndex < 0) {
						posIndex += vertPos.size();
					}
					if (texCoordIndex < 0) {
						texCoordIndex += vertTexCoord.size();
					}
					if (normalIndex < 0) {
						normalIndex += vertNorm.size();
					}

					TupleUInt3 vertexTuple{posIndex, texCoordIndex, normalIndex};

					auto iter = vertexMap.find(vertexTuple);
					if (iter != vertexMap.end()) {
						// We found a match
						if (i == 0) {
							firstIndex = iter->second;
						} else {
							lastIndex = iter->second;
						}
					} else {
						// No match. Make a new one
						uint index = meshData->Vertices.size();
						vertexMap[vertexTuple] = index;

						DirectX::XMFLOAT3 position = posIndex == 0 ? DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) : vertPos[posIndex - 1];
						DirectX::XMFLOAT3 normal = normalIndex == 0 ? DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) : vertNorm[normalIndex - 1];
						DirectX::XMFLOAT2 texCoord = texCoordIndex == 0 ? DirectX::XMFLOAT2(0.0f, 0.0f) : vertTexCoord[texCoordIndex - 1];

						meshData->Vertices.push_back(Vertex(position, normal, texCoord));
						if (i == 0) {
							firstIndex = index;
						} else {
							lastIndex = index;
						}
					}
				}

				// If there are more than three vertices in the face definition, we need to make sure
				// we convert the face to triangles. We created our first triangle above, now we will
				// create a new triangle for every new vertex in the face, using the very first vertex
				// of the face, and the last vertex from the triangle before the current triangle
				for (uint i = 2; i < indexGroups.size(); ++i)    // Loop through the next vertices to create new triangles
				{
					// First vertex of this triangle (the very first vertex of the face too)
					meshData->Indices.push_back(firstIndex);           // Set index for this vertex

					// Second Vertex of this triangle (the last vertex used in the tri before this one)
					meshData->Indices.push_back(lastIndex);            // Set index for this vertex

					// Get the third vertex for this triangle
					indexStrings.clear();
					Tokenize(indexGroups[i], indexStrings, "/", false);
					int posIndex = std::stoul(indexStrings[0]);
					int texCoordIndex = indexStrings.size() > 1 && indexStrings[1].length() > 0 ? std::stoul(indexStrings[1]) : 0;
					int normalIndex = indexStrings.size() > 2 ? std::stoul(indexStrings[2]) : 0;

					if (posIndex < 0) {
						posIndex += vertPos.size();
					}
					if (texCoordIndex < 0) {
						texCoordIndex += vertTexCoord.size();
					}
					if (normalIndex < 0) {
						normalIndex += vertNorm.size();
					}

					TupleUInt3 vertexTuple{posIndex, texCoordIndex, normalIndex};

					auto iter = vertexMap.find(vertexTuple);
					if (iter != vertexMap.end()) {
						// We found a match
						meshData->Indices.push_back(iter->second);
						lastIndex = iter->second;
					} else {
						// No match. Make a new one
						lastIndex = meshData->Vertices.size();
						vertexMap[vertexTuple] = lastIndex;

						DirectX::XMFLOAT3 position = posIndex == 0 ? DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) : vertPos[posIndex - 1];
						DirectX::XMFLOAT3 normal = normalIndex == 0 ? DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) : vertNorm[normalIndex - 1];
						DirectX::XMFLOAT2 texCoord = texCoordIndex == 0 ? DirectX::XMFLOAT2(0.0f, 0.0f) : vertTexCoord[texCoordIndex - 1];

						meshData->Vertices.push_back(Vertex(position, normal, texCoord));
						meshData->Indices.push_back(lastIndex);
					}
				}
			}
			break;

		case 'm': // mtllib - material library filename
			if (line.find("mtllib") != std::string::npos) {
				// Store the material libraries file name
				// We can't use sscanf because the path can contain spaces
				std::string temp(line.substr(7));
				// Reserve enough space
				std::wstring tempWide(temp.length(), L' ');
				std::copy(temp.begin(), temp.end(), tempWide.begin());
				meshMatLibs.push_back(tempWide);
			}

			break;

		case 'u': // usemtl - which material to use
			if (line.find("usemtl") != std::string::npos) {
				subsetMaterialNames.push_back(line.substr(7));

				// Set the length of the previous subset
				if (meshSubsets->size() != 0) {
					GeometryGenerator::MeshSubset *lastSubset = &meshSubsets->back();
					lastSubset->IndexCount = meshData->Indices.size() - lastSubset->IndexStart;
				}

				meshSubsets->push_back(GeometryGenerator::MeshSubset());
				GeometryGenerator::MeshSubset *subset = &meshSubsets->back();
				subset->IndexStart = meshData->Indices.size();
			}
			break;

		default:
			break;
		}
	}

	// Make sure there is at least one subset
	if (meshSubsets->size() == 0) {
		meshSubsets->push_back({0, 0, 0, meshData->Indices.size()});
	} else {
		// Set the length of the last subset
		GeometryGenerator::MeshSubset *lastSubset = &meshSubsets->back();
		lastSubset->IndexCount = meshData->Indices.size() - lastSubset->IndexStart;
	}

	// Release the obj file memory
	delete[] fileBuffer;

	// Materials aren't required
	if (meshMatLibs.size() == 0) {
		return true;
	}


	std::unordered_map<std::string, ObjMaterial> materialMap;
	ObjMaterial *currentMaterial;


	// Run through each mtl file and fill materialMap from them
	for (auto iter = meshMatLibs.begin(); iter != meshMatLibs.end(); ++iter) {
		fileBuffer = ReadWholeFile(iter->c_str(), &bytesRead);
		if (fileBuffer == NULL) {
			return false;
		}

		Common::MemoryInputStream fin(fileBuffer, bytesRead);
		
		while (SafeGetLine(fin, line)) {
			Trim(line);

			if (line.empty()) {
				continue;
			}

			switch (line[0]) {
			// Check for comment
			case '#':
				break;

			// Set the colors
			case 'K':
				nextChar = line[1];
				
				if (nextChar == ' ') {
					// Diffuse Color
					float dx, dy, dz;
					sscanf_s(line.c_str(), "%*s %f %f %f", &dx, &dy, &dz);    //Store the next three types

					currentMaterial->Diffuse.x = dx;
					currentMaterial->Diffuse.y = dy;
					currentMaterial->Diffuse.z = dz;
					currentMaterial->Diffuse.w = 1.0f;
				} else if (nextChar == 'a') {
					// Ambient Color
					float ax, ay, az;
					sscanf_s(line.c_str(), "%*s %f %f %f", &ax, &ay, &az);    //Store the next three types

					currentMaterial->Ambient.x = ax;
					currentMaterial->Ambient.y = ay;
					currentMaterial->Ambient.z = az;
					currentMaterial->Ambient.w = 0.0f;
				} else if (nextChar == 's') {
					// Specular Color
					float sx, sy, sz;
					sscanf_s(line.c_str(), "%*s %f %f %f", &sx, &sy, &sz);    //Store the next three types

					currentMaterial->Specular.x = sx;
					currentMaterial->Specular.y = sy;
					currentMaterial->Specular.z = sz;
					currentMaterial->Specular.w = 8.0f;
				}
				break;

			case 'N':
				if (line[1] == 's') {
					// Specular Power (Coefficient)
					sscanf_s(line.c_str(), "%*s %f", &currentMaterial->Specular.w);
				}

				break;

			// Check for transparency
			case 'T':
				if (line[1] == 'r') {
					float temp;
					sscanf_s(line.c_str(), "%*s %f", &temp);

					currentMaterial->Diffuse.w = 1.0f - temp;
				}
				break;

			// Some obj files specify d for transparency
			case 'd':
				if (line[1] == ' ') {
					sscanf_s(line.c_str(), "%*s %f", &currentMaterial->Diffuse.w);
				}
				break;

			case 'm':
				// Get the diffuse map (texture)
				if (line.find("map_Kd") != std::string::npos) {
					// We can't use sscanf because the path can contain spaces
					std::string temp(line.substr(7));
					// Reserve enough space
					std::wstring wideFileName(temp.length(), L' ');
					std::copy(temp.begin(), temp.end(), wideFileName.begin());

					currentMaterial->DiffuseMapFile = wideFileName;
				} else if (line.find("map_Ka") != std::string::npos) {
					// We can't use sscanf because the path can contain spaces
					std::string temp(line.substr(7));
					// Reserve enough space
					std::wstring wideFileName(temp.length(), L' ');
					std::copy(temp.begin(), temp.end(), wideFileName.begin());

					currentMaterial->AmbientMapFile = wideFileName;
				} else if (line.find("map_Ks") != std::string::npos) {
					// We can't use sscanf because the path can contain spaces
					std::string temp(line.substr(7));
					// Reserve enough space
					std::wstring wideFileName(temp.length(), L' ');
					std::copy(temp.begin(), temp.end(), wideFileName.begin());

					currentMaterial->SpecularColorMapFile = wideFileName;
				} else if (line.find("map_Ns") != std::string::npos) {
					// We can't use sscanf because the path can contain spaces
					std::string temp(line.substr(7));
					// Reserve enough space
					std::wstring wideFileName(temp.length(), L' ');
					std::copy(temp.begin(), temp.end(), wideFileName.begin());

					currentMaterial->SpecularHighlightMapFile = wideFileName;
				} else if (line.find("map_d") != std::string::npos) {
					// We can't use sscanf because the path can contain spaces
					std::string temp(line.substr(6));
					// Reserve enough space
					std::wstring wideFileName(temp.length(), L' ');
					std::copy(temp.begin(), temp.end(), wideFileName.begin());

					currentMaterial->AlphaMapFile = wideFileName;
				} else if (line.find("map_bump") != std::string::npos) {
					// We can't use sscanf because the path can contain spaces
					std::string temp(line.substr(9));
					// Reserve enough space
					std::wstring wideFileName(temp.length(), L' ');
					std::copy(temp.begin(), temp.end(), wideFileName.begin());

					currentMaterial->BumpMapFile = wideFileName;
				}

				break;

			case 'n': // newmtl - Declare new material
				if (line.find("newmtl") != std::string::npos) {
					// New material, set its defaults
					ObjMaterial newMaterial;
					newMaterial.Diffuse = DirectX::XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
					newMaterial.Ambient = DirectX::XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
					newMaterial.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 8.0f);

					currentMaterial = &materialMap[line.substr(7)];
					*currentMaterial = newMaterial;
				}
				break;

			default:
				break;
			}
		}

		delete[] fileBuffer;
	}

	uint totalVertices = meshData->Vertices.size();

	// Apply the parsed materials to the subsets
	for (uint i = 0; i < meshSubsets->size(); ++i) {
		auto iter = materialMap.find(subsetMaterialNames[i]);
		if (iter != materialMap.end()) {
			(*meshSubsets)[i].Diffuse = iter->second.Diffuse;
			(*meshSubsets)[i].Ambient = iter->second.Ambient;
			(*meshSubsets)[i].Specular = iter->second.Specular;
			(*meshSubsets)[i].DiffuseMapFile = iter->second.DiffuseMapFile;
			(*meshSubsets)[i].AmbientMapFile = iter->second.AmbientMapFile;
			(*meshSubsets)[i].SpecularColorMapFile = iter->second.SpecularColorMapFile;
			(*meshSubsets)[i].SpecularHighlightMapFile = iter->second.SpecularHighlightMapFile;
			(*meshSubsets)[i].AlphaMapFile = iter->second.AlphaMapFile;
			(*meshSubsets)[i].BumpMapFile = iter->second.BumpMapFile;
		} else {
			// TODO: Add error message
			return false;
		}

		// Since we're already iterating through the subsets, set all the vertex count and start
		(*meshSubsets)[i].VertexStart = 0;
		(*meshSubsets)[i].VertexCount = totalVertices;
	}

	// Calculate the AABB
	if (calculateAABB) {
		for (auto iter = meshSubsets->begin(); iter != meshSubsets->end(); ++iter) {
			DirectX::XMVECTOR min = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			DirectX::XMVECTOR max = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			
			for (uint i = iter->IndexStart; i < iter->IndexStart + iter->IndexCount; ++i) {
				DirectX::XMVECTOR position(DirectX::XMLoadFloat3(&meshData->Vertices[meshData->Indices[i]].Position));
				min = DirectX::XMVectorMin(position, min);
				max = DirectX::XMVectorMax(position, max);
			}

			DirectX::XMStoreFloat3(&iter->AABBMin, min);
			DirectX::XMStoreFloat3(&iter->AABBMax, max);
		}
	}

	//// If the model doesn't have normals, we have to compute them ourselves
	//if (!hasNormals) {
	//	std::vector<DirectX::XMFLOAT3> tempNormal;

	//	//normalized and unnormalized normals
	//	DirectX::XMFLOAT3 unnormalized = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	//	//Used to get vectors (sides) from the position of the verts
	//	float vecX, vecY, vecZ;

	//	//Two edges of our triangle
	//	DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//	DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	//	//Compute face normals
	//	for (int i = 0; i < meshData->Indices.size(); i += 3) {
	//		//Get the vector describing one edge of our triangle (edge 0,2)
	//		uint index = meshData->Indices[i];
	//		vecX = meshData->Vertices[index].Position.x - meshData->Vertices[meshData->Indices[i + 2]].Position.x;
	//		vecY = meshData->Vertices[meshData->Indices[i]].Position.y - meshData->Vertices[meshData->Indices[i + 2]].Position.y;
	//		vecZ = meshData->Vertices[meshData->Indices[i]].Position.z - meshData->Vertices[meshData->Indices[i + 2]].Position.z;
	//		edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our first edge

	//		//Get the vector describing another edge of our triangle (edge 2,1)
	//		vecX = meshData->Vertices[meshData->Indices[i + 2]].Position.x - meshData->Vertices[meshData->Indices[i + 1]].Position.x;
	//		vecY = meshData->Vertices[meshData->Indices[i + 2]].Position.y - meshData->Vertices[meshData->Indices[i + 1]].Position.y;
	//		vecZ = meshData->Vertices[meshData->Indices[i + 2]].Position.z - meshData->Vertices[meshData->Indices[i + 1]].Position.z;
	//		edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our second edge

	//		//Cross multiply the two edge vectors to get the un-normalized face normal
	//		DirectX::XMStoreFloat3(&unnormalized, DirectX::XMVector3Cross(edge1, edge2));
	//		tempNormal.push_back(unnormalized);			//Save unormalized normal (for normal averaging)
	//	}

	//	//Compute vertex normals (normal Averaging)
	//	DirectX::XMVECTOR normalSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//	int facesUsing = 0;
	//	float tX;
	//	float tY;
	//	float tZ;

	//	//Go through each vertex
	//	for (int i = 0; i < meshData->Vertices.size(); ++i) {
	//		//Check which triangles use this vertex
	//		for (int j = 0; j < meshData->Indices.size() / 3; ++j) {
	//			if (meshData->Indices[j * 3] == i || meshData->Indices[(j * 3) + 1] == i || meshData->Indices[(j * 3) + 2] == i) {
	//				tX = DirectX::XMVectorGetX(normalSum) + tempNormal[j].x;
	//				tY = DirectX::XMVectorGetY(normalSum) + tempNormal[j].y;
	//				tZ = DirectX::XMVectorGetZ(normalSum) + tempNormal[j].z;

	//				normalSum = DirectX::XMVectorSet(tX, tY, tZ, 0.0f);	//If a face is using the vertex, add the unormalized face normal to the normalSum
	//				facesUsing++;
	//			}
	//		}

	//		//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
	//		normalSum = DirectX::XMVectorScale(normalSum, 1.0f / facesUsing);

	//		//Normalize the normalSum vector
	//		normalSum = DirectX::XMVector3Normalize(normalSum);

	//		//Store the normal in our current vertex
	//		meshData->Vertices[i].Normal.x = DirectX::XMVectorGetX(normalSum);
	//		meshData->Vertices[i].Normal.y = DirectX::XMVectorGetY(normalSum);
	//		meshData->Vertices[i].Normal.z = DirectX::XMVectorGetZ(normalSum);

	//		//Clear normalSum and facesUsing for next vertex
	//		normalSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//		facesUsing = 0;
	//	}
	//}

	return true;
}



} // End of namespace Common
