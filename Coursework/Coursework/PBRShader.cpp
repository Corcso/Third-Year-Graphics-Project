#include "PBRShader.h"

PBRShader::PBRShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	this->device = device;
	initShader(L"PBR_vs.cso", L"PBR_ps.cso");
}

PBRShader::~PBRShader()
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

	// Release the dof plane buffer.
	if (dofPlaneBuffer)
	{
		dofPlaneBuffer->Release();
		dofPlaneBuffer = 0;
	}

	// Release the material buffer.
	if (materialBuffer)
	{
		materialBuffer->Release();
		materialBuffer = 0;
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

void PBRShader::SetRenderer(D3D* renderer)
{
	this->renderer = renderer;
}

void PBRShader::SetCurrentCamera(Camera* camera)
{
	this->currentCamera = camera;
}

void PBRShader::SetLightAsCamera(WorldLight* light, int shadowMapIndex)
{
	this->lightCamera = light;
	this->shadowMapIndex = shadowMapIndex;
	usingLightCamera = true;
}

void PBRShader::SetCameraAsCamera()
{
	usingLightCamera = false;
}

void PBRShader::SetLightAsCamera()
{
	usingLightCamera = true;
}

void PBRShader::SetShaderParameters(const XMMATRIX& world, PBRMaterial* material, WorldLight* lights, int lightCount, XMFLOAT2 DOFKeepingRange)
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
	renderer->getDeviceContext()->VSSetConstantBuffers(0, 1, &projectionBuffer); // Projection buffer b0 in Vertex Shader

	// Map camera buffer data
	result = renderer->getDeviceContext()->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CameraBufferData* cameraBufferData;
	cameraBufferData = (CameraBufferData*)mappedResource.pData;
	// If using camera as our camera, use normal paramaters, otherwise use light camera's parameters
	if (!usingLightCamera) {
		cameraBufferData->cameraPosition = XMFLOAT4(currentCamera->getPosition().x, currentCamera->getPosition().y, currentCamera->getPosition().z, 1);
		cameraBufferData->viewMatrix = currentCamera->getViewMatrix();
	}
	else {
		cameraBufferData->cameraPosition = XMFLOAT4(lightCamera->getPosition().x, lightCamera->getPosition().y, lightCamera->getPosition().z, 1);
		cameraBufferData->viewMatrix = lightCamera->GetViewMatrix(shadowMapIndex);
	}
	renderer->getDeviceContext()->Unmap(cameraBuffer, 0);
	renderer->getDeviceContext()->VSSetConstantBuffers(1, 1, &cameraBuffer); // Camera buffer b1 in Vertex Shader

	// Map world buffer data
	result = renderer->getDeviceContext()->Map(worldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WorldBufferData* worldBufferData;
	worldBufferData = (WorldBufferData*)mappedResource.pData;
	worldBufferData->worldMatrix = world;
	worldBufferData->normalWorldMatrix = world;
	renderer->getDeviceContext()->Unmap(worldBuffer, 0);
	renderer->getDeviceContext()->VSSetConstantBuffers(2, 1, &worldBuffer); // Camera buffer b2 in Vertex Shader

	// Map material buffer data
	result = renderer->getDeviceContext()->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	PBRMaterialData* materialBufferData;
	materialBufferData = (PBRMaterialData*)mappedResource.pData;
	materialBufferData->anisotropy = material->anisotropy;
	materialBufferData->diffuseColor = material->diffuseColor;
	materialBufferData->specularColor = material->specularColor;
	materialBufferData->specularity = material->specularity;
	materialBufferData->smoothness = material->smoothness;
	materialBufferData->textureFlags = material->textureFlags;
	renderer->getDeviceContext()->Unmap(materialBuffer, 0);
	renderer->getDeviceContext()->PSSetConstantBuffers(0, 1, &materialBuffer); // Material buffer b0 in Pixel Shader
	// Set maps
	renderer->getDeviceContext()->PSSetShaderResources(0, 1, &(material->colorMap));
	renderer->getDeviceContext()->PSSetShaderResources(1, 1, &(material->normalMap));
	renderer->getDeviceContext()->PSSetShaderResources(2, 1, &(material->AOMap));
	renderer->getDeviceContext()->PSSetShaderResources(3, 1, &(material->roughnessMap));

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
			renderer->getDeviceContext()->PSSetShaderResources(4 + i, 1, &tempAddress);
		}
		else{
			ID3D11ShaderResourceView* tempAddress = lights[i].GetDirectionalShadowMap()->getDepthMapSRV();
			renderer->getDeviceContext()->PSSetShaderResources(12 + i, 1, &tempAddress);
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
	renderer->getDeviceContext()->PSSetConstantBuffers(1, 1, &lightBuffer); // Material buffer b1 in Pixel Shader

	// Setup DOF Plane Buffer Data
	result = renderer->getDeviceContext()->Map(dofPlaneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	XMFLOAT2* dofPlaneBufferContents;
	dofPlaneBufferContents = (XMFLOAT2*)mappedResource.pData;
	*dofPlaneBufferContents = DOFKeepingRange;
	renderer->getDeviceContext()->Unmap(dofPlaneBuffer, 0);
	renderer->getDeviceContext()->PSSetConstantBuffers(2, 1, &dofPlaneBuffer); // Height Map buffer b1 in Pixel Shader

	// Setup samplers
	renderer->getDeviceContext()->PSSetSamplers(1, 1, &shadowSampler);
}

void PBRShader::DisplayMaterialUI(std::string name, PBRMaterial* material)
{

	if (ImGui::TreeNode((std::string(name) + " PBR Material").c_str())) {

		// Get texture flags as bools 
		bool colorMapEnabled = material->textureFlags & (int)TextureFlag::COLOR;
		bool normalMapEnabled = material->textureFlags & (int)TextureFlag::NORMAL;
		bool AOMapEnabled = material->textureFlags & (int)TextureFlag::AO;
		bool roughnessMapEnabled = material->textureFlags & (int)TextureFlag::ROUGHNESS;

		// Display material options
		ImGui::ColorEdit3("Diffuse Color", reinterpret_cast<float*>(&material->diffuseColor));
		ImGui::ColorEdit3("Specular Color", reinterpret_cast<float*>(&material->specularColor));
		ImGui::SliderFloat("Specularity", &material->specularity, 1, 128);
		if(!roughnessMapEnabled) ImGui::SliderFloat("Smoothness", &material->smoothness, 0, 1);
		ImGui::SliderFloat("Anisotropy", &material->anisotropy, 0, 1);

		ImGui::Text("Enabled Maps");
		ImGui::Columns(4);
		ImGui::Checkbox("Color", &colorMapEnabled); ImGui::NextColumn();
		ImGui::Checkbox("Normal", &normalMapEnabled); ImGui::NextColumn();
		ImGui::Checkbox("AO", &AOMapEnabled); ImGui::NextColumn();
		ImGui::Checkbox("Roughness", &roughnessMapEnabled); ImGui::NextColumn();
		material->textureFlags = ((colorMapEnabled) ? (int)TextureFlag::COLOR : 0)
									| ((normalMapEnabled) ? (int)TextureFlag::NORMAL : 0)
									| ((AOMapEnabled) ? (int)TextureFlag::AO : 0)
									| ((roughnessMapEnabled) ? (int)TextureFlag::ROUGHNESS : 0);
		ImGui::Columns(1);

		// Display maps
		if (ImGui::CollapsingHeader("View Maps")) {
			ImGui::Columns(2);
			ImGui::Text("Colour Map");
			ImGui::Image(reinterpret_cast<ImTextureID*>(material->colorMap), ImVec2(128, 128));
			ImGui::Text("Normal Map");
			ImGui::Image(reinterpret_cast<ImTextureID*>(material->normalMap), ImVec2(128, 128));
			ImGui::NextColumn();
			ImGui::Text("Ambient Occlusion Map");
			ImGui::Image(reinterpret_cast<ImTextureID*>(material->AOMap), ImVec2(128, 128));
			ImGui::Text("Roughness Map");
			ImGui::Image(reinterpret_cast<ImTextureID*>(material->roughnessMap), ImVec2(128, 128));
			ImGui::Columns(1);
		}
		ImGui::TreePop();
	}
}

void PBRShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	D3D11_BUFFER_DESC projectionBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC worldBufferDesc;
	
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC materialBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	// Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	// Setup all buffers

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

	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(PBRMaterialData);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&materialBufferDesc, NULL, &materialBuffer);

	materialBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	device->CreateBuffer(&materialBufferDesc, NULL, &dofPlaneBuffer);

	// Sampler for shadow map sampling
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&samplerDesc, &shadowSampler);
}
