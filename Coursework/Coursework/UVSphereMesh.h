#pragma once
#include "BaseMesh.h"


/// <summary>
/// UV Sphere mesh
/// WITH Tangents & Bitangents
/// </summary>
class UVSphereMesh :
    public BaseMesh
{
public:
	UVSphereMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 20);
	~UVSphereMesh();

protected:
	void initBuffers(ID3D11Device* device);
	int resolution;
};

