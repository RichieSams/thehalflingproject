/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef OBJ_HMF_CONVERTER_HMF_CONVERTER_H
#define OBJ_HMF_CONVERTER_HMF_CONVERTER_H

#include <filesystem>


namespace ObjHmfConverter {

bool ConvertToHMF(std::tr2::sys::path &baseDirectory, std::tr2::sys::path &inputFilePath, std::tr2::sys::path &iniFilePath, std::tr2::sys::path &outputFilePath);

} // End of namespace ObjHmfConverter

#endif