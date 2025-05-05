#pragma once
#include <DirectXMath.h>

#include "Transform.h"
#include <memory>
#include <string>

enum class CameraProjectionType
{
	Perspective,
	Orthographic,
	Third
};


class Camera
{
public:
	Camera(
		DirectX::XMFLOAT3 position,
		float movementSpeed,
		float mouseLookSpeed,
		float fieldOfView,
		float aspectRatio,
		float nearClip = 0.01f,
		float farClip = 100.0f,
		CameraProjectionType projType = CameraProjectionType::Perspective);

	~Camera();

	// Updating methods
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	// Getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	std::shared_ptr<Transform> GetTransform();
	float GetNearClip();
	float GetFarClip();
	float GetOrthographicWidth();
	CameraProjectionType GetProjectionType();
	float GetFieldOfView();
	std::string GetName();

	//Setters
	void SetFieldOfView(float fov);
	void SetOrthographicWidth(float width);
	void SetFarClip(float distance);
	void SetNearClip(float distance);
	void SetProjectionType(CameraProjectionType type);


private:
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	std::shared_ptr<Transform> transform;

	float fieldOfView;
	float aspectRatio;
	float nearClip;
	float farClip;
	float orthographicWidth;
	float movementSpeed;
	float mouseLookSpeed;

	CameraProjectionType projectionType;
};
