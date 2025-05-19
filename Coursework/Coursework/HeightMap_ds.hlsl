#include "Common.hlsli"

// Matrix buffers
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
// Height map buffer (we only care about amplitude at this point)
cbuffer HeightMapBuffer : register(b3)
{
    float amplitude;
    float2 worldSizeOfPlane;
    int isSmoothingOn;
};

// Height map texture and sampler
Texture2D heightMap : register(t0);
SamplerState heightMapSampler : register(s0);

struct ConstantOutputType
{
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
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
};

// Returns a value between -1 and 1 which is the height from the height map.
// Uses plain sample if not smoothing
// Uses smoothed sample with custom bilinear sampling if smoothing, See Common.hlsli
float GetHeightMapOffset(float2 texCoord)
{
    if (isSmoothingOn == 0)
        return (heightMap.SampleLevel(heightMapSampler, texCoord, 0).r * 2) - 1;
    return (SmoothedSample(heightMapSampler, heightMap, texCoord).r * 2) - 1;
}

// Set up to recieve quad information
[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
	OutputType output;
 
    // Determine the position of the new vertex.
	// Since using a quad use bilinear approach
	float3 vertexPosition = lerp(
        lerp(patch[1].position, patch[0].position, uvwCoord.y),
        lerp(patch[2].position, patch[3].position, uvwCoord.y),
        uvwCoord.x);
	
	// Do this for all properties of the vertex
	float2 textureCoordinate = lerp(
        lerp(patch[1].tex, patch[0].tex, uvwCoord.y),
        lerp(patch[2].tex, patch[3].tex, uvwCoord.y),
        uvwCoord.x);
	
	float3 normal = lerp(
        lerp(patch[1].normal, patch[0].normal, uvwCoord.y),
        lerp(patch[2].normal, patch[3].normal, uvwCoord.y),
        uvwCoord.x);
	
	float3 tangent = lerp(
        lerp(patch[1].tangent, patch[0].tangent, uvwCoord.y),
        lerp(patch[2].tangent, patch[3].tangent, uvwCoord.y),
        uvwCoord.x);
	
	float3 bitangent = lerp(
        lerp(patch[1].bitangent, patch[0].bitangent, uvwCoord.y),
        lerp(patch[2].bitangent, patch[3].bitangent, uvwCoord.y),
        uvwCoord.x);
    
	// Now continue with height mapping and projecting. 
	// This used to be in the vertex shader. 
	float4 newPosition = float4(vertexPosition, 1) + float4(normal * GetHeightMapOffset(textureCoordinate) * amplitude, 0);

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(worldMatrix, newPosition);
	output.position = mul(viewMatrix, output.position);
	output.position = mul(projectionMatrix, output.position);
    
    output.worldPosition = mul(worldMatrix, newPosition).xyz;

	// Store the texture coordinates for the pixel shader.
	output.tex = textureCoordinate;

    // Transform directional components. 
	output.normal = normalize(mul(float4(normal, 0), normalWorldMatrix)).xyz;
	output.tangent = normalize(mul(float4(tangent, 0), normalWorldMatrix)).xyz;
	output.bitangent = normalize(mul(float4(bitangent, 0), normalWorldMatrix)).xyz;

	return output;
}