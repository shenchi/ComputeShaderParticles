#pragma once

#include <DirectXMath.h>

#define MAX_EMITTERS 1024

struct ParticleSystemArgs
{
	float		deltaTime;
	uint32_t	emitterCount;
	uint32_t	maxParticles;
	uint32_t	numParticles;
};

struct Emitter
{
	DirectX::XMFLOAT4	position;	// w = 0.0 (initial age)
	DirectX::XMFLOAT4	velocity;	// w = life time
};

struct Particle
{
	DirectX::XMFLOAT4	position;	// w = age
	DirectX::XMFLOAT4	velocity;	// w = life time
};
