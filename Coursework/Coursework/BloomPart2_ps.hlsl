// Bloom
// Technique from (De Vries, 2014)

// Part 2, Gaussian blur (Wikipedia, no date a) the bright areas

// Bloom Info.
cbuffer BloomInformation : register(b0)
{
    float brightnessAbove;
    int blurDistance;
    float blurSkips;
    int blurOnX;
}

static const int MAX_BLUR_DISTANCE = 31;

// Texture to be blurred
Texture2D bloomTexture : register(t0);
// Sampler
SamplerState Sampler : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    // Get input image dimentions
    int width, height, unused;
    bloomTexture.GetDimensions(0, width, height, unused);
    
    // Store the final colour for this pixel, start fully transparent
    float4 finalColor = float4(0, 0, 0, 0);
    
	// Calculate standard deviation for gaussian blur I found 0.4 to roughly be all on this pixel so use that for sharpest layer. Add 0.4 per layer's pixels 
    float sDev = (blurDistance + 1) * 0.4;

    // Make an array of sample weights for this pixel, weights calculated with gaussian function (Wikipedia, no date a)
    float weights[MAX_BLUR_DISTANCE];
    float weightSum = 0; // This is used to correct the weights to make sure they = 1 when totaled. 
    for (int i = 0; i < blurDistance + 1; ++i)
    {
        weights[i] = (1.0f / sqrt(2 * 3.14 * sDev * sDev)) * exp(-((float) i * (float) i) / (2.0f * sDev * sDev));
        weightSum += weights[i];
        // Add twice if not the middle pixel
        if (i != 0)
            weightSum += weights[i];
    }

    // Now correct all weights so the total == 1
    float correctionFactor = 1.0f / weightSum;
    for (int j = 0; j < blurDistance + 1; ++j)
    {
        weights[j] *= correctionFactor;
    }
    
    // Add this pixel to the total 
    finalColor += bloomTexture.Sample(Sampler, input.tex) * weights[0];
	
    for (int p = -blurDistance; p <= blurDistance; ++p)
    {
        // Dont do origin pixel (This one) we already did it above
        if (p != 0)
        {
            // If we are Y blurring
            if (blurOnX == 0)
            {
                finalColor += bloomTexture.Sample(Sampler, input.tex + float2(0, (blurSkips / (float) height) * p)) * weights[abs(p)];
            }
            // If we are X blurring
            else
            {
                // Add the blurred colour
                finalColor += bloomTexture.Sample(Sampler, input.tex + float2((blurSkips / (float) width) * p, 0)) * weights[abs(p)];
            }
        }
    }
    
    
    // Return final color
    return finalColor;
}