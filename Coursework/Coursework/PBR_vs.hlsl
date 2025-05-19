// PBR Vertex Shader
// Setup input for pixel shader and transform to screen space. 


// Projection, Camera and World Buffers
cbuffer ProjectionBuffer : register(b0)
{
	matrix projectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
    matrix viewMatrix;
    float3 cameraPosition;
};

cbuffer WorldBuffer : register(b2)
{
    matrix worldMatrix;
    matrix normalWorldMatrix;
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
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float3 worldPosition : POSITION;
    float3 cameraVector : CAMVECTOR;
};

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(worldMatrix, input.position);
	output.position = mul(viewMatrix, output.position);
	output.position = mul(projectionMatrix, output.position);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul((float3x3)worldMatrix, input.normal);
	output.normal = normalize(output.normal);

	// Do the same for tangent and bitangent
	output.tangent = mul((float3x3)worldMatrix, input.tangent);
	output.tangent = normalize(output.tangent);

	output.bitangent = mul((float3x3)worldMatrix, input.bitangent);
	output.bitangent = normalize(output.bitangent);

	output.worldPosition = mul(worldMatrix, input.position).xyz;
	
	// Calculate the view vector for this vertex
    output.cameraVector = output.worldPosition - cameraPosition;
	
	return output;
}