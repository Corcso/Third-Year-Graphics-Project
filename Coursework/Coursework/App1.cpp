// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include "UVSphereMesh.h"
#include "TessPlaneMesh.h"
App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Set width and height
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	// Setup scene shaders
	pbrShader = new PBRShader(renderer->getDevice(), hwnd);
	pbrShader->SetRenderer(renderer);
	pbrShader->SetCurrentCamera(camera);

	heightMapShader = new HeightMapShader(renderer->getDevice(), hwnd);
	heightMapShader->SetRenderer(renderer);
	heightMapShader->SetCurrentCamera(camera);

	wavesShader = new WavesShader(renderer->getDevice(), hwnd);
	wavesShader->SetRenderer(renderer);
	wavesShader->SetCurrentCamera(camera);

	// Initalise scene objects.
	temple.SetRenderer(renderer);
	temple.SetShader(static_cast<BaseShader*>(pbrShader));
	temple.SetMesh(new AModel(renderer->getDevice(), "./res/temple.obj"));
	temple.SetPosition(XMFLOAT3(0, -10.5, -5));

	// Setup terrain plane, use the TessPlaneMesh which is designed for patches of 4 control points (quads)
	groundPlane.SetRenderer(renderer);
	groundPlane.SetShader(static_cast<BaseShader*>(heightMapShader));
	groundPlane.SetMesh(new TessPlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 200));
	groundPlane.SetScale(XMFLOAT3(1, 1, 1));
	groundPlane.SetPosition(XMFLOAT3(-100, -10.5, -100));
	// Load height map textures
	textureMgr->loadTexture(L"IslandHeightMap", L"res/IslandHeight.png"); // (Demes, 2020)
	textureMgr->loadTexture(L"IslandTextureMap", L"res/IslandColor.jpg"); // (Demes, 2020)


	// Setup water 
	water.SetRenderer(renderer);
	water.SetShader(static_cast<BaseShader*>(wavesShader));
	water.SetMesh(new TessPlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 200));
	water.SetScale(XMFLOAT3(1, 1, 1));
	water.SetPosition(XMFLOAT3(-100, -11, -100));

	// Setup light sphere which shows light location
	lightSphere.SetRenderer(renderer);
	lightSphere.SetShader(static_cast<BaseShader*>(pbrShader));
	lightSphere.SetMesh(new SphereMesh(renderer->getDevice(), renderer->getDeviceContext()));
	lightSphere.SetScale(XMFLOAT3(0.2, 0.2, 0.2));

	// Setup PBR Sphere
	PBRSphere.SetRenderer(renderer);
	PBRSphere.SetShader(static_cast<BaseShader*>(pbrShader));
	PBRSphere.SetMesh(new UVSphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 40));
	PBRSphere.SetPosition(XMFLOAT3(0, -9, 10));

	// Setup Sausage roll mesh
	SausageRoll.SetRenderer(renderer);
	SausageRoll.SetShader(static_cast<BaseShader*>(pbrShader));
	SausageRoll.SetMesh(new AModel(renderer->getDevice(), "./res/SausageRoll/model.obj")); // (Demes, 2021 b)
	SausageRoll.SetPosition(XMFLOAT3(0, -9, -5));
	SausageRoll.SetScale(XMFLOAT3(50, 50, 50));
	
	//Setup lights
	lights.push_back(WorldLight()); // Sun
	lights[0].setPosition(1.7, 1.7, 0);
	lights[0].setDirection(1, -1.0, -1);
	lights[0].setAmbientColour(0.1, 0.1, 0.12, 1);
	lights[0].setDiffuseColour(0.8, 0.8, 0.75, 1);
	lights[0].SetLightType(0);
	lights[0].GenerateShadowMatrices();
	lights[0].CreateShadowMaps(renderer);
	lights[0].SetLightPower(0.5);

	lights.push_back(WorldLight()); // Spotlight 1
	lights[1].setPosition(4, -5, -8.5);
	lights[1].setDirection(-0.8, -1.0, 0);
	lights[1].setAmbientColour(0.0, 0.1, 0.12, 1);
	lights[1].setDiffuseColour(0.0, 0.8, 0.75, 1);
	lights[1].SetLightType(2);
	lights[1].GenerateShadowMatrices();
	lights[1].CreateShadowMaps(renderer);
	lights[1].SetLightPower(0.3);
	lights[1].SetSpotlightAngles(23, 25);

	lights.push_back(WorldLight()); // Spotlight 2
	lights[2].setPosition(-3.6, -5, -4.9);
	lights[2].setDirection(0.8, -1.0, 0);
	lights[2].setAmbientColour(0.1, 0.1, 0.0, 1);
	lights[2].setDiffuseColour(0.9, 0.8, 0.0, 1);
	lights[2].SetLightType(2);
	lights[2].GenerateShadowMatrices();
	lights[2].CreateShadowMaps(renderer);
	lights[2].SetLightPower(0.3);
	lights[2].SetSpotlightAngles(15, 25);

	lights.push_back(WorldLight()); // Spotlight 3
	lights[3].setPosition(-0.2, -5, -2.1);
	lights[3].setDirection(0, -1.0, 0);
	lights[3].setAmbientColour(0.1, 0.02, 0.12, 1);
	lights[3].setDiffuseColour(0.5, 0.2, 0.75, 1);
	lights[3].SetLightType(2);
	lights[3].GenerateShadowMatrices();
	lights[3].CreateShadowMaps(renderer);
	lights[3].SetLightPower(0.3);
	lights[3].SetSpotlightAngles(25, 30);

	lights.push_back(WorldLight()); // Swinging Point Light
	lights[4].setPosition(-0.2, -5, -2.1);
	lights[4].setDirection(0, -1.0, 0);
	lights[4].setAmbientColour(0.1, 0.0, 0.0, 1);
	lights[4].setDiffuseColour(0.5, 0.1, 0.1, 1);
	lights[4].SetLightType(1);
	lights[4].GenerateShadowMatrices();
	lights[4].CreateShadowMaps(renderer);
	lights[4].SetLightPower(0.3);

	// Post processing
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	textureShader->SetRenderer(renderer);

	fullScreenOrthoMesh.SetRenderer(renderer);
	fullScreenOrthoMesh.SetShader(static_cast<BaseShader*>(textureShader));
	fullScreenOrthoMesh.SetMesh(new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight));

	fullSceneNoPP = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Depth of field setup
	depthOfFieldPP = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	for (int i = 0; i < DOF_LAYER_COUNT; i++) {
		depthOfFieldLayers[i] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		depthOfFieldLayersHBlur[i] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
		depthOfFieldLayersVBlur[i] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	}
	dofShader = new DepthOfFieldShader(renderer->getDevice(), hwnd);
	dofShader->SetRenderer(renderer);

	// Material Setup for PBR Shader
	// First load all textures
	textureMgr->loadTexture(L"PBRSphereColorMap", L"./res/BrickPBR/Color.png"); // (Demes, 2021 a) 
	textureMgr->loadTexture(L"PBRSphereNormalMap", L"./res/BrickPBR/Normal.png"); // (Demes, 2021 a)
	textureMgr->loadTexture(L"PBRSphereAOMap", L"./res/BrickPBR/AmbientOcclusion.png"); // (Demes, 2021 a)
	textureMgr->loadTexture(L"PBRSphereRoughnessMap", L"./res/BrickPBR/Roughness.png"); // (Demes, 2021 a)

	textureMgr->loadTexture(L"BrushedMetalColorMap", L"./res/BrushedMetal/Color.png"); // (Demes, 2018)
	textureMgr->loadTexture(L"BrushedMetalNormalMap", L"./res/BrushedMetal/Normal.png"); // (Demes, 2018)
	textureMgr->loadTexture(L"BrushedMetalRoughnessMap", L"./res/BrushedMetal/Roughness.png"); // (Demes, 2018)

	textureMgr->loadTexture(L"WoodFloorColorMap", L"./res/WoodFloor/Color.png"); // (Demes, 2022)
	textureMgr->loadTexture(L"WoodFloorNormalMap", L"./res/WoodFloor/Normal.png"); // (Demes, 2022)
	textureMgr->loadTexture(L"WoodFloorAOMap", L"./res/WoodFloor/AmbientOcclusion.png"); // (Demes, 2022)
	textureMgr->loadTexture(L"WoodFloorRoughnessMap", L"./res/WoodFloor/Roughness.png"); // (Demes, 2022)

	textureMgr->loadTexture(L"SausageRollColorMap", L"./res/SausageRoll/Color.png"); // (Demes, 2021 b)
	textureMgr->loadTexture(L"SausageRollNormalMap", L"./res/SausageRoll/Normal.png"); // (Demes, 2021 b)
	textureMgr->loadTexture(L"SausageRollAOMap", L"./res/SausageRoll/AmbientOcclusion.png"); // (Demes, 2021 b)

	// Then setup material defaults
	templeMaterial = PBRShader::PBRMaterial{
		XMFLOAT4(0.98, 0.98, 0.90, 1),
		XMFLOAT4(1, 1, 1, 1),
		64, 0.2, 0, 0, nullptr
	};
	
	GreyBricksMaterial = PBRShader::PBRMaterial{
		XMFLOAT4(1, 1, 1, 1),
		XMFLOAT4(1, 1, 1, 1),
		64, 0.2, 0, (int)PBRShader::TextureFlag::NORMAL | (int)PBRShader::TextureFlag::COLOR| (int)PBRShader::TextureFlag::AO| (int)PBRShader::TextureFlag::ROUGHNESS,
		textureMgr->getTexture(L"PBRSphereColorMap"), textureMgr->getTexture(L"PBRSphereNormalMap"),
		textureMgr->getTexture(L"PBRSphereAOMap"), textureMgr->getTexture(L"PBRSphereRoughnessMap")
	};

	BrushedMetalMaterial = PBRShader::PBRMaterial{
		XMFLOAT4(1, 1, 1, 1),
		XMFLOAT4(1, 1, 1, 1),
		64, 0.2, 1, (int)PBRShader::TextureFlag::NORMAL | (int)PBRShader::TextureFlag::COLOR | (int)PBRShader::TextureFlag::ROUGHNESS,
		textureMgr->getTexture(L"BrushedMetalColorMap"), textureMgr->getTexture(L"BrushedMetalNormalMap"),
		nullptr, textureMgr->getTexture(L"BrushedMetalRoughnessMap")
	};

	WoorFloorMaterial = PBRShader::PBRMaterial{
		XMFLOAT4(1, 1, 1, 1),
		XMFLOAT4(1, 1, 1, 1),
		64, 0.2, 0, (int)PBRShader::TextureFlag::NORMAL | (int)PBRShader::TextureFlag::COLOR | (int)PBRShader::TextureFlag::AO | (int)PBRShader::TextureFlag::ROUGHNESS,
		textureMgr->getTexture(L"WoodFloorColorMap"), textureMgr->getTexture(L"WoodFloorNormalMap"),
		textureMgr->getTexture(L"WoodFloorAOMap"), textureMgr->getTexture(L"WoodFloorRoughnessMap")
	};

	SausageRollMaterial = PBRShader::PBRMaterial{
		XMFLOAT4(1, 1, 1, 1),
		XMFLOAT4(1, 1, 1, 1),
		64, 0.1, 0, (int)PBRShader::TextureFlag::NORMAL | (int)PBRShader::TextureFlag::COLOR | (int)PBRShader::TextureFlag::AO,
		textureMgr->getTexture(L"SausageRollColorMap"), textureMgr->getTexture(L"SausageRollNormalMap"),
		textureMgr->getTexture(L"SausageRollAOMap"), nullptr
	};

	// Setup height map parameters
	amplitude = 20;
	isSmoothingOn = false;

	// Set time elapsed for the waves to 0
	totalTimeElapsed = 0;

	// Set base wave data
	waveData[0] = {
		totalTimeElapsed, 0.186, 0.59, 0.45, XMFLOAT2(0.7, 0.7), 0
	};
	angleOfWave[0] = XMConvertToRadians(45);
	waveData[1] = {
		totalTimeElapsed, 0.44, 0.28, 1, XMFLOAT2(-0.9, -0.42), 1
	};
	angleOfWave[1] = XMConvertToRadians(245);
	waveData[2] = {
		totalTimeElapsed, 0.28, 0.91, 2, XMFLOAT2(0, -1), 1
	};
	angleOfWave[2] = XMConvertToRadians(180);

	// Bloom setup
	bloomShader = new BloomShader(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	bloomShader->SetRenderer(renderer);
	bloomScene = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	bloomSceneAlt = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	// Swing the point light
	if(swingPointLight) lights[4].setPosition(-0.2, -5, sin(totalTimeElapsed) * 3 - 6);

	// Recalculate shadow matrices as light could have moved. 
	for (int lightIndex = 0; lightIndex < lights.size(); ++lightIndex) {
		lights[lightIndex].GenerateShadowMatrices();
	}
   
	// Add onto total time elapsed 
	totalTimeElapsed += timer->getTime();

	// Set wave data time to total time elapsed
	waveData[0].time = totalTimeElapsed;
	waveData[1].time = totalTimeElapsed;
	waveData[2].time = totalTimeElapsed;

	

	return true;
}

