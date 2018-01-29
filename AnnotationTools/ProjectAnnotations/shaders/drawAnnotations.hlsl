
cbuffer ConstantBuffer : register(b0)
{
    matrix worldViewProj;
    float4 modelColor;
}


struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	nointerpolation float4 color : TEXCOORD0;
};

VertexShaderOutput vertexShaderMain(
	float4 position : position,
    float3 normal : normal,
    float4 color : color,
    float2 texCoord : texCoord)
{
	VertexShaderOutput output;
	output.position = mul(position, worldViewProj);
	output.color = color;
	return output;
}


float4 pixelShaderMain(VertexShaderOutput input) : SV_Target
{
	//return float4(input.color.xyz, 1.0f);
	return input.color;
}
