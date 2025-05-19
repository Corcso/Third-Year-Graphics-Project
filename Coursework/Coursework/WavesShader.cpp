#include "WavesShader.h"

WavesShader::WavesShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	this->device = device;
	initShader(L"HeightMap_vs.cso", L"HeightMap_hs.cso", L"Waves_ds.cso", L"Waves_ps.cso");
}

WavesShader::~WavesShader()
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

	// Release the camera buffer.
	if (cameraBuffer)
	{
		cameraBuffer->Release();
		cameraBuffer = 0;
	}

	// Release the world buffer.
	if (worldBuffer)
	{
		worldBuffer->Release();
		worldBuffer = 0;
	}

	// Release the light buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	// Release the height map buffer.
	if (wavesBuffer)
	{
		wavesBuffer->Release();
		wavesBuffer = 0;
	}

	// Release the tesselation buffer.
	if (tessInfoBuffer)
	{
		tessInfoBuffer->Release();
		tessInfoBuffer = 0;
	}

	// Release the dof plane buffer.
	if (dofPlaneBuffer)
	{
		dofPlaneBuffer->Release();
		dofPlaneBuffer = 0;
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

void WavesShader::SetRenderer(D3D* renderer)
{
	this->renderer = renderer;
}

void WavesShader::SetCurrentCamera(Camera* camera)
{
	this->currentCamera = camera;
}

void WavesShader::SetLightAsCamera(WorldLight* light, int shadowMapIndex)
{
	this->lightCamera = light;
	this->shadowMapIndex = shadowMapIndex;
}

void WavesShader::SetCameraAsCamera()
{
	usingLightCamera = false;
}

void WavesShader::SetLightAsCamera()
{
	usingLightCamera = true;
}

void WavesShader::SetShaderParameters(const XMMATRIX& world, WavesData* waveBufferData, WorldLight* lights, int lightCount, XMFLOAT2 minMaxTess, XMFLOAT2 minMaxDist, XMFLOAT2 DOFKeepingRange)
{
	// Clear all PS Shader Resource Views, stops type mismatch errors
	// 20 is the most, used by PBR Shader
	for (int i = 0; i < 20; ++i) {
		ID3D11ShaderResourceView* unbind = nullptr;
		renderer->getDeviceContext()->PSSetShaderResources(i, 1, &unbind);
	}
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Set projection buffer data
	result = renderer->getDeviceContext()->Map(projectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	XMMATRIX* projectionMatrix;
	projectionMatrix = (XMMATRIX*)mappedResource.pData;
	// If using camera as our camera, use normal projection, otherwise use lights projection
	if (!usingLightCamera) *projectionMatrix = renderer->getProjectionMatrix();
	else *projectionMatrix = lightCamera->GetProjMatrix(0);
	renderer->getDeviceContext()->Unmap(projectionBuffer, 0);
	renderer->getDeviceContext()->DSSetConstantBuffers(0, 1, &projectionBuffer); // Projection buffer b0 in Vertex Shader

	// Map camera buffer data
	result = renderer->getDeviceContext()->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CameraBufferData* cameraBufferData;
	cameraBufferData = (CameraBufferData*)mappedResource.pData;
	// If using camera as our camera, use normal paramaters, otherwise use light camera's parameters
	if (!usingLightCamera) {
		cameraBufferData->cameraPosition = XMFLOAT4(currentCamera->getPosition().x, currentCamera->getPosition().y, currentCamera->getPosition().z, 1);
		cameraBufferData->viewMatrix = currentCamera->getViewMatrix();
	}
	else{
		cameraBufferData->cameraPosition = XMFLOAT4(lightCamera->getPosition().x, lightCamera->getPosition().y, lightCamera->getPosition().z, 1);
		cameraBufferData->viewMatrix = lightCamera->GetViewMatrix(shadowMapIndex);
	}
	renderer->getDeviceContext()->Unmap(cameraBuffer, 0);
	renderer->getDeviceContext()->DSSetConstantBuffers(1, 1, &cameraBuffer); // Camera buffer b1 in Vertex Shader

	// Map world buffer data
	result = renderer->getDeviceContext()->Map(worldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WorldBufferData* worldBufferData;
	worldBufferData = (WorldBufferData*)mappedResource.pData;
	worldBufferData->worldMatrix = world;
	worldBufferData->normalWorldMatrix = world;
	renderer->getDeviceContext()->Unmap(worldBuffer, 0);
	renderer->getDeviceContext()->DSSetConstantBuffers(2, 1, &worldBuffer); // Camera buffer b2 in Vertex Shader



	// Map light buffer data
	result = renderer->getDeviceContext()->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	LightBufferData* lightBufferData;
	lightBufferData = (LightBufferData*)mappedResource.pData;
	for (int i = 0; i < lightCount; i++) {
		lightBufferData->lights[i].ambientColor = lights[i].getAmbientColour();
		lightBufferData->lights[i].diffuseColor = lights[i].getDiffuseColour();
		lightBufferData->lights[i].position = XMFLOAT3A(lights[i].getPosition().x, lights[i].getPosition().y, lights[i].getPosition().z);
		lightBufferData->lights[i].direction = lights[i].getDirection();
		lightBufferData->lights[i].lightType = lights[i].GetLightType(); 
		lightBufferData->lights[i].innerSpotlightCutoffAngle = lights[i].GetInnerSpotlightCutoffAngle();
		lightBufferData->lights[i].outerSpotlightCutoffAngle = lights[i].GetOuterSpotlightCutoffAngle();
		lightBufferData->lights[i].lightViewMatrix[0] = lights[i].GetViewMatrix(0);
		if (lightBufferData->lights[i].lightType != 0) {
			for (int f = 1; f < 6; ++f) {
				lightBufferData->lights[i].lightViewMatrix[f] = lights[i].GetViewMatrix(f);
			}
			ID3D11ShaderResourceView* tempAddress = lights[i].GetTCubeShadowMap()->getDepthMapSRV();
			renderer->getDeviceContext()->PSSetShaderResources(0 + i, 1, &tempAddress);
		}
		else {
			ID3D11ShaderResourceView* tempAddress = lights[i].GetDirectionalShadowMap()->getDepthMapSRV();
			renderer->getDeviceContext()->PSSetShaderResources(8 + i, 1, &tempAddress);
		}
		lightBufferData->lights[i].lightProjectionMatrix = lights[i].GetProjMatrix(0);
		lightBufferData->lights[i].constantAttenuation = lights[i].GetConstantAttenuation();
		lightBufferData->lights[i].linearAttenuation = lights[i].GetLinearAttenuation();
		lightBufferData->lights[i].quadraticAttenuation = lights[i].GetQuadraticAttenuation();
		lightBufferData->lights[i].lightPower = lights[i].GetLightPower();

	}
	for (int i = lightCount; i < 8; i++) {
		lightBufferData->lights[i].ambientColor = XMFLOAT4(0, 0, 0, 0);
		lightBufferData->lights[i].diffuseColor = XMFLOAT4(0, 0, 0, 0);
		lightBufferData->lights[i].position = XMFLOAT3A(0, 0, 0);
		lightBufferData->lights[i].direction = XMFLOAT3(0, 0, 0);
		lightBufferData->lights[i].lightType = 0;
		lightBufferData->lights[i].constantAttenuation = 0;
		lightBufferData->lights[i].linearAttenuation = 0;
		lightBufferData->lights[i].quadraticAttenuation = 0;
		lightBufferData->lights[i].lightPower = 0;
	}
	lightBufferData->lightCount = lightCount;
	renderer->getDeviceContext()->Unmap(lightBuffer, 0);
	renderer->getDeviceContext()->PSSetConstantBuffers(0, 1, &lightBuffer); // Light buffer b0 in Pixel Shader

	// Map height map buffer data
	result = renderer->getDeviceContext()->Map(wavesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WavesData* waveBufferContents;
	waveBufferContents = (WavesData*)mappedResource.pData;
	waveBufferContents[0] = waveBufferData[0];
	waveBufferContents[1] = waveBufferData[2];
	waveBufferContents[2] = waveBufferData[1];
	renderer->getDeviceContext()->Unmap(wavesBuffer, 0);
	renderer->getDeviceContext()->DSSetConstantBuffers(3, 1, &wavesBuffer); // Height buffer b3 in Vertex Shader
	renderer->getDeviceContext()->PSSetConstantBuffers(1, 1, &wavesBuffer); // Wave buffer b1 in Pixel Shader

	// Setup DOF Plane Buffer Data
	result = renderer->getDeviceContext()->Map(dofPlaneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	XMFLOAT2* dofPlaneBufferContents;
	dofPlaneBufferContents = (XMFLOAT2*)mappedResource.pData;
	*dofPlaneBufferContents = DOFKeepingRange;
	renderer->getDeviceContext()->Unmap(dofPlaneBuffer, 0);
	renderer->getDeviceContext()->PSSetConstantBuffers(2, 1, &dofPlaneBuffer); // Height Map buffer b1 in Pixel Shader

	// Setup tesselation information buffer
	result = renderer->getDeviceContext()->Map(tessInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TessInfoData* dataPtrTess = (TessInfoData*)mappedResource.pData;
	dataPtrTess->minMaxTess = minMaxTess;
	dataPtrTess->minMaxDist = minMaxDist;
	dataPtrTess->camPos = XMFLOAT4(currentCamera->getPosition().x, currentCamera->getPosition().y, currentCamera->getPosition().z, 1);
	dataPtrTess->worldMatrix = world;
	renderer->getDeviceContext()->Unmap(tessInfoBuffer, 0);
	renderer->getDeviceContext()->HSSetConstantBuffers(0, 1, &tessInfoBuffer);

	// Setup samplers
	renderer->getDeviceContext()->PSSetSamplers(0, 1, &shadowSampler);
}

void WavesShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	D3D11_BUFFER_DESC projectionBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC worldBufferDesc;

	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC wavesBufferDesc;
	D3D11_SAMPLER_DESC shadowSamplerDesc;
	ZeroMemory(&shadowSamplerDesc, sizeof(D3D11_SAMPLER_DESC));

	// Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	// Setup buffers
	projectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	projectionBufferDesc.ByteWidth = sizeof(XMMATRIX);
	projectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	projectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	projectionBufferDesc.MiscFlags = 0;
	projectionBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&projectionBufferDesc, NULL, &projectionBuffer);

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferData);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	worldBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	worldBufferDesc.ByteWidth = sizeof(WorldBufferData);
	worldBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	worldBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	worldBufferDesc.MiscFlags = 0;
	worldBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&worldBufferDesc, NULL, &worldBuffer);

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferData);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	wavesBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	wavesBufferDesc.ByteWidth = sizeof(WavesData) * 3;
	wavesBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	wavesBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	wavesBufferDesc.MiscFlags = 0;
	wavesBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&wavesBufferDesc, NULL, &wavesBuffer);

	// Tesselelation buffer information
	D3D11_BUFFER_DESC tessInfoBufferDesc;
	tessInfoBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessInfoBufferDesc.ByteWidth = sizeof(TessInfoData);
	tessInfoBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessInfoBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessInfoBufferDesc.MiscFlags = 0;
	tessInfoBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&tessInfoBufferDesc, NULL, &tessInfoBuffer);

	tessInfoBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	device->CreateBuffer(&tessInfoBufferDesc, NULL, &dofPlaneBuffer);

	// Sampler for shadow map sampling
	shadowSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSamplerDesc, &shadowSampler);
}

void WavesShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}
