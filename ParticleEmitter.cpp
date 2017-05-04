#include "ParticleEmitter.h"

#include "ParticlePool.h"
#include "ParticleSystem.h"

ParticleEmitter::ParticleEmitter(ParticleSystem * ps, uint32_t poolIdx, uint32_t emitterIdx)
	:
	ps(ps),
	poolIdx(poolIdx),
	emitterIdx(emitterIdx)
{
	auto& pool = ps->pools[poolIdx];
	auto& emitter = pool.emitters[emitterIdx];

	emitter.counter = 0.0f;
	emitter.deadParticles = 0;
	emitter.emitCount = 0;
	emitter.emitRate = 0.0f;
	emitter.position = DirectX::XMFLOAT4();
	emitter.velocity = DirectX::XMFLOAT4();
	emitter.totalTime = 0.0f;

}

void ParticleEmitter::SetParameters(DirectX::XMFLOAT3 & position, DirectX::XMFLOAT3 & velocity, float lifeTime, float emitRate)
{
	auto& pool = ps->pools[poolIdx];
	auto& emitter = pool.emitters[emitterIdx];
	
	emitter.position = DirectX::XMFLOAT4(
		position.x,
		position.y,
		position.z,
		0
	);

	emitter.velocity = DirectX::XMFLOAT4(
		velocity.x,
		velocity.y,
		velocity.z,
		lifeTime
	);

	emitter.emitRate = emitRate;
}
