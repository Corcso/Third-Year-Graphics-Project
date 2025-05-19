// PBR pixel shader

#include "Common.hlsli"

// Maps SRVS
Texture2D colorMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D AOMap : register(t2);
Texture2D roughnessMap : register(t3);

// Shadow Map SRVS
TextureCube shadowMaps[8] : register(t4);
Texture2D directionalShadowMaps[8] : register(t12);

// Texture and shadow samplers
SamplerState textureSampler : register(s0);
SamplerState shadowSampler : register(s1);

// Material buffer
cbuffer MaterialBuffer : register(b0)
{
    float4 diffuseColor;
    float4 specularColor;
    float specularity;
    float smoothness;
    float anisotropy;
    int textureFlags;
};

// Since ENUMS are not supported, use static const ints
static const int TEX_FLAG_COLOR = 0x1;
static const int TEX_FLAG_NORMAL = 0x2;
static const int TEX_FLAG_AO = 0x4;
static const int TEX_FLAG_ROUGHNESS = 0x8;

// Light buffer, see LightData struct in Common.hlsli
cbuffer LightBuffer : register(b1)
{
    LightData lights[8]; // Support 8 lights max
    int lightCount; // LightData ends on a block of 16 fully used bytes so this wont be packed
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
    float3 cameraVector : CAMVECTOR;
};




// Calculate diffuse lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 lightColor, float3 tangent)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(lightColor * intensity);
	return colour;
}

// Calculate specular power of the light
// Uses Blinn Phong for Isotropic specular lighting 
// Uses Heidrich-Seidel for Anisotropic specular lighting (Wikipedia, no date b)
// Both are combined when anisotropy != 1 != 0
float calculateSpecularPower(float3 lightDirection, float3 normal, float3 viewVector, float specularPower, float3 tangent, float anisotropy)
{
    // Flip view vector
    viewVector *= -1;

    // Calculate the isotropic specularity
    float3 halfway = normalize(lightDirection + viewVector);
    float blinnPhong = pow(max(dot(normal, halfway), 0), specularPower);
    
    // TODO if anisotropy is 0, return isotropic only
    if (anisotropy == 0) return blinnPhong;

    // Set thread direction to be along the tangent. 
    float3 threadDirection = normalize(tangent);

    // Calculate T P and R as per (Wikipedia, no date)
    float3 threadByNormal = normalize(threadDirection + (dot(-threadDirection, normal) * normal));
    float3 projected = normalize(lightDirection + (dot(-lightDirection, threadByNormal) * threadByNormal));
    float3 reflectedRay = normalize(-lightDirection + (dot(lightDirection, projected) * projected * 2));

    // Use these to calculate specular anisotropic component
    float anisotropicPower = max(pow(dot(viewVector, reflectedRay), specularPower), 0);
    
    // If any of the following are true, the anisotropic component is 0
    if (dot(reflectedRay, viewVector) < 0) anisotropicPower = 0;
    if (dot(projected, viewVector) < 0) anisotropicPower = 0;
    if (dot(normal, viewVector) < 0) anisotropicPower = 0;

    // Return a blend of anisotropic light and isotrpoic light
    // 0 -> 0.5 anisotropic power lerps from 1 to power after 0.5 it stays at power
    // 0.5 -> 1 isotropic power lerps from power to 1 before 0.5 it stays at power
    // The 2 are then blended together:
    // At anisotropy 1 the calculation is (anisotropic power * 1) so fully anisotrpoic
    // At anisotropy 0 the calculation is (1 * isotropic power) so fully isotropic
    // At anisotropy 0.5 the calculation is (anisotropic power * isotropic power) so a blend of the 2
    // Values closer to 0 make the anisotropic smear less, closer to 1 make more smear
    return saturate(lerp(1, anisotropicPower, saturate(anisotropy * 2.0)) * lerp(blinnPhong, 1, saturate(anisotropy * 2.0 - 1)));

}