bool App1::render()
{
	// Shadow passes first
	shadowDepthPasses();

	// If no DOF do normal scene render
	if (!DOFEnabled) sceneRenderPass();

	// TURN OFF WIREFRAME BEFORE POST PROCESSING
	// This means we can still see the wireframe of the world. Not the ortho quad.
	renderer->setWireframeMode(false);

	// If DOF then do DOF specific render
	if (DOFEnabled) depthOfFieldPass();

	// Bloom post processing
	bloomPass();

	// Final pass to put bloom output on screen. 
	finalPass();

	return true;
}

bool App1::shadowDepthPasses()
{
	// To do loop over all lights
	for (int lightIndex = 0; lightIndex < lights.size(); ++lightIndex) {
		// For all the faces to map on this light
		int facesToMap = (lights[lightIndex].GetLightType() != 0) ? 6 : 1;
		if (lights[lightIndex].GetLightType() != 0) lights[lightIndex].GetTCubeShadowMap()->ClearDSV(renderer->getDeviceContext());
		for (int f = 0; f < facesToMap; ++f) {
			// Set this face's shadow map to be rendered on to 
			if (lights[lightIndex].GetLightType() == 0) lights[lightIndex].GetDirectionalShadowMap()->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
			else lights[lightIndex].GetTCubeShadowMap()->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext(), f);

			// Get lights view and projection matrix
			XMMATRIX lightViewMatrix = lights[lightIndex].GetViewMatrix(f);
			XMMATRIX lightProjMatrix = lights[lightIndex].GetProjMatrix(f);

			// Temp pointer while loop is incomplete. 
			WorldLight* temp = &lights[lightIndex];

			// Set light as camera for PBR shader and draw test sphere
			pbrShader->SetLightAsCamera(&lights[lightIndex], f);
			pbrShader->SetShaderParameters(temple.GetWorldMatrix(), &templeMaterial, lights.data(), lights.size());
			temple.Render();

			// Draw PBR Spheres or sausage roll
			if (!sausageRollReplaceSpheres) {
				PBRSphere.SetPosition(XMFLOAT3(0, -9, -2));
				pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &BrushedMetalMaterial, lights.data(), lights.size());
				PBRSphere.Render();

				PBRSphere.SetPosition(XMFLOAT3(0, -9, -5));
				pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &WoorFloorMaterial, lights.data(), lights.size());
				PBRSphere.Render();

				PBRSphere.SetPosition(XMFLOAT3(0, -9, -8));
				pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &GreyBricksMaterial, lights.data(), lights.size());
				PBRSphere.Render();
			}
			else {
				pbrShader->SetShaderParameters(SausageRoll.GetWorldMatrix(), &SausageRollMaterial, lights.data(), lights.size());
				SausageRoll.Render();
			}
			// Set light as camera for height map shader and draw terrain
			heightMapShader->SetLightAsCamera(&lights[lightIndex], f);
			heightMapShader->SetLightAsCamera();
			HeightMapShader::HeightMapBufferData heightMapSettings{
			amplitude, XMFLOAT2(200, 200), (isSmoothingOn) ? 1 : 0
			};

			// Tessellation will still tessellate at user camera so to cast correct shadows. 
			heightMapShader->SetShaderParameters(groundPlane.GetWorldMatrix(), &heightMapSettings, lights.data(), lights.size(), textureMgr->getTexture(L"IslandHeightMap"), textureMgr->getTexture(L"IslandTextureMap"), terrainTessellationMinAndMaxTesselation, terrainTessellationMinAndMaxDistance);
			groundPlane.Render(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		}
	}
	return true;
}

bool App1::sceneRenderPass()
{
	// Clear the scene. (default blue colour)
	fullSceneNoPP->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);
	fullSceneNoPP->setRenderTarget(renderer->getDeviceContext());

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Render objects

	// Set the camera to be the camera for the PBR Shader
	pbrShader->SetCameraAsCamera();

	// Draw the test sphere
	pbrShader->SetShaderParameters(temple.GetWorldMatrix(), &templeMaterial, lights.data(), lights.size());
	temple.Render();

	// Draw the light sphere
	for (int lightIndex = 0; lightIndex < lights.size(); ++lightIndex) {
		lightSphere.SetPosition(lights[lightIndex].getPosition());
		pbrShader->SetShaderParameters(lightSphere.GetWorldMatrix(), &templeMaterial, lights.data(), lights.size());
		lightSphere.Render();
	}

	// Draw PBR Spheres or sausage roll
	if (!sausageRollReplaceSpheres) {
		PBRSphere.SetPosition(XMFLOAT3(0, -9, -2));
		pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &BrushedMetalMaterial, lights.data(), lights.size());
		PBRSphere.Render();

		PBRSphere.SetPosition(XMFLOAT3(0, -9, -5));
		pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &WoorFloorMaterial, lights.data(), lights.size());
		PBRSphere.Render();

		PBRSphere.SetPosition(XMFLOAT3(0, -9, -8));
		pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &GreyBricksMaterial, lights.data(), lights.size());
		PBRSphere.Render();
	}
	else {
		pbrShader->SetShaderParameters(SausageRoll.GetWorldMatrix(), &SausageRollMaterial, lights.data(), lights.size());
		SausageRoll.Render();
	}
	HeightMapShader::HeightMapBufferData heightMapSettings{
			amplitude, XMFLOAT2(200, 200), (isSmoothingOn) ? 1 : 0
	};

	// Setup height map shader to now use camera and draw terrain
	heightMapShader->SetCameraAsCamera();
	heightMapShader->SetShaderParameters(groundPlane.GetWorldMatrix(), &heightMapSettings, lights.data(), lights.size(), textureMgr->getTexture(L"IslandHeightMap"), textureMgr->getTexture(L"IslandTextureMap"), terrainTessellationMinAndMaxTesselation, terrainTessellationMinAndMaxDistance);
	groundPlane.Render(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	// Draw the water test plane
	renderer->setAlphaBlending(true);
	wavesShader->SetCameraAsCamera();
	wavesShader->SetShaderParameters(water.GetWorldMatrix(), waveData, lights.data(), lights.size(), terrainTessellationMinAndMaxTesselation, terrainTessellationMinAndMaxDistance);
	water.Render(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	renderer->setAlphaBlending(false);

	return true;
}

bool App1::depthOfFieldPass()
{
	// Setup the depth layers based on plane in focus
	float maxDepths[DOF_LAYER_COUNT];
	float minDepths[DOF_LAYER_COUNT];
	float fullDepthRange = 0.009;
	float focusPlaneStep = fullDepthRange / DOF_LAYER_COUNT;
	float currentEdge = focusPlane + fullDepthRange / 2.0f;
	for (int i = 0; i < DOF_LAYER_COUNT; ++i) {
		maxDepths[i] = currentEdge;
		currentEdge -= focusPlaneStep;
		minDepths[i] = currentEdge;
	}
	// Make sure we start at depth 1 and end at 0
	maxDepths[0] = 1;
	minDepths[DOF_LAYER_COUNT - 1] = 0;

	// Array for easy sending of SRVs.
	ID3D11ShaderResourceView* layerSRVs[DOF_LAYER_COUNT];
	// For every layer
	for (int i = 0; i < DOF_LAYER_COUNT; ++i) {
		// ================================
		// Render the scene's render but using the depth map to clip pixels not in the layer
		// ================================
		dofShader->ReadyPart1();
		if(i == 0) depthOfFieldLayers[i]->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);
		else depthOfFieldLayers[i]->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);
		depthOfFieldLayers[i]->setRenderTarget(renderer->getDeviceContext());
		
		// Draw the test sphere
		pbrShader->SetCameraAsCamera();
		pbrShader->SetShaderParameters(temple.GetWorldMatrix(), &templeMaterial, lights.data(), lights.size(), XMFLOAT2(minDepths[i], maxDepths[i]));
		temple.Render();

		// Draw the light sphere
		for (int lightIndex = 0; lightIndex < lights.size(); ++lightIndex) {
			lightSphere.SetPosition(lights[lightIndex].getPosition());
			pbrShader->SetShaderParameters(lightSphere.GetWorldMatrix(), &templeMaterial, lights.data(), lights.size(), XMFLOAT2(minDepths[i], maxDepths[i]));
			lightSphere.Render();
		}

		// Draw PBR Spheres or sausage roll
		if (!sausageRollReplaceSpheres) {
			PBRSphere.SetPosition(XMFLOAT3(0, -9, -2));
			pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &BrushedMetalMaterial, lights.data(), lights.size(), XMFLOAT2(minDepths[i], maxDepths[i]));
			PBRSphere.Render();

			PBRSphere.SetPosition(XMFLOAT3(0, -9, -5));
			pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &WoorFloorMaterial, lights.data(), lights.size(), XMFLOAT2(minDepths[i], maxDepths[i]));
			PBRSphere.Render();

			PBRSphere.SetPosition(XMFLOAT3(0, -9, -8));
			pbrShader->SetShaderParameters(PBRSphere.GetWorldMatrix(), &GreyBricksMaterial, lights.data(), lights.size(), XMFLOAT2(minDepths[i], maxDepths[i]));
			PBRSphere.Render();
		}
		else {
			pbrShader->SetShaderParameters(SausageRoll.GetWorldMatrix(), &SausageRollMaterial, lights.data(), lights.size(), XMFLOAT2(minDepths[i], maxDepths[i]));
			SausageRoll.Render();
		}
		// Draw the water test plane
		renderer->setAlphaBlending(true);
		wavesShader->SetCameraAsCamera();
		wavesShader->SetShaderParameters(water.GetWorldMatrix(), waveData, lights.data(), lights.size(), terrainTessellationMinAndMaxTesselation, terrainTessellationMinAndMaxDistance, XMFLOAT2(minDepths[i], maxDepths[i]));
		water.Render(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		renderer->setAlphaBlending(false);

		HeightMapShader::HeightMapBufferData heightMapSettings{
			amplitude, XMFLOAT2(200, 200), (isSmoothingOn) ? 1 : 0
		};

		// Setup height map shader to now use camera and draw terrain
		heightMapShader->SetCameraAsCamera();
		heightMapShader->SetShaderParameters(groundPlane.GetWorldMatrix(), &heightMapSettings, lights.data(), lights.size(), textureMgr->getTexture(L"IslandHeightMap"), textureMgr->getTexture(L"IslandTextureMap"), terrainTessellationMinAndMaxTesselation, terrainTessellationMinAndMaxDistance, XMFLOAT2(minDepths[i], maxDepths[i]));
		groundPlane.Render(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		// ================================
		
		// Blur the layer, with a gausian blur. Horizontal and vertical are seperated. 
		dofShader->ReadyPart2();

		depthOfFieldLayersHBlur[i]->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);
		depthOfFieldLayersHBlur[i]->setRenderTarget(renderer->getDeviceContext());
		dofShader->SetShaderParametersPart2(depthOfFieldLayers[i]->getShaderResourceView(), depthOfFieldLayers[i]->getDepthShaderResourceView(), screenWidth, screenHeight, maxDepths, minDepths, true, i);
		fullScreenOrthoMesh.SetShader(dofShader);
		fullScreenOrthoMesh.Render();

		depthOfFieldLayersVBlur[i]->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);
		depthOfFieldLayersVBlur[i]->setRenderTarget(renderer->getDeviceContext());
		dofShader->SetShaderParametersPart2(depthOfFieldLayersHBlur[i]->getShaderResourceView(), depthOfFieldLayers[i]->getDepthShaderResourceView(), screenWidth, screenHeight, maxDepths, minDepths, false, i);
		fullScreenOrthoMesh.SetShader(dofShader);
		fullScreenOrthoMesh.Render();

		// Set this layers SRV for easy access
		layerSRVs[i] = depthOfFieldLayersVBlur[i]->getShaderResourceView();
	}
	// Clear the depth of field final output for part 3
	depthOfFieldPP->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);
	depthOfFieldPP->setRenderTarget(renderer->getDeviceContext());

	// Render the scene, combining all the layers
	dofShader->ReadyPart3();
	dofShader->SetShaderParametersPart3(layerSRVs, screenWidth, screenHeight);
	fullScreenOrthoMesh.SetShader(dofShader);
	fullScreenOrthoMesh.Render();

	return true;
}

