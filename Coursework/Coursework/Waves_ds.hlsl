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

// Wave data buffer
struct Wave
{
    float time;
    float amplitude;
    float frequency;
    float speed;
    float2 direction;
    float steepness;
};

cbuffer WavesBuffer : register(b3)
{
    Wave waves[3];
};

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
    float2 positionForNormalGeneration : NORMGENPOS;
    float3 cameraVector : CAMVECTOR;
};

// Calculate gerstner wave position based on inputted position (Fernando, 2004)
float3 CalculateGerstnerWavePosition(float3 position)
{
    // Value for summation of wave properties
    float3 sums = float3(0, 0, 0);
    
    for (int w = 0; w < 3; ++w)
    {
        // Calculate actual steepness Q value
        // This allows user steepness to be 0 - 1 and actual steepness q value to be between 0 and 1 / (frequency * amplitude)
        float qValue = (1.0f / (waves[w].frequency * waves[w].amplitude * 3)) * waves[w].steepness;
        
        // Sum individual wave calculations
        sums += float3(
                (qValue * waves[w].amplitude * waves[w].direction.x * cos(waves[w].frequency * dot(waves[w].direction, position.xz) + waves[w].speed * waves[w].time)),
                waves[w].amplitude * sin(waves[w].frequency * dot(waves[w].direction, position.xz) + waves[w].speed * waves[w].time),
                (qValue * waves[w].amplitude * waves[w].direction.y * cos(waves[w].frequency * dot(waves[w].direction, position.xz) + waves[w].speed * waves[w].time))
                );
    }
    
    // Calculate rest of equation with sums and return position
    return float3(
    position.x + sums.x,
    sums.y,
    position.z + sums.z
    );
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
    
	// Now calculate the wave position based off of the mesh position.
    float4 newPosition = float4(CalculateGerstnerWavePosition(vertexPosition), 1);

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(worldMatrix, newPosition);
    output.position = mul(viewMatrix, output.position);
    output.position = mul(projectionMatrix, output.position);
    
    output.worldPosition = mul(worldMatrix, newPosition).xyz;

	// Store the texture coordinates for the pixel shader.
    output.tex = textureCoordinate;

    // Transform directional components. 
    output.normal = normalize(mul(normal, (float3x3) normalWorldMatrix)).xyz;
    output.tangent = normalize(mul(tangent, (float3x3) normalWorldMatrix)).xyz;
    output.bitangent = normalize(mul(bitangent, (float3x3) normalWorldMatrix)).xyz;
    
    // Calculate the view vector for this vertex
    output.cameraVector = normalize(output.worldPosition - cameraPosition);
    
    // Setup position for use in normal generation, since wave position is based on local mesh position that needs used for normal calculation
    // Only pass X and Z as that is all we care about
    output.positionForNormalGeneration = vertexPosition.xz;

    return output;
}