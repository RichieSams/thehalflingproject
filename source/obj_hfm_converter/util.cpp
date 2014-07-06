/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_hfm_converter/util.h"

#include "common/file_io_util.h"
#include "common/memory_stream.h"

#include <json/writer.h>
#include <json/value.h>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

#include <fstream>

using filepath = std::tr2::sys::path;

namespace ObjHmfConverter {

std::string ConvertToDDS(const char *filePath, filepath &baseDirectory, filepath &rootInputDirectory, filepath &rootOutputDirectory) {
	filepath relativePath(filePath);
	filepath relativeDDSPath(relativePath);
	relativeDDSPath.replace_extension("dds");
	
	// If the file already exists in the output directory, we don't need to do anything
	filepath outputFilePath(rootOutputDirectory.file_string() + "\\" + relativeDDSPath.file_string());
	if (exists(outputFilePath)) {
		return relativeDDSPath;
	}

	// Guarantee the output directory exists
	filepath outputDirectory(outputFilePath.parent_path());
	create_directories(outputDirectory);

	// If input is already dds, but doesn't exist in the output directory, just copy the file to the output
	filepath inputFilePath(rootInputDirectory.file_string() + "\\" + relativePath.file_string());
	if (_stricmp(relativePath.extension().c_str(), "dds") == 0) {
		copy_file(inputFilePath, outputFilePath);
		return relativeDDSPath;
	}

	// Otherwise, convert the file to DDS
	std::string call = baseDirectory.file_string() + "\\texconv.exe -ft dds -o " + outputDirectory.file_string() + " " + inputFilePath.file_string() + " > NUL";
	std::system(call.c_str());

	return relativeDDSPath;
}

D3D11_USAGE ParseUsageFromString(std::string &inputString) {
	if (_stricmp(inputString.c_str(), "default") == 0) {
		return D3D11_USAGE_DEFAULT;
	} else if (_stricmp(inputString.c_str(), "dynamic") == 0) {
		return D3D11_USAGE_DYNAMIC;
	} else if (_stricmp(inputString.c_str(), "staging") == 0) {
		return D3D11_USAGE_STAGING;
	} else {
		return D3D11_USAGE_IMMUTABLE;
	}
}

aiTextureType ParseTextureTypeFromString(std::string &inputString, aiTextureType defaultType) {
	if (_stricmp(inputString.c_str(), "diffuse") == 0) {
		return aiTextureType_DIFFUSE;
	} else if (_stricmp(inputString.c_str(), "normal") == 0) {
		return aiTextureType_NORMALS;
	} else if (_stricmp(inputString.c_str(), "height") == 0) {
		return aiTextureType_HEIGHT;
	} else if (_stricmp(inputString.c_str(), "displacement") == 0) {
		return aiTextureType_DISPLACEMENT;
	} else if (_stricmp(inputString.c_str(), "alpha") == 0) {
		return aiTextureType_OPACITY;
	} else if (_stricmp(inputString.c_str(), "specColor") == 0) {
		return aiTextureType_SPECULAR;
	} else if (_stricmp(inputString.c_str(), "specPower") == 0) {
		return aiTextureType_SHININESS;
	} else {
		return defaultType;
	}
}

Common::TextureSampler ParseSamplerTypeFromString(std::string &inputString, Common::TextureSampler defaultType) {
	if (_stricmp(inputString.c_str(), "linear_clamp") == 0) {
		return Common::LINEAR_CLAMP;
	} else if (_stricmp(inputString.c_str(), "linear_border") == 0) {
		return Common::LINEAR_BORDER;
	} else if (_stricmp(inputString.c_str(), "linear_wrap") == 0) {
		return Common::LINEAR_WRAP;
	} else if (_stricmp(inputString.c_str(), "point_clamp") == 0) {
		return Common::POINT_CLAMP;
	} else if (_stricmp(inputString.c_str(), "point_wrap") == 0) {
		return Common::POINT_WRAP;
	} else if (_stricmp(inputString.c_str(), "anisotropic_wrap") == 0) {
		return Common::ANISOTROPIC_WRAP;
	} else {
		return defaultType;
	}
}

void CreateDefaultJsonFile(filepath filePath) {
	// Import the model file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, 0u);

	// Start creating the json file
	Json::Value root;
	root["GenNormals"] = true;
	root["CalcTangents"] = true;
	root["VertexBufferUsage"] = "immutable";
	root["IndexBufferUsage"] = "immutable";
	root["MaterialDefinitions"] = Json::arrayValue;

	for (uint i = 0; i < scene->mNumMaterials; ++i) {
		aiMaterial *material = scene->mMaterials[i];
		aiString string;

		Json::Value newMaterialDefinition(Json::objectValue);
		material->Get(AI_MATKEY_NAME, string);
		newMaterialDefinition["MaterialName"] = string.C_Str();
		newMaterialDefinition["HMATFilePath"] = "default.hmat.hlsli";
		newMaterialDefinition["TextureDefinitions"] = Json::arrayValue;

		

		const static aiTextureType types[13] = {aiTextureType_NONE, aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_AMBIENT, aiTextureType_EMISSIVE, aiTextureType_HEIGHT, aiTextureType_NORMALS,
		                                        aiTextureType_SHININESS, aiTextureType_OPACITY, aiTextureType_DISPLACEMENT, aiTextureType_LIGHTMAP, aiTextureType_REFLECTION, aiTextureType_UNKNOWN};
		
		for (uint j = 0; j < 13; ++j) {
			for (uint k = 0; k < material->GetTextureCount(types[j]); ++k) {
				material->GetTexture(types[j], k, &string);
				Json::Value textureDefinition(Json::objectValue);
				textureDefinition["FilePath"] = string.C_Str();
				textureDefinition["Sampler"] = "linear_wrap";

				newMaterialDefinition["TextureDefinitions"].append(textureDefinition);
			}
		}
		
		root["MaterialDefinitions"].append(newMaterialDefinition);
	}

	// Write everything to file
	filePath.replace_extension("hmf.json");

	std::ofstream fout(filePath);
	fout << root << std::endl;

	// Cleanup
	fout.flush();
	fout.close();
}

} // End of namespace ObjHmfConverter
