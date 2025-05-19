// Vertex shader for Texture_ps and all post processing. 

cbuffer ProjectionBuffer : register(b0)
{
    matrix projectionMatrix;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

OutputType main(InputType input)
{
	OutputType output;

	// Project the vertex, we dont need the other this is assuming we are working with ortho meshes
    output.position = mul(projectionMatrix, input.position);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	return output;
}