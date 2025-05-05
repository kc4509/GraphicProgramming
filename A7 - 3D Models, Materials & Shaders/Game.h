#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	void LoadShaders();
	void CreateGeometry();
	bool windowOpen;
	void BuildUI();

	std::vector<std::shared_ptr<GameEntity>> entities;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//constant buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	std::vector<std::shared_ptr<Mesh>> meshes;

	// Camera for the 3D scene
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Camera> cameraTwo;
};


