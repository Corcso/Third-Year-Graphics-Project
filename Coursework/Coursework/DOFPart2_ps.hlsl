// Depth of field
// Technique is Forward-Mapped Z-Buffer (Fernando, 2004)

// NOT USED
// NOT USED
// NOT USED
// NOT USED
// NOT USED

cbuffer DepthLayers : register(b0)
{
    float4 minMaxDepths[9]; // Z and W unused. 
}

Texture2D sceneDepth : register(t0);
Texture2D dofLayers[9] : register(t1);
SamplerState Sampler : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

int GetDepthLayer(float2 tex) {
    float depth = sceneDepth.Sample(Sampler, tex).r;
    for (int depthLayer = 0; depthLayer < 9; ++depthLayer)
    {
        if (depth < minMaxDepths[depthLayer].y && depth >= minMaxDepths[depthLayer].x)
        {
            return depthLayer;
        }
    }

    return 0;
}

float4 main(InputType input) : SV_TARGET
{
    int width, height, unused;
    dofLayers[0].GetDimensions(0, width, height, unused);
    
    float4 finalColor = float4(0, 0, 0, 0);
    
    // We only want to blend with the layers more in focus than us!
    // Calcualte what layer we are to start on. 
    float ourLayer = GetDepthLayer(input.tex);
    
    
    for (int layer = 0; layer < 9; ++layer)
    {
        const int BLUR_MULTIPLIER = 1;
        float sDev = (abs(layer - 4) * BLUR_MULTIPLIER + 1) * 0.4;
        //float sDev = (5 * BLUR_MULTIPLIER + 1) * 0.4;
        
    
        //float4 textureColor = dofLayers[layer].Sample(Sampler, input.tex) * (1.0f / sqrt(2 * 3.14 * sDev * sDev));
    
        // Make an array of sample weights for this pixel 
        float weights[5 * BLUR_MULTIPLIER];
        float weightSum = 0; // This is used to correct the weights to make sure they = 1 when totaled. 
        for (int i = 0; i < 5 * BLUR_MULTIPLIER; ++i)
        {
            weights[i] = (1.0f / sqrt(2 * 3.14 * sDev * sDev)) * exp(-((float) i * (float) i) / (2.0f * sDev * sDev));
            weightSum += weights[i];
        // Add twice if not the middle pixel
            if (i != 0)
                weightSum += weights[i];
        }
        // Now correct all weights so the total == 1
        float correctionFactor = 1.0f / weightSum;
        for (int j = 0; j < 5 * BLUR_MULTIPLIER; ++j)
        {
            weights[j] *= correctionFactor;
        }
        
        finalColor += dofLayers[layer].Sample(Sampler, input.tex) * weights[0];
        for (int y = -abs(layer - 4) * BLUR_MULTIPLIER; y <= abs(layer - 4) * BLUR_MULTIPLIER; ++y)
        {
            for (int x = -abs(layer - 4) * BLUR_MULTIPLIER; x <= abs(layer - 4) * BLUR_MULTIPLIER; ++x)
            {
                //if (true) {
                if (GetDepthLayer(input.tex + float2((1.0f / (float)width) * x, (1.0f / (float)width) * y)) >= ourLayer)
                    finalColor += dofLayers[layer].Sample(Sampler, input.tex + float2((1.0f / (float)width) * x, (1.0f / (float)width) * y)) *weights[abs(x)] * weights[abs(y)];
                else
                    finalColor += dofLayers[layer].Sample(Sampler, input.tex) * weights[abs(x)] * weights[abs(y)];
                //}
                //
                //if (GetDepthLayer(input.tex + float2((1.0f / (float)width) * x, 0)) >= ourLayer)
                //    finalColor += dofLayers[layer].Sample(Sampler, input.tex + float2((1.0f / (float)width) * x, 0)) * weights[x];
                //else
                //    finalColor += dofLayers[layer].Sample(Sampler, input.tex) * weights[x];
            }
        }
        
    }
    
    // Correct brightness
    finalColor *= 1.0f / finalColor.a;
    
    return finalColor;
    //return float4((float) level / 5.0f, (float) level / 5.0f, (float) level / 5.0f, 1);
}