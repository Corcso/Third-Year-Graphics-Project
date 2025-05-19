// Depth of field
// Technique is Forward-Mapped Z-Buffer (Fernando, 2004)

#include "Common.hlsli"

// This shader does the horizontal and vertical blur

// All min and max depths, if we should blur on X or Y, and they layer this pixel is on
cbuffer DepthLayers : register(b0)
{
    float4 minMaxDepths[DOF_LAYER_COUNT]; // Z and W unused. 
    float blurOnX; // If 1 blur x, If 0 blur y.
    float thisLayer;
}

// Scene depth, for comparison with neighboring pixels
Texture2D sceneDepth : register(t0);
// The layer which we are bluring
Texture2D dofLayer : register(t1);
// Sampler
SamplerState Sampler : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

// Return the depth layer number of the texel provided. 
int GetDepthLayer(float2 tex) {
    float depth = sceneDepth.Sample(Sampler, tex).r;
    for (int depthLayer = 0; depthLayer < DOF_LAYER_COUNT; ++depthLayer)
    {
        if (depth <= minMaxDepths[depthLayer].y && depth >= minMaxDepths[depthLayer].x)
        {
            return depthLayer;
        }
    }

    return 0;
}

float4 main(InputType input) : SV_TARGET
{
    int width, height, unused;
    dofLayer.GetDimensions(0, width, height, unused);

    float4 finalColor = float4(0, 0, 0, 0);

    // Get the layer this pixel is on
    float ourLayer = GetDepthLayer(input.tex);
    
    if (ourLayer > thisLayer)
        return dofLayer.Sample(Sampler, input.tex);


    // Set the blur multiplier, this is the number of pixels are blurred per level from the sharpest. E.g 10 means on layer 7 (4 being sharpest) is 3 away so blur 30 pixels. 
    const int BLUR_MULTIPLIER = 2;

    // Calculate standard deviation for gaussian blur I found 0.4 to roughly be all on this pixel so use that for sharpest layer. Add 0.4 per layer's pixels 
    const int blurLayersOut = floor((float) DOF_LAYER_COUNT / 2.0f);
    float sDev = (abs(thisLayer - blurLayersOut) * BLUR_MULTIPLIER + 1) * 0.4;

    // Make an array of sample weights for this pixel, weights calculated with gaussian function (Wikipedia, no date a)
    const int numWeights = ceil((float) DOF_LAYER_COUNT / 2.0f);
    
    float weights[numWeights * BLUR_MULTIPLIER];
    float weightSum = 0; // This is used to correct the weights to make sure they = 1 when totaled. 
    for (int i = 0; i < numWeights * BLUR_MULTIPLIER; ++i)
    {
        weights[i] = (1.0f / sqrt(2 * 3.14 * sDev * sDev)) * exp(-((float)i * (float)i) / (2.0f * sDev * sDev));
        weightSum += weights[i];
        // Add twice if not the middle pixel
        if (i != 0)
            weightSum += weights[i];
    }

    // Now correct all weights so the total == 1
    float correctionFactor = 1.0f / weightSum;
    for (int j = 0; j < numWeights * BLUR_MULTIPLIER; ++j)
    {
        weights[j] *= correctionFactor;
    }
    // Add this pixel to the total 
    finalColor += dofLayer.Sample(Sampler, input.tex) * weights[0];
    
    // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
    // ============================
    // Store furthest positive and negative directional colours, for use when we are not blending. 
    //float4 furthestPositiveColor = dofLayer.Sample(Sampler, input.tex);
    //float furthestPositiveColorAt = 0;
    //float4 furthestNegativeColor = furthestPositiveColor;
    //float furthestNegativeColorAt = 0;
    //bool blendIn = false;
    //if (ourLayer > thisLayer) blendIn = true;
    // ============================ 
   
    // Loop the blur p is the pixel distance from origin
    [loop]
    for (int p = -abs(thisLayer - blurLayersOut) * BLUR_MULTIPLIER; p <= abs(thisLayer - blurLayersOut) * BLUR_MULTIPLIER; ++p)
    {
        // Dont do origin pixel (This one) we already did it above
        if (p != 0) {
            // If we are Y blurring
            if (blurOnX == 0) {
                // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
                // ============================
                // Compare our depth layers, if pixel to blur is below us and we are blending out, then blend
                // If pixel to blur is above us and we are blending in, then blend. Otherwise dont blend, using the furthest blended colour as an approximation
                //if ((GetDepthLayer(input.tex + float2(0, (1.0f / (float) width) * p)) <= thisLayer && !blendIn) ||
                //    (GetDepthLayer(input.tex + float2(0, (1.0f / (float)width) * p)) >= thisLayer && blendIn)) {
                //===============================
                    
                // Only blur this layer and below,
                if (GetDepthLayer(input.tex + float2(0, (1.0f / (float) width) * p)) <= thisLayer) {
                    // Add the blurred colour
                    finalColor += dofLayer.Sample(Sampler, input.tex + float2(0, (1.0f / (float)width) * p)) * weights[abs(p)];
                    // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
                    // ============================
                    // check if this pixel is our furthest blended colour if so replace what we already have there.
                    //if (p < 0 && furthestNegativeColorAt < abs(p)) {
                    //    furthestNegativeColor = dofLayer.Sample(Sampler, input.tex + float2(0, (1.0f / (float)width) * p));
                    //    furthestNegativeColorAt = abs(p);
                    //}
                    //else if (p > 0 && furthestPositiveColorAt < abs(p)) {
                    //    furthestPositiveColor = dofLayer.Sample(Sampler, input.tex + float2(0, (1.0f / (float)width) * p));
                    //    furthestPositiveColorAt = abs(p);
                    //}
                    // ============================
                    
                } 
                // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
                // ============================
                // If we are not blending take the furthest colour we blended and just add it with that weight. 
                // ============================
                
                // Use our colour if not bluring
                else
                {
                    finalColor += dofLayer.Sample(Sampler, input.tex) * weights[abs(p)];
                    // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
                    // ============================
                    //if (p < 0)
                    //    finalColor += furthestNegativeColor * weights[abs(p)];
                    //else
                    //    finalColor += furthestPositiveColor * weights[abs(p)];
                    // ============================
                }
            }
            // If we are X blurring
            else {
                // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
                // ============================
                // Compare our depth layers, if pixel to blur is below us and we are blending out, then blend
                // If pixel to blur is above us and we are blending in, then blend. Otherwise dont blend, using the furthest blended colour as an approximation
                //if ((GetDepthLayer(input.tex + float2((1.0f / (float)width) * p, 0)) <= thisLayer && !blendIn) || (GetDepthLayer(input.tex + float2((1.0f / (float)width) * p, 0)) >= thisLayer && blendIn)) {
                // ============================
                
                // Only blur this layer and below,
                if (GetDepthLayer(input.tex + float2((1.0f / (float)width) * p, 0)) <= thisLayer) {
                    // Add the blurred colour
                    finalColor += dofLayer.Sample(Sampler, input.tex + float2((1.0f / (float)width) * p, 0)) * weights[abs(p)];
                    // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
                    // ============================
                    // check if this pixel is our furthest blended colour if so replace what we already have there.
                    //if (p < 0 && furthestNegativeColorAt < abs(p)) {
                    //    furthestNegativeColor = dofLayer.Sample(Sampler, input.tex + float2((1.0f / (float)width) * p, 0));
                    //    furthestNegativeColorAt = abs(p);
                    //}
                    //else if (p > 0 && furthestPositiveColorAt < abs(p)) {
                    //    furthestPositiveColor = dofLayer.Sample(Sampler, input.tex + float2((1.0f / (float)width) * p, 0));
                    //    furthestPositiveColorAt = abs(p);
                    //}
                    // ============================
                }
                // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
                // ============================
                // If we are not blending take the furthest colour we blended and just add it with that weight. 
                // ============================
                
                // Use our colour if not bluring
                else
                {
                    finalColor += dofLayer.Sample(Sampler, input.tex) * weights[abs(p)];
                    // Old comments below for when whole scene render was used and objects occluded others removing the colour data behind them. 
                    // ============================
                    //if(p < 0)
                    //    finalColor += furthestNegativeColor * weights[abs(p)];
                    //else
                    //    finalColor += furthestPositiveColor * weights[abs(p)];
                    // ============================
                }
            }
        }
    }

    // Return the blurred colour
    return finalColor;
    //return float4((float) level / 5.0f, (float) level / 5.0f, (float) level / 5.0f, 1);
}