/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_GEOMETRY_GENERATOR_H
#define COMMON_GEOMETRY_GENERATOR_H

#include "common/typedefs.h"

#include "common/materials.h"

#include "DirectXMath.h"
#include <vector>


namespace Common {

class GeometryGenerator {
public:
	struct Vertex {
	public:
		Vertex() {}
		Vertex(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &normal, const DirectX::XMFLOAT2 &uv)
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

	struct MeshSubset {
		uint VertexStart;
		uint VertexCount;

		uint IndexStart;
		uint IndexCount;

		DirectX::XMFLOAT4 Ambient; // w = SpecularIntensity
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Specular; // w = SpecPower

		std::wstring DiffuseMapFile;
		std::wstring AmbientMapFile;
		std::wstring SpecularColorMapFile;
		std::wstring SpecularHighlightMapFile;
		std::wstring AlphaMapFile;
		std::wstring BumpMapFile;

		DirectX::XMFLOAT3 AABBMin;
		DirectX::XMFLOAT3 AABBMax;
	};

	/**
	 * Creates a rectangular prism with 6 quads
	 * (All quads are triangulated)
	 *
	 * @param width       The width of the box
	 * @param height      The height of the box
	 * @param depth       The depth of the box
	 * @param meshData    Pointer to the MeshData object that will be filled with the box data
	 */
	static void CreateBox(float width, float height, float depth, MeshData *meshData);
	/**
	 * Creates a sphere by dividing it into radial slices and vertical stacks to form quads
	 * (All quads are triangulated)
	 *
	 * @param radius        Radius of the sphere
	 * @param sliceCount    Number of slices to divide the sphere into
	 * @param stackCount    Number of stacks to divide the sphere into
	 * @param meshData      Pointer to the MeshData object that will be filled with the box data
	 */
	static void CreateSphere(float radius, uint sliceCount, uint stackCount, MeshData *meshData);
	/**
	 * Creates a sphere by repeatedly subdividing the triangles making up the mesh and re-projecting them onto the sphere
	 *
	 * @param radius             Radius of the sphere
	 * @param numSubdivisions    The number of subdivisions to perform
	 * @param meshData           Pointer to the MeshData object that will be filled with the sphere data
	 */
	static void CreateGeosphere(float radius, uint numSubdivisions, MeshData *meshData) {}
	/**
	 * Creates a cylinder by dividing it into radial slices and vertical stacks to form quads and triangles
	 * (All quads are triangulated)
	 *
	 * @param bottomRadius    Radius of the bottom cap of the cylinder
	 * @param topRadius       Radius of the top cap of the cylinder
	 * @param height          Height of the cylinder
	 * @param sliceCount      Number of slices to divide the sphere into
	 * @param stackCount      Number of stacks to divide the sphere into
	 * @param meshData        Pointer to the MeshData object that will be filled with the cylinder data
	 */
	static void CreateCylinder(float bottomRadius, float topRadius, float height, uint sliceCount, uint stackCount, MeshData *meshData) {}
	/**
	 * Creates a cone by dividing it into radial slices.
	 *
	 * @param angle         Angle of the cone (In radians)
	 * @param height        Height of the cone
	 * @param sliceCount    Number of slices to divide the cone into
	 * @param meshData      Pointer to the MeshData object that will be filled with the cone data
	 * @return
	 */
	static void CreateCone(float angle, float height, uint sliceCount, MeshData *meshData, bool invert = false);
	/**
	 * Creates a grid of quads
	 *
	 * @param width             The width of the grid
	 * @param depth             The depth of the grid
	 * @param m                 The number of grid subdivisions in the x direction
	 * @param n                 The number of grid subdivisions in the z direction
	 * @param meshData          Pointer to the MeshData object that will be filled with the grid data
	 * @param textureTilingX    How much to tile the texture coordinates in the U direction. 1.0 means the texture will be stretched across the entire grid, 2.0 means the texture will be tiled twice, etc.
	 * @param textureTilingY    How much to tile the texture coordinates in the V direction. 1.0 means the texture will be stretched across the entire grid, 2.0 means the texture will be tiled twice, etc.
	 */
	static void CreateGrid(float width, float depth, uint m, uint n, MeshData *meshData, float textureTilingX = 1.0f, float textureTilingY = 1.0f);
	/**
	 * Create a triangulated quad the size of the screen
	 *
	 * @param meshData    Pointer to the MeshData object that will be filled with the quad data
	 */
	static void CreateFullscreenQuad(MeshData &meshData);
	static bool LoadFromOBJ(const wchar *fileName, MeshData *meshData, std::vector<MeshSubset> *meshSubsets, bool calculateAABB = false, bool fileIsRightHanded = false, bool flipFaces = false);
};

} // End of namespace Common

#endif
