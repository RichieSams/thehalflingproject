/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/typedefs.h"

#include "common/allocator_16_byte_aligned.h"

#include <string>
#include <vector>
#include <DirectXMath.h>


struct ID3D11Device;
struct ID3D11SamplerState;

namespace Common {

enum TextureSampler {
	LINEAR_CLAMP = 1,
	LINEAR_BORDER = 2,
	LINEAR_WRAP = 3,
	POINT_CLAMP = 4,
	POINT_WRAP = 5,
	ANISOTROPIC_WRAP = 6
};

/**
 * Tries to parse a string into a TextureSampler
 * If the parse fails, the default return is 'defaultType'
 *
 * @param inputString    The string to parse into a sampler type
 * @param defaultType    The value that should be returned if the parse fails
 * @return               The sampler type
 */
TextureSampler ParseSamplerTypeFromString(std::string &inputString, Common::TextureSampler defaultType);

class Model;
class ModelManager;
class TextureManager;
class MaterialShaderManager;
class SamplerStates;

ID3D11SamplerState *GetSamplerStateFromSamplerType(TextureSampler samplerType, SamplerStates *samplerStates);

struct TextureDescription {
	std::wstring FilePath;
	Common::TextureSampler Sampler;
};

struct ModelToLoadMaterial {
	std::wstring HMATFilePath;
	std::vector<TextureDescription> Textures;
};


class ModelToLoad {
protected:
	ModelToLoad(std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *instances)
		: Instances(instances) {
	}

public:
	virtual ~ModelToLoad() {}

public:
	std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *Instances;

public:
	virtual Model *CreateModel(ID3D11Device *device, Common::TextureManager *textureManager, Common::ModelManager *modelManager, Common::MaterialShaderManager *materialShaderManager, Common::SamplerStates *samplerStates) = 0;
};


class FileModelToLoad : public ModelToLoad {
public:
	FileModelToLoad(const std::string &filePath, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *instances)
		: ModelToLoad(instances),
		  m_filePath(filePath.begin(), filePath.end()) {
	}

private:
	std::wstring m_filePath;
	ID3D11Device *m_device;
	Common::TextureManager *m_textureManager;
	Common::ModelManager *m_modelManager;

public:
	Model *CreateModel(ID3D11Device *device, Common::TextureManager *textureManager, Common::ModelManager *modelManager, Common::MaterialShaderManager *materialShaderManager, Common::SamplerStates *samplerStates);
};


class PlaneModelToLoad : public ModelToLoad {
public:
	PlaneModelToLoad(float width, float depth, uint x_subdivisions, uint z_subdivisions, float x_textureTiling, float z_textureTiling, const ModelToLoadMaterial &material, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *instances)
		: ModelToLoad(instances),
		  m_width(width),
		  m_depth(depth),
		  m_x_subdivisions(x_subdivisions),
		  m_z_subdivisions(z_subdivisions),
		  m_x_textureTiling(x_textureTiling),
		  m_z_textureTiling(z_textureTiling),
		  m_material(material) {
	}

private:
	float m_width;
	float m_depth;
	uint m_x_subdivisions;
	uint m_z_subdivisions;
	float m_x_textureTiling;
	float m_z_textureTiling;
	ModelToLoadMaterial m_material;

public:
	Model *CreateModel(ID3D11Device *device, Common::TextureManager *textureManager, Common::ModelManager *modelManager, Common::MaterialShaderManager *materialShaderManager, Common::SamplerStates *samplerStates);
};


class BoxModelToLoad : public ModelToLoad {
public:
	BoxModelToLoad(float width, float depth, float height, const ModelToLoadMaterial &material, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *instances)
		: ModelToLoad(instances),
		  m_width(width),
		  m_depth(depth),
		  m_height(height),
		  m_material(material) {
	}

private:
	float m_width;
	float m_depth;
	float m_height;
	ModelToLoadMaterial m_material;

public:
	Model *CreateModel(ID3D11Device *device, Common::TextureManager *textureManager, Common::ModelManager *modelManager, Common::MaterialShaderManager *materialShaderManager, Common::SamplerStates *samplerStates);
};


class SphereModelToLoad : public ModelToLoad {
public:
	SphereModelToLoad(float radius, uint sliceCount, uint stackCount, const ModelToLoadMaterial &material, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *instances)
		: ModelToLoad(instances),
		  m_radius(radius),
		  m_sliceCount(sliceCount),
		  m_stackCount(stackCount),
		  m_material(material) {
	}

private:
	float m_radius;
	uint m_sliceCount;
	uint m_stackCount;
	ModelToLoadMaterial m_material;

public:
	Model *CreateModel(ID3D11Device *device, Common::TextureManager *textureManager, Common::ModelManager *modelManager, Common::MaterialShaderManager *materialShaderManager, Common::SamplerStates *samplerStates);
};

} // End of namespace Common
