#pragma once
#include "DXF.h"

// Texture shader for rendering textures onto ortho meshes. 

class TextureShader :
    public BaseShader
{
public:
	TextureShader(ID3D11Device* device, HWND hwnd);
	~TextureShader();

	void SetRenderer(D3D* renderer); // Set render after shader init, used to reduce parameter passing.

	// We dont need the camera for post processing as view isnt taken into account.

	/// <summary>
	/// Setup parameters
	/// </summary>
	/// <param name="texture">Texture to render</param>
	/// <param name="screenWidth">Screen width</param>
	/// <param name="screenHeight">Screen height</param>
	void SetShaderParameters(ID3D11ShaderResourceView* texture, int screenWidth, int screenHeight);
private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	// Vertex Shader Buffers
	ID3D11Buffer* projectionBuffer;

	// Sampler for the texture
	ID3D11SamplerState* textureSampler;

	// Renderer pointer, reduces number of parameters needing passed around. 
	D3D* renderer;
	ID3D11Device* device;
};

