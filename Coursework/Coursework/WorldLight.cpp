#include "WorldLight.h"
#include "imGUI/imgui.h"
#include <string>
#include "ShadowMap.h"

WorldLight::WorldLight()
{
	viewMatrices = new XMMATRIX[6];
	projectionMatrices = new XMMATRIX[6];

	constantAttenuation = 1;
	linearAttenuation = 0;
	quadraticAttenuation = 0;
}

void WorldLight::CreateShadowMaps(D3D* renderer)
{
	if(lightType == 0) directionalShadowMap = new ShadowMap(renderer->getDevice(), 8192, 8192);
	else tCubeShadowMap = new TextureCubeShadowMaps(renderer->getDevice(), 1024, 1024);
}

ShadowMap* WorldLight::GetDirectionalShadowMap()
{
	return directionalShadowMap;
}

TextureCubeShadowMaps* WorldLight::GetTCubeShadowMap()
{
	return tCubeShadowMap;
}

XMMATRIX WorldLight::GetViewMatrix(int index)
{
	return viewMatrices[index];
}

XMMATRIX WorldLight::GetProjMatrix(int index)
{
	return projectionMatrices[index];
}

void WorldLight::GenerateShadowMatrices()
{
	// If directional light, set position to be 100 away (viewing whole scene)
	// Set ortho matrix to be 200 units wide, (viewing whole scene)
	if (lightType == 0) {
		XMStoreFloat3(&direction, (XMVector3Normalize(XMLoadFloat3(&direction))));
		XMFLOAT3 newPos = XMFLOAT3(-direction.x * 100.0f, -direction.y * 100.0f, -direction.z * 100.0f);
		position = XMLoadFloat3(&newPos);
		generateOrthoMatrix(200, 200, 0.01, 200);
		generateViewMatrix();
		viewMatrices[0] = getViewMatrix();
		projectionMatrices[0] = getOrthoMatrix();
	}
	// If point or spot light, cube maps are being used, use perspective projection with 90DEG FOV
	// Keep a note of light direction before modifying it for matrices, its needed for spotlight
	if (lightType == 1 || lightType == 2) {
		XMFLOAT3 directionBefore = direction;
		direction = XMFLOAT3(1, 0, 0);
		generateProjectionMatrix(0.1, 200);
		generateViewMatrix();
		viewMatrices[0] = getViewMatrix();
		projectionMatrices[0] = getProjectionMatrix();
		direction = XMFLOAT3(-1, 0, 0);
		generateViewMatrix();
		viewMatrices[1] = getViewMatrix();
		projectionMatrices[1] = getProjectionMatrix();
		direction = XMFLOAT3(0, 1, 0);
		generateViewMatrix();
		viewMatrices[2] = getViewMatrix();
		projectionMatrices[2] = getProjectionMatrix();
		direction = XMFLOAT3(0, -1, 0);
		generateViewMatrix();
		viewMatrices[3] = getViewMatrix();
		projectionMatrices[3] = getProjectionMatrix();
		direction = XMFLOAT3(0, 0, 1);
		generateViewMatrix();
		viewMatrices[4] = getViewMatrix();
		projectionMatrices[4] = getProjectionMatrix();
		direction = XMFLOAT3(0, 0, -1);
		generateViewMatrix();
		viewMatrices[5] = getViewMatrix();
		projectionMatrices[5] = getProjectionMatrix();
		direction = directionBefore;
	}
}

void WorldLight::SetAttenuation(float constant, float linear, float quadratic)
{
	constantAttenuation = constant;
	linearAttenuation = linear;
	quadraticAttenuation = quadratic;
}

float WorldLight::GetConstantAttenuation()
{
	return constantAttenuation;
}

float WorldLight::GetLinearAttenuation()
{
	return linearAttenuation;
}

float WorldLight::GetQuadraticAttenuation()
{
	return quadraticAttenuation;
}

float WorldLight::GetLightPower()
{
	return lightPower;
}

void WorldLight::SetLightType(int type)
{
	lightType = type;
}

void WorldLight::SetLightPower(float power)
{
	this->lightPower = power;
}

int WorldLight::GetLightType()
{
	return lightType;
}

void WorldLight::SetSpotlightAngles(float innerCutoff, float outerCutoff)
{
	innerSpotlightCutoffAngle = innerCutoff;
	outerSpotlightCutoffAngle = outerCutoff;
}

float WorldLight::GetInnerSpotlightCutoffAngle()
{
	return innerSpotlightCutoffAngle;
}

float WorldLight::GetOuterSpotlightCutoffAngle()
{
	return outerSpotlightCutoffAngle;
}

void WorldLight::ShowGuiControls(const char* label)
{
	if (ImGui::TreeNode((std::string(label) + " Light").c_str())) {
		// Only show position if spot or point
		if (lightType == 1 || lightType == 2)ImGui::DragFloat3("Position", reinterpret_cast<float*>(&position), 0.1f);
		// Onlt show direction if directional or point
		if (lightType == 0 || lightType == 2)ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&direction), 0.1f);
		// Other paramaters
		ImGui::ColorEdit3("Diffuse Light Color", reinterpret_cast<float*>(&diffuseColour));
		ImGui::ColorEdit3("Ambient Light Color", reinterpret_cast<float*>(&ambientColour));
		ImGui::DragFloat("Power", &lightPower, 0.001, 0, 5);
		if (lightType == 2)ImGui::SliderFloat("Inner Cutoff", &innerSpotlightCutoffAngle, 0, 89);
		if (lightType == 2)ImGui::SliderFloat("Outer Cutoff", &outerSpotlightCutoffAngle, 0, 90);
		// Attenuation & Graph
		ImGui::Text("Attenuation");
		ImGui::DragFloat("Constant", &constantAttenuation, 0.001, 1, 100);
		ImGui::DragFloat("Linear", &linearAttenuation, 0.001, 0, 100);
		ImGui::DragFloat("Quadratic", &quadraticAttenuation, 0.001, 0, 100);
		float* data = new float[100];
		for (int i = 0; i < 100; i++) {
			data[i] = 1 / (constantAttenuation + linearAttenuation * i + quadraticAttenuation * i * i);
		}
		ImGui::PlotLines("10 Unit Graph", data, 100, 0, nullptr, 0, 1);

		ImGui::TreePop();
	}
}

WorldLight::~WorldLight()
{
}
