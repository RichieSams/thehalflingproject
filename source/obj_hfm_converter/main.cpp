/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "obj_hfm_converter/hmf_converter.h"
#include "obj_hfm_converter/util.h"

#include <iostream>


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
	std::tr2::sys::path iniFilePath;

	// Parse the command line arguments
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-c") == 0) {
			ObjHmfConverter::CreateDefaultIniFile(argv[++i]);
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

	return ObjHmfConverter::ConvertToHMF(baseDirectory, inputPath, iniFilePath, outputPath) ? 0 : 1;
}