bool App1::bloomPass()
{
	// Set the full screen ortho mesh up for bloom
	fullScreenOrthoMesh.SetShader(bloomShader);

	// Part 1 : Render only bright parts of scene above threshold. 
	bloomShader->ReadyPart1();

	// Use bloom scene alternate RT first
	bloomSceneAlt->setRenderTarget(renderer->getDeviceContext());
	bloomSceneAlt->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);

	// If DOF, use that RT output, otherwise use non DOF one
	if (DOFEnabled) bloomShader->SetShaderParametersPart1(depthOfFieldPP->getShaderResourceView(), luminocityThreshold);
	else bloomShader->SetShaderParametersPart1(fullSceneNoPP->getShaderResourceView(), luminocityThreshold);
	fullScreenOrthoMesh.Render();

	// Part 2: Blur the bright part, seperate horizontal and vertical
	bloomShader->ReadyPart2();

	// Use bloom scene RT, alrernating the RTs for good memory usage
	bloomScene->setRenderTarget(renderer->getDeviceContext());
	bloomScene->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);

	bloomShader->SetShaderParametersPart2(bloomSceneAlt->getShaderResourceView(), true, blurSize, blurSkip);
	fullScreenOrthoMesh.Render();

	// Alternate RT
	bloomSceneAlt->setRenderTarget(renderer->getDeviceContext());
	bloomSceneAlt->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);

	bloomShader->SetShaderParametersPart2(bloomScene->getShaderResourceView(), false, blurSize, blurSkip);
	fullScreenOrthoMesh.Render();

	// Part 3 : paste bloomy blurred texture ontop of scene render.

	bloomShader->ReadyPart3();

	bloomScene->setRenderTarget(renderer->getDeviceContext());
	bloomScene->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);

	// Pick whichever scene render depending on DOF being used. 
	if (DOFEnabled) bloomShader->SetShaderParametersPart3(depthOfFieldPP->getShaderResourceView(), bloomSceneAlt->getShaderResourceView());
	else bloomShader->SetShaderParametersPart3(fullSceneNoPP->getShaderResourceView(), bloomSceneAlt->getShaderResourceView());
	fullScreenOrthoMesh.Render();

	return true;
}

bool App1::finalPass()
{
	// Set back buffer as our render target
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
	renderer->beginScene(0, 0, 0, 1);

	// Draw bloom output on top
	textureShader->SetShaderParameters(bloomScene->getShaderResourceView(), screenWidth, screenHeight);
	fullScreenOrthoMesh.SetShader(textureShader);
	fullScreenOrthoMesh.Render();
	
	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("Sausage Roll Model", &sausageRollReplaceSpheres);

	// Lights menu
	ImGui::Begin("Lights");
	ImGui::Checkbox("Swing Point Light?", &swingPointLight);
	std::string lightNames[8] = { "Sun", "Spot 1", "Spot 2", "Spot 3", "Swinging Point", "", "", "" };
	for (int lightIndex = 0; lightIndex < lights.size(); ++lightIndex) {
		lights[lightIndex].ShowGuiControls(lightNames[lightIndex].c_str());
	}
	ImGui::End();

	// Materials menu
	ImGui::Begin("Materials");
	PBRShader::DisplayMaterialUI("Temple", &templeMaterial);
	if (!sausageRollReplaceSpheres) {
		PBRShader::DisplayMaterialUI("Bricks Sphere", &GreyBricksMaterial);
		PBRShader::DisplayMaterialUI("Wood Sphere", &WoorFloorMaterial);
		PBRShader::DisplayMaterialUI("Brushed Metal Sphere", &BrushedMetalMaterial);
	}
	else {
		PBRShader::DisplayMaterialUI("Sausage Roll", &SausageRollMaterial);
	}
	ImGui::End();

	// Display tessellation menu
	ImGui::Begin("Tessellation");
	ImGui::SliderFloat2("Min and Max Tessellation", reinterpret_cast<float*>(&terrainTessellationMinAndMaxTesselation), 1, 64);
	ImGui::SliderFloat2("Min and Max Distance", reinterpret_cast<float*>(&terrainTessellationMinAndMaxDistance), 0, 100);
	ImGui::SliderFloat("Height Map Amplitude", &amplitude, 0, 30);
	ImGui::Checkbox("Height Map Smoothing On", &isSmoothingOn);
	ImGui::End();

	// Display Wave menu
	ImGui::Begin("Waves");
	if (ImGui::TreeNode("Wave 1")) {
		ImGui::SliderFloat("Amplitude", &waveData[0].amplitude, 0, 5);
		ImGui::SliderFloat("Frequency", &waveData[0].frequency, 0, 5);
		ImGui::SliderFloat("Speed", &waveData[0].speed, 0, 5);
		ImGui::SliderFloat("Steepness", &waveData[0].steepness, 0, 5);
		ImGui::SliderAngle("Angle", &angleOfWave[0], 0, 360);
		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Wave 2")) {
		ImGui::SliderFloat("Amplitude", &waveData[1].amplitude, 0, 5);
		ImGui::SliderFloat("Frequency", &waveData[1].frequency, 0, 5);
		ImGui::SliderFloat("Speed", &waveData[1].speed, 0, 5);
		ImGui::SliderFloat("Steepness", &waveData[1].steepness, 0, 5);
		ImGui::SliderAngle("Angle", &angleOfWave[1], 0, 360);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Wave 3")) {
		ImGui::SliderFloat("Amplitude", &waveData[2].amplitude, 0, 5);
		ImGui::SliderFloat("Frequency", &waveData[2].frequency, 0, 5);
		ImGui::SliderFloat("Speed", &waveData[2].speed, 0, 5);
		ImGui::SliderFloat("Steepness", &waveData[2].steepness, 0, 5);
		ImGui::SliderAngle("Angle", &angleOfWave[2], 0, 360);
		ImGui::TreePop();
	}
	ImGui::End();

	// Calculate direction of wave based on users angle input
	waveData[0].direction = XMFLOAT2(sin(angleOfWave[0]), cos(angleOfWave[0]));
	waveData[1].direction = XMFLOAT2(sin(angleOfWave[1]), cos(angleOfWave[1]));
	waveData[2].direction = XMFLOAT2(sin(angleOfWave[2]), cos(angleOfWave[2]));

	// Post processing menu
	ImGui::Begin("Post Processing");
	ImGui::Text("Depth Of Field");
	ImGui::Checkbox("DOF Enabled", &DOFEnabled);
	ImGui::SliderFloat("DOF Focus Plane", &focusPlane, 0.98, 0.995);
	ImGui::Text("Bloom");
	ImGui::SliderFloat("Bloom Luminosity Threshold", &luminocityThreshold, 0, 5);
	ImGui::SliderInt("Blur Size", &blurSize, 0, 30);
	ImGui::SliderFloat("Blur Skip", &blurSkip, 1, 10);
	ImGui::End();


	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

