#include "ShaderCommon.h"

#define MAX_EMITTERS 1024

CBUFFER Emitter REGISTER(b0)
{
	float4		position;	// w = 0.0 (initial age)
	float4		velocity;	// w = life time
	uint		emitCount;
	uint		deadParticles;
	float		emitRate;	// particles per second
	float		counter;	//
};