#pragma once

#include <d3d11.h>

#include "SimpleShader.h"

class Material
{
public:
	Material(SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11ShaderResourceView* tex, ID3D11SamplerState* sampler)
		:
		vertexShader(vs),
		pixelShader(ps),
		textureSRV(tex),
		sampler(sampler)
	{
		if (nullptr != textureSRV) textureSRV->AddRef();
		if (nullptr != this->sampler) this->sampler->AddRef();
	}

	Material(const Material&) = delete;

	~Material()
	{
		if (nullptr != textureSRV) textureSRV->Release();
		if (nullptr != sampler) sampler->Release();
	}

	void SetVertexShader(SimpleVertexShader* vs) { vertexShader = vs; }
	SimpleVertexShader* GetVertexShader() const { return vertexShader; }

	void SetPixelShader(SimplePixelShader* ps) { pixelShader = ps; }
	SimplePixelShader* GetPixelShader() const { return pixelShader; }

	void SetTexture(ID3D11ShaderResourceView* srv) 
	{
		if (nullptr != textureSRV) textureSRV->Release();
		textureSRV = srv;
		if (nullptr != textureSRV) textureSRV->AddRef();
	}
	ID3D11ShaderResourceView* GetTexture() const { return textureSRV; }

	void SetSampler(ID3D11SamplerState* s) 
	{
		if (nullptr != sampler) sampler->Release();
		sampler = s;
		if (nullptr != sampler) sampler->AddRef();
	}
	ID3D11SamplerState* GetSampler() const { return sampler; }

private:
	SimpleVertexShader*		vertexShader;
	SimplePixelShader*		pixelShader;

	ID3D11ShaderResourceView*	textureSRV;
	ID3D11SamplerState*			sampler;
};