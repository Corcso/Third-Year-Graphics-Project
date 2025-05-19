// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "WorldObject.h"
#include "WorldLight.h"
#include "PBRShader.h"
//#include "ShadowDepthShader.h"
#include "HeightMapShader.h"
#include "TextureShader.h"
#include "WavesShader.h"
#include "DepthOfFieldShader.h"
#include "BloomShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();

	/// <summary>
	/// Init function, full of scene setup
	/// </summary>
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	/// <summary>
	/// Frame update function, carries out pre render actions 
	/// </summary>
	bool frame();

protected:
	/// <summary>
	/// Overall render function calls all the relevant passes
	/// </summary>
	bool render();

	/// <summary>
	/// 1st Pass
	/// Shadow passes for every light
	/// </summary>
	bool shadowDepthPasses();

	/// <summary>
	/// 2nd Pass
	/// Scene Pass without DOF
	/// </summary>
	bool sceneRenderPass();

	/// <summary>
	/// 2nd Pass
	/// Scene pass with DOF
	/// </summary>
	bool depthOfFieldPass();

	/// <summary>
	/// 3rd Pass
	/// Bloom Pass
	/// </summary>
	bool bloomPass();

	/// <summary>
	/// 4th Pass
	/// Final pass rendering bloom output to back buffer
	/// </summary>
	bool finalPass();

	/// <summary>
	/// 5th Pass
	/// GUI Overlay
	/// </summary>
	void gui();

private:
	// Width and height for use throughout
	int screenWidth, screenHeight;

	// Shaders used
	PBRShader* pbrShader;
	HeightMapShader* heightMapShader;
	WavesShader* wavesShader;

	// Temple & Spheres 
	WorldObject temple;
	PBRShader::PBRMaterial templeMaterial;
	WorldObject lightSphere; // Sphere for easy showing where lights are
	WorldObject groundPlane; // Terrain
	WorldObject water;		// Water

	WorldObject PBRSphere; // Sphere used for 3 spheres 
	// Materials for Spheres & Sausage roll
	PBRShader::PBRMaterial GreyBricksMaterial;
	PBRShader::PBRMaterial BrushedMetalMaterial;
	PBRShader::PBRMaterial WoorFloorMaterial;
	PBRShader::PBRMaterial SausageRollMaterial;
	// Sausage roll object
	WorldObject SausageRoll;
	// Bool for toggle between the 2
	bool sausageRollReplaceSpheres = false;

	// Vector of all lights (MAX 8)
	std::vector<WorldLight> lights;
	// If the point light is swinging or not. 
	bool swingPointLight = true;

	// Tessellation variables
	XMFLOAT2 terrainTessellationMinAndMaxTesselation = XMFLOAT2(1, 1);
	XMFLOAT2 terrainTessellationMinAndMaxDistance;

	// Height map variables
	float amplitude;
	bool isSmoothingOn;

	// General Scene Render Texture
	RenderTexture* fullSceneNoPP;

	// Texture shader for final pass
	TextureShader* textureShader;

	// Ortho mesh for all post processing (as world object here)
	WorldObject fullScreenOrthoMesh;

	// Post processing shaders
	DepthOfFieldShader* dofShader;
	BloomShader* bloomShader;

	// DOF render texture, & variables
	RenderTexture* depthOfFieldPP;
	RenderTexture* depthOfFieldLayers[DOF_LAYER_COUNT];
	RenderTexture* depthOfFieldLayersHBlur[DOF_LAYER_COUNT];
	RenderTexture* depthOfFieldLayersVBlur[DOF_LAYER_COUNT];
	bool DOFEnabled;

	float focusPlane = 0.990; // Depth value which is in focus

	// Wave Variables
	float totalTimeElapsed;
	WavesShader::WavesData waveData[3];
	float angleOfWave[3]; // Used for easier user editing of direction
	

	// Bloom Variables & RTs
	RenderTexture* bloomScene;
	RenderTexture* bloomSceneAlt;
	int blurSize = 0;
	float blurSkip = 1.0f;
	float luminocityThreshold = 1.0f;
};

#endif