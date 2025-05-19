// Vertex shader for height map
// Now just passthrough as all maniuplation happens on the domain shader

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
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

OutputType main(InputType input)
{
	OutputType output;

    // Hull shader passthrough
    output.position = input.position;
    output.tex = input.tex;
    output.normal = input.normal;
    output.tangent = input.tangent;
    output.bitangent = input.bitangent;

	return output;
}