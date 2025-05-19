// Bloom
// Technique from (De Vries, 2014)

// Part 3, combine blurred bloomy layer and normal scene render.

// Scene render and blurred bloom texture
Texture2D sceneTexture : register(t0);
Texture2D bloomedTexture : register(t1);

// Sampler
SamplerState Sampler : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    // Combine the two layers with inverse source alpha blending
    float4 finalColor = bloomedTexture.Sample(Sampler, input.tex);
    finalColor += sceneTexture.Sample(Sampler, input.tex) * (1 - finalColor.a);
    
    // Return the output
    return finalColor;
}