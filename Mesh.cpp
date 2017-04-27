#include "Mesh.h"

#include "Vertex.h"

#include <fstream>
#include <vector>
#include <DirectXMath.h>

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT2;

Mesh::Mesh()
	:
	vertexBuffer(nullptr),
	indexBuffer(nullptr),
	stride(0),
	verticesNum(0),
	indicesNum(0)
{
}

Mesh::Mesh(ID3D11Device * device, const void * vertices, int stride, int verticesNum, const int * indices, int indicesNum)
	:
	Mesh()
{
	Create(device, vertices, stride, verticesNum, indices, indicesNum);
}

Mesh::Mesh(ID3D11Device * device, const char * filename)
	:
	Mesh()
{
	Create(device, filename);
}

Mesh::Mesh(const Mesh & mesh)
	:
	vertexBuffer(mesh.vertexBuffer),
	indexBuffer(mesh.indexBuffer),
	stride(mesh.stride),
	verticesNum(mesh.verticesNum),
	indicesNum(mesh.indicesNum)
{
	if (nullptr != vertexBuffer) vertexBuffer->AddRef();
	if (nullptr != indexBuffer) indexBuffer->AddRef();
}

Mesh::~Mesh()
{
	Release();
}

Mesh & Mesh::operator=(const Mesh & mesh)
{
	if (&mesh == this)
		return *this;

	Release();

	vertexBuffer = mesh.vertexBuffer;
	indexBuffer = mesh.indexBuffer;
	stride = mesh.stride;
	verticesNum = mesh.verticesNum;
	indicesNum = mesh.indicesNum;

	if (nullptr != vertexBuffer) vertexBuffer->AddRef();
	if (nullptr != indexBuffer) indexBuffer->AddRef();

	return *this;
}

void Mesh::Create(ID3D11Device * device, const void * vertices, int stride, int verticesNum, const int * indices, int indicesNum)
{
	Release();

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = stride * verticesNum;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA dataDesc = {};
	dataDesc.pSysMem = vertices;

	if (S_OK != device->CreateBuffer(&bufferDesc, &dataDesc, &vertexBuffer))
	{
		return;
	}

	this->stride = stride;
	this->verticesNum = verticesNum;

	if (nullptr == indices)	return;

	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(int) * indicesNum;

	dataDesc.pSysMem = reinterpret_cast<const void*>(indices);

	if (S_OK != device->CreateBuffer(&bufferDesc, &dataDesc, &indexBuffer))
	{
		Release();
		return;
	}

	this->indicesNum = indicesNum;
}

void Mesh::Create(ID3D11Device* device, const char* filename)
{
	// File input object
	std::ifstream obj(filename);

	// Check for successful open
	if (!obj.is_open())
		return;

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading

										 // Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			unsigned int i[12];
			int facesRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Flip Z (LH vs. RH)
			v1.Position.z *= -1.0f;
			v2.Position.z *= -1.0f;
			v3.Position.z *= -1.0f;

			// Flip normal Z
			v1.Normal.z *= -1.0f;
			v2.Normal.z *= -1.0f;
			v3.Normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);

			// Add three more indices
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;

			// Was there a 4th face?
			if (facesRead == 12)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal
				v4.UV.y = 1.0f - v4.UV.y;
				v4.Position.z *= -1.0f;
				v4.Normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);

				// Add three more indices
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();

	Create(device, 
		reinterpret_cast<const void*>(verts.data()),
		sizeof(Vertex), 
		verts.size(), 
		reinterpret_cast<const int*>(indices.data()), 
		indices.size());
}

ID3D11Buffer * Mesh::GetVertexBuffer()
{
	if (nullptr != vertexBuffer) vertexBuffer->AddRef();
	return vertexBuffer;
}

ID3D11Buffer * Mesh::GetIndexBuffer()
{
	if (nullptr != indexBuffer) indexBuffer->AddRef();
	return indexBuffer;
}

void Mesh::Release()
{
	if (nullptr != vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}
	if (nullptr != indexBuffer)
	{
		indexBuffer->Release();
		indexBuffer = nullptr;
	}
}
