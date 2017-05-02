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

		particles[pid].position = float4(snoise(float2(position.x, totalTime)), snoise(float2(position.y, totalTime)), snoise(float2(position.z, totalTime)), position.w);
		particles[pid].velocity = velocity;
	}
}
