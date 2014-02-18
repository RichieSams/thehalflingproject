/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "obj_loader_demo/obj_loader_demo.h"

#include "obj_loader_demo/shader_constants.h"

#include "common/geometry_generator.h"
#include "common/math.h"


namespace ObjLoaderDemo {

void LoadScene(std::atomic<bool> *sceneIsLoaded, std::vector<SceneLoaderModel> *sceneModelList);

bool ObjLoaderDemo::Initialize(LPCTSTR mainWndCaption, uint32 screenWidth, uint32 screenHeight, bool fullscreen) {
	if (!Halfling::HalflingEngine::Initialize(mainWndCaption, screenWidth, screenHeight, fullscreen)) {
		return false;
	}

	InitTweakBar();

	m_sceneLoaderThread = std::thread(LoadScene, &m_sceneLoaded, &m_sceneLoaderModels);
	BuildGeometryBuffers();
	CreateLights();

	LoadShaders();
	CreateShaderBuffers();

	// Set the view matrices to identity
	DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
	m_worldViewProj.world = identity;
	m_worldViewProj.view = identity;
	m_worldViewProj.view = identity;

	m_spriteRenderer.Initialize(m_device);
	m_timesNewRoman12Font.Initialize(L"Times New Roman", 12, Common::SpriteFont::Regular, true, m_device);

	m_blendStates.Initialize(m_device);
	m_depthStencilStates.Initialize(m_device);
	m_rasterizerStates.Initialize(m_device);
	m_samplerStates.Initialize(m_device);

	return true;
}

void ObjLoaderDemo::InitTweakBar() {
	int success = TwInit(TW_DIRECT3D11, m_device);

	m_settingsBar = TwNewBar("settings");
	TwDefine(" settings label='Settings' size='300 175' position='480 20' valueswidth=120 movable=true resizable=false fontresizable=false contained=true iconified=true ");

	TwAddVarRW(m_settingsBar, "V-Sync", TwType::TW_TYPE_BOOLCPP, &m_vsync, "");
	TwAddVarRW(m_settingsBar, "Wireframe", TwType::TW_TYPE_BOOLCPP, &m_wireframe, "");
	TwAddVarRW(m_settingsBar, "Animate Lights", TW_TYPE_BOOLCPP, &m_animateLights, "");
	TwAddVarRW(m_settingsBar, "Show light locations", TwType::TW_TYPE_BOOLCPP, &m_showLightLocations, "");
	TwAddVarRW(m_settingsBar, "Show GBuffer parts", TwType::TW_TYPE_BOOLCPP, &m_showGBuffers, "");

	TwEnumVal shaderEV[] = {{ShadingType::Forward, "Forward"}, {ShadingType::NoCullDeferred, "No-cull Deferred"}};
	TwType shaderType = TwDefineEnum("ShaderType", shaderEV, 2);
	TwAddVarRW(m_settingsBar, "Shader Type", shaderType, &m_shadingType, NULL);

	TwAddVarRW(m_settingsBar, "Number of PointLights", TW_TYPE_INT32, &m_numPointLightsToDraw, " min=0 max=500 ");
	TwAddVarRW(m_settingsBar, "Number of SpotLights", TW_TYPE_INT32, &m_numSpotLightsToDraw, " min=0 max=500 ");
}

void LoadScene(std::atomic<bool> *sceneIsLoaded, std::vector<SceneLoaderModel> *sceneModelList) {
	Common::GeometryGenerator::MeshData meshData;
	std::vector<Common::GeometryGenerator::MeshSubset> meshSubsets;
	Common::GeometryGenerator::LoadFromOBJ(L"sponza.obj", &meshData, &meshSubsets);

	uint vertexCount = meshData.Vertices.size();
	uint indexCount = meshData.Indices.size();
	uint subsetCount = meshSubsets.size();

	Vertex *vertices = new Vertex[vertexCount];
	for (uint i = 0; i < vertexCount; ++i) {
		vertices[i].pos = meshData.Vertices[i].Position;
		vertices[i].normal = meshData.Vertices[i].Normal;
		vertices[i].texCoord = meshData.Vertices[i].TexCoord;
	}

	uint *indices = new uint[indexCount];
	for (uint i = 0; i < indexCount; ++i) {
		indices[i] = meshData.Indices[i];
	}

	// Create subsets
	Common::ModelSubset *subsets = new Common::ModelSubset[subsetCount];

	for (uint i = 0; i < subsetCount; ++i) {
		subsets[i].FaceCount = meshSubsets[i].FaceCount;
		subsets[i].FaceStart = meshSubsets[i].FaceStart;
		subsets[i].VertexCount = meshSubsets[i].VertexCount;
		subsets[i].VertexStart = meshSubsets[i].VertexStart;
		subsets[i].Material = Common::BlinnPhongMaterial {DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f),
		                                                  DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
		                                                  DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f)};
		subsets[i].SRV = nullptr;
	}

	sceneModelList->push_back({vertices, indices, subsets, vertexCount, indexCount, subsetCount});
	sceneIsLoaded->store(true, std::memory_order_relaxed);
}

void ObjLoaderDemo::SetupScene() {
	for (auto iter = m_sceneLoaderModels.begin(); iter != m_sceneLoaderModels.end(); ++iter) {
		m_models.push_back(Common::Model<Vertex>());
		Common::Model<Vertex> *model = &m_models.back();

		model->CreateVertexBuffer(m_device, iter->Vertices, iter->VertexCount);
		model->CreateIndexBuffer(m_device, iter->Indices, iter->IndexCount);
		model->CreateSubsets(iter->Subsets, iter->SubsetCount);
	}

	// Cleanup
	m_sceneLoaderModels.clear();
	m_sceneIsSetup = true;
}

void ObjLoaderDemo::BuildGeometryBuffers() {
	Common::GeometryGenerator::MeshData meshData;

	// Create debug sphere
	Common::GeometryGenerator::CreateSphere(1.0f, 10, 10, &meshData);
	uint vertexCount = meshData.Vertices.size();
	uint indexCount = meshData.Indices.size();

	DebugObjectVertex *debugSphereVertices = new DebugObjectVertex[vertexCount];
	for (uint i = 0; i < vertexCount; ++i) {
		debugSphereVertices[i].pos = meshData.Vertices[i].Position;
	}
	m_debugSphere.CreateVertexBuffer(m_device, debugSphereVertices, vertexCount);

	uint *debugSphereIndices = new uint[indexCount];
	for (uint i = 0; i < indexCount; ++i) {
		debugSphereIndices[i] = meshData.Indices[i];
	}
	m_debugSphere.CreateIndexBuffer(m_device, debugSphereIndices, indexCount);
	m_debugSphereNumIndices = indexCount;

	// Create subsets
	Common::ModelSubset *debugSphereSubsets = new Common::ModelSubset[1] {
		{0, vertexCount, 0, indexCount / 3, {{0.0f, 0.0f, 0.0f, 0.0f}}, nullptr}
	};
	m_debugSphere.CreateSubsets(debugSphereSubsets, 1);

	m_debugSphere.CreateInstanceBuffer(m_device, 500);


	meshData.Indices.clear();
	meshData.Vertices.clear();

	// Create debug cone
	Common::GeometryGenerator::CreateCone(0.8029f, 5.0f, 10, &meshData, true); // ~ 60 degrees
	vertexCount = meshData.Vertices.size();
	indexCount = meshData.Indices.size();

	DebugObjectVertex *debugConeVertices = new DebugObjectVertex[vertexCount];
	for (uint i = 0; i < vertexCount; ++i) {
		debugConeVertices[i].pos = meshData.Vertices[i].Position;
	}
	m_debugCone.CreateVertexBuffer(m_device, debugConeVertices, vertexCount);

	uint *debugConeIndices = new uint[indexCount];
	for (uint i = 0; i < indexCount; ++i) {
		debugConeIndices[i] = meshData.Indices[i];
	}
	m_debugCone.CreateIndexBuffer(m_device, debugConeIndices, indexCount);
	m_debugConeNumIndices = indexCount;

	// Create subsets
	Common::ModelSubset *debugConeSubsets = new Common::ModelSubset[1] {
		{0, vertexCount, 0, indexCount / 3, {{0.0f, 0.0f, 0.0f, 0.0f}}, nullptr}
	};
	m_debugCone.CreateSubsets(debugConeSubsets, 1);

	m_debugCone.CreateInstanceBuffer(m_device, 500);
}

float ObjLoaderDemo::GetHillHeight(float x, float z) const {
	return 0.3f * ((z * sinf(0.1f * x)) + (x * cosf(0.1f * z)));
}

DirectX::XMFLOAT3 ObjLoaderDemo::GetHillNormal(float x, float z) const {
	DirectX::XMFLOAT3 normal(
	    -0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
	    1.0f,
	    0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	DirectX::XMVECTOR unitNormal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal));
	DirectX::XMStoreFloat3(&normal, unitNormal);

	return normal;
}

