#include "TextureCubeShadowMaps.h"

TextureCubeShadowMaps::TextureCubeShadowMaps(ID3D11Device* device, int mWidth, int mHeight) : ShadowMap()
{
	// Use typeless format because the DSV is going to interpret
	// the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going to interpret
	// the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6; // A texture cube has 6 textures in its array, one for each face. 
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // Set texture as texture cube rather than normal texture

	//ID3D11Texture2D* depthMap = 0;
	device->CreateTexture2D(&texDesc, 0, &depthMap);

	// We need a seperate DSV for every face. 
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 6;
	dsvDesc.Texture2DArray.FirstArraySlice = 0;
	device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSVPX);
	dsvDesc.Texture2DArray.ArraySize = 5;
	dsvDesc.Texture2DArray.FirstArraySlice = 1;
	device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSVNX);
	dsvDesc.Texture2DArray.ArraySize = 4;
	dsvDesc.Texture2DArray.FirstArraySlice = 2;
	device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSVPY);
	dsvDesc.Texture2DArray.ArraySize = 3;
	dsvDesc.Texture2DArray.FirstArraySlice = 3;
	device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSVNY);
	dsvDesc.Texture2DArray.ArraySize = 2;
	dsvDesc.Texture2DArray.FirstArraySlice = 4;
	device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSVPZ);
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.FirstArraySlice = 5;
	device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSVNZ);

	// We only need one SRV, this knows that the resource is a texture cube and can pass into TextureCube type in hlsl
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2DArray.ArraySize = 6;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(depthMap, &srvDesc, &mDepthMapSRV);

	// Setup the viewport for rendering.
	viewport.Width = (float)mWidth;
	viewport.Height = (float)mHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	//NULL render target
	renderTargets[1] = { 0 };
}

void TextureCubeShadowMaps::ClearDSV(ID3D11DeviceContext* dc)
{
	dc->ClearDepthStencilView(mDepthMapDSVPX, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void TextureCubeShadowMaps::BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc, int faceIndex)
{
	dc->RSSetViewports(1, &viewport);

	// Set null render target because we are only going to draw to depth buffer.
	// Setting a null render target will disable color writes.
	ID3D11RenderTargetView* renderTargets[1] = { 0 };

	// Switch over each face
	switch (faceIndex) {
	case 0:
		dc->OMSetRenderTargets(1, renderTargets, mDepthMapDSVPX);
		//dc->ClearDepthStencilView(mDepthMapDSVPX, D3D11_CLEAR_DEPTH, 1.0f, 0);
		break;
	case 1:
		dc->OMSetRenderTargets(1, renderTargets, mDepthMapDSVNX);
		//dc->ClearDepthStencilView(mDepthMapDSVNX, D3D11_CLEAR_DEPTH, 1.0f, 0);
		break;
	case 2:
		dc->OMSetRenderTargets(1, renderTargets, mDepthMapDSVPY);
		//dc->ClearDepthStencilView(mDepthMapDSVPY, D3D11_CLEAR_DEPTH, 1.0f, 0);
		break;
	case 3:
		dc->OMSetRenderTargets(1, renderTargets, mDepthMapDSVNY);
		//dc->ClearDepthStencilView(mDepthMapDSVNY, D3D11_CLEAR_DEPTH, 1.0f, 0);
		break;
	case 4:
		dc->OMSetRenderTargets(1, renderTargets, mDepthMapDSVPZ);
		//dc->ClearDepthStencilView(mDepthMapDSVPZ, D3D11_CLEAR_DEPTH, 1.0f, 0);
		break;
	case 5:
		dc->OMSetRenderTargets(1, renderTargets, mDepthMapDSVNZ);
		//dc->ClearDepthStencilView(mDepthMapDSVNZ, D3D11_CLEAR_DEPTH, 1.0f, 0);
		break;
	}
}
