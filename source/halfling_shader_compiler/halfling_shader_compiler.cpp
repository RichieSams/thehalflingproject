/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "halfling_shader_compiler/halfling_shader_compiler.h"

#include "halfling_shader_compiler/util.h"

#include "common/file_io_util.h"
#include "common/memory_stream.h"

#include <json/reader.h>
#include <json/json.h>

#include <iostream>
#include <vector>
#include <cassert>


using filepath = std::tr2::sys::path;

namespace HalflingShaderCompiler {

bool CompileFiles(filepath jsonFilePath) {
	filepath jsonDirectory(jsonFilePath.parent_path());
	if (!jsonFilePath.has_parent_path()) {
		jsonDirectory = std::tr2::sys::current_path<filepath>();
	}

	// Read the entire file into memory
	DWORD bytesRead;
	std::string str(jsonFilePath);
	std::wstring wideStr(str.begin(), str.end());
	char *fileBuffer = Common::ReadWholeFile(wideStr.c_str(), &bytesRead);

	// TODO: Add error handling
	if (fileBuffer == nullptr) {
		std::cerr << "JSON file could not be opened:\n\t\"" << jsonFilePath << "\"" << std::endl;
		return true;
	}

	Common::MemoryInputStream fin(fileBuffer, bytesRead);

	Json::Reader reader;
	Json::Value root;
	reader.parse(fin, root, false);

	// Clean-up the file buffer
	delete[] fileBuffer;

	filepath fxcPath(ConvertFilePathMacros(root["FXCPath"].asString()));

	filepath outputDirectory(ConvertFilePathMacros(root["OutputDirectory"].asString()));
	if (!outputDirectory.is_complete()) {
		outputDirectory = jsonDirectory / outputDirectory;
	}
	std::tr2::sys::create_directories(outputDirectory);

	Json::Value additionalIncludeDirectories(root["AdditionalIncludeDirectories"]);
	std::vector<std::string> includeDirectories;
	for (uint i = 0; i < additionalIncludeDirectories.size(); ++i) {
		includeDirectories.push_back(ConvertFilePathMacros(additionalIncludeDirectories[i].asString()));
	}


	Json::Value shaders = root["ShadersToCompile"];
	for (uint i = 0; i < shaders.size(); ++i) {
		std::string inputFilePath(shaders[i]["InputFilePath"].asString());

		// Users don't have to supply and output directory
		// Default to the same name as the input, but with a .cso extension instead
		std::string outputFilePath(shaders[i]["OutputFilePath"].asString());
		if (shaders[i]["OutputFilePath"].isNull()) {
			filepath outputPath(inputFilePath);
			outputPath.replace_extension("cso");
			outputFilePath = outputPath.file_string();
		}

		// Entry point name
		std::string entryPointName(shaders[i]["EntryPointName"].asString());

		// Shader type
		ShaderType shaderType(ParseStringToShaderType(shaders[i]["ShaderType"].asString()));
		if (shaderType == UNDEFINED_SHADER) {
			std::cerr << "Error - Undefined shader type \"" << shaders[i]["ShaderType"].asString() << "\"\n\tFor input file: \"" << inputFilePath << "\"\n\tShader number: " << i;
			continue;
		}

		// Shader model
		ShaderModel shaderModel(ParseStringToShaderModel(shaders[i].get("ShaderModel", "5.0").asString()));

		// Parse the includes
		// Then concatenate them all together, adding the necessary '#include '
		Json::Value additionalIncludes = shaders[i]["AdditionalIncludes"];
		std::vector<std::string> includes;
		for (uint j = 0; j < additionalIncludes.size(); ++j) {
			includes.push_back(additionalIncludes[j].asString());
		}

		std::stringstream ss;
		for (uint j = 0; j < includes.size(); ++j) {
			ss << "#include \"" << includes[j] << "\"\r\n";
		}
		// Guarantee contiguous memory
		std::string concatenatedIncludes = ss.str();

		// Parse off the pre-processor defines as-is
		Json::Value preProcessorDefines = shaders[i]["Pre-ProcessorDefines"];
		std::vector<std::string> defines;
		for (uint j = 0; j < preProcessorDefines.size(); ++j) {
			defines.push_back(preProcessorDefines[j].asString());
		}

		// We need to create a temp file so we can add the #includes
		// Since fxc.exe expects everything in a file
		HANDLE hFile = CreateFile(L"tmp.hlsl",
		                          FILE_APPEND_DATA,
		                          0,
		                          0,
		                          CREATE_ALWAYS,
		                          FILE_ATTRIBUTE_NORMAL,
		                          0);

		// First write the #include data
		DWORD bytesWritten;
		WriteFile(hFile, concatenatedIncludes.c_str(), static_cast<DWORD>(concatenatedIncludes.size()), &bytesWritten, nullptr);

		// Then read the shader file in and immediately append its contents to the end of temp file
		std::wstring wideInputPath(inputFilePath.begin(), inputFilePath.end());
		char *buffer = Common::ReadWholeFile(wideInputPath.c_str(), &bytesRead);
		if (buffer == nullptr) {
			std::cerr << "Could not open shader file:\n\t\"" << inputFilePath << "\"" << std::endl;
			continue;
		}

		WriteFile(hFile, buffer, bytesRead, &bytesWritten, nullptr);

		// Cleanup
		CloseHandle(hFile);

		// Create the command to run fxc.exe
		//
		// system() likes to remove beginning and ending quotes
		// Since fxcPath needs to have quotes around it to protect against 
		// spaces, we need to surround the entire command with quotes
		std::stringstream command;
		command << "\"\"" << fxcPath.file_string() << "\" " <<
		           SHADER_DEBUG_FLAGS << 
				   "/E\"" << entryPointName << "\" " <<
		           "/Fo\"" << outputFilePath << "\" " <<
		           ShaderParamsToCommandLineArg(shaderType, shaderModel) <<
		           "/nologo ";

		// Add the directories for fxc to look in for files #include'ed in the shader
		if (additionalIncludeDirectories.size() > 0) {
			// Re-use the previous string stream
			ss.str(std::string());
			ss.clear();

			std::copy(includeDirectories.begin(), includeDirectories.end(), std::ostream_iterator<std::string>(ss, ";"));
			
			// Guarantee contiguous memory
			std::string concatenatedIncludeDirectories = ss.str();

			command << "/I\"" << concatenatedIncludeDirectories << "\" ";
		}

		// Add the pre-processor macros
		// Each one requires its own "/D"
		// As far as I have tested, you can't concatenate them together like you can with include directories
		for (uint j = 0; j < defines.size(); ++j) {
			command << "/D\"" << defines[j] << "\" ";
		}

		// Add the file to be compiled
		// And suppress output (Still allows errors)
		command << "tmp.hlsl > nul \"";

		// And finally run the command
		if (system(command.str().c_str()) != 0) {
			std::cerr << "Compile failed. This is the command that was run:\n\n" << command.str() << std::endl << std::endl;
		}
	}

	// Delete the temp file
	DeleteFile(L"tmp.hlsl");

	return false;
}

} // End of namespace HalflingShaderCompiler