// Calculate normal from normal map, converting tangent space normals into world space
float3 CalculateNormalFromNormalMap(InputType pixelData) {
    
    // Create TBN matrix and multiply (mtrebi, 2017)
    float3x3 TBN = float3x3(pixelData.tangent, pixelData.bitangent, pixelData.normal);

    // Sample normal from normal map
    float3 mapNormal = normalMap.Sample(textureSampler, pixelData.tex);
    // Remap to -1 -> 1 from 0 -> 1
    mapNormal = mapNormal * 2 - 1;

    // Return mapNormal * TBN
    return mul(mapNormal, TBN);
}

float4 main(InputType input) : SV_TARGET
{
    DiscardForDOF(minMaxDepth, input.position.z);
    
    // Sample the texture and set up base light color (black no lights applied) Specular seperate as applied on top of the texture
    float4 ambientAndDiffuseLightColor = float4(0, 0, 0, 1);
    float4 specularLightColor = float4(0, 0, 0, 1);

    // Do calculations based on textures
    float4 textureModulate = float4(1, 1, 1, 1);
    float ambientModulate = 1;
    float smoothnessTextureAccounted = smoothness;
    
    if (textureFlags & TEX_FLAG_COLOR)
    {
        textureModulate = colorMap.Sample(textureSampler, input.tex);
    }
    
    // Replace input.normal with normal map one if using
    if (textureFlags & TEX_FLAG_NORMAL)
    {
        input.normal = CalculateNormalFromNormalMap(input);
    }
    
    // Set ambient modulate to AO map value if available (McReynolds and Blythe, 2005)
    if (textureFlags & TEX_FLAG_AO)
    {
        ambientModulate = AOMap.Sample(textureSampler, input.tex).r;
    }
    
    // Set smoothness texture accounted value to inverse roughness
    if (textureFlags & TEX_FLAG_ROUGHNESS)
    {
        smoothnessTextureAccounted = 1 - roughnessMap.Sample(textureSampler, input.tex).r;
    }

    // For each light calculate lighting
    for (int i = 0; i < lightCount; i++) {
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
        
        // Calculate ambient, modulate with AO Map factor (McReynolds and Blythe, 2005)
        float4 localLightColor = lights[i].ambient * lights[i].lightPower * ambientModulate;
        
        // If we are not in shadow do specular and diffuse
        if (!IsInShadow(lights[i], input.worldPosition, -normalize(lights[i].position - input.worldPosition), i, directionalShadowMaps[i], shadowMaps[i], shadowSampler))
        {
            // Add diffuse light, modulate with AO Map factor (McReynolds and Blythe, 2005)
            localLightColor += calculateLighting(lightVector, input.normal, lights[i].diffuse, input.bitangent) * lights[i].lightPower * spotlightFactor * ambientModulate;
            
            // Calculate specular light and add onto the specular total
            // Also attenuate it
            specularLightColor += calculateSpecularPower(lightVector, normalize(input.normal), normalize(input.cameraVector), specularity, input.tangent, anisotropy) * smoothnessTextureAccounted * lights[i].lightPower * lights[i].diffuse * spotlightFactor
            / (lights[i].constantAttenuation + lights[i].linearAttenuation * distanceToLight + lights[i].quadraticAttenuation * distanceToLight * distanceToLight);

        }
        // Attenuate light (Add attenuation variables to light buffer input)
        // Since ambient is being attenuated we must still attenuate in shadow. 
        localLightColor *= 1 / (lights[i].constantAttenuation + lights[i].linearAttenuation * distanceToLight + lights[i].quadraticAttenuation * distanceToLight * distanceToLight);
        
        // Add this lights complete color to the overall light color for this fragment/pixel
        ambientAndDiffuseLightColor += localLightColor;
    }
    // Debug output normals
    //return float4((input.normal / 2) + 0.5, 1);
    
    // Combine ambient and diffuse with specular
    return float4(((ambientAndDiffuseLightColor * diffuseColor * textureModulate) + (specularLightColor * specularColor)).rgb, 1);
}



