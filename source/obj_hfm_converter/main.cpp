/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/typedefs.h"

#include "common/console_progress_bar.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



int main(int argc, char *argv[]) {
	rlutil::cls();
	std::cout << "Converting..." << std::endl;
	
	Common::ConsoleProgressBar progressBar;
	progressBar.SetVerticalOffset(3);

	// Import the obj file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("sponza.obj", aiProcess_MakeLeftHanded | 
	                                                       aiProcess_Triangulate |
	                                                       aiProcess_JoinIdenticalVertices |
	                                                       aiProcess_GenSmoothNormals |
	                                                       aiProcess_ValidateDataStructure |
	                                                       aiProcess_ImproveCacheLocality |
	                                                       aiProcess_RemoveRedundantMaterials |
	                                                       aiProcess_FindInvalidData |
	                                                       aiProcess_OptimizeMeshes |
	                                                       aiProcess_OptimizeGraph);

	// If the import failed, report it
	if (!scene) {
		std::cout << importer.GetErrorString();
		return 1;
	}
	
	std::cout << "Done Importing" << std::endl;

	std::cout << "Exporting..." << std::endl;

	Assimp::Exporter exporter;
	exporter.Export(scene, "obj", "output.obj",  aiProcess_MakeLeftHanded);

	std::cout << "Finished. Press any key to continue..." << std::endl;

	rlutil::anykey();
}