#pragma once
#include "DXF.h"
#include "CommonStructs.h"
#include "WorldLight.h"

/// <summary>
/// Waves shader for water rendering
/// </summary>
class WavesShader :
    public BaseShader
{
public:
	WavesShader(ID3D11Device* device, HWND hwnd);
	~WavesShader();


	/// <summary>
	/// Waves buffer data structure
	/// </summary>
	struct WavesData {
		float time;
		float amplitude;
		float frequency;
		float speed;
		XMFLOAT2 direction;
		float steepness;
		float p_0;
	};

	/// <summary>
	/// Tessellation buffer data structure
	/// </summary>
	struct TessInfoData {
		XMFLOAT2 minMaxTess;
		XMFLOAT2 minMaxDist;
		XMFLOAT4 camPos;
		XMMATRIX worldMatrix;
	};

	

	void SetRenderer(D3D* renderer); // Set render after shader init, used to reduce parameter passing.
	void SetCurrentCamera(Camera* camera);
	void SetLightAsCamera(WorldLight* light, int shadowMapIndex = 0);

	void SetCameraAsCamera();
	void SetLightAsCamera();

	/// <summary>
	/// Setup parameters
	/// </summary>
	/// <param name="world">World matrix of plane</param>
	/// <param name="waveData">Waves data struct with wave profile information, array of 3 expected</param>
	/// <param name="lights">Array of lights</param>
	/// <param name="lightCount">Total light count</param>
	/// <param name="minMaxTess">X = minimum tessellation, Y = maximum tessellation</param>
	/// <param name="minMaxDist">X = distance to start interpolation, Y = distance to stop interpolation</param>
	/// <param name="DOFKeepingRange">DOF Pass Data, What range are we in, defaults to entire scene</param>
	void SetShaderParameters(const XMMATRIX& world, WavesData* waveData, WorldLight* lights, int lightCount, XMFLOAT2 minMaxTess, XMFLOAT2 minMaxDist, XMFLOAT2 DOFKeepingRange = XMFLOAT2(0, 1));

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

	// Vertex Shader Buffers
	ID3D11Buffer* projectionBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* worldBuffer;

	ID3D11SamplerState* heightMapSampler;

	// Pixel Shader Buffers
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* wavesBuffer;
	ID3D11Buffer* tessInfoBuffer;
	ID3D11Buffer* dofPlaneBuffer;
	
	ID3D11SamplerState* textureSampler;
	ID3D11SamplerState* shadowSampler;

	// Tie shader directly to camera, reduces number of parameters needing passed around. 
	Camera* currentCamera;
	// Have a light we can use as the current camera too.
	WorldLight* lightCamera;
	int shadowMapIndex;
	bool usingLightCamera = true;

	// Renderer pointer, reduces number of parameters needing passed around. 
	D3D* renderer;
	ID3D11Device* device;
};

