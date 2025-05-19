
// Light struct
struct LightData
{
    float4 ambient;
    float4 diffuse;
    float3 position;
    float3 direction;
    int lightType;
    matrix lightViewMatrix[6];
    matrix lightProjMatrix;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float lightPower;
    float innerSpotlightCutoffAngle;
    float outerSpotlightCutoffAngle;
    float2 p_0;
};

// spotlight multiplication factor (De Vries, 2014 b)
float4 calculateSpotlightPower(float3 lightDirection, float3 lightPointing, float innerCutoff, float outerCutoff)
{
    float angleThisPixel = degrees(acos(max(dot(lightDirection, lightPointing), 0)));
    if (angleThisPixel > outerCutoff)
        return 0;
    if (angleThisPixel < innerCutoff)
        return 1;
    return (angleThisPixel - outerCutoff) / (innerCutoff - outerCutoff);
}

// Calculate Shadow function
// Using the TextureCube for spotlights and point lights (lights which use perspective projection)
// Using the Texture2D for directional lights (lights which use the orthographic projection)
// TextureCubes / Cube maps used, Microsoft (no date a, no date b)
bool IsInShadow(LightData light, float3 worldPos, float3 lightDirection, int lightIndex, Texture2D directionalShadowMap, TextureCube projectionShadowMap, SamplerState shadowSampler)
{
    int shadowMapIndex = 0;
    float4 lightViewPosition = float4(0, 0, 0, 0);
    // Calculate which view matrix we are using, for directional its always 0 
    if (light.lightType != 0)
    {
        float3 absoluteLightDirection = abs(lightDirection);
        if (absoluteLightDirection.x > absoluteLightDirection.y && absoluteLightDirection.x > absoluteLightDirection.z)
        {
            if (lightDirection.x < 0) 
                shadowMapIndex = 1;
            else
                shadowMapIndex = 0;
        }
        else if (absoluteLightDirection.y > absoluteLightDirection.z && absoluteLightDirection.y > absoluteLightDirection.x)
        {
            if (lightDirection.y < 0)
                shadowMapIndex = 3;
            else
                shadowMapIndex = 2;
        }
        else
        {
            if (lightDirection.z < 0)
                shadowMapIndex = 5;
            else
                shadowMapIndex = 4;
        }
    }
    float4 pixelByLightView = mul(light.lightViewMatrix[shadowMapIndex], float4(worldPos, 1));
    lightViewPosition = mul(light.lightProjMatrix, pixelByLightView);
    
    // FOR DIRECTIONAL If projection is behind us, we are not in the shadow map (return out of shadow)
    if (light.lightType == 0 && lightViewPosition.z < 0)
    {
        return false;
    }
    
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    
    float3 testVector = float3(projTex.x, projTex.y, shadowMapIndex);
    
    // FOR DIRECTIONAL Check if UV space projection is within 0 to 1 range
    if (light.lightType == 0 && (projTex.x < 0.f || projTex.x > 1.f || projTex.y < 0.f || projTex.y > 1.f))
    {
        return false;
    }
    
    // Sample the shadow map (get depth of geometry)
    float depthValue = 0;
    if (light.lightType == 0)
        depthValue = directionalShadowMap.Sample(shadowSampler, projTex).r;
    else
        depthValue = projectionShadowMap.Sample(shadowSampler, lightDirection).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    // Perform a higher bias for directional lights, they are typically further away
    if (light.lightType == 0)
        lightDepthValue -= 0.002;
    else
        lightDepthValue -= 0.0002;
    

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue >= depthValue)
    {
        return true;
    }
    return false;
}

// For depth of field discarding
void DiscardForDOF(float2 minMaxDepth, float SVPositionZ)
{
    // If we are this layer, or the one ahead (-0.001) then draw otherwise dont. 
    if (SVPositionZ < minMaxDepth.x -0.001 || SVPositionZ > minMaxDepth.y)
        discard;
    return;
}

// Depth of field layer count
static const int DOF_LAYER_COUNT = 9;

// Custom Bilinear Sample function
// Used for height map sampling, bilinear samples 16 away for better smoothing. 
float4 CustomBilinearSample(SamplerState samplerToUse, Texture2D textureToUse, float2 texCoord)
{
    // Get a return variable and width and height of texture
    float4 finalColor;
    int width, height, levels;
    textureToUse.GetDimensions(0, width, height, levels);
    
    // Calculate what to sample between, using 16 here for better smoothing.
    float oneUTexel = 16.0f / width;
    float oneVTexel = 16.0f / height;
    
    // Calculate where to sample and how far between each location we are
    float uMin = (floor(texCoord.x / oneUTexel) * oneUTexel);
    float uMax = (ceil(texCoord.x / oneUTexel) * oneUTexel);
    float uAlong = (texCoord.x - uMin) / (oneUTexel);
    
    float vMin = (floor(texCoord.y / oneVTexel) * oneVTexel);
    float vMax = (ceil(texCoord.y / oneVTexel) * oneVTexel);
    float vAlong = (texCoord.y - vMin) / (oneUTexel);
    
    // Sample all 4 sample points
    
    // Use array access rather than sampling, this provides better performance. 
    float4 topLeft = textureToUse[uint2(clamp(uMin * width, 0, width), clamp(vMin * height, 0, height))];
    float4 topRight = textureToUse[uint2(clamp(uMax * width, 0, width), clamp(vMin * height, 0, height))];
    float4 bottomLeft = textureToUse[uint2(clamp(uMin * width, 0, width), clamp(vMax * height, 0, height))];
    float4 bottomRight = textureToUse[uint2(clamp(uMax * width, 0, width), clamp(vMax * height, 0, height))];
  
    // Linear interpolate between them, and return.
    float4 topEdge = lerp(topLeft, topRight, uAlong);
    float4 bottomEdge = lerp(bottomLeft, bottomRight, uAlong);

    return lerp(topEdge, bottomEdge, vAlong);
}

// "Gaussian" blurs sample with a 5x5 kernel. 
// Used for height map smoothing (ananbd, 2020)
// Need to smooth because height map image only can store 256 colours which isnt precise enough for a smooth height map at high tessellation. 
// Uses the custom bilinear sample function. 
// Sample used should be a min mag mip point sampler for best performance (as linear sampling is emulated by CustomBilinearSample)
float4 SmoothedSample(SamplerState samplerToUse, Texture2D textureToUse, float2 texCoord)
{
    // Get dimentions and setup a base colour
    float4 finalColor = float4(0, 0, 0, 0);
    int width, height, levels;
    textureToUse.GetDimensions(0, width, height, levels);
    
    // Calculate the distance between blur samples. 
    float oneUTexel = 32.0f / width;
    float oneVTexel = 32.0f / height;
    
    // Weights are simple and gaussian esque but add up to 1. 
    float weights[3] = { 0.5, 0.2, 0.05 };
    
    // Since a small kernel, do all samples in a double loop.
    for (int u = -2; u <= 2; ++u)
    {
        for (int v = -2; v <= 2; ++v)
        {
            // Sample and multiply by weights
            float2 texToSample = texCoord + float2((float) u * oneUTexel, (float) v * oneVTexel);
            finalColor += CustomBilinearSample(samplerToUse, textureToUse, texToSample) * weights[abs(u)] * weights[abs(v)];
        }
    }
    
    // Return smoothed colour
    return finalColor;
}