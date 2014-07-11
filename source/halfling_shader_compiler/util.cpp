/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "halfling_shader_compiler/util.h"

#include "common/string_util.h"


namespace HalflingShaderCompiler {

ShaderType ParseStringToShaderType(const std::string &input) {
	if (_stricmp(input.c_str(), "vertex") == 0) {
		return VERTEX_SHADER;
	} else if (_stricmp(input.c_str(), "pixel") == 0) {
		return PIXEL_SHADER;
	} else if (_stricmp(input.c_str(), "geometry") == 0) {
		return GEOMETRY_SHADER;
	} else if (_stricmp(input.c_str(), "hull") == 0) {
		return HULL_SHADER;
	} else if (_stricmp(input.c_str(), "domain") == 0) {
		return DOMAIN_SHADER;
	} else if (_stricmp(input.c_str(), "compute") == 0) {
		return COMPUTE_SHADER;
	} else {
		return UNDEFINED_SHADER;
	}
}

ShaderModel ParseStringToShaderModel(const std::string &input) {
	if (_stricmp(input.c_str(), "2.0") == 0) {
		return SM_2_0;
	} else if (_stricmp(input.c_str(), "3.0") == 0) {
		return SM_3_0;
	} else if (_stricmp(input.c_str(), "4.0") == 0) {
		return SM_4_0;
	} else if (_stricmp(input.c_str(), "4.1") == 0) {
		return SM_4_1;
	} else {
		return SM_5_0;
	}
}

std::string ShaderParamsToCommandLineArg(ShaderType shaderType, ShaderModel shaderModel) {
	std::string output;
	switch (shaderType) {
	case VERTEX_SHADER:
		output += "/T\"vs";
		break;
	case PIXEL_SHADER:
		output += "/T\"ps";
		break;
	case GEOMETRY_SHADER:
		output += "/T\"gs";
		break;
	case HULL_SHADER:
		output += "/T\"hs";
		break;
	case DOMAIN_SHADER:
		output += "/T\"ds";
		break;
	case COMPUTE_SHADER:
		output += "/T\"cs";
		break;
	}

	switch (shaderModel) {
	case SM_2_0:
		output += "_2_0\" ";
		break;
	case SM_3_0:
		output += "_3_0\" ";
		break;
	case SM_4_0:
		output += "_4_0\" ";
		break;
	case SM_4_1:
		output += "_4_1\" ";
		break;
	case SM_5_0:
		output += "_5_0\" ";
		break;
	}

	return output;
}

} // End of namespace HalflingShaderCompiler