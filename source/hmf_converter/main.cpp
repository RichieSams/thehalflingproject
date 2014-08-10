/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "hmf_converter/hmf_converter.h"
#include "hmf_converter/util.h"

#include <iostream>


/**
 * Converts a given model input into a HalflingModelFile format
 */
int main(int argc, char *argv[]) {
	// Check the number of parameters
    if (argc < 2) {
        // Tell the user how to run the program
        std::cerr << "Usage: HMFConverter.exe -j <json filePath> [-o <output filePath>] <model filePath>" << std::endl << std::endl <<
					 "Other Usage:" << std::endl << std::endl <<
					 "HMFConverter.exe -c <model filePath>" << std::endl <<
					 "    to generate a json file with default values" << std::endl;
        return 1;
    }
	
	std::tr2::sys::path basePath(argv[0]);
	std::tr2::sys::path baseDirectory(basePath.parent_path());
	if (!basePath.has_parent_path()) {
		baseDirectory = std::tr2::sys::current_path<std::tr2::sys::path>();
	}
	
	std::tr2::sys::path inputPath;
	std::tr2::sys::path outputPath;
	std::tr2::sys::path jsonFilePath;

	// Parse the command line arguments
	for (int i = 1; i < argc - 1; ++i) {
		if (strcmp(argv[i], "-c") == 0) {
			if (++i >= argc) {
				std::cerr << "-c requires an argument";
				return 1;
			}

			ObjHmfConverter::CreateDefaultJsonFile(argv[i]);
			return 0;
		} else if (strcmp(argv[i], "-j") == 0) {
			if (++i >= argc - 1) {
				std::cerr << "-j requires an argument";
				return 1;
			}

			jsonFilePath = argv[i];
		} else if (strcmp(argv[i], "-o") == 0) {
			if (++i >= argc - 1) {
				std::cerr << "-o requires an argument";
				return 1;
			}

			outputPath = argv[i];
		}
	}

	inputPath = argv[argc - 1];

	// If the input path doesn't exist, tell the user how to use the program
	if (inputPath.empty()) {
        std::cerr << "Usage: HMFConverter.exe -j <json filePath> [-o <output filePath>] <model filePath>" << std::endl << std::endl <<
					 "Other Usage:" << std::endl << std::endl <<
					 "OBJ_HMFConverter.exe -c <model filePath>" << std::endl <<
					 "    to generate a json file with default values" << std::endl;
        return 1;
	}

	return ObjHmfConverter::ConvertToHMF(baseDirectory, inputPath, jsonFilePath, outputPath) ? 0 : 1;
}
