/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include <string>


enum ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER,
	GEOMETRY_SHADER,
	HULL_SHADER,
	DOMAIN_SHADER,
	COMPUTE_SHADER,
	UNDEFINED_SHADER
};

enum ShaderModel {
	SM_2_0,
	SM_3_0,
	SM_4_0,
	SM_4_1,
	SM_5_0
};

namespace HalflingShaderCompiler {

ShaderType ParseStringToShaderType(const std::string &input);
ShaderModel ParseStringToShaderModel(const std::string &input);

std::string ShaderParamsToCommandLineArg(ShaderType shaderType, ShaderModel shaderModel);

std::string ConvertFilePathMacros(const std::string &input);

} // End of namespace HalflingShaderCompiler

 
#ifdef _DEBUG
	#define SHADER_DEBUG_FLAGS " /Od /Zi "
#else
	#define SHADER_DEBUG_FLAGS ""
#endif
