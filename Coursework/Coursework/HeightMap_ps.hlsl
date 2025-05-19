
#include "Common.hlsli"

// Take in height map
Texture2D heightMap : register(t0);
// Terain texture (not used at the moment)
Texture2D terrainColor : register(t1);

// Shadow maps
TextureCube shadowMaps[8] : register(t2);
Texture2D directionalShadowMaps[8] : register(t10);

// And sampler states for each 
SamplerState heightMapSampler : register(s0);
SamplerState textureSampler : register(s1);
SamplerState shadowSampler : register(s2);

// Light buffer
cbuffer LightBuffer : register(b0)
{
    LightData lights[8]; // Support 8 lights max
    int lightCount; // LightData ends on a block of 16 fully used bytes so this wont be packed
};

// Height map buffer
cbuffer HeightMapBuffer : register(b1)
{
    float amplitude;
    float2 worldSizeOfPlane;
    int isSmoothingOn;
};

// DOF discarding
cbuffer DepthOfFieldDiscardRange : register(b2)
{
    float2 minMaxDepth;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 worldPosition : POSITION;
};



// Returns a value between -1 and 1 which is the height from the height map.
// Always regularly sample, smoothed sample is too expensive for a per pixel level
float GetHeightMapOffset(float2 texCoord)
{
    //if (isSmoothingOn == 0) 
    return (heightMap.Sample(heightMapSampler, texCoord).r * 2) - 1;
    //return (SmoothedSample(heightMapSampler, heightMap, texCoord).r * 2) - 1;
}

// A number which determines how far the texels are when calculating the normal. Higher numbers stop pixelation.
// Due to the quality of the current height map, 1 can be used. 
static const float HEIGHTMAP_BLEND_AMOUNT = 1;

// Calculates the tangent based off of the rate of change
float3 CalculateTangent(float2 texCoord)
{
    // Get dimentions of image
    float height;
    float width;
    float numLevels;
    heightMap.GetDimensions(0, width, height, numLevels);
    // Get HEIGHTMAP_BLEND_AMOUNT texel length in UV space and world space, we only care about U for Tangent
    float uvStepU = HEIGHTMAP_BLEND_AMOUNT / width;
    float worldStepU = uvStepU * worldSizeOfPlane.x;
    // Calculate the rate of change from our left to our right along U
    float rateOfChange = ((-GetHeightMapOffset(texCoord - float2(uvStepU, 0)) * amplitude) + (GetHeightMapOffset(texCoord + float2(uvStepU, 0)) * amplitude));
    // Rate of change is along 2 steps. 
    return normalize(float3(2 * worldStepU, rateOfChange, 0));
}

// Calculates the bitangent based off the rate of change
float3 CalculateBitangent(float2 texCoord)
{
    // Get dimentions of image
    float height;
    float width;
    float numLevels;
    heightMap.GetDimensions(0, width, height, numLevels);
    // Get HEIGHTMAP_BLEND_AMOUNT texel length in UV space and world space, we only care about V for Bitangent
    float uvStepV = HEIGHTMAP_BLEND_AMOUNT / height;
    float worldStepV = uvStepV * worldSizeOfPlane.y;
    // Calculate the rate of change from our up to down along V
    float rateOfChange = ((-GetHeightMapOffset(texCoord - float2(0, uvStepV)) * amplitude) + (GetHeightMapOffset(texCoord + float2(0, uvStepV)) * amplitude));
    // Rate of change is along 2 steps. 
    return normalize(float3(0, rateOfChange, 2 * worldStepV));
}

// Calculates the normal based off the rate of change
float3 CalculateNormal(float2 texCoord)
{
    // Return tangent cross bitangent
    return normalize(cross(CalculateBitangent(texCoord), CalculateTangent(texCoord)));
}

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 lightColor, float3 tangent)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(lightColor * intensity);
    return colour;
}

float4 main(InputType input) : SV_TARGET
{
    DiscardForDOF(minMaxDepth, input.position.z);
    
    // Get heightmap actual normal
    float3 heightMapCalculatedNormal = CalculateNormal(input.tex); ///*float3(0, 1, 0) +*/ normalize(-cross(tangent, bitangent));

    // Sample the texture and set up base light color (black no lights applied) Specular seperate as applied on top of the texture
    float4 ambientAndDiffuseLightColor = float4(0, 0, 0, 1);

    // For each light calculate lighting
    for (int i = 0; i < lightCount; i++)
    {
        // Get distance for attenuation
        float distanceToLight = length(lights[i].position - input.worldPosition);

        // Get light vector, for point and spotlights its the lights direction to the light position, for directional lights its just light direction
        float3 lightVector = normalize(lights[i].position - input.worldPosition);
        if (lights[i].lightType == 0)
            lightVector = normalize(-lights[i].direction);
        
        // Calculate the spotlight factor
        float spotlightFactor = 1;
        if (lights[i].lightType == 2)
            spotlightFactor = calculateSpotlightPower(-lightVector, normalize(lights[i].direction), lights[i].innerSpotlightCutoffAngle, lights[i].outerSpotlightCutoffAngle);
        
        // Calculate ambient 
        float4 localLightColor = lights[i].ambient * lights[i].lightPower;
        
        if (!IsInShadow(lights[i], input.worldPosition, -normalize(lights[i].position - input.worldPosition), i, directionalShadowMaps[i], shadowMaps[i], shadowSampler))
        {
            // Add diffuse light
            localLightColor += calculateLighting(lightVector, heightMapCalculatedNormal, lights[i].diffuse, input.bitangent) * lights[i].lightPower * spotlightFactor;
        }
        
        // Attenuate light (Add attenuation variables to light buffer input)
        // Since ambient is being attenuated we must still attenuate in shadow. 
        localLightColor *= 1 / (lights[i].constantAttenuation + lights[i].linearAttenuation * distanceToLight + lights[i].quadraticAttenuation * distanceToLight * distanceToLight);
        
        // Add this lights complete color to the overall light color for this fragment/pixel
        ambientAndDiffuseLightColor += localLightColor;
    }
    
    
	float4 textureColour;
    
	// Sample the texture for ground colour.
    textureColour = terrainColor.Sample(textureSampler, input.tex);
    
    
    // Set colour based on height
    //if (GetHeightMapOffset(input.tex) < -0.8f)
    //    textureColour = float4(0, 0, 1, 1);
    //else if (GetHeightMapOffset(input.tex) < -0.5f)
    //    textureColour = float4(1, 1, 0, 1);
    //else if (GetHeightMapOffset(input.tex) < 0.0f)
    //    textureColour = float4(0, 1, 0, 1);
    //else if (GetHeightMapOffset(input.tex) < 0.2f)
    //    textureColour = float4(0.5, 0.5, 0.5, 1);
    //else
    //    textureColour = float4(0.8, 0.8, 0.8, 1);
	
    
    // Direct normal output for debug
    //return float4((heightMapCalculatedNormal / 2) + 0.5, 1);
    
    return float4((ambientAndDiffuseLightColor * textureColour).rgb, 1);
}



