/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_HALFLING_MODEL_FILE_H
#define COMMON_HALFLING_MODEL_FILE_H

#include "common/model.h"


namespace Common {

class HalflingModelFile {
private:
	HalflingModelFile();

private:
	enum Flags {
		HAS_INSTANCE_BUFFER = 0x0002,
		HAS_INSTANCE_BUFFER_DATA = 0x0004,
		HAS_STRING_TABLE = 0x0008
	};

	struct Subset {
		uint64 VertexStart;
		uint64 VertexCount;
		uint64 IndexStart;
		uint64 IndexCount;

		float MatAmbientColor[3];
		float MatDiffuseColor[4];
		float MatSpecColor[3];
		float MatSpecIntensity;
		float MatSpecPower;

		uint32 diffuseColorMapIndex;
		uint32 specColorMapIndex;
		uint32 specPowerMapIndex;
		uint32 alphaMapIndex;
		uint32 displacementMapIndex;
		uint32 normalMapIndex;
	};

private:
	static const byte kFileFormatVersion = 1;

public:
	static Common::Model *Load(ID3D11Device *device, ID3D11DeviceContext *context, Common::TextureManager *textureManager, const wchar *filePath);
	
};

} // End of namespace Common

#endif
