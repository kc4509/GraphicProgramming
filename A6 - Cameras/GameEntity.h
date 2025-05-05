#pragma once
#include "Transform.h"
#include <memory>

#include "Mesh.h"
#include "Camera.h"
class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

	void SetMesh(std::shared_ptr<Mesh> mesh);

	void gDraw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer, std::shared_ptr<Camera> camera);

private:
	std::shared_ptr<Transform> gTransform;
	std::shared_ptr<Mesh> gMesh;

};

