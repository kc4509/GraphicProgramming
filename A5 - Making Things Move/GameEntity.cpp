#include "GameEntity.h"
#include "BufferStructs.h"
#include "Graphics.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh):
    gMesh(mesh)
{
    gTransform = std::make_shared<Transform>();
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
    return gMesh;
}

std::shared_ptr<Transform> GameEntity::GetTransform()
{
    return gTransform;
}

void GameEntity::SetMesh(std::shared_ptr<Mesh> sMesh) { gMesh = sMesh; }


void GameEntity::gDraw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer)
{
	VertexShaderData vsData = {};
	vsData.colorTint = DirectX::XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.world = gTransform->GetWorldMatrix();

	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	Graphics::Context->Unmap(vsConstantBuffer.Get(), 0);
	gMesh->Draw();
}