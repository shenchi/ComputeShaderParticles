#include "Entity.h"

using namespace DirectX;

const XMFLOAT4X4& Entity::GetWorldMatrix()
{
	if (dirty)
	{
		XMStoreFloat4x4(&worldMatrix,
			XMMatrixTranspose(
				XMMatrixMultiply(
					XMMatrixMultiply(
						XMMatrixScalingFromVector(XMLoadFloat3(&scale)),
						XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))
					),
					XMMatrixTranslationFromVector(XMLoadFloat3(&translation))
				)
			)
		);

		dirty = false;
	}

	return worldMatrix;
}

void Entity::PrepareMaterial(const XMFLOAT4X4& viewMatrix, const XMFLOAT4X4 projMatrix)
{
	auto vs = material->GetVertexShader();

	const XMFLOAT4X4& worldMatrix = GetWorldMatrix();
	XMFLOAT4X4 inverseTransposedWorldMatrix;
	XMStoreFloat4x4(
		&inverseTransposedWorldMatrix,
		XMMatrixTranspose(
			XMMatrixInverse(nullptr, XMLoadFloat4x4(&worldMatrix))
		)
	);

	vs->SetMatrix4x4("world", worldMatrix);
	vs->SetMatrix4x4("view", viewMatrix);
	vs->SetMatrix4x4("projection", projMatrix);
	vs->SetMatrix4x4("world_it", inverseTransposedWorldMatrix);
	vs->CopyAllBufferData();
	vs->SetShader();

	auto ps = material->GetPixelShader();
	ps->CopyAllBufferData();
	ps->SetShaderResourceView("mainTexture", material->GetTexture());
	ps->SetSamplerState("mainSampler", material->GetSampler());
	ps->SetShader();
}
