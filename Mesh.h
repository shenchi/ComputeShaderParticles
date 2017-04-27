#pragma once

#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

class Mesh
{
public:
	Mesh();

	Mesh(ID3D11Device* device,
		const void* vertices, int stride, int verticesNum,
		const int* indices = nullptr, int indicesNum = 0);

	Mesh(ID3D11Device* device, const char* filename);

	Mesh(const Mesh& mesh);

	~Mesh();

	Mesh& operator = (const Mesh& mesh);

	void Create(ID3D11Device* device,
		const void* vertices, int stride, int verticesNum,
		const int* indices = nullptr, int indicesNum = 0);

	void Create(ID3D11Device* device, const char* filename);

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();

	inline UINT GetStride() const { return stride; }
	inline UINT GetVertexCount() const { return verticesNum; }
	inline UINT GetIndexCount() const { return indicesNum; }

	void Release();

private:
	ID3D11Buffer*	vertexBuffer;
	ID3D11Buffer*	indexBuffer;

	UINT			stride;
	UINT			verticesNum;
	UINT			indicesNum;
};
