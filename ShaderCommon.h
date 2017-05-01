#ifndef _SHADER_COMMON_

#define _SHADER_COMMON_

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

#endif