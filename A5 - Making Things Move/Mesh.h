#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Vertex.h"


class Mesh
{

public:
	Mesh(size_t indiceCount,size_t verticeCount, Vertex* verticeArr, unsigned int* indicesArr, const char* name);

	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	unsigned int GetVertexCount();
	const char* GetName();
	void Draw();

	
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	unsigned int indicesCount;
	unsigned int verticesCount;
	const char* name;
};

