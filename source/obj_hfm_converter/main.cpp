/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/typedefs.h"

#include "common/halfling_model_file.h"

#define DINI_MAX_LINE 1000
#include "INIReader.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DirectXMath.h>

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>


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
		  IndexBufferUsage(D3D11_USAGE_IMMUTABLE) {
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
};

/**
 * Tries to parse a string into a D3D11_USAGE
 * If the parse fails, the default return is D3D11_USAGE_IMMUTABLE
 * 
 * @param inputString    The string to parse into a usage
 * @return               The usage
 */
D3D11_USAGE ParseUsageFromString(std::string &inputString);
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

/**
 * Converts a given model input into a HalflingModelFile format
 */
int main(int argc, char *argv[]) {
	// Check the number of parameters
    if (argc < 2) {
        // Tell the user how to run the program
        std::cerr << "Usage: OBJ-HMFConverter.exe -f <input filePath>" << std::endl << std::endl <<
		             "Optional parameters:" << std::endl <<
		             "    -i <ini filePath>" << std::endl << 
		             "    -o <output filePath>" << std::endl << std::endl <<
					 "Other Usage:" << std::endl << std::endl <<
					 "OBJ_HMFConverter.exe -c <iniOutput filePath>" << std::endl <<
					 "    to generate an ini file with default values" << std::endl;
        return 1;
    }
	
	std::tr2::sys::path basePath(argv[0]);
	std::tr2::sys::path baseDirectory(basePath.parent_path());
	if (!basePath.has_parent_path()) {
		baseDirectory = std::tr2::sys::current_path<std::tr2::sys::path>();
	}
	
	std::tr2::sys::path inputPath;
	std::tr2::sys::path outputPath;
	std::string iniFilePath;

	// Parse the command line arguments
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-c") == 0) {
			CreateDefaultIniFile(argv[++i]);
			return 0;
		} else if (strcmp(argv[i], "-f") == 0) {
			inputPath = argv[++i];
		} else if (strcmp(argv[i], "-i") == 0) {
			iniFilePath = argv[++i];
		} else if (strcmp(argv[i], "-o") == 0) {
			outputPath = argv[++i];
		}
	}

	// If the input path doesn't exist, tell the user how to use the program
	if (inputPath.empty()) {
        std::cerr << "Usage: OBJ-HMFConverter.exe -f <input filePath>" << std::endl << std::endl <<
		             "Optional parameters:" << std::endl <<
		             "    -i <ini filePath>" << std::endl << 
		             "    -o <output filePath>" << std::endl << std::endl <<
					 "Other Usage:" << std::endl << std::endl <<
					 "OBJ_HMFConverter.exe -c <iniOutput filePath>" << std::endl <<
					 "    to generate an ini file with default values" << std::endl;
        return 1;
	}

	std::tr2::sys::path inputDirectory(inputPath.parent_path());
	if (!inputPath.has_parent_path()) {
		inputDirectory = std::tr2::sys::current_path<std::tr2::sys::path>();
	}

	// If the output path doesn't exist, just use the input path with the .hmf extension
	if (outputPath.empty()) {
		outputPath = inputPath;
		outputPath.replace_extension("hmf");
	}

	std::tr2::sys::path outputDirectory(outputPath.parent_path());
	if (!outputPath.has_parent_path()) {
		outputDirectory = std::tr2::sys::current_path<std::tr2::sys::path>();
	}

	// Set default values
	ImporterIniFile iniFile;

	// Process the ini file
	if (!iniFilePath.empty()) {
		std::cout << "Parsing ini file... ";
		
		INIReader reader(iniFilePath);
		if (reader.ParseError() != 0) {
			std::cout << "Ini file parse error" << std::endl;
			return 1;
		}

		iniFile.GenNormals = reader.GetBoolean("post-processing", "gennormals", iniFile.GenNormals);
		iniFile.CalcTangents = reader.GetBoolean("post-processing", "calctangents", iniFile.CalcTangents);

		iniFile.UseMaterialAmbientColor = reader.GetBoolean("materialpropertyoverrides", "ambientcolor", iniFile.UseMaterialAmbientColor);
		iniFile.UseMaterialDiffuseColor = reader.GetBoolean("materialpropertyoverrides", "diffusecolor", iniFile.UseMaterialDiffuseColor);
		iniFile.UseMaterialSpecColor = reader.GetBoolean("materialpropertyoverrides", "speccolor", iniFile.UseMaterialSpecColor);
		iniFile.UseMaterialOpacity = reader.GetBoolean("materialpropertyoverrides", "opacity", iniFile.UseMaterialOpacity);
		iniFile.UseMaterialSpecPower = reader.GetBoolean("materialpropertyoverrides", "specpower", iniFile.UseMaterialSpecPower);
		iniFile.UseMaterialSpecIntensity = reader.GetBoolean("materialpropertyoverrides", "specintensity", iniFile.UseMaterialSpecIntensity);

		iniFile.UseDiffuseColorMap = reader.GetBoolean("textureoverrides", "diffusecolormap", iniFile.UseDiffuseColorMap);
		iniFile.UseNormalMap = reader.GetBoolean("textureoverrides", "normalmap", iniFile.UseNormalMap);
		iniFile.UseDisplacementMap = reader.GetBoolean("textureoverrides", "displacementmap", iniFile.UseDisplacementMap);
		iniFile.UseAlphaMap = reader.GetBoolean("textureoverrides", "alphamap", iniFile.UseAlphaMap);
		iniFile.UseSpecColorMap = reader.GetBoolean("textureoverrides", "speccolormap", iniFile.UseSpecColorMap);
		iniFile.UseSpecPowerMap = reader.GetBoolean("textureoverrides", "specpowermap", iniFile.UseSpecPowerMap);

		iniFile.VertexBufferUsage = ParseUsageFromString(reader.Get("bufferdesc", "vertexbufferusage", "immutable"));
		iniFile.IndexBufferUsage = ParseUsageFromString(reader.Get("bufferdesc", "indexbufferusage", "immutable"));

		std::cout << "Done" << std::endl;
	}

	uint postProcessingFlags = aiProcess_ConvertToLeftHanded | 
	                           aiProcess_Triangulate |
	                           aiProcess_JoinIdenticalVertices |
	                           aiProcess_ValidateDataStructure |
	                           aiProcess_ImproveCacheLocality |
	                           aiProcess_RemoveRedundantMaterials |
	                           aiProcess_FindInvalidData |
	                           aiProcess_OptimizeMeshes |
	                           aiProcess_OptimizeGraph;

	if (iniFile.GenNormals) {
		postProcessingFlags |= aiProcess_GenSmoothNormals;
	}


	std::cout << "Importing model... ";
	
	// Import the model file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inputPath, postProcessingFlags);

	// Generate tangents
	if (iniFile.CalcTangents) {
		scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
	}

	// If the import failed, report it
	if (!scene) {
		std::cout << importer.GetErrorString();
		return 1;
	}

	std::cout << "Done" << std::endl << "Start Processing... ";

	// Extract the data from the assimp scene
	std::vector<Vertex> vertices;
	std::vector<uint> indices;
	std::vector<Common::HalflingModelFile::Subset> subsets;
	std::vector<std::string> stringTable;

	for (uint i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh *mesh = scene->mMeshes[i];

		Common::HalflingModelFile::Subset subset;
		subset.VertexCount = mesh->mNumVertices;
		subset.VertexStart = vertices.size();
		subset.IndexStart = indices.size();
		subset.IndexCount = mesh->mNumFaces * 3;

		DirectX::XMVECTOR AABB_min = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR AABB_max = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		for (uint j = 0; j < mesh->mNumVertices; ++j) {
			Vertex vertex;
			vertex.pos = DirectX::XMFLOAT3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
			vertex.normal = mesh->HasNormals() ? DirectX::XMFLOAT3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z) : DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertex.texCoord = mesh->HasTextureCoords(0) ? DirectX::XMFLOAT2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y) : DirectX::XMFLOAT2(0.0f, 0.0f);
			vertex.tangent = mesh->HasTangentsAndBitangents() ? DirectX::XMFLOAT3(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z) : DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			
			AABB_min = DirectX::XMVectorMin(AABB_min, DirectX::XMLoadFloat3(&vertex.pos));
			AABB_max = DirectX::XMVectorMax(AABB_max, DirectX::XMLoadFloat3(&vertex.pos));

			vertices.push_back(vertex);
		}

		DirectX::XMStoreFloat3(&subset.AABB_min, AABB_min);
		DirectX::XMStoreFloat3(&subset.AABB_max, AABB_max);

		for (uint j = 0; j < mesh->mNumFaces; ++j) {
			for (uint k = 0; k < mesh->mFaces[j].mNumIndices; ++k) {
				indices.push_back(mesh->mFaces[j].mIndices[k]);
			}
		}

		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

		aiColor3D color;
		float value;
		aiString string;

		if (iniFile.UseMaterialAmbientColor && material->Get(AI_MATKEY_COLOR_AMBIENT, color) == aiReturn_SUCCESS) {
			subset.MatAmbientColor = DirectX::XMFLOAT3(color.r, color.g, color.b);
		}
		if (iniFile.UseMaterialDiffuseColor && material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS) {
			subset.MatDiffuseColor = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f);
		}
		if (iniFile.UseMaterialSpecColor && material->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS) {
			subset.MatSpecColor = DirectX::XMFLOAT3(color.r, color.g, color.b);
		}
		if (iniFile.UseMaterialOpacity && material->Get(AI_MATKEY_OPACITY, value) == aiReturn_SUCCESS) {
			subset.MatDiffuseColor.w = value;
		}
		if (iniFile.UseMaterialSpecPower && material->Get(AI_MATKEY_SHININESS, value) == aiReturn_SUCCESS) {
			subset.MatSpecPower = value;
		}
		if (iniFile.UseMaterialSpecIntensity && material->Get(AI_MATKEY_SHININESS_STRENGTH, value) == aiReturn_SUCCESS) {
			subset.MatSpecIntensity = value;
		}

		if (iniFile.UseDiffuseColorMap && material->GetTexture(aiTextureType_DIFFUSE, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.DiffuseColorMapIndex = stringTable.size();
			// Guarantee it's a dds file
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseNormalMap && material->GetTexture(aiTextureType_NORMALS, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.NormalMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseDisplacementMap && material->GetTexture(aiTextureType_HEIGHT, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.DisplacementMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseAlphaMap && material->GetTexture(aiTextureType_OPACITY, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.AlphaMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseSpecColorMap && material->GetTexture(aiTextureType_SPECULAR, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.SpecColorMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}
		if (iniFile.UseSpecPowerMap && material->GetTexture(aiTextureType_SHININESS, 0, &string, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			subset.SpecPowerMapIndex = stringTable.size();
			stringTable.push_back(ConvertToDDS(string.data, baseDirectory, inputDirectory, outputDirectory));
		}

		subsets.push_back(subset);
	}

	std::cout << "Done" << std::endl << "Writing to file... ";

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(D3D11_BUFFER_DESC));
	vbd.Usage = iniFile.VertexBufferUsage;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(D3D11_BUFFER_DESC));
	ibd.Usage = iniFile.IndexBufferUsage;
	ibd.ByteWidth = sizeof(uint)* indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	std::string outputPathStr(outputPath.file_string());
	std::wstring wideString(outputPathStr.begin(), outputPathStr.end());
	Common::HalflingModelFile::Write(wideString.c_str(), vertices.size(), indices.size(), &vbd, &ibd, nullptr, &vertices[0], &indices[0], nullptr, subsets, stringTable);

	std::cout << "Done" << std::endl << "Finished" << std::endl;

	return 0;
}

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
	if (stricmp(relativePath.extension().c_str(), "dds") == 0) {
		copy_file(inputFilePath, outputFilePath);
	}

	// Otherwise, convert the file to DDS
	std::stringstream call;
	call << baseDirectory.file_string() << "\\texconv.exe -ft dds -o " << outputDirectory.file_string() << " " << outputFilePath.file_string() << " > NUL";
	std::system(call.str().c_str());

	return relativeDDSPath;
}

D3D11_USAGE ParseUsageFromString(std::string &inputString) {
	if (inputString.compare("default") == 0) {
		return D3D11_USAGE_DEFAULT;
	} else if (inputString.compare("dynamic") == 0) {
		return D3D11_USAGE_DYNAMIC;
	} else if (inputString.compare("staging") == 0) {
		return D3D11_USAGE_STAGING;
	} else {
		return D3D11_USAGE_IMMUTABLE;
	}
}

void CreateDefaultIniFile(const char *filePath) {
	std::ofstream fout(filePath);

	fout << "[Post-Processing]\n" <<
	        "; If normals already exist, setting GenNormals to true will do nothing\n" <<
	        "GenNormals = true\n" <<
	        "; If tangents already exist, setting GenNormals to true will do nothing\n" <<
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
	        "[BufferDesc]\n" <<
	        "; Usages can be 'default', 'immutable', 'dynamic', or 'staging'\n" <<
	        "; In the case of a mis-spelling, immutable is assumed\n" <<
	        "VertexBufferUsage = immutable\n" <<
	        "IndexBufferUsage = immutable\n";

	fout.flush();
	fout.close();
}


