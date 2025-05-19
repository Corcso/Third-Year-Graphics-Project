// Depth of field
// Technique is Forward-Mapped Z-Buffer (Fernando, 2004)

// NOT USED
// NOT USED
// NOT USED
// NOT USED
// NOT USED

// Min and max depth of the current layer.
cbuffer DepthLayer : register(b0)
{
	float maxDepth;
	float minDepth;
}

// Scene render
Texture2D sceneTexture : register(t0);
// Scene render depth buffer
Texture2D depthTexture : register(t1);
// Sampler
SamplerState Sampler : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
	// If we are not on this depth layer, discard.
	float depth = depthTexture.Sample(Sampler, input.tex);
	if (depth > maxDepth || depth <= minDepth)
		discard;
	
	// Set alpha to 1, we need to do this for part 2 to fix luminosity. 
	return float4(sceneTexture.Sample(Sampler, input.tex).rgb, 1);
    //return float4((float) level / 5.0f, (float) level / 5.0f, (float) level / 5.0f, 1);
}