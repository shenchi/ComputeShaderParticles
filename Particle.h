#if defined(__cplusplus)

#pragma once
#include <DirectXMath.h>
typedef DirectX::XMFLOAT4	float4;
typedef uint32_t			uint;

#define CBUFFER				struct
#define REGISTER(x)			/* empty */

#else

#define CBUFFER				cbuffer
#define REGISTER(x)			: register(x)

#endif

#define MAX_EMITTERS 1024

CBUFFER ParticleSystemArgs REGISTER(b0)
{
	float		deltaTime;
	uint		emitterCount;
	uint		maxParticles;
	uint		numParticles;
};

CBUFFER Emitter REGISTER(b2)
{
	float4		position;	// w = 0.0 (initial age)
	float4		velocity;	// w = life time
};

struct Particle
{
	float4		position;	// w = age
	float4		velocity;	// w = life time
};
