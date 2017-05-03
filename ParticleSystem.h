#pragma once

#include <d3d11.h>
#include "SimpleShader.h"
#include "ParticlePool.h"

#include <string>
#include <unordered_map>
#include <vector>

class ParticleSystem
{
public:
	ParticleSystem(ID3D11Device* device, ID3D11DeviceContext* context)
		:
		device(device),
		context(context),
		particleVS(nullptr),
		particlePS(nullptr),
		particleCS(nullptr),
		particleEmitterCS(nullptr)
	{}

	bool Init();

	bool CleanUp();

private:
	bool InitParticlePool(uint32_t maxParticles, const std::wstring& texFileName);

private:
	ID3D11Device*					device;
	ID3D11DeviceContext*			context;

	SimpleVertexShader*				particleVS;
	SimplePixelShader*				particlePS;
	SimpleComputeShader*			particleInitCS;
	SimpleComputeShader*			particleEmitterCS;
	SimpleComputeShader*			particleCS;

	ID3D11Buffer*					bufQuadIndices;
	ID3D11Buffer*					bufIndirectDrawArgs;
	ID3D11Buffer*					bufEmitter;

	typedef std::unordered_map<std::wstring, uint32_t> map_t;
	map_t							poolMap;

	std::vector<ParticlePool>		pools;
};