#pragma once
#include "DXF.h"

// ====================================================
// UNUSED SHADER
// Now depth pass uses normal shaders!
// ====================================================


class ShadowDepthShader :
    public BaseShader
{
public:

	ShadowDepthShader(ID3D11Device* device, HWND hwnd);
	~ShadowDepthShader();

	void SetRenderer(D3D* renderer); // Set render after shader init, used to reduce parameter passing.

	void SetShaderParameters(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	D3D* renderer;
	ID3D11Device* device;
};