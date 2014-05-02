/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_hfm_converter/util.h"

#include <iostream>


namespace ObjHmfConverter {

std::string ConvertToDDS(const char *filePath, std::tr2::sys::path &baseDirectory, std::tr2::sys::path &rootInputDirectory, std::tr2::sys::path &rootOutputDirectory) {
	std::tr2::sys::path relativePath(filePath);
	std::tr2::sys::path relativeDDSPath(relativePath);
	relativeDDSPath.replace_extension("dds");
	
	// If the file already exists in the output directory, we don't need to do anything
	std::tr2::sys::path outputFilePath(rootOutputDirectory.file_string() + "\\" + relativeDDSPath.file_string());
	if (exists(outputFilePath)) {
		return relativeDDSPath;
	}

	// Guarantee the output directory exists
	std::tr2::sys::path outputDirectory(outputFilePath.parent_path());
	create_directories(outputDirectory);

	// If input is already dds, but doesn't exist in the output directory, just copy the file to the output
	std::tr2::sys::path inputFilePath(rootInputDirectory.file_string() + "\\" + relativePath.file_string());
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

void CreateDefaultIniFile(const char *filePath) {
	std::ofstream fout(filePath);

	// Make sure open was successful
	if (!fout) {
		std::cerr << "File could not be opened";
		return;
	}

	fout << "; Post-processing can automatically calculate the mesh normals and/or tangents"
	        "; If normals already exist, setting GenNormals to true will do nothing\n" <<
	        "; If tangents already exist, setting GenTangents to true will do nothing\n" <<
	        "[Post-Processing]\n" <<
	        "GenNormals = true\n" <<
	        "CalcTangents = true\n" <<
			"\n" <<
	        "; The booleans represent a high level override for these material properties.\n" <<
	        "; If the boolean is false, the property will be set to NULL, even if the property exists within the input model file\n" <<
	        "; If the boolean is true, but the value doesn't exist within the input model file, the property will be set to NULL\n" <<
	        "[MaterialPropertyOverrides]\n" <<
	        "AmbientColor = true\n" <<
	        "DiffuseColor = true\n" <<
	        "SpecColor = true\n" <<
	        "Opacity = true\n" <<
	        "SpecPower = true\n" <<
	        "SpecIntensity = true\n" <<
			"\n" <<
	        "; The booleans represent a high level override for these textures.\n" <<
	        "; If the boolean is false, the texture will be excluded, even if the texture exists within the input model file\n" <<
	        "; If the boolean is true, but the texture doesn't exist within the input model file properties, the texture will still be excluded\n" <<
	        "[TextureOverrides]\n" <<
	        "DiffuseColorMap = true\n" <<
	        "NormalMap = true\n" <<
	        "DisplacementMap = true\n" <<
	        "AlphaMap = true\n" <<
	        "SpecColorMap = true\n" <<
	        "SpecPowerMap = true\n" <<
			"\n" <<
	        "; Usages can be 'default', 'immutable', 'dynamic', or 'staging'\n" <<
	        "; In the case of a mis-spelling, immutable is assumed\n" <<
	        "[BufferDesc]\n" <<
	        "VertexBufferUsage = immutable\n" <<
	        "IndexBufferUsage = immutable\n" <<
			"\n" <<
	        "; TextureMapRedirects allow you to interpret certain textures as other kinds\n" <<
	        "; For example, OBJ doesn't directly support normal maps. Often, you will then see\n" <<
	        "; the normal map in the height (bump) map slot. These options allow you to specify\n" <<
	        "; what texture goes where.\n" <<
	        ";\n" <<
	        "; Any Maps that are excluded are treated as mapping to their own kind\n" <<
	        "; IE. excluding DiffuseColorMap is interpreted as:\n" <<
	        ";       DiffuseColorMap = diffuse\n" <<
	        ";\n" <<
	        "; The available kinds are: 'diffuse', 'normal', 'height', 'displacement', 'alpha', 'specColor', and 'specPower'\n" <<
	        "[TextureMapRedirects]\n" <<
	        "DiffuseColorMap = diffuse\n" <<
	        "NormalMap = normal\n" <<
	        "DisplacementMap = displacement\n" <<
	        "AlphaMap = alpha\n" <<
	        "SpecColorMap = specColor\n" <<
	        "SpecPowerMap = specPower\n";

	fout.flush();
	fout.close();
}

} // End of namespace ObjHmfConverter
