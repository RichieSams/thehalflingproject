/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include <filesystem>


namespace HalflingShaderCompiler {

bool CompileFiles(std::tr2::sys::path outputDirectory, std::tr2::sys::path jsonFilePath);

} // End of namespace HalflingShaderCompiler
