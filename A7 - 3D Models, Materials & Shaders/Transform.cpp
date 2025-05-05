#include "Transform.h"

using namespace DirectX;


Transform::Transform() :
	position(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1),
	up(0, 1, 0),
	right(1, 0, 0),
	forward(0, 0, 1)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	position.x += offset.x;
	position.y += offset.y;
	position.z += offset.z;
}

void Transform::MoveRelative(float x, float y, float z)
{
	XMVECTOR movement = XMVectorSet(x, y, z, 0);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMVECTOR dir = XMVector3Rotate(movement, rotQuat);


	XMStoreFloat3(&position, XMLoadFloat3(&position) + dir);
}

void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	MoveRelative(offset.x, offset.y, offset.z);
}

void Transform::Rotate(float p, float y, float r)
{
	rotation.x += p;
	rotation.y += y;
	rotation.z += r;
}

void Transform::Rotate(DirectX::XMFLOAT3 pitchYawRoll)
{
	rotation.x += pitchYawRoll.x;
	rotation.y += pitchYawRoll.y;
	rotation.z += pitchYawRoll.z;
}

void Transform::Scale(float uniformScale)
{
	scale.x *= uniformScale;
	scale.y *= uniformScale;
	scale.z *= uniformScale;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	this->scale.x *= scale.x;
	this->scale.y *= scale.y;
	this->scale.z *= scale.z;
}

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
}

void Transform::SetRotation(float p, float y, float r)
{
	rotation.x = p;
	rotation.y = y;
	rotation.z = r;
}

void Transform::SetRotation(DirectX::XMFLOAT3 pitchYawRoll)
{
	rotation = pitchYawRoll;
}

void Transform::SetScale(float uniformScale)
{
	scale.x = uniformScale;
	scale.y = uniformScale;
	scale.z = uniformScale;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
}

void Transform::SetTransformsFromMatrix(DirectX::XMFLOAT4X4 worldMatrix)
{
	XMVECTOR localPos;
	XMVECTOR localRotQuat;
	XMVECTOR localScale;
	XMMatrixDecompose(&localScale, &localRotQuat, &localPos, XMLoadFloat4x4(&worldMatrix));

	XMStoreFloat3(&position, localPos);
	XMStoreFloat3(&scale, localScale);
}


DirectX::XMFLOAT3 Transform::GetPosition() { return position; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return rotation; }
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }

DirectX::XMFLOAT3 Transform::GetUp()
{
	UpdateVectors();
	return up;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	UpdateVectors();
	return right;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	UpdateVectors();
	return forward;
}


void Transform::UpdateVectors()
{
	// Update all three vectors
	XMVECTOR rotationQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMStoreFloat3(&up, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rotationQuat));
	XMStoreFloat3(&right, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), rotationQuat));
	XMStoreFloat3(&forward, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rotationQuat));
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateMatrices();
	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateMatrices();
	return worldMatrix;
}

void Transform::UpdateMatrices()
{

	XMMATRIX trans = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMMATRIX sc = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

	XMMATRIX wm = sc * rot * trans;
	XMStoreFloat4x4(&worldMatrix, wm);
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(wm)));
}