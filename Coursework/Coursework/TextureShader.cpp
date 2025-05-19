#include "TextureShader.h"

TextureShader::TextureShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	this->device = device;
	initShader(L"Texture_vs.cso", L"Texture_ps.cso");
}

TextureShader::~TextureShader()
{
	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the projection buffer.
	if (projectionBuffer)
	{
		projectionBuffer->Release();
		projectionBuffer = 0;
	}

	// Release the texture sampler.
	if (textureSampler)
	{
		textureSampler->Release();
		textureSampler = 0;
	}

	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void TextureShader::SetRenderer(D3D* renderer)
{
	this->renderer = renderer;
}

void TextureShader::SetShaderParameters(ID3D11ShaderResourceView* texture, int screenWidth, int screenHeight)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Set projection buffer data
	result = renderer->getDeviceContext()->Map(projectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	XMMATRIX* projectionMatrix;
	projectionMatrix = (XMMATRIX*)mappedResource.pData;
	// Setup with an orthographic projection
	*projectionMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, 0, 1);
	renderer->getDeviceContext()->Unmap(projectionBuffer, 0);
	renderer->getDeviceContext()->VSSetConstantBuffers(0, 1, &projectionBuffer); // Projection buffer b0 in Vertex Shader

	// Set texture and sampler
	renderer->getDeviceContext()->PSSetShaderResources(0, 1, &texture);
	renderer->getDeviceContext()->PSSetSamplers(0, 1, &textureSampler);
}

void TextureShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	D3D11_BUFFER_DESC projectionBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	// Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	// Projection buffer setup
	projectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	projectionBufferDesc.ByteWidth = sizeof(XMMATRIX);
	projectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	projectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	projectionBufferDesc.MiscFlags = 0;
	projectionBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&projectionBufferDesc, NULL, &projectionBuffer);

	// Sampler for texture sampling
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&samplerDesc, &textureSampler);
}
