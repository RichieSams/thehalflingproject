/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include <filesystem>


namespace ObjHmfConverter {

bool ConvertToHMF(std::tr2::sys::path &baseDirectory, std::tr2::sys::path &inputFilePath, std::tr2::sys::path &jsonFilePath, std::tr2::sys::path &outputFilePath);

} // End of namespace ObjHmfConverter
