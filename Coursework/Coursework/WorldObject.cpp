#include "WorldObject.h"

WorldObject::WorldObject()
{
	// Set default transform
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);

	// World matrix for this is identity
	worldMatrix = XMMatrixIdentity();
}

void WorldObject::SetRenderer(D3D* renderer)
{
	this->renderer = renderer;
}

void WorldObject::SetShader(BaseShader* shader)
{
	this->shader = shader;
}

void WorldObject::SetMesh(BaseMesh* mesh)
{
	// Delete old mesh (if there was one) and add new one
	if (this->mesh.get() != nullptr) this->mesh.release();
	this->mesh.reset(mesh);
}

void WorldObject::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
	RefreshWorldMatrix();
}

void WorldObject::SetRotation(DirectX::XMFLOAT3 rotation)
{
	this->rotation = rotation;
	RefreshWorldMatrix();
}

void WorldObject::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
	RefreshWorldMatrix();
}

DirectX::XMFLOAT3 WorldObject::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 WorldObject::GetRotation()
{
	return rotation;
}

DirectX::XMFLOAT3 WorldObject::GetScale()
{
	return scale;
}

DirectX::XMMATRIX WorldObject::GetWorldMatrix()
{
	return worldMatrix;
}

void WorldObject::Render(D3D_PRIMITIVE_TOPOLOGY topology)
{
	mesh->sendData(renderer->getDeviceContext(), topology);
	shader->render(renderer->getDeviceContext(), mesh->getIndexCount());
}

void WorldObject::RefreshWorldMatrix()
{
	worldMatrix = XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) * XMMatrixTranslation(position.x, position.y, position.z);
}

