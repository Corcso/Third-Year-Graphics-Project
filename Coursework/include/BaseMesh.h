/**
* \class Base Mesh
*
* \brief The parent for all mesh objects. Provides default functionality.
*
* Can be inherited to create custom meshes. Provide functions for sending data to GPU memory, getting index count and storing geometry data.
*
* \author Paul Robertson
*/


#ifndef _BASEMESH_H_
#define _BASEMESH_H_

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

class BaseMesh
{
protected:

	/// Default struct for general vertex data include position, texture coordinates and normals
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 bitangent;

		bool operator== (const VertexType& other) {
			return (
				position.x == other.position.x && position.y == other.position.y && position.z == other.position.z &&
				texture.x == other.texture.x && texture.y == other.texture.y &&
				normal.x == other.normal.x && normal.y == other.normal.y && normal.z == other.normal.z &&
				tangent.x == other.tangent.x && tangent.y == other.tangent.y && tangent.z == other.tangent.z &&
				bitangent.x == other.bitangent.x && bitangent.y == other.bitangent.y && bitangent.z == other.bitangent.z
				);
		}
	};

	/// Default vertex struct for geometry with only position and colour
	struct VertexType_Colour
	{
		XMFLOAT3 position;
		XMFLOAT4 colour;
	};

	/// Default vertex struct for geometry with only position and texture coordinates.
	struct VertexType_Texture
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	/// Empty constructor
	BaseMesh();
	~BaseMesh();

	/// Transfers mesh data to the GPU.
	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	int getIndexCount();			///< Returns total index value of the mesh
	//D3D11_INPUT_ELEMENT_DESC getInputLayout();

protected:
	virtual void initBuffers(ID3D11Device*) = 0;

	ID3D11Buffer *vertexBuffer, *indexBuffer;
	//D3D11_INPUT_ELEMENT_DESC *inputLayout;
	int vertexCount, indexCount;
};

#endif