#pragma once

#include <DirectXMath.h>

// assuming we don't know compilers default pack size

#pragma pack(push, 1)

struct DirectionalLight
{
	DirectX::XMFLOAT4	AmbientColor;
	DirectX::XMFLOAT4	DiffuseColor;
	DirectX::XMFLOAT3	Direction;
	float				_padding;
};


struct PointLight
{
	DirectX::XMFLOAT4	AmbientColor;
	DirectX::XMFLOAT4	DiffuseColor;
	DirectX::XMFLOAT4	PositionAndRadius;
};

#pragma pack(pop)