void ObjLoaderDemo::CreateLights() {
	m_directionalLight.Ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_directionalLight.Diffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_directionalLight.Specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_directionalLight.Direction = DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	for (uint i = 0; i < 500; ++i) {
		Common::PointLight pointLight;
		pointLight.Diffuse = pointLight.Specular = DirectX::XMFLOAT4(Common::RandF(), Common::RandF(), Common::RandF(), 1.0f);
		pointLight.AttenuationDistanceUNorm = 0.75;
		pointLight.Range = 25.0f;
		pointLight.Position = DirectX::XMFLOAT3(Common::RandF(-80.0f, 80.0f), Common::RandF(-40.0f, 40.0f), Common::RandF(-80.0f, 80.0f));

		m_pointLights.push_back(pointLight);

		m_pointLightAnimators.emplace_back(DirectX::XMFLOAT3(Common::RandF(-0.006f, 0.006f), Common::RandF(-0.006f, 0.006f), Common::RandF(-0.006f, 0.006f)),
		                                   DirectX::XMFLOAT3(-80.0f, -40.0f, -80.0f),
		                                   DirectX::XMFLOAT3(80.0f, 40.0f, 80.0f));
	}

	m_pointLightBufferNeedsRebuild = true;

	for (uint i = 0; i < 500; ++i) {
		Common::SpotLight spotLight;
		spotLight.Diffuse = spotLight.Specular = DirectX::XMFLOAT4(Common::RandF(), Common::RandF(), Common::RandF(), 1.0f);
		spotLight.AttenuationDistanceUNorm = 0.75;
		spotLight.Range = 35.0f;
		spotLight.Position = DirectX::XMFLOAT3(Common::RandF(-80.0f, 80.0f), Common::RandF(-40.0f, 40.0f), Common::RandF(-80.0f, 80.0f));
		float outerConeAngle = Common::RandF(0.2967f, 0.7854f); // ~ 35 - 90 degrees
		spotLight.CosOuterConeAngle = cos(outerConeAngle);
		spotLight.CosInnerConeAngle = cos(outerConeAngle - 0.17f); // ~ 10 degrees
		spotLight.Direction = DirectX::XMFLOAT3(Common::RandF(), Common::RandF(), Common::RandF());
		// Normalize
		DirectX::XMVECTOR normalizedDirection = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&spotLight.Direction));
		DirectX::XMStoreFloat3(&spotLight.Direction, normalizedDirection);

		m_spotLights.push_back(spotLight);

		m_spotLightAnimators.emplace_back(DirectX::XMFLOAT3(Common::RandF(-0.006f, 0.006f), Common::RandF(-0.006f, 0.006f), Common::RandF(-0.006f, 0.006f)),
		                                  DirectX::XMFLOAT3(Common::RandF(-0.125f, 0.125f), Common::RandF(-0.125f, 0.125f), Common::RandF(-0.125f, 0.125f)),
		                                  DirectX::XMFLOAT3(-80.0f, -40.0f, -80.0f),
		                                  DirectX::XMFLOAT3(80.0f, 40.0f, 80.0f));
	}

	m_spotLightBufferNeedsRebuild = true;
}

void ObjLoaderDemo::LoadShaders() {
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
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

	HR(Common::LoadVertexShader("forward_vs.cso", m_device, &m_forwardVertexShader, nullptr));
	HR(Common::LoadPixelShader("forward_ps.cso", m_device, &m_forwardPixelShader));
	HR(Common::LoadVertexShader("gbuffer_vs.cso", m_device, &m_gbufferVertexShader, &m_gBufferInputLayout, vertexDesc, 3));
	HR(Common::LoadPixelShader("gbuffer_ps.cso", m_device, &m_gbufferPixelShader));
	HR(Common::LoadVertexShader("fullscreen_triangle_vs.cso", m_device, &m_fullscreenTriangleVertexShader, nullptr));
	HR(Common::LoadPixelShader("no_cull_final_gather_ps.cso", m_device, &m_noCullFinalGatherPixelShader));
	HR(Common::LoadVertexShader("debug_object_vs.cso", m_device, &m_debugObjectVertexShader, &m_debugObjectInputLayout, instanceVertexDesc, 6));
	HR(Common::LoadPixelShader("debug_object_ps.cso", m_device, &m_debugObjectPixelShader));
	HR(Common::LoadVertexShader("transformed_fullscreen_triangle_vs.cso", m_device, &m_transformedFullscreenTriangleVertexShader, nullptr));
	HR(Common::LoadPixelShader("render_gbuffers_ps.cso", m_device, &m_renderGbuffersPixelShader));
}

void ObjLoaderDemo::CreateShaderBuffers() {
	D3D11_BUFFER_DESC forwardPixelShaderFrameBufferDesc;
	forwardPixelShaderFrameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	forwardPixelShaderFrameBufferDesc.ByteWidth = Common::CBSize(sizeof(ForwardPixelShaderFrameConstants));
	forwardPixelShaderFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	forwardPixelShaderFrameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	forwardPixelShaderFrameBufferDesc.MiscFlags = 0;
	forwardPixelShaderFrameBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&forwardPixelShaderFrameBufferDesc, NULL, &m_forwardPixelShaderFrameConstantsBuffer);

	D3D11_BUFFER_DESC forwardPixelShaderObjectBufferDesc;
	forwardPixelShaderObjectBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	forwardPixelShaderObjectBufferDesc.ByteWidth = Common::CBSize(sizeof(ForwardPixelShaderObjectConstants));
	forwardPixelShaderObjectBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	forwardPixelShaderObjectBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	forwardPixelShaderObjectBufferDesc.MiscFlags = 0;
	forwardPixelShaderObjectBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&forwardPixelShaderObjectBufferDesc, NULL, &m_forwardPixelShaderObjectConstantsBuffer);

	D3D11_BUFFER_DESC vertexShaderObjectBufferDesc;
	vertexShaderObjectBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexShaderObjectBufferDesc.ByteWidth = Common::CBSize(sizeof(GBufferVertexShaderObjectConstants));
	vertexShaderObjectBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vertexShaderObjectBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexShaderObjectBufferDesc.MiscFlags = 0;
	vertexShaderObjectBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&vertexShaderObjectBufferDesc, NULL, &m_gBufferVertexShaderObjectConstantsBuffer);

	D3D11_BUFFER_DESC pixelShaderFrameBufferDesc;
	pixelShaderFrameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelShaderFrameBufferDesc.ByteWidth = Common::CBSize(sizeof(GBufferPixelShaderObjectConstants));
	pixelShaderFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelShaderFrameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelShaderFrameBufferDesc.MiscFlags = 0;
	pixelShaderFrameBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&pixelShaderFrameBufferDesc, NULL, &m_gBufferPixelShaderObjectConstantsBuffer);

	D3D11_BUFFER_DESC noCullFinalGatherPixelShaderFrameBufferDesc;
	noCullFinalGatherPixelShaderFrameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	noCullFinalGatherPixelShaderFrameBufferDesc.ByteWidth = Common::CBSize(sizeof(NoCullFinalGatherPixelShaderFrameConstants));
	noCullFinalGatherPixelShaderFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	noCullFinalGatherPixelShaderFrameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	noCullFinalGatherPixelShaderFrameBufferDesc.MiscFlags = 0;
	noCullFinalGatherPixelShaderFrameBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&noCullFinalGatherPixelShaderFrameBufferDesc, NULL, &m_noCullFinalGatherPixelShaderConstantsBuffer);

	D3D11_BUFFER_DESC transformedFullScreenTriangleBufferDesc;
	transformedFullScreenTriangleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	transformedFullScreenTriangleBufferDesc.ByteWidth = Common::CBSize(sizeof(TransformedFullScreenTriangleVertexShaderConstants));
	transformedFullScreenTriangleBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transformedFullScreenTriangleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	transformedFullScreenTriangleBufferDesc.MiscFlags = 0;
	transformedFullScreenTriangleBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&transformedFullScreenTriangleBufferDesc, NULL, &m_transformedFullscreenTriangleVertexShaderConstantsBuffer);

	D3D11_BUFFER_DESC renderGBuffersBufferDesc;
	renderGBuffersBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	renderGBuffersBufferDesc.ByteWidth = Common::CBSize(sizeof(RenderGBuffersPixelShaderConstants));
	renderGBuffersBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	renderGBuffersBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	renderGBuffersBufferDesc.MiscFlags = 0;
	renderGBuffersBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&renderGBuffersBufferDesc, NULL, &m_renderGbuffersPixelShaderConstantsBuffer);


	if (m_pointLights.size() > 0) {
		m_pointLightBuffer = new Common::StructuredBuffer<Common::PointLight>(m_device, m_pointLights.size(), D3D11_BIND_SHADER_RESOURCE, true);
	}
	if (m_spotLights.size() > 0) {
		m_spotLightBuffer = new Common::StructuredBuffer<Common::SpotLight>(m_device, m_spotLights.size(), D3D11_BIND_SHADER_RESOURCE, true);
	}

	m_frameMaterialListBuffer = new Common::StructuredBuffer<Common::BlinnPhongMaterial>(m_device, kMaxMaterialsPerFrame, D3D11_BIND_SHADER_RESOURCE, true);

	D3D11_SAMPLER_DESC diffuseSamplerDesc;
	memset(&diffuseSamplerDesc, 0, sizeof(D3D11_SAMPLER_DESC));
	diffuseSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	diffuseSamplerDesc.MaxAnisotropy = 4;
	diffuseSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	diffuseSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	diffuseSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	HR(m_device->CreateSamplerState(&diffuseSamplerDesc, &m_diffuseSampleState));
}

} // End of namespace ObjLoaderDemo
