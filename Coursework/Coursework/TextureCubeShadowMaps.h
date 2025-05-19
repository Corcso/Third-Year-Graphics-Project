#pragma once
#include "DXF.h"

/// <summary>
/// Adapted shadow map class for use with texture cubes for cube mapping point light shadows. 
/// </summary>
class TextureCubeShadowMaps : public ShadowMap
{
public:
	TextureCubeShadowMaps(ID3D11Device* device, int mWidth, int mHeight);

	void ClearDSV(ID3D11DeviceContext* dc);
	void BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc, int faceIndex);
protected:
	ID3D11DepthStencilView* mDepthMapDSVPX;
	ID3D11DepthStencilView* mDepthMapDSVNX;
	ID3D11DepthStencilView* mDepthMapDSVPY;
	ID3D11DepthStencilView* mDepthMapDSVNY;
	ID3D11DepthStencilView* mDepthMapDSVPZ;
	ID3D11DepthStencilView* mDepthMapDSVNZ;
};

