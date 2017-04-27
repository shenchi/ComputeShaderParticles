struct Particle
{
	float4 position;  // w = age
	float4 velocity;  // w = life time;
};

RWStructuredBuffer<Particle> particles : register(u0);

AppendStructuredBuffer<uint> deadList : register(u1);

AppendStructuredBuffer<uint> drawList : register(u2);

cbuffer Constants : register(b0)
{
	float	deltaTime;
	uint	emitterCount;
	uint	maxParticles;
	uint	numParticles;
}

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x >= maxParticles || particles[DTid.x].velocity.w < 0.01)
		return;

	particles[DTid.x].position.w += deltaTime;

	if (particles[DTid.x].position.w > particles[DTid.x].velocity.w)
	{
		particles[DTid.x].velocity.w = 0;
		deadList.Append(DTid.x + 1);
		return;
	}

	particles[DTid.x].position.xyz += particles[DTid.x].velocity.xyz * deltaTime;
	drawList.Append(DTid.x);
}