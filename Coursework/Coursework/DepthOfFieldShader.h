#pragma once
#include "DXF.h"

// Number of layers, must match number in Common.hlsli
// 9 Gives good results
constexpr int DOF_LAYER_COUNT = 9;

class DepthOfFieldShader :
    public BaseShader
{
public:
	DepthOfFieldShader(ID3D11Device* device, HWND hwnd);
	~DepthOfFieldShader();

	void SetRenderer(D3D* renderer); // Set render after shader init, used to reduce parameter passing.

	// We dont need the camera for post processing as view isnt taken into account.

	void ReadyPart1(); // Load Part 1 PS NOT USED
	void ReadyPart2(); // Load Part 2 PS
	void ReadyPart3(); // Load Part 3 PS

	/// <summary>
	/// NOT USED
	/// Old part 1 setup
	/// </summary>
	/// <param name="texture">Scene Render</param>
	/// <param name="depthFromScene">Depth Scene Render</param>
	/// <param name="screenWidth">Screen Width </param>
	/// <param name="screenHeight">Screen Height</param>
	/// <param name="maxDepth">Max depth of layer </param>
	/// <param name="minDepth">Min depth of layer</param>
	void SetShaderParametersPart1(ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthFromScene, int screenWidth, int screenHeight, float maxDepth, float minDepth);
	// OLD PART 2, Used in scene render DOF
	//void SetShaderParametersPart2(ID3D11ShaderResourceView** layers, ID3D11ShaderResourceView* depthFromScene, int screenWidth, int screenHeight, float* maxDepths, float* minDepths);

	/// <summary>
	/// Part 2 Setup
	/// </summary>
	/// <param name="layer">Layer to blur</param>
	/// <param name="depthFromScene">Depth from this layer render</param>
	/// <param name="screenWidth">Screen width</param>
	/// <param name="screenHeight">Screen height</param>
	/// <param name="maxDepths">List of max depths of layers</param>
	/// <param name="minDepths">List of min depths of layers</param>
	/// <param name="xPass">True if X blur, False if Y blur</param>
	/// <param name="layerNum">Layer number we are on</param>
	void SetShaderParametersPart2(ID3D11ShaderResourceView* layer, ID3D11ShaderResourceView* depthFromScene, int screenWidth, int screenHeight, float* maxDepths, float* minDepths, bool xPass, int layerNum);

	/// <summary>
	/// Part 3 Setup
	/// </summary>
	/// <param name="layers">All layers to combine</param>
	/// <param name="screenWidth">Screen width</param>
	/// <param name="screenHeight">Screen height</param>
	void SetShaderParametersPart3(ID3D11ShaderResourceView** layers, int screenWidth, int screenHeight);
private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	// Vertex Shader Buffers
	ID3D11Buffer* projectionBuffer;

	// Depth layer buffer for part one
	ID3D11Buffer* depthLayerBuffer;
	ID3D11Buffer* depthLayersBuffer;

	// Sampler for the texture
	ID3D11SamplerState* textureSampler;

	// Renderer pointer, reduces number of parameters needing passed around. 
	D3D* renderer;
	ID3D11Device* device;
};

