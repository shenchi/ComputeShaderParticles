#include "Particle.h"
#include "Emitter.h"
#include "Noise.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);

ConsumeStructuredBuffer<uint> deadList : register(u1);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x < deadParticles && DTid.x < emitCount)
	{
		uint pid = deadList.Consume();

		particles[pid].position = position;
		particles[pid].position.x += (curlNoise3D(particles[pid].position.xyz, totalTime).x % 10) / 100;
		particles[pid].position.z += (curlNoise3D(particles[pid].position.xyz, totalTime).z % 10) / 100;
		particles[pid].velocity = velocity;
	}
}
