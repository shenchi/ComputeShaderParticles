#include "Particle.h"
#include "Emitter.h"

cbuffer GPUConstants : register(b0)
{
	uint	deadParticles;
	uint3	_padding;
}


RWStructuredBuffer<Particle> particles : register(u0);

ConsumeStructuredBuffer<uint> deadList : register(u1);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x < deadParticles && DTid.x < emitCount)
		return;

	uint pid = deadList.Consume();

	particles[pid].position = position;
	particles[pid].velocity = velocity;
}
