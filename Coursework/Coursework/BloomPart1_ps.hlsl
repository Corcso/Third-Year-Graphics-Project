// Bloom
// Technique from (De Vries, 2014)

// Part 1, Render only the parts of the scene with sufficient brightness

// Bloom Info.
cbuffer BloomInformation : register(b0)
{
    float luminosityThreshold;
}

// Scene render
Texture2D sceneTexture : register(t0);
// Sampler
SamplerState Sampler : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
	// Get the colour of this pixel, we only care about RGB, A is 1 as it is a rendered scene we are sampling. 
    float3 color = sceneTexture.Sample(Sampler, input.tex);
    // Calculate luminosity (Anonymous, 2009)
    float luminosity = (0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b);
    // If below threshold, discard
    if (luminosity < luminosityThreshold)
        discard;
	
    // Set alpha to 1 and return
    return float4(color, 1);
}