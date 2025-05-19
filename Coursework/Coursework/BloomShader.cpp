#include "BloomShader.h"

BloomShader::BloomShader(ID3D11Device* device, HWND hwnd, int screenWidth, int screenHeight) : BaseShader(device, hwnd)
{
	this->device = device;
	initShader(L"Texture_vs.cso", L"BloomPart1_ps.cso");
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
}

BloomShader::~BloomShader()
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
	
	// Release the depth layer buffer.
	if (bloomBuffer)
	{
		bloomBuffer->Release();
		bloomBuffer = 0;
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

void BloomShader::SetRenderer(D3D* renderer)
{
	this->renderer = renderer;
}

void BloomShader::ReadyPart1()
{
	loadPixelShader(L"BloomPart1_ps.cso");
}

void BloomShader::ReadyPart2()
{
	loadPixelShader(L"BloomPart2_ps.cso");
}

void BloomShader::ReadyPart3()
{
	loadPixelShader(L"BloomPart3_ps.cso");
}

void BloomShader::SetShaderParametersPart1(ID3D11ShaderResourceView* sceneTexture, float luminosityThreshold)
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

	// Set bloom info data
	result = renderer->getDeviceContext()->Map(bloomBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	BloomInfo* bufferContents;
	bufferContents = (BloomInfo*)mappedResource.pData;
	bufferContents->luminosityThreshold = luminosityThreshold;
	renderer->getDeviceContext()->Unmap(bloomBuffer, 0);
	renderer->getDeviceContext()->PSSetConstantBuffers(0, 1, &bloomBuffer); // Bloom buffer b0 in Pixel Shader

	// Set textures and sampler
	renderer->getDeviceContext()->PSSetShaderResources(0, 1, &sceneTexture);
	renderer->getDeviceContext()->PSSetSamplers(0, 1, &textureSampler);
}

void BloomShader::SetShaderParametersPart2(ID3D11ShaderResourceView* toBlur, bool xPass, int blurSize, float blurSkip)
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

	// Set bloom info data
	result = renderer->getDeviceContext()->Map(bloomBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	BloomInfo* bufferContents;
	bufferContents = (BloomInfo*)mappedResource.pData;
	bufferContents->blurDistance = blurSize;
	bufferContents->blurSkips = blurSkip;
	bufferContents->blurOnX = xPass ? 1 : 0;
	renderer->getDeviceContext()->Unmap(bloomBuffer, 0);
	renderer->getDeviceContext()->PSSetConstantBuffers(0, 1, &bloomBuffer); // Bloom buffer b0 in Pixel Shader

	// Set textures and sampler 
	renderer->getDeviceContext()->PSSetShaderResources(0, 1, &toBlur);
	renderer->getDeviceContext()->PSSetSamplers(0, 1, &textureSampler);
}

void BloomShader::SetShaderParametersPart3(ID3D11ShaderResourceView* sceneTexture, ID3D11ShaderResourceView* blurredTexture)
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

	// Set textures and sampler 
	renderer->getDeviceContext()->PSSetShaderResources(0, 1, &sceneTexture);
	renderer->getDeviceContext()->PSSetShaderResources(1, 1, &blurredTexture);
	renderer->getDeviceContext()->PSSetSamplers(0, 1, &textureSampler);
}

void BloomShader::initShader(const wchar_t* vs, const wchar_t* ps)
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

	// Bloom buffer setup, reuse the description but change the size
	projectionBufferDesc.ByteWidth = sizeof(BloomInfo);
	device->CreateBuffer(&projectionBufferDesc, NULL, &bloomBuffer);

	// Sampler for texture sampling
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&samplerDesc, &textureSampler);
}
