// Depth of field
// Technique is Forward-Mapped Z-Buffer (Fernando, 2004)

#include "Common.hlsli"

// This shader (Part3) combines all the layers into one output. 

// All blurred layers
Texture2D dofLayers[DOF_LAYER_COUNT] : register(t0);
// The sampler
SamplerState Sampler : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    // Start with no colour
    float4 finalColor = float4(0, 0, 0, 0);
    
    // Blend each layer, starting with the closest(last) to furthest, use additive blending but with 1 - source alpha
    for (int layer = DOF_LAYER_COUNT - 1; layer >= 0; --layer)
    {
        finalColor += dofLayers[layer].Sample(Sampler, input.tex) * (1 - finalColor.a);
    
    }

    // Correct brightness
    finalColor *= 1.0f / finalColor.a;

    return finalColor;
}