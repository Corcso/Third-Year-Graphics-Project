// Depth of field
// Technique is Reverse-Mapped Z-Buffer (Fernando, 2004)

// NOT USED
// NOT USED
// NOT USED
// NOT USED
// NOT USED

Texture2D sceneTexture : register(t0);
Texture2D depthTexture : register(t1);
SamplerState Sampler0 : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

static const float dofplane = 0.995f;

int CalculateDepthLevel(float2 tex)
{
    float depth = depthTexture.Sample(Sampler0, tex).r;
    
    float depthDistance = depth - dofplane;
    
    int level = clamp(floor(depthDistance * 2500), 0, 10) * 3;
    
    return level;
}


float4 main(InputType input) : SV_TARGET
{
    int width, height, unused;
    sceneTexture.GetDimensions(0, width, height, unused);
    
    int thisPixelLevel = CalculateDepthLevel(input.tex);
    float sDev = (abs(thisPixelLevel) + 1) * 0.4;
    
    float4 textureColor = sceneTexture.Sample(Sampler0, input.tex) * (1.0f / sqrt(2 * 3.14 * sDev * sDev));
    
    // Make an array of sample weights for this pixel 
    float weights[30];
    float weightSum = 0; // This is used to correct the weights to make sure they = 1 when totaled. 
    for (int i = 0; i < 30; ++i)
    {
        weights[i] = (1.0f / sqrt(2 * 3.14 * sDev * sDev)) * exp(-((float) i * (float) i) / (2.0f * sDev * sDev));
        weightSum += weights[i];
        // Add twice if not the middle pixel
        if (i != 0)
            weightSum += weights[i];
    }
    // Now correct all weights so the total == 1
    float correctionFactor = 1.0f / weightSum;
    for (int j = 0; j < 30; ++j)
    {
        weights[j] *= correctionFactor;
    }
    
    for (int k = 1; k <= abs(thisPixelLevel); ++k)
    {
        if (CalculateDepthLevel(input.tex - float2((1.0f / (float) width) * k, 0)) >= thisPixelLevel)
            textureColor += sceneTexture.Sample(Sampler0, input.tex - float2((1.0f / (float) width) * k, 0)) * weights[k];
        else
            textureColor += sceneTexture.Sample(Sampler0, input.tex) * weights[k];
        if (CalculateDepthLevel(input.tex + float2((1.0f / (float) width) * k, 0)) >= thisPixelLevel)
            textureColor += sceneTexture.Sample(Sampler0, input.tex + float2((1.0f / (float) width) * k, 0)) * weights[k];
        else
            textureColor += sceneTexture.Sample(Sampler0, input.tex) * weights[k];

    }
    
    return textureColor;
    //return float4((float) level / 5.0f, (float) level / 5.0f, (float) level / 5.0f, 1);
}