#pragma once
#include "DXF.h"

// Camera data buffer struct
struct CameraBufferData {
	XMMATRIX viewMatrix;
	XMFLOAT4 cameraPosition;
};

// World data buffer struct
struct WorldBufferData {
	XMMATRIX worldMatrix;
	XMMATRIX normalWorldMatrix;
};


// Singular light struct
struct LightData {
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT3A position;
	XMFLOAT3 direction;
	int lightType;
	XMMATRIX lightViewMatrix[6];
	XMMATRIX lightProjectionMatrix;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float lightPower;
	float innerSpotlightCutoffAngle;
	float outerSpotlightCutoffAngle;
	float p_0;
	float p_1;
};

// Light buffer struct
struct LightBufferData {
	LightData lights[8];
	int lightCount;
};