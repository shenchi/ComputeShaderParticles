#pragma once

#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4X4;

class Entity
{
public:
	Entity(const Mesh& mesh, Material* material)
		:
		translation(0.0f, 0.0f, 0.0f),
		rotation(0.0f, 0.0f, 0.0f),
		scale(1.0f, 1.0f, 1.0f),
		worldMatrix(),
		dirty(false),
		mesh(mesh),
		material(material)
	{
		DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
	}

	// translation

	void SetTranslation(const XMFLOAT3& t) { translation = t; dirty = true; }

	void SetTranslation(float x, float y, float z) { SetTranslation(XMFLOAT3(x, y, z)); }

	const XMFLOAT3& GetTranslation() const { return translation; }

	// rotation

	void SetRotation(const XMFLOAT3& r) { rotation = r; dirty = true; }

	void SetRotation(float x, float y, float z) { SetRotation(XMFLOAT3(x, y, z)); }

	const XMFLOAT3& GetRotation() const { return rotation; }

	// scale

	void SetScale(const XMFLOAT3& s) { scale = s; dirty = true; }

	void SetScale(float s) { SetScale(XMFLOAT3(s, s, s)); };

	void SetScale(float x, float y, float z) { SetScale(XMFLOAT3(x, y, z)); }

	const XMFLOAT3& GetScale() const { return scale; }

	// world matrix

	const XMFLOAT4X4& GetWorldMatrix();

	// mesh

	Mesh& GetMesh() { return mesh; }

	// material

	void PrepareMaterial(const XMFLOAT4X4& viewMatrix, const XMFLOAT4X4 projMatrix);

	Material* GetMaterial() { return material; }

private:
	XMFLOAT3		translation;
	XMFLOAT3		rotation;
	XMFLOAT3		scale;

	XMFLOAT4X4		worldMatrix;

	bool			dirty;

	// class Mesh follows RAII
	Mesh			mesh;

	// TODO: we cannot ref count SimpleVertexShader in class Material now
	Material*		material;
};

