/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_loader_demo/obj_loader_demo.h"

#include "obj_loader_demo/shader_constants.h"

#include "common/geometry_generator.h"
#include "common/math.h"
#include "common/halfling_model_file.h"
#include "common/string_util.h"
#include "common/file_io_util.h"
#include "common/memory_stream.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <json/reader.h>
#include <json/value.h>


namespace ObjLoaderDemo {

void LoadScene(std::atomic<bool> *sceneIsLoaded, 
               ID3D11Device *device, 
               Common::TextureManager *textureManager, Common::ModelManager *modelManager, 
               std::vector<ModelToLoad> *modelsToLoad, 
               std::vector<std::pair<Common::Model *, DirectX::XMMATRIX>, Common::Allocator16ByteAligned<std::pair<Common::Model *, DirectX::XMMATRIX> > > *modelList, 
               std::vector<std::pair<Common::Model *, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *> > *instancedModelList,
			   uint modelInstanceThreshold);

bool ObjLoaderDemo::Initialize(LPCTSTR mainWndCaption, uint32 screenWidth, uint32 screenHeight, bool fullscreen) {
	LoadSceneJson();

	// Initialize the Engine
	if (!Halfling::HalflingEngine::Initialize(mainWndCaption, screenWidth, screenHeight, fullscreen)) {
		return false;
	}

	InitTweakBar();

	// HACK: TextureManager isn't thread safe. It works right now because we can guarantee the
	//       main thread won't access TextureManager. Also LoadScene() is completely serial.
	// TODO: Make TextureManager thread safe
	// HACK: ModelManager isn't thread safe. Same argument as TextureManager
	// TODO: Make ModelManager thread safe
	m_sceneLoaderThread = std::thread(LoadScene, &m_sceneLoaded, m_device, &m_textureManager, &m_modelManager, &m_modelsToLoad, &m_models, &m_instancedModels, m_modelInstanceThreshold);

	BuildGeometryBuffers();

	LoadShaders();
	m_frameMaterialListBuffer = new Common::StructuredBuffer<Common::BlinnPhongMaterial>(m_device, kMaxMaterialsPerFrame, D3D11_BIND_SHADER_RESOURCE, true);

	m_instanceBuffer = new Common::StructuredBuffer<DirectX::XMVECTOR>(m_device, kMaxInstanceVectorsPerFrame, D3D11_BIND_SHADER_RESOURCE, true);

	// Create light buffers
	// This has to be done after the Engine has been Initialized so we have a valid m_device
	if (m_pointLights.size() > 0) {
		m_pointLightBuffer = new Common::StructuredBuffer<Common::PointLight>(m_device, m_pointLights.size(), D3D11_BIND_SHADER_RESOURCE, true);
	}
	if (m_spotLights.size() > 0) {
		m_spotLightBuffer = new Common::StructuredBuffer<Common::SpotLight>(m_device, m_spotLights.size(), D3D11_BIND_SHADER_RESOURCE, true);
	}

	m_spriteRenderer.Initialize(m_device);
	m_timesNewRoman12Font.Initialize(L"Times New Roman", 12, Common::SpriteFont::Regular, true, m_device);
	m_courierNew10Font.Initialize(L"Courier New", 12, Common::SpriteFont::Regular, true, m_device);

	// Initialize the console
	m_console.Initialize(Common::Rect(20, m_clientHeight - 320, m_clientWidth - 20, m_clientHeight - 10), &m_spriteRenderer, &m_courierNew10Font);

	m_blendStates.Initialize(m_device);
	m_depthStencilStates.Initialize(m_device);
	m_rasterizerStates.Initialize(m_device);
	m_samplerStates.Initialize(m_device);

	return true;
}

void ObjLoaderDemo::LoadSceneJson() {
	// Read the entire file into memory
	DWORD bytesRead;
	char *fileBuffer = Common::ReadWholeFile(L"scene.json", &bytesRead);
	
	// TODO: Add error handling
	assert(fileBuffer != NULL);

	Common::MemoryInputStream fin(fileBuffer, bytesRead);
	
	Json::Reader reader;
	Json::Value root;
	reader.parse(fin, root, false);

	m_nearClip = root.get("NearClip", m_nearClip).asSingle();
	m_farClip = root.get("FarClip", m_farClip).asSingle();
	m_sceneScaleFactor = root.get("SceneScaleFactor", 1.0).asSingle();
	m_globalWorldTransform = DirectX::XMMatrixScaling(m_sceneScaleFactor, m_sceneScaleFactor, m_sceneScaleFactor);
	m_modelInstanceThreshold = root.get("ModelInstanceThreshold", m_modelInstanceThreshold).asUInt();

	Json::Value models = root["Models"];
	for (uint i = 0; i < models.size(); ++i) {
		std::string filePath = models[i]["FilePath"].asString();

		Json::Value instances = models[i]["Instances"];
		auto *instanceVector = new std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> >();
		for (uint j = 0; j < instances.size(); ++j) {
			instanceVector->push_back(DirectX::XMMatrixSet(instances[j][0u].asSingle(), instances[j][1u].asSingle(), instances[j][2u].asSingle(), instances[j][3u].asSingle(), 
			                                               instances[j][4u].asSingle(), instances[j][5u].asSingle(), instances[j][6u].asSingle(), instances[j][7u].asSingle(), 
			                                               instances[j][8u].asSingle(), instances[j][9u].asSingle(), instances[j][10u].asSingle(), instances[j][11u].asSingle(), 
			                                               instances[j][12u].asSingle(), instances[j][13u].asSingle(), instances[j][14u].asSingle(), instances[j][15u].asSingle()));
		}

		m_modelsToLoad.emplace_back(filePath, instanceVector);
	}

	Json::Value directionalLight = root["DirectionalLight"];
	if (!directionalLight.isNull()) {
		m_directionalLight.Ambient = DirectX::XMFLOAT4(directionalLight["Ambient"][0u].asSingle(), directionalLight["Ambient"][1u].asSingle(), directionalLight["Ambient"][2u].asSingle(), 1.0f);
		m_directionalLight.Diffuse = DirectX::XMFLOAT4(directionalLight["Diffuse"][0u].asSingle(), directionalLight["Diffuse"][1u].asSingle(), directionalLight["Diffuse"][2u].asSingle(), 1.0f);
		m_directionalLight.Specular = DirectX::XMFLOAT4(directionalLight["Specular"][0u].asSingle(), directionalLight["Specular"][1u].asSingle(), directionalLight["Specular"][2u].asSingle(), 1.0f);
		m_directionalLight.Direction = DirectX::XMFLOAT3(directionalLight["Direction"][0u].asSingle(), directionalLight["Direction"][1u].asSingle(), directionalLight["Direction"][2u].asSingle());
	}

	Json::Value pointLights = root["PointLights"];
	for (uint i = 0; i < pointLights.size(); ++i) {
		
		if (pointLights[i]["NumberOfLights"].isNull()) {
			m_pointLights.emplace_back(DirectX::XMFLOAT4(pointLights[i]["Diffuse"][0u].asSingle(), pointLights[i]["Diffuse"][1u].asSingle(), pointLights[i]["Diffuse"][2u].asSingle(), 1.0f),
			                           DirectX::XMFLOAT4(pointLights[i]["Specular"][0u].asSingle(), pointLights[i]["Specular"][1u].asSingle(), pointLights[i]["Specular"][2u].asSingle(), 1.0f),
			                           DirectX::XMFLOAT3(pointLights[i]["Position"][0u].asSingle(), pointLights[i]["Position"][1u].asSingle(), pointLights[i]["Position"][2u].asSingle()),
			                           pointLights[i]["Range"].asSingle(),
			                           pointLights[i]["AttenuationDistanceUNorm"].asSingle());

			// All three values must exist for a linear velocity to be valid
			if (!pointLights[i]["LinearVelocity"].isNull() && !pointLights[i]["AABB_min"].isNull() && !pointLights[i]["AABB_max"]) {
				m_pointLightAnimators.emplace_back(DirectX::XMFLOAT3(pointLights[i]["LinearVelocity"][0u].asSingle(), pointLights[i]["LinearVelocity"][1u].asSingle(), pointLights[i]["LinearVelocity"][2u].asSingle()),
				                                   DirectX::XMFLOAT3(pointLights[i]["AABB_min"][0u].asSingle(), pointLights[i]["AABB_min"][1u].asSingle(), pointLights[i]["AABB_min"][2u].asSingle()),
				                                   DirectX::XMFLOAT3(pointLights[i]["AABB_max"][0u].asSingle(), pointLights[i]["AABB_max"][1u].asSingle(), pointLights[i]["AABB_max"][2u].asSingle()),
				                                   &m_pointLights,
				                                   m_pointLights.size() - 1);
			}
		} else {
			uint numPointLights = pointLights[i]["NumberOfLights"].asUInt();
			for (uint j = 0; j < numPointLights; ++j) {
				DirectX::XMFLOAT3 AABB_min(pointLights[i]["AABB_min"][0u].asSingle(), pointLights[i]["AABB_min"][1u].asSingle(), pointLights[i]["AABB_min"][2u].asSingle());
				DirectX::XMFLOAT3 AABB_max(pointLights[i]["AABB_max"][0u].asSingle(), pointLights[i]["AABB_max"][1u].asSingle(), pointLights[i]["AABB_max"][2u].asSingle());

				DirectX::XMFLOAT2 rangeRange(pointLights[i]["RangeRange"][0u].asSingle(), pointLights[i]["RangeRange"][1u].asSingle());

				m_pointLights.emplace_back(DirectX::XMFLOAT4(Common::RandF(), Common::RandF(), Common::RandF(), 1.0f),
										   DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
										   DirectX::XMFLOAT3(Common::RandF(AABB_min.x, AABB_max.x), Common::RandF(AABB_min.y, AABB_max.y), Common::RandF(AABB_min.z, AABB_max.z)),
										   Common::RandF(rangeRange.x, rangeRange.y),
										   pointLights[i]["AttenuationDistanceUNorm"].asSingle());

				DirectX::XMFLOAT3 linearVelocityMin(0.0f, 0.0f, 0.0f);
				DirectX::XMFLOAT3 linearVelocityMax(0.0f, 0.0f, 0.0f);

				Json::Value value = pointLights[i];

				if (!pointLights[i]["LinearVelocityMinRange"].isNull()) {
					linearVelocityMin.x = pointLights[i]["LinearVelocityMinRange"][0u].asSingle();
					linearVelocityMin.y = pointLights[i]["LinearVelocityMinRange"][1u].asSingle();
					linearVelocityMin.z = pointLights[i]["LinearVelocityMinRange"][2u].asSingle();
				}
				if (!pointLights[i]["LinearVelocityMaxRange"].isNull()) {
					linearVelocityMax.x = pointLights[i]["LinearVelocityMaxRange"][0u].asSingle();
					linearVelocityMax.y = pointLights[i]["LinearVelocityMaxRange"][1u].asSingle();
					linearVelocityMax.z = pointLights[i]["LinearVelocityMaxRange"][2u].asSingle();
				}
				// Only create an animator if there is non-zero velocity
				if (linearVelocityMin.x != 0.0f || linearVelocityMin.y != 0.0f || linearVelocityMin.z != 0.0f ||
					linearVelocityMax.x != 0.0f || linearVelocityMax.y != 0.0f || linearVelocityMax.z != 0.0f) {
					m_pointLightAnimators.emplace_back(DirectX::XMFLOAT3(Common::RandF(linearVelocityMin.x, linearVelocityMax.x), Common::RandF(linearVelocityMin.y, linearVelocityMax.y), Common::RandF(linearVelocityMin.z, linearVelocityMax.z)),
					                                   AABB_min,
					                                   AABB_max,
					                                   &m_pointLights,
					                                   m_pointLights.size() - 1);
				}
			}
		}
	}

	Json::Value spotLights = root["SpotLights"];
	for (uint i = 0; i < spotLights.size(); ++i) {
		if (spotLights[i]["NumberOfLights"] == Json::Value::null) {
			m_spotLights.emplace_back(DirectX::XMFLOAT4(spotLights[i]["Diffuse"][0u].asSingle(), spotLights[i]["Diffuse"][1u].asSingle(), spotLights[i]["Diffuse"][2u].asSingle(), 1.0f),
			                          DirectX::XMFLOAT4(spotLights[i]["Specular"][0u].asSingle(), spotLights[i]["Specular"][1u].asSingle(), spotLights[i]["Specular"][2u].asSingle(), 1.0f),
			                          DirectX::XMFLOAT3(spotLights[i]["Position"][0u].asSingle(), spotLights[i]["Position"][1u].asSingle(), spotLights[i]["Position"][2u].asSingle()),
			                          spotLights[i]["Range"].asSingle(),
			                          DirectX::XMFLOAT3(spotLights[i]["Direction"][0u].asSingle(), spotLights[i]["Direction"][1u].asSingle(), spotLights[i]["Direction"][2u].asSingle()),
			                          spotLights[i]["AttenuationDistanceUNorm"].asSingle(),
			                          std::cos(spotLights[i]["InnerConeAngle"].asSingle()),
			                          std::cos(spotLights[i]["OuterConeAngle"].asSingle()));

			DirectX::XMFLOAT3 linearVelocity(0.0f, 0.0f, 0.0f);
			DirectX::XMFLOAT3 AABB_min(0.0f, 0.0f, 0.0f);
			DirectX::XMFLOAT3 AABB_max(0.0f, 0.0f, 0.0f);
			DirectX::XMFLOAT3 angularVelocity(0.0f, 0.0f, 0.0f);

			if (!spotLights[i]["LinearVelocity"].isNull() && !pointLights[i]["AABB_min"].isNull() && !pointLights[i]["AABB_max"]) {
				linearVelocity.x = spotLights[i]["LinearVelocity"][0u].asSingle();
				linearVelocity.y = spotLights[i]["LinearVelocity"][1u].asSingle();
				linearVelocity.z = spotLights[i]["LinearVelocity"][2u].asSingle();

				AABB_min.x = spotLights[i]["AABB_min"][0u].asSingle();
				AABB_min.y = spotLights[i]["AABB_min"][1u].asSingle();
				AABB_min.z = spotLights[i]["AABB_min"][2u].asSingle();

				AABB_max.x = spotLights[i]["AABB_max"][0u].asSingle();
				AABB_max.y = spotLights[i]["AABB_max"][1u].asSingle();
				AABB_max.z = spotLights[i]["AABB_max"][2u].asSingle();
			}
			if (!spotLights[i]["AngularVelocity"].isNull()) {
				angularVelocity.x = spotLights[i]["AngularVelocity"][0u].asSingle();
				angularVelocity.y = spotLights[i]["AngularVelocity"][1u].asSingle();
				angularVelocity.z = spotLights[i]["AngularVelocity"][2u].asSingle();
			}

			// Only create an animator if one of the velocities is non-zero
			if (linearVelocity.x != 0.0f || linearVelocity.y != 0.0f || linearVelocity.z != 0.0f || angularVelocity.x != 0.0f || angularVelocity.y != 0.0f || angularVelocity.z != 0.0f) {
				m_spotLightAnimators.emplace_back(linearVelocity,
				                                  AABB_min,
				                                  AABB_max,
				                                  angularVelocity,
				                                  &m_spotLights,
				                                  m_spotLights.size() - 1);
			}
		} else {
			uint numSpotLights = spotLights[i]["NumberOfLights"].asUInt();
			for (uint j = 0; j < numSpotLights; ++j) {
				DirectX::XMFLOAT3 AABB_min(spotLights[i]["AABB_min"][0u].asSingle(), spotLights[i]["AABB_min"][1u].asSingle(), spotLights[i]["AABB_min"][2u].asSingle());
				DirectX::XMFLOAT3 AABB_max(spotLights[i]["AABB_max"][0u].asSingle(), spotLights[i]["AABB_max"][1u].asSingle(), spotLights[i]["AABB_max"][2u].asSingle());

				DirectX::XMFLOAT2 rangeRange(spotLights[i]["RangeRange"][0u].asSingle(), spotLights[i]["RangeRange"][1u].asSingle());
				DirectX::XMFLOAT2 outerAngleRange(spotLights[i]["OuterAngleRange"][0u].asSingle(), spotLights[i]["OuterAngleRange"][1u].asSingle());
				float outerAngle = Common::RandF(outerAngleRange.x, outerAngleRange.y);
				float innerAngle = outerAngle - spotLights[i]["InnerAngleDifference"].asSingle();
				float cosOuterAngle = std::cos(outerAngle);
				float cosInnerAngle = std::cos(innerAngle);

				m_spotLights.emplace_back(DirectX::XMFLOAT4(Common::RandF(), Common::RandF(), Common::RandF(), 1.0f),
				                          DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				                          DirectX::XMFLOAT3(Common::RandF(AABB_min.x, AABB_max.x), Common::RandF(AABB_min.y, AABB_max.y), Common::RandF(AABB_min.z, AABB_max.z)),
				                          Common::RandF(rangeRange.x, rangeRange.y),
				                          DirectX::XMFLOAT3(Common::RandF(-1.0f, 1.0f), Common::RandF(-1.0f, 1.0f), Common::RandF(-1.0f, 1.0f)),
				                          spotLights[i]["AttenuationDistanceUNorm"].asSingle(),
				                          cosInnerAngle,
				                          cosOuterAngle);

				DirectX::XMFLOAT3 linearVelocityMin(0.0f, 0.0f, 0.0f);
				DirectX::XMFLOAT3 linearVelocityMax(0.0f, 0.0f, 0.0f);
				DirectX::XMFLOAT3 angularVelocityMin(0.0f, 0.0f, 0.0f);
				DirectX::XMFLOAT3 angularVelocityMax(0.0f, 0.0f, 0.0f);

				if (!spotLights[i]["LinearVelocityMinRange"].isNull()) {
					linearVelocityMin.x = spotLights[i]["LinearVelocityMinRange"][0u].asSingle();
					linearVelocityMin.y = spotLights[i]["LinearVelocityMinRange"][1u].asSingle();
					linearVelocityMin.z = spotLights[i]["LinearVelocityMinRange"][2u].asSingle();
				}
				if (!spotLights[i]["LinearVelocityMaxRange"].isNull()) {
					linearVelocityMax.x = spotLights[i]["LinearVelocityMaxRange"][0u].asSingle();
					linearVelocityMax.y = spotLights[i]["LinearVelocityMaxRange"][1u].asSingle();
					linearVelocityMax.z = spotLights[i]["LinearVelocityMaxRange"][2u].asSingle();
				}
				if (!spotLights[i]["AngularVelocityMinRange"].isNull()) {
					angularVelocityMin.x = spotLights[i]["AngularVelocityMinRange"][0u].asSingle();
					angularVelocityMin.y = spotLights[i]["AngularVelocityMinRange"][1u].asSingle();
					angularVelocityMin.z = spotLights[i]["AngularVelocityMinRange"][2u].asSingle();
				}
				if (!spotLights[i]["AngularVelocityMaxRange"].isNull()) {
					angularVelocityMax.x = spotLights[i]["AngularVelocityMaxRange"][0u].asSingle();
					angularVelocityMax.y = spotLights[i]["AngularVelocityMaxRange"][1u].asSingle();
					angularVelocityMax.z = spotLights[i]["AngularVelocityMaxRange"][2u].asSingle();
				}
				// Only create an animator if there is non-zero velocity
				if (linearVelocityMin.x != 0.0f || linearVelocityMin.y != 0.0f || linearVelocityMin.z != 0.0f ||
					linearVelocityMax.x != 0.0f || linearVelocityMax.y != 0.0f || linearVelocityMax.z != 0.0f ||
					angularVelocityMin.x != 0.0f || angularVelocityMin.y != 0.0f || angularVelocityMin.z != 0.0f ||
					angularVelocityMax.x != 0.0f || angularVelocityMax.y != 0.0f || angularVelocityMax.z != 0.0f) {
					m_spotLightAnimators.emplace_back(DirectX::XMFLOAT3(Common::RandF(linearVelocityMin.x, linearVelocityMax.x), Common::RandF(linearVelocityMin.y, linearVelocityMax.y), Common::RandF(linearVelocityMin.z, linearVelocityMax.z)),
					                                  AABB_min,
					                                  AABB_max,
					                                  DirectX::XMFLOAT3(Common::RandF(angularVelocityMin.x, angularVelocityMax.x), Common::RandF(angularVelocityMin.y, angularVelocityMax.y), Common::RandF(angularVelocityMin.z, angularVelocityMax.z)),
					                                  &m_spotLights,
					                                  m_spotLights.size() - 1);
				}
			}
		}
	}
}

void ObjLoaderDemo::InitTweakBar() {
	TwInit(TW_DIRECT3D11, m_device);

	m_settingsBar = TwNewBar("settings");
	TwDefine(" settings label='Settings'  valueswidth=120 movable=true resizable=false fontresizable=false contained=true iconified=true ");

	int barSize[2] = {325, 350};
	TwSetParam(m_settingsBar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	int barPos[2] = {(m_clientWidth - barSize[0] - 20), 20};
	TwSetParam(m_settingsBar, NULL, "position", TW_PARAM_INT32, 2, barPos);

	TwAddVarRW(m_settingsBar, "Show Console", TW_TYPE_BOOLCPP, &m_showConsole, "");
	TwAddVarRW(m_settingsBar, "V-Sync", TwType::TW_TYPE_BOOLCPP, &m_vsync, "");
	TwAddVarRW(m_settingsBar, "Wireframe", TwType::TW_TYPE_BOOLCPP, &m_wireframe, "");
	TwAddVarRW(m_settingsBar, "Animate Lights", TW_TYPE_BOOLCPP, &m_animateLights, "");
	TwAddVarRW(m_settingsBar, "Show light locations", TwType::TW_TYPE_BOOLCPP, &m_showLightLocations, "");
	TwAddVarRW(m_settingsBar, "Show GBuffer parts", TwType::TW_TYPE_BOOLCPP, &m_showGBuffers, "");

	TwEnumVal shaderEV[] = {{ShadingType::Forward, "Forward"}, {ShadingType::NoCullDeferred, "No-cull Deferred"}};
	TwType shaderType = TwDefineEnum("ShaderType", shaderEV, 2);
	TwAddVarRW(m_settingsBar, "Shader Type", shaderType, &m_shadingType, NULL);

	TwEnumVal gbufferEV[] = {{GBufferSelector::None, "None"}, {GBufferSelector::Diffuse, "Diffuse"}, {GBufferSelector::Specular, "Specular"}, {GBufferSelector::Normal_Spherical, "Normal Spherical"}, {GBufferSelector::Normal_Cartesian, "Normal Cartesian"}, {GBufferSelector::Depth, "Depth"}};
	TwType gbufferViewType = TwDefineEnum("GBufferViewType", gbufferEV, 6);
	TwAddVarRW(m_settingsBar, "GBuffer View", gbufferViewType, &m_gbufferSelector, NULL);

	TwAddVarRW(m_settingsBar, "Directional Light Ambient", TW_TYPE_COLOR3F, &m_directionalLight.Ambient, "");
	TwAddVarRW(m_settingsBar, "Directional Light Diffuse", TW_TYPE_COLOR3F, &m_directionalLight.Diffuse, "");
	TwAddVarRW(m_settingsBar, "Directional Light Specular", TW_TYPE_COLOR3F, &m_directionalLight.Specular, "");
	TwAddVarRW(m_settingsBar, "Directional Light Direction", TW_TYPE_DIR3F, &m_directionalLight.Direction, "");

	TwAddVarRW(m_settingsBar, "Number of PointLights", TW_TYPE_INT32, &m_numPointLightsToDraw, " min=0 max=1000 ");
	TwAddVarRW(m_settingsBar, "Number of SpotLights", TW_TYPE_INT32, &m_numSpotLightsToDraw, " min=0 max=1000 ");
}

void LoadScene(std::atomic<bool> *sceneIsLoaded, 
               ID3D11Device *device, 
               Common::TextureManager *textureManager, Common::ModelManager *modelManager, 
               std::vector<ModelToLoad> *modelsToLoad, 
               std::vector<std::pair<Common::Model *, DirectX::XMMATRIX>, Common::Allocator16ByteAligned<std::pair<Common::Model *, DirectX::XMMATRIX> > > *modelList, 
               std::vector<std::pair<Common::Model *, std::vector<DirectX::XMMATRIX, Common::Allocator16ByteAligned<DirectX::XMMATRIX> > *> > *instancedModelList,
			   uint modelInstanceThreshold) {
	// WARNING: Do not parallelize this code until you make TextureManager and ModelManager thread safe
	for (auto iter = modelsToLoad->begin(); iter != modelsToLoad->end(); ++iter) {
		std::wstring wideFilePath(iter->FilePath.begin(), iter->FilePath.end());
		Common::Model *newModel = modelManager->GetModel(device, textureManager, wideFilePath.c_str());
		
		if (iter->Instances->size() > modelInstanceThreshold) {
			instancedModelList->emplace_back(newModel, iter->Instances);
		} else {
			modelList->emplace_back(newModel, (*iter->Instances)[0]);
		}
	}

	sceneIsLoaded->store(true, std::memory_order_relaxed);
}

void ObjLoaderDemo::BuildGeometryBuffers() {
	size_t vertexStride = sizeof(DebugObjectVertex);
	size_t instanceStride = sizeof(DebugObjectInstance);
	
	Common::GeometryGenerator::MeshData meshData;
	ZeroMemory(&meshData, sizeof(Common::GeometryGenerator::MeshData));

	// Create debug sphere
	Common::GeometryGenerator::CreateSphere(2.0f, 10, 10, &meshData);
	uint vertexCount = meshData.Vertices.size();
	uint indexCount = meshData.Indices.size();

	DebugObjectVertex *debugSphereVertices = new DebugObjectVertex[vertexCount];
	for (uint i = 0; i < vertexCount; ++i) {
		debugSphereVertices[i].pos = meshData.Vertices[i].Position;
	}
	m_debugSphere.CreateVertexBuffer(m_device, debugSphereVertices, vertexStride, vertexCount);

	uint *debugSphereIndices = new uint[indexCount];
	for (uint i = 0; i < indexCount; ++i) {
		debugSphereIndices[i] = meshData.Indices[i];
	}
	m_debugSphere.CreateIndexBuffer(m_device, debugSphereIndices, indexCount);

	// Create subsets
	Common::ModelSubset *debugSphereSubsets = new Common::ModelSubset[1] {
		{0, vertexCount, 0, indexCount, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {{0.0f, 0.0f, 0.0f, 0.0f}}, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0}
	};
	m_debugSphere.CreateSubsets(debugSphereSubsets, 1);

	m_debugSphere.CreateInstanceBuffer(m_device, instanceStride, 1000);


	meshData.Indices.clear();
	meshData.Vertices.clear();

	// Create debug cone
	Common::GeometryGenerator::CreateCone(0.8029f, 8.0f, 10, &meshData, true); // ~ 60 degrees
	vertexCount = meshData.Vertices.size();
	indexCount = meshData.Indices.size();

	DebugObjectVertex *debugConeVertices = new DebugObjectVertex[vertexCount];
	for (uint i = 0; i < vertexCount; ++i) {
		debugConeVertices[i].pos = meshData.Vertices[i].Position;
	}
	m_debugCone.CreateVertexBuffer(m_device, debugConeVertices, vertexStride, vertexCount);

	uint *debugConeIndices = new uint[indexCount];
	for (uint i = 0; i < indexCount; ++i) {
		debugConeIndices[i] = meshData.Indices[i];
	}
	m_debugCone.CreateIndexBuffer(m_device, debugConeIndices, indexCount);

	// Create subsets
	Common::ModelSubset *debugConeSubsets = new Common::ModelSubset[1] {
		{0, vertexCount, 0, indexCount, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {{0.0f, 0.0f, 0.0f, 0.0f}}, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0}
	};
	m_debugCone.CreateSubsets(debugConeSubsets, 1);

	m_debugCone.CreateInstanceBuffer(m_device, instanceStride, 1000);
}

void ObjLoaderDemo::LoadShaders() {
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3D11_INPUT_ELEMENT_DESC instanceVertexDesc[] = {
		// Data from the vertex buffer
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},

		// Data from the instance buffer
		{"INSTANCE_WORLDVIEWPROJ", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"INSTANCE_WORLDVIEWPROJ", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"INSTANCE_WORLDVIEWPROJ", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"INSTANCE_WORLDVIEWPROJ", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	m_forwardVertexShader = new Common::VertexShader<Common::DefaultShaderConstantType, ForwardVertexShaderObjectConstants>(L"forward_vs.cso", m_device, false, true);
	m_instancedForwardVertexShader = new Common::VertexShader<InstancedForwardVertexShaderFrameConstants, InstancedForwardVertexShaderObjectConstants>(L"instanced_forward_vs.cso", m_device, true, true);
	m_forwardPixelShader = new Common::PixelShader<ForwardPixelShaderFrameConstants, ForwardPixelShaderObjectConstants>(L"forward_ps.cso", m_device, true, true);
	m_gbufferVertexShader = new Common::VertexShader<Common::DefaultShaderConstantType, GBufferVertexShaderObjectConstants>(L"gbuffer_vs.cso", m_device, false, true, &m_defaultInputLayout, vertexDesc, 4);
	m_instancedGBufferVertexShader = new Common::VertexShader<InstancedGBufferVertexShaderFrameConstants, InstancedGBufferVertexShaderObjectConstants>(L"instanced_gbuffer_vs.cso", m_device, true, true);
	m_gbufferPixelShader = new Common::PixelShader<Common::DefaultShaderConstantType, GBufferPixelShaderObjectConstants>(L"gbuffer_ps.cso", m_device, false, true);
	m_fullscreenTriangleVertexShader = new Common::VertexShader<>(L"fullscreen_triangle_vs.cso", m_device, false, false);
	m_noCullFinalGatherPixelShader = new Common::PixelShader<NoCullFinalGatherPixelShaderFrameConstants, Common::DefaultShaderConstantType>(L"no_cull_final_gather_ps.cso", m_device, true, false);
	m_debugObjectVertexShader = new Common::VertexShader<>(L"debug_object_vs.cso", m_device, false, false, &m_debugObjectInputLayout, instanceVertexDesc, 6);
	m_debugObjectPixelShader = new Common::PixelShader<>(L"debug_object_ps.cso", m_device, false, false);
	m_transformedFullscreenTriangleVertexShader = new Common::VertexShader<Common::DefaultShaderConstantType, TransformedFullScreenTriangleVertexShaderConstants>(L"transformed_fullscreen_triangle_vs.cso", m_device, false, true);
	m_renderGbuffersPixelShader = new Common::PixelShader<RenderGBuffersPixelShaderConstants, Common::DefaultShaderConstantType>(L"render_gbuffers_ps.cso", m_device, true, false);
}

} // End of namespace ObjLoaderDemo
