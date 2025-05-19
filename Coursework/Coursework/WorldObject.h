#pragma once
#include <memory>
#include "DXF.h"

/// <summary>
/// World Object class
/// To make for easier positioning of meshes in scene. 
/// </summary>
class WorldObject
{
public:
	WorldObject();
	
	void SetRenderer(D3D* renderer); // Setter for renderer, must be done
	void SetShader(BaseShader* shader); // Setter for shader, must be done
	void SetMesh(BaseMesh* mesh); // Setter for mesh, must be done

	void SetPosition(DirectX::XMFLOAT3 position); // Setter for position
	void SetRotation(DirectX::XMFLOAT3 rotation); // Setter for rotation
	void SetScale(DirectX::XMFLOAT3 scale); // Setter for scale

	DirectX::XMFLOAT3 GetPosition(); // Getter for position
	DirectX::XMFLOAT3 GetRotation(); // Getter for rotation
	DirectX::XMFLOAT3 GetScale(); // Getter for scale
	DirectX::XMMATRIX GetWorldMatrix(); // Getter for world matrix

	/// <summary>
	/// Sends the mesh data to the GPU
	/// NOT USED, now in render
	/// </summary>
	//void SendMeshData();

	/// <summary>
	/// Renders the mesh using the shader set.
	/// Does not set any CB values!
	/// </summary>
	void Render(D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

private:
	// This objects transform components
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation; // In Euler Angles
	DirectX::XMFLOAT3 scale;

	// This object's specific world matrix
	DirectX::XMMATRIX worldMatrix;
	/// <summary>
	/// Refreshes the world matrix with latest transform data, should be done on any of the transform setters. 
	/// </summary>
	void RefreshWorldMatrix();

	// A pointer to this objects mesh (For now its 1 to 1)
	std::unique_ptr<BaseMesh> mesh;

	// A pointer to the renderer
	D3D* renderer;

	// A pointer to the shader
	// As base shader as only used for calling render, not used to send data to buffers.
	BaseShader* shader;
}; 

