#pragma once
#include "DXF.h"

class BloomShader :
    public BaseShader
{
public:
	BloomShader(ID3D11Device* device, HWND hwnd, int screenWidth, int screenHeight);
	~BloomShader();

	struct BloomInfo {
		float luminosityThreshold;
		int blurDistance;
		float blurSkips;
		int blurOnX;
	};

	void SetRenderer(D3D* renderer); // Set render after shader init, used to reduce parameter passing.

	// We dont need the camera for post processing as view isnt taken into account.

	void ReadyPart1(); // Load part 1 PS
	void ReadyPart2(); // Load part 2 PS
	void ReadyPart3(); // Load part 3 PS

	/// <summary>
	/// Load buffers and views for part 1
	/// </summary>
	/// <param name="sceneTexture">Scene render to seperate</param>
	/// <param name="luminosityThreshold">Luminosity threshold</param>
	void SetShaderParametersPart1(ID3D11ShaderResourceView* sceneTexture, float luminosityThreshold); 
	/// <summary>
	/// Load buffers and views for part 2
	/// </summary>
	/// <param name="toBlur">Light scene render to blur</param>
	/// <param name="xPass">True if X blur False if Y blur</param>
	/// <param name="blurSize">Size of the blur</param>
	/// <param name="blurSkip">Texels to skip when sampling for blur</param>
	void SetShaderParametersPart2(ID3D11ShaderResourceView* toBlur, bool xPass, int blurSize, float blurSkip = 1);
	/// <summary>
	/// Load buffers and views for part 3
	/// </summary>
	/// <param name="sceneTexture">Scene texture in full</param>
	/// <param name="blurredTexture">Blurred bloom texture from part 2</param>
	void SetShaderParametersPart3(ID3D11ShaderResourceView* sceneTexture, ID3D11ShaderResourceView* blurredTexture); 
private: 
	void initShader(const wchar_t* vs, const wchar_t* ps);

	// Vertex Shader Buffers
	ID3D11Buffer* projectionBuffer;

	// Bloom information buffer, used in part 1 and 2
	ID3D11Buffer* bloomBuffer;

	// Sampler for the texture
	ID3D11SamplerState* textureSampler;

	// Renderer pointer, reduces number of parameters needing passed around. 
	D3D* renderer;
	ID3D11Device* device;
	int screenWidth, screenHeight;
};

