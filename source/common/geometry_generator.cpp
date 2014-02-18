/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/geometry_generator.h"

#include "common/file_io_util.h"
#include "common/string_util.h"

#include <fstream>
#include <stdio.h>
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

bool GeometryGenerator::LoadFromOBJ(const wchar *fileName,  MeshData *meshData, std::vector<MeshSubset> *meshSubsets, bool fileIsRightHanded) {
	//Arrays to store our model's information
	std::unordered_map<Vertex, uint, Vertex> vertexMap;

	std::vector<DirectX::XMFLOAT3> vertPos;
	std::vector<DirectX::XMFLOAT3> vertNorm;
	std::vector<DirectX::XMFLOAT2> vertTexCoord;
	std::vector<FaceTuple> faces;

	std::vector<std::string> meshMaterials;

	std::wstring materialFile;
	materialFile.reserve(200);

	char materialName[200];

	std::string line;
	char nextChar;

	std::ifstream fin(fileName, std::ios::in);
	uint lineNumber = 0;

	if (!fin.is_open()) {
		return false;
	}

	while (!fin.eof()) {
		SafeGetLine(fin, line); //Get next line
		Trim(line);
		++lineNumber;

		if (line.empty()) {
			continue;
		}

		switch (line[0]) {
		// Comments
		case '#':
			break;
		// Vertex Descriptions
		case 'v':
			nextChar = line[1];
			// v - vert position
			if (nextChar == ' ') {
				float vx, vy, vz;
				sscanf(line.c_str(), "%*s %f %f %f", &vx, &vy, &vz);    //Store the next three types

				if (fileIsRightHanded) { //If model is from an RH Coord System
					vertPos.push_back(DirectX::XMFLOAT3(vx, vy, vz * -1.0f));    //Invert the Z axis
				} else {
					vertPos.push_back(DirectX::XMFLOAT3(vx, vy, vz));
				}
			}
			// vt - vert tex coords
			if (nextChar == 't') {
				float vtcu, vtcv;
				sscanf(line.c_str(), "%*s %f %f", &vtcu, &vtcv);    //Store the next two types

				if (fileIsRightHanded) { //If model is from an RH Coord System
					vertTexCoord.push_back(DirectX::XMFLOAT2(vtcu, 1.0f - vtcv));    //Reverse the "v" axis
				} else {
					vertTexCoord.push_back(DirectX::XMFLOAT2(vtcu, vtcv));
				}
			}
			// vn - vert normal
			if (nextChar == 'n') {
				float vnx, vny, vnz;
				sscanf(line.c_str(), "%*s %f %f %f", &vnx, &vny, &vnz); //Store the next three types

				if (fileIsRightHanded) { //If model is from an RH Coord System
					vertNorm.push_back(DirectX::XMFLOAT3(vnx, vny, vnz * -1.0f));    //Invert the Z axis
				} else {
					vertNorm.push_back(DirectX::XMFLOAT3(vnx, vny, vnz));
				}
			}
			break;

		// New group (Subset)
		case 'g':   //g - defines a group
			nextChar = line[1];
			if (nextChar == ' ') {
				// TODO: Contemplate saving the group names and being able to render them by name as well as by index

				// Do nothing, AKA ignore groups
			}
			break;

		case 'f': {
			bool isQuad = false;
			char prefix[2];
			int a[4], b[4], c[4];

			// Support triangles and quads
			if (sscanf(line.c_str(), "%2s %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", prefix, &a[0], &b[0], &c[0], &a[1], &b[1], &c[1], &a[2], &b[2], &c[2], &a[3], &b[3], &c[3]) == 13) {
				isQuad = true;
			} else if (sscanf(line.c_str(), "%2s %d//%d %d//%d %d//%d %d//%d", prefix, &a[0], &c[0], &a[1], &c[1], &a[2], &c[2], &a[3], &c[3]) == 9) {
				isQuad = true;
				b[0] = b[1] = b[2] = b[3] = 0;
			} else if (sscanf(line.c_str(), "%2s %d/%d %d/%d %d/%d %d/%d", prefix, &a[0], &b[0], &a[1], &b[1], &a[2], &b[2]) == 9) {
				isQuad = true;
				c[0] = c[1] = c[2] = c[3] = 0;
			} else if (sscanf(line.c_str(), "%2s %d/%d/%d %d/%d/%d %d/%d/%d", prefix, &a[0], &b[0], &c[0], &a[1], &b[1], &c[1], &a[2], &b[2], &c[2]) == 10) {
				a[3] = b[3] = c[3] = 0;
			} else if (sscanf(line.c_str(), "%2s %d//%d %d//%d %d//%d", prefix, &a[0], &c[0], &a[1], &c[1], &a[2], &c[2]) == 7) {
				b[0] = b[1] = b[2] = 0;
				a[3] = b[3] = c[3] = 0;
			} else if (sscanf(line.c_str(), "%2s %d/%d %d/%d %d/%d", prefix, &a[0], &b[0], &a[1], &b[1], &a[2], &b[2]) == 7) {
				c[0] = c[1] = c[2] = 0;
				a[3] = b[3] = c[3] = 0;
			} else {
				break;
			}

			// Handle negative indices
			for (uint i = 0; i < 4; ++i) {
				if (a[i] < 0) {
					a[i] += vertPos.size() + 1;
				}
				if (b[i] < 0) {
					b[i] += vertTexCoord.size() + 1;
				}
				if (c[i] < 0) {
					c[i] += vertNorm.size() + 1;
				}
			}

			// Store the index of the current material and the vertex indices
			if (isQuad) {
				// Triangulate quads
				faces.push_back(FaceTuple(TupleUInt3(a[0], b[0], c[0]), TupleUInt3(a[1], b[1], c[1]), TupleUInt3(a[3], b[3], c[3])));
				faces.push_back(FaceTuple(TupleUInt3(a[3], b[3], c[3]), TupleUInt3(a[1], b[1], c[1]), TupleUInt3(a[2], b[2], c[2])));
			} else {
				faces.push_back(FaceTuple(TupleUInt3(a[0], b[0], c[0]), TupleUInt3(a[1], b[1], c[1]), TupleUInt3(a[2], b[2], c[2])));
			}

			break;
		}
		case 'm':   //mtllib - material library filename
			if (line.find("mtllib") != std::string::npos) {
				//Store the material libraries file name
				sscanf(line.c_str(), "mtllib %200s", materialFile.c_str());
			}
			break;

		case 'u':   //usemtl - which material to use
			if (line.find("usemtl") != std::string::npos) {
				//Store the material libraries file name
				sscanf(line.c_str(), "usemtl %200s", materialName);

				meshMaterials.push_back(materialName);

				// Set the length of the previous subset
				if (meshSubsets->size() != 0) {
					GeometryGenerator::MeshSubset *lastSubset = &meshSubsets->back();
					lastSubset->FaceCount = faces.size() - lastSubset->FaceStart;
				}

				meshSubsets->push_back(GeometryGenerator::MeshSubset());
				GeometryGenerator::MeshSubset *subset = &meshSubsets->back();
				subset->FaceStart = faces.size();
			}
			break;

		default:
			break;
		}
	}

	// Make sure there is at least one subset
	if (meshSubsets->size() == 0) {
		meshSubsets->push_back({0, 0, 0, faces.size()});
	} else {
		// Set the length of the last subset
		GeometryGenerator::MeshSubset *lastSubset = &meshSubsets->back();
		lastSubset->FaceCount = faces.size() - lastSubset->FaceStart;
	}


	// Post processing

	for (uint i = 0; i < faces.size(); ++i) {
		TupleUInt3 firstTuple = std::get<0>(faces[i]);
		TupleUInt3 secondTuple = std::get<1>(faces[i]);
		TupleUInt3 thirdTuple = std::get<2>(faces[i]);

		uint posIndex[3];
		posIndex[0] = std::get<0>(firstTuple);
		posIndex[1] = std::get<0>(secondTuple);
		posIndex[2] = std::get<0>(thirdTuple);

		uint texCoordIndex[3];
		texCoordIndex[0] = std::get<1>(firstTuple);
		texCoordIndex[1] = std::get<1>(secondTuple);
		texCoordIndex[2] = std::get<1>(thirdTuple);

		uint normalIndex[3];
		normalIndex[0] = std::get<2>(firstTuple);
		normalIndex[1] = std::get<2>(secondTuple);
		normalIndex[2] = std::get<2>(thirdTuple);

		for (uint j = 0; j < 3; ++j) {
			DirectX::XMFLOAT3 position = posIndex[j] == 0 ? DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) : vertPos[posIndex[j] - 1];
			DirectX::XMFLOAT3 normal = normalIndex[j] == 0 ? DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) : vertNorm[normalIndex[j] - 1];
			DirectX::XMFLOAT2 texCoord = texCoordIndex[j] == 0 ? DirectX::XMFLOAT2(0.0f, 0.0f) : vertTexCoord[texCoordIndex[j] - 1];

			Vertex newVertex = Vertex(position, normal, texCoord);

			auto iter = vertexMap.find(newVertex);
			if (iter != vertexMap.end()) {
				// We found a match
				meshData->Indices.push_back(iter->second);
			} else {
				// No match. Make a new one
				uint index = meshData->Vertices.size();
				vertexMap[newVertex] = index;
				meshData->Vertices.push_back(newVertex);

				meshData->Indices.push_back(index);
			}
		}
	}

	// Process subsets
	// TODO: Think of a better way to optimize the vertex buffer so we don't have to use the whole buffer for each subset
	for (auto iter = meshSubsets->begin(); iter != meshSubsets->end(); ++iter) {
		iter->VertexStart = 0;
		iter->VertexCount = meshData->Vertices.size();
	}
}

} // End of namespace Common
