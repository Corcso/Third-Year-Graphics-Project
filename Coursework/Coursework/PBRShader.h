#pragma once
#include "DXF.h"
#include "WorldLight.h"
#include "CommonStructs.h"
class PBRShader :
    public BaseShader
{
public:
	PBRShader(ID3D11Device* device, HWND hwnd);
	~PBRShader();

	enum class TextureFlag {
		NONE = 0x0,
		COLOR = 0x1,
		NORMAL = 0x2,
		AO = 0x4,
		ROUGHNESS = 0x8
	};

	/// <summary>
	/// PBR Material struct
	/// </summary>
	struct PBRMaterial {
		XMFLOAT4 diffuseColor; // Diffuse colour
		XMFLOAT4 specularColor; // Specular colour 
		float specularity; // Specularity, bigger number sharper specular reflection
		float smoothness; // Smoothness, inverse roughness
		float anisotropy; // Anisotropy along tangent, 0 is isotropic specular, 1 is anisotropic specular. 
		int textureFlags; // Flags for which textures are set. 

		ID3D11ShaderResourceView* colorMap; // Diffuse colour map 
		ID3D11ShaderResourceView* normalMap; // Normal map
		ID3D11ShaderResourceView* AOMap; // Ambient occlusion map
		ID3D11ShaderResourceView* roughnessMap; // Roughness, inverse smoothness map
	};

	// Pure data PBR material struct
	struct PBRMaterialData {
		XMFLOAT4 diffuseColor;
		XMFLOAT4 specularColor;
		float specularity;
		float smoothness;
		float anisotropy;
		int textureFlags;
	};
	


	void SetRenderer(D3D* renderer); // Set render after shader init, used to reduce parameter passing.
	void SetCurrentCamera(Camera* camera);
	void SetLightAsCamera(WorldLight* light, int shadowMapIndex = 0);

	void SetCameraAsCamera();
	void SetLightAsCamera();

	/// <summary>
	/// Setup data for shader
	/// </summary>
	/// <param name="world">World matrix of object</param>
	/// <param name="material">Material</param>
	/// <param name="lights">Array of lights</param>
	/// <param name="lightCount">Number of active lights in that array</param>
	/// <param name="DOFKeepingRange">DOF Pass Data, What range are we in, defaults to entire scene</param>
	void SetShaderParameters(const XMMATRIX& world, PBRMaterial* material, WorldLight* lights, int lightCount, XMFLOAT2 DOFKeepingRange = XMFLOAT2(0, 1));

	/// <summary>
	/// Display ImGUI UI for the material passed in
	/// </summary>
	/// <param name="name">Name of material</param>
	/// <param name="material">Material pointer</param>
	static void DisplayMaterialUI(std::string name, PBRMaterial* material);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	// Vertex Shader Buffers
	ID3D11Buffer* projectionBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* worldBuffer;
	

	// Pixel Shader Buffers
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* materialBuffer;
	ID3D11Buffer* dofPlaneBuffer;
	ID3D11SamplerState* sampleState;
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

