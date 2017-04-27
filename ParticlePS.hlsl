struct V2F
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

float4 main(V2F input) : SV_TARGET
{
	float val =
		input.texcoord.x * input.texcoord.x +
		input.texcoord.y * input.texcoord.y;

	return float4(1, 0, 0, 1);// float4(val, 0, 0, 1.0f);
}