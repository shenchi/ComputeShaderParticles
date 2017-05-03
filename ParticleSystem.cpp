#include "ParticleSystem.h"
#include "Particle.h"
#include "Emitter.h"

#include <WICTextureLoader.h>

bool ParticleSystem::Init()
{
	HRESULT hr = S_OK;

	particleInitCS = new SimpleComputeShader(device, context);
	assert(particleInitCS->LoadShaderFile(L"Assets/Shaders/ParticleInitCS.cso"));

	particleCS = new SimpleComputeShader(device, context);
	assert(particleCS->LoadShaderFile(L"Assets/Shaders/ParticleCS.cso"));

	particleEmitterCS = new SimpleComputeShader(device, context);
	assert(particleEmitterCS->LoadShaderFile(L"Assets/Shaders/ParticleEmitterCS.cso"));

	particleVS = new SimpleVertexShader(device, context);
	assert(particleVS->LoadShaderFile(L"Assets/Shaders/ParticleVS.cso"));

	particlePS = new SimplePixelShader(device, context);
	assert(particlePS->LoadShaderFile(L"Assets/Shaders/ParticlePS.cso"));

	{
		CD3D11_BUFFER_DESC indicesDesc(
			sizeof(uint32_t) * 6,
			D3D11_BIND_INDEX_BUFFER,
			D3D11_USAGE_IMMUTABLE
		);

		uint32_t indices[] = { 0, 2, 3, 0, 3, 1 };

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = indices;

		hr = device->CreateBuffer(&indicesDesc, &data, &bufQuadIndices);
		assert(hr == S_OK);
	}

	{
		CD3D11_BUFFER_DESC indDrawDesc(
			sizeof(uint32_t) * 8, // 5, but align with 16 bytes
			D3D11_BIND_UNORDERED_ACCESS,
			D3D11_USAGE_DEFAULT,
			0,
			D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS
		);

		uint32_t args[] = { 6, 0, 0, 0, 0, 0, 0, 0 };

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = args;

		hr = device->CreateBuffer(&indDrawDesc, &data, &bufIndirectDrawArgs);
		assert(hr == S_OK);
	}


	CD3D11_BUFFER_DESC bufEmittersDesc
	(
		sizeof(Emitter),
		D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

	hr = device->CreateBuffer(&bufEmittersDesc, nullptr, &bufEmitter);
	assert(hr == S_OK);

	return true;
}

bool ParticleSystem::CleanUp()
{
	return false;
}

bool ParticleSystem::InitParticlePool(uint32_t maxParticles, const std::wstring& texFileName)
{
	ParticlePool pool;
	pool.particleConstants.maxParticles = maxParticles;

	HRESULT hr = S_OK;

	CD3D11_BUFFER_DESC cbDesc(sizeof(pool.particleConstants), D3D11_BIND_CONSTANT_BUFFER);
	hr = device->CreateBuffer(&cbDesc, nullptr, &(pool.bufParticleConstants));
	assert(hr == S_OK);

	CD3D11_BUFFER_DESC bufDesc(
		pool.particleConstants.maxParticles * sizeof(Particle),
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
		D3D11_USAGE_DEFAULT,
		0,
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		sizeof(Particle)
	);

	hr = device->CreateBuffer(&bufDesc, nullptr, &pool.bufParticles);
	assert(hr == S_OK);

	bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufDesc.ByteWidth = pool.particleConstants.maxParticles * sizeof(uint32_t);
	bufDesc.StructureByteStride = sizeof(uint32_t);

	hr = device->CreateBuffer(&bufDesc, nullptr, &pool.bufDeadList);
	assert(hr == S_OK);

	bufDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

	hr = device->CreateBuffer(&bufDesc, nullptr, &pool.bufDrawList);
	assert(hr == S_OK);

	CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(
		(ID3D11Buffer*)nullptr,
		DXGI_FORMAT_UNKNOWN,
		0, pool.particleConstants.maxParticles,
		D3D11_BUFFER_UAV_FLAG_APPEND
	);

	hr = device->CreateUnorderedAccessView(pool.bufParticles, nullptr, &pool.bufParticlesUAV);
	assert(hr == S_OK);

	hr = device->CreateShaderResourceView(pool.bufParticles, nullptr, &pool.bufParticlesSRV);
	assert(hr == S_OK);

	hr = device->CreateUnorderedAccessView(pool.bufDeadList, nullptr, &pool.bufDeadListUAV);
	assert(hr == S_OK);

	hr = device->CreateUnorderedAccessView(pool.bufDrawList, &uavDesc, &pool.bufDrawListUAV);
	assert(hr == S_OK);

	hr = device->CreateShaderResourceView(pool.bufDrawList, nullptr, &pool.bufDrawListSRV);
	assert(hr == S_OK);

	particleInitCS->SetUnorderedAccessView("particles", pool.bufParticlesUAV);
	particleInitCS->SetUnorderedAccessView("deadList", pool.bufDeadListUAV);
	particleInitCS->SetShader();
	context->Dispatch((pool.particleConstants.maxParticles + 1023) / 1024, 1, 1);
	{
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		uint32_t val = D3D11_KEEP_UNORDERED_ACCESS_VIEWS;
		context->CSSetUnorderedAccessViews(0, 1, &nullUAV, &val);
	}
	context->CSSetShader(nullptr, nullptr, 0);

	pool.bufDeadListUAV->Release();

	hr = device->CreateUnorderedAccessView(pool.bufDeadList, &uavDesc, &pool.bufDeadListUAV);
	assert(hr == S_OK);

	hr = DirectX::CreateWICTextureFromFile(device, texFileName.c_str(), nullptr, &pool.texSRV);
	assert(hr == S_OK);

	pool.particleFirstUpdate = true;

	

	return false;
}
