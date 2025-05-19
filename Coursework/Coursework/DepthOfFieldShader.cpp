#include "DepthOfFieldShader.h"

DepthOfFieldShader::DepthOfFieldShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	this->device = device;
	initShader(L"Texture_vs.cso", L"DOFPart2_v2_ps.cso");
}

DepthOfFieldShader::~DepthOfFieldShader()
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
	if (depthLayerBuffer)
	{
		depthLayerBuffer->Release();
		depthLayerBuffer = 0;
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

void DepthOfFieldShader::SetRenderer(D3D* renderer)
{
	this->renderer = renderer;
}

void DepthOfFieldShader::ReadyPart1()
{
	//loadPixelShader(L"DOFPart1_ps.cso");
}

void DepthOfFieldShader::ReadyPart2()
{
	loadPixelShader(L"DOFPart2_v2_ps.cso");
}

void DepthOfFieldShader::ReadyPart3()
{
	loadPixelShader(L"DOFPart3_ps.cso");
}

void DepthOfFieldShader::SetShaderParametersPart1(ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthFromScene, int screenWidth, int screenHeight, float maxDepth, float minDepth)
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

	// Set layer data
	result = renderer->getDeviceContext()->Map(depthLayerBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	float* layerData;
	layerData = (float*)mappedResource.pData;
	// Setup data, its just 2 floats so no need for a struct
	layerData[0] = maxDepth;
	layerData[1] = minDepth;
	renderer->getDeviceContext()->Unmap(depthLayerBuffer, 0);
	renderer->getDeviceContext()->PSSetConstantBuffers(0, 1, &depthLayerBuffer); // Depth layer buffer b0 in Pixel Shader

	// Set textures and sampler
	renderer->getDeviceContext()->PSSetShaderResources(0, 1, &texture);
	renderer->getDeviceContext()->PSSetShaderResources(1, 1, &depthFromScene);
	renderer->getDeviceContext()->PSSetSamplers(0, 1, &textureSampler);
}

void DepthOfFieldShader::SetShaderParametersPart2(ID3D11ShaderResourceView* layer, ID3D11ShaderResourceView* depthFromScene, int screenWidth, int screenHeight, float* maxDepths, float* minDepths, bool xPass, int layerNum)
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

	// Set layer data
	result = renderer->getDeviceContext()->Map(depthLayersBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	XMFLOAT4* layersData;
	layersData = (XMFLOAT4*)mappedResource.pData;
	// Setup data
	for (int i = 0; i < DOF_LAYER_COUNT; ++i) {
		layersData[i] = XMFLOAT4(minDepths[i], maxDepths[i], 0, 0);
	}
	layersData[DOF_LAYER_COUNT] = XMFLOAT4((xPass) ? 1 : 0, (float)layerNum, 0, 0);
	renderer->getDeviceContext()->Unmap(depthLayersBuffer, 0);
	renderer->getDeviceContext()->PSSetConstantBuffers(0, 1, &depthLayersBuffer); // Depth layers buffer b0 in Pixel Shader

	// Set textures and sampler 
	renderer->getDeviceContext()->PSSetShaderResources(0, 1, &depthFromScene);
	renderer->getDeviceContext()->PSSetShaderResources(1, 1, &layer);
	renderer->getDeviceContext()->PSSetSamplers(0, 1, &textureSampler);
}

void DepthOfFieldShader::SetShaderParametersPart3(ID3D11ShaderResourceView** layers, int screenWidth, int screenHeight)
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
	renderer->getDeviceContext()->PSSetShaderResources(0, DOF_LAYER_COUNT, layers);
	renderer->getDeviceContext()->PSSetSamplers(0, 1, &textureSampler);
}

//void DepthOfFieldShader::SetShaderParametersPart2(ID3D11ShaderResourceView** layers, ID3D11ShaderResourceView* depthFromScene, int screenWidth, int screenHeight, float* maxDepths, float* minDepths)
//{
//	HRESULT result;
//	D3D11_MAPPED_SUBRESOURCE mappedResource;
//
//	// Set projection buffer data
//	result = renderer->getDeviceContext()->Map(projectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
//	XMMATRIX* projectionMatrix;
//	projectionMatrix = (XMMATRIX*)mappedResource.pData;
//	// Setup with an orthographic projection
//	*projectionMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, 0, 1);
//	renderer->getDeviceContext()->Unmap(projectionBuffer, 0);
//	renderer->getDeviceContext()->VSSetConstantBuffers(0, 1, &projectionBuffer); // Projection buffer b0 in Vertex Shader
//
//	// Set layer data
//	result = renderer->getDeviceContext()->Map(depthLayersBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
//	XMFLOAT4* layersData;
//	layersData = (XMFLOAT4*)mappedResource.pData;
//	// Setup data
//	for (int i = 0; i < 9; ++i) {
//		layersData[i] = XMFLOAT4(minDepths[i], maxDepths[i], 0, 0);
//	}
//	renderer->getDeviceContext()->Unmap(depthLayersBuffer, 0);
//	renderer->getDeviceContext()->PSSetConstantBuffers(0, 1, &depthLayersBuffer); // Depth layers buffer b0 in Pixel Shader
//
//	// Set textures and sampler 
//	renderer->getDeviceContext()->PSSetShaderResources(0, 1, &depthFromScene);
//	renderer->getDeviceContext()->PSSetShaderResources(1, 9, layers);
//	renderer->getDeviceContext()->PSSetSamplers(0, 1, &textureSampler);
//}

void DepthOfFieldShader::initShader(const wchar_t* vs, const wchar_t* ps)
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

	// Depth layer buffer setup, reuse the description but change the size
	projectionBufferDesc.ByteWidth = sizeof(float) * 4;
	device->CreateBuffer(&projectionBufferDesc, NULL, &depthLayerBuffer);

	// Depth layers buffer setup, reuse the description but change the size
	projectionBufferDesc.ByteWidth = sizeof(XMFLOAT4) * (DOF_LAYER_COUNT + 1);
	device->CreateBuffer(&projectionBufferDesc, NULL, &depthLayersBuffer);

	// Sampler for texture sampling
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&samplerDesc, &textureSampler);
}
