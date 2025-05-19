#pragma once
#include "DXF.h"
// Adapted PlaneMesh to be stored as 4 point patches
class TessPlaneMesh :
    public BaseMesh
{
public:
	TessPlaneMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 100);
	~TessPlaneMesh();

protected:
	void initBuffers(ID3D11Device* device);
	int resolution;
};

