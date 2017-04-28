
struct Emitter
{
	float4 position;	// w = 0.0 (initial age)
	float4 velocity;	// w = life time
};

struct Particle
{
	float4 position;	// w = age
	float4 velocity;	// w = life time;
};

cbuffer CPUConstants : register(b0)
{
	float	deltaTime;
	uint	maxParticles;
}

cbuffer GPUConstants : register(b1)
{
	uint	deadParticles;
	uint3	_padding;
}

cbuffer Emitter : register(b2)
{

}

RWStructuredBuffer<Particle> particles : register(u0);

ConsumeStructuredBuffer<uint> deadList : register(u1);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x >= emitterCount)
		return;

	uint pid = deadList.Consume();
	if (pid == 0)
		return;

	particles[pid - 1].position = emitters[DTid.x].position;
	particles[pid - 1].velocity = emitters[DTid.x].velocity;
}
