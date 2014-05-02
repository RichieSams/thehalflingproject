/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef OBJ_HMF_CONVERTER_UTIL_H
#define OBJ_HMF_CONVERTER_UTIL_H

#include <d3d11.h>
#include <DirectXMath.h>

#include <assimp/scene.h>

#include <string>
#include <filesystem>


namespace ObjHmfConverter {

/**
 * Tries to parse a string into a D3D11_USAGE
 * If the parse fails, the default return is D3D11_USAGE_IMMUTABLE
 * 
 * @param inputString    The string to parse into a usage
 * @return               The usage
 */
D3D11_USAGE ParseUsageFromString(std::string &inputString);
/**
 * Tries to parse a string into an aiTextureType
 * If the parse fails, the default return is 'defaultType'
 * 
 * @param inputString    The string to parse into a texture type
 * @param defaultType    The value that should be returned if the parse fails
 * @return               The texture type
 */
aiTextureType ParseTextureTypeFromString(std::string &inputString, aiTextureType defaultType);
/**
 * Creates an ini file with the default values. This is useful
 * if the user wants to know how to use the ini file.
 * 
 * @param filePath    The path to the new file
 */
void CreateDefaultIniFile(const char *filePath);
/**
 * Converts a texture to a dds file. 
 * If the source file is already in dds format, it is just copied to the destination directory.
 * If the file already exists in the destination directory, the function does nothing
 * 
 * @param filePath               The relative input path. Relative to rootInputDirectory
 * @param baseDirectory          The directory of OBJ-HMFConverter.exe. This is needed to find textconv.exe
 * @param rootInputDirectory     The directory of the input model file
 * @param rootOutputDirectory    The directory of the output model file

 * @return                       For convenience. Returns the filePath with the extension changed to .dds
 */
std::string ConvertToDDS(const char *filePath, std::tr2::sys::path &baseDirectory, std::tr2::sys::path &rootInputDirectory, std::tr2::sys::path &rootOutputDirectory);

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 tangent;
};

struct ImporterIniFile {
public:
	ImporterIniFile()
		: GenNormals(true),
		  CalcTangents(true),
		  UseMaterialAmbientColor(true),
		  UseMaterialDiffuseColor(true),
		  UseMaterialSpecColor(true),
		  UseMaterialOpacity(true),
		  UseMaterialSpecPower(true),
		  UseMaterialSpecIntensity(true),
		  UseDiffuseColorMap(true),
		  UseNormalMap(true),
		  UseDisplacementMap(true),
		  UseAlphaMap(true),
		  UseSpecColorMap(true),
		  UseSpecPowerMap(true),
		  VertexBufferUsage(D3D11_USAGE_IMMUTABLE),
		  IndexBufferUsage(D3D11_USAGE_IMMUTABLE),
		  DiffuseColorMapTextureType(aiTextureType_DIFFUSE),
		  NormalMapTextureType(aiTextureType_NORMALS),
		  DisplacementMapTextureType(aiTextureType_DISPLACEMENT),
		  AlphaMapTextureType(aiTextureType_OPACITY),
		  SpecColorMapTextureType(aiTextureType_SPECULAR),
		  SpecPowerMapTextureType(aiTextureType_SHININESS) {
	}

	bool GenNormals;
	bool CalcTangents;

	bool UseMaterialAmbientColor;
	bool UseMaterialDiffuseColor;
	bool UseMaterialSpecColor;
	bool UseMaterialOpacity;
	bool UseMaterialSpecPower;
	bool UseMaterialSpecIntensity;

	bool UseDiffuseColorMap;
	bool UseNormalMap;
	bool UseDisplacementMap;
	bool UseAlphaMap;
	bool UseSpecColorMap;
	bool UseSpecPowerMap;

	D3D11_USAGE VertexBufferUsage;
	D3D11_USAGE IndexBufferUsage;

	aiTextureType DiffuseColorMapTextureType;
	aiTextureType NormalMapTextureType;
	aiTextureType DisplacementMapTextureType;
	aiTextureType AlphaMapTextureType;
	aiTextureType SpecColorMapTextureType;
	aiTextureType SpecPowerMapTextureType;
};

} // End of namespace ObjHmfConverter

#endif
