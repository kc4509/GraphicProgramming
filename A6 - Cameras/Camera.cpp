#include "Camera.h"
#include "Input.h"

using namespace DirectX;


Camera::Camera(
	DirectX::XMFLOAT3 position,
	float movementSpeed,
	float mouseLookSpeed,
	float fieldOfView,
	float aspectRatio,
	float nearClip,
	float farClip,
	CameraProjectionType projType) :
	movementSpeed(movementSpeed),
	mouseLookSpeed(mouseLookSpeed),
	fieldOfView(fieldOfView),
	aspectRatio(aspectRatio),
	nearClip(nearClip),
	farClip(farClip),
	projectionType(projType),
	orthographicWidth(10.0f)
{
	transform = std::make_shared<Transform>();
	transform->SetPosition(position);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{ }


void Camera::Update(float dt)
{
	float speed = dt * movementSpeed;

	if (Input::KeyDown(VK_SHIFT)) { speed *= 5; }
	if (Input::KeyDown(VK_CONTROL)) { speed *= 0.1f; }

	// Movement
	if (Input::KeyDown('W')) { transform->MoveRelative(0, 0, speed); }
	if (Input::KeyDown('S')) { transform->MoveRelative(0, 0, -speed); }
	if (Input::KeyDown('A')) { transform->MoveRelative(-speed, 0, 0); }
	if (Input::KeyDown('D')) { transform->MoveRelative(speed, 0, 0); }
	if (Input::KeyDown('X')) { transform->MoveAbsolute(0, -speed, 0); }
	if (Input::KeyDown(' ')) { transform->MoveAbsolute(0, speed, 0); }

	if (Input::MouseLeftDown())
	{

		float xDiff = mouseLookSpeed * Input::GetMouseXDelta();
		float yDiff = mouseLookSpeed * Input::GetMouseYDelta();
		transform->Rotate(yDiff, xDiff, 0);

		XMFLOAT3 rot = transform->GetPitchYawRoll();
		if (rot.x > XM_PIDIV2) rot.x = XM_PIDIV2;
		if (rot.x < -XM_PIDIV2) rot.x = -XM_PIDIV2;
		transform->SetRotation(rot);
	}

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 direction = transform->GetForward();
	XMFLOAT3 position = transform->GetPosition();

	XMMATRIX view = XMMatrixLookToLH(
		XMLoadFloat3(&position),
		XMLoadFloat3(&direction),
		XMVectorSet(0, 1, 0, 0)); 
	XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	this->aspectRatio = aspectRatio;

	XMMATRIX P;

	if (projectionType == CameraProjectionType::Perspective)
	{
		P = XMMatrixPerspectiveFovLH(
			fieldOfView,		
			aspectRatio,		
			nearClip,			
			farClip);			
	}
	else 
	{
		P = XMMatrixOrthographicLH(
			orthographicWidth,	
			orthographicWidth / aspectRatio,
			nearClip,		
			farClip);			
	}

	XMStoreFloat4x4(&projMatrix, P);
}

DirectX::XMFLOAT4X4 Camera::GetView() { return viewMatrix; }
DirectX::XMFLOAT4X4 Camera::GetProjection() { return projMatrix; }
std::shared_ptr<Transform> Camera::GetTransform() { return transform; }

float Camera::GetNearClip()
{
	return nearClip;
}

float Camera::GetFarClip()
{
	return farClip;
}

float Camera::GetOrthographicWidth()
{
	return orthographicWidth;
}

CameraProjectionType Camera::GetProjectionType()
{
	return projectionType;
}

float Camera::GetFieldOfView()
{
	return fieldOfView;
}

void Camera::SetFieldOfView(float fov)
{
	fieldOfView = fov;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetOrthographicWidth(float width)
{
	orthographicWidth = width;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetFarClip(float distance)
{
	farClip = distance;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetNearClip(float distance)
{
	nearClip = distance;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetProjectionType(CameraProjectionType type)
{
	projectionType = type;
	UpdateProjectionMatrix(aspectRatio);
}


