
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	//float4 color		: COLOR;
	float3 worldPos		: POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float4 Direction;
};

struct PointLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float4 PositionAndRadius;
};

cbuffer lightData : register(b0)
{
	DirectionalLight	directionalLights[32];
	PointLight			pointLights[32];
	int					directionalLightCount;
	int					pointLightCount;
}

Texture2D mainTexture : register(t0);
SamplerState mainSampler : register(s0);

float4 calcDirLight(int idx, float3 normal)
{
	DirectionalLight light = directionalLights[idx];
	float3 lightDir = normalize(light.Direction.xyz);
	float NdotL = saturate(dot(normal, lightDir));
	return light.AmbientColor + light.DiffuseColor * NdotL;
}

float4 calcPointLight(int idx, float3 worldPos, float3 normal)
{
	PointLight light = pointLights[idx];
	float3 lightDir = light.PositionAndRadius.xyz - worldPos;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);
	float NdotL = saturate(dot(normal, lightDir));
	float atten = saturate(1 - pow(dist / light.PositionAndRadius.w, 2));
	return light.AmbientColor + light.DiffuseColor * NdotL * atten;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering

	float4 texColor = mainTexture.Sample(mainSampler, input.uv);
	float4 finalColor = float4(0, 0, 0, 1);
	int i;

	for (i = 0; i < directionalLightCount; i++)
		finalColor.xyz += calcDirLight(i, input.normal).xyz;

	for (i = 0; i < pointLightCount; i++)
		finalColor.xyz += calcPointLight(i, input.worldPos, input.normal).xyz;

	return finalColor * texColor;
}