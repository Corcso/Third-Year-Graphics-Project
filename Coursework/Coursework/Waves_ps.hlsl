// Include common functions
#include "Common.hlsli"

// Shadow maps
TextureCube shadowMaps[8] : register(t0);
Texture2D directionalShadowMaps[8] : register(t8);

// And sampler states for each 
SamplerState shadowSampler : register(s0);

// Light buffer
cbuffer LightBuffer : register(b0)
{
    LightData lights[8]; // Support 8 lights max
    int lightCount; // LightData ends on a block of 16 fully used bytes so this wont be packed
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

cbuffer WavesBuffer : register(b1)
{
    Wave waves[3];
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
    float2 positionForNormalGeneration : NORMGENPOS;
    float3 cameraVector : CAMVECTOR;
};

// Calculates the normal based off the partial derivative of the position function (Fernando, 2004)
float3 CalculateNormal(float2 worldPos)
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
                waves[w].direction.x * waves[w].frequency * waves[w].amplitude * cos(waves[w].frequency * dot(waves[w].direction, worldPos) + waves[w].speed * waves[w].time),
                qValue * waves[w].frequency * waves[w].amplitude * sin(waves[w].frequency * dot(waves[w].direction, worldPos) + waves[w].speed * waves[w].time),
                waves[w].direction.y * waves[w].frequency * waves[w].amplitude * cos(waves[w].frequency * dot(waves[w].direction, worldPos) + waves[w].speed * waves[w].time)
                );
    }
    
    // Calculate rest of equation with sums and return normal
    return normalize(
    float3(
        -sums.x,
        1 - sums.y,
        -sums.z
    )
    );
}

// Get the wave height between -1 and 1. Disregarding amplitude
float GetNormalizedWaveHeight(float2 worldPos)
{
    float sums = 0;
    
    for (int w = 0; w < 3; ++w)
    {
        sums += sin(waves[w].frequency * dot(waves[w].direction, worldPos) + waves[w].speed * waves[w].time);
    }
    
    return (sums / 3.0f + 1.0f) / 2.0f;
}

// Calculate diffuse lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 lightColor, float3 tangent)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(lightColor * intensity);
    return colour;
}

// Calculate specularity, no anisotropy
float calculateSpecularPower(float3 lightDirection, float3 normal, float3 viewVector, float specularPower)
{
    // Flip view vector
    viewVector *= -1;

    // Calculate the isotropic specularity
    float3 halfway = normalize(lightDirection + viewVector);
    float blinnPhong = pow(max(dot(normal, halfway), 0), specularPower);
    
    return blinnPhong;
}

float4 main(InputType input) : SV_TARGET
{
    // Do depth of field discard if any
    DiscardForDOF(minMaxDepth, input.position.z);
    
    // Setup base waves colour
    float4 wavesColor = float4(0.2, 0.4, 0.8, 1);
    
    // Get waves actual normal
    input.normal = CalculateNormal(input.positionForNormalGeneration);

    // Sample the texture and set up base light color (black no lights applied) Specular seperate as applied on top of the texture
    float4 ambientAndDiffuseLightColor = float4(0, 0, 0, 1);
    float4 specularColor = float4(0, 0, 0, 0);
    
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
            localLightColor += calculateLighting(lightVector, input.normal, lights[i].diffuse, input.bitangent) * lights[i].lightPower * spotlightFactor;
            
            // Add and attenuate specular light
            specularColor += calculateSpecularPower(lightVector, input.normal, input.cameraVector, 64.0f) * lights[i].lightPower * spotlightFactor * lights[i].diffuse
            / (lights[i].constantAttenuation + lights[i].linearAttenuation * distanceToLight + lights[i].quadraticAttenuation * distanceToLight * distanceToLight);
            
        }
        
        // Attenuate light (Add attenuation variables to light buffer input)
        // Since ambient is being attenuated we must still attenuate in shadow. 
        localLightColor *= 1 / (lights[i].constantAttenuation + lights[i].linearAttenuation * distanceToLight + lights[i].quadraticAttenuation * distanceToLight * distanceToLight);
        
        // Add this lights complete color to the overall light color for this fragment/pixel
        ambientAndDiffuseLightColor += localLightColor;
    }
    
    // Non amplitude wave height, wave height between 0 and 1
    float normalizedWaveHeight = GetNormalizedWaveHeight(input.positionForNormalGeneration);
    float whiteLerpFactor = min(pow(normalizedWaveHeight * min(waves[0].steepness + waves[1].steepness + waves[2].steepness, 3) / 3.0f, 2), 1);
    // Lerp wave colour between itself and white depending on how high the wave is
    float4 waveDiffuse = lerp(
        float4((ambientAndDiffuseLightColor * wavesColor).rgb, lerp(0.9, 0.8, normalizedWaveHeight)), 
        float4(1, 1, 1, 1),
        whiteLerpFactor
    );
    
    // Direct normal output for debug
    //return float4((input.normal / 2) + 0.5, 1);
    
    // Output wave colour + specular, making sure to cap Alpha at 1
    return float4(waveDiffuse.rgb + specularColor.rgb, min(waveDiffuse.a + specularColor.a, 1));
}



