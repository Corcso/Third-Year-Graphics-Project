#pragma once
#include "Light.h"
#include "DXF.h"
#include "TextureCubeShadowMaps.h"

/// <summary>
/// Extension of light, used in scene
/// </summary>
class WorldLight :
    public Light
{
public:
    WorldLight();

    /// <summary>
    /// Creates shadow maps in memory, must be done
    /// </summary>
    /// <param name="renderer"></param>
    void CreateShadowMaps(D3D* renderer);

    ShadowMap* GetDirectionalShadowMap(); // Get shadow map for directional light
    TextureCubeShadowMaps* GetTCubeShadowMap(); // Get shadow map for point & spot light
    XMMATRIX GetViewMatrix(int index); // Get view matrix, index if for point/spot light
    XMMATRIX GetProjMatrix(int index); // Get projection matrix

    void GenerateShadowMatrices(); // Generate new matrices for shadows, do this every time a light moves.

    

    // Getters
    float GetConstantAttenuation();
    float GetLinearAttenuation();
    float GetQuadraticAttenuation();
    float GetLightPower();
    int GetLightType();
    float GetInnerSpotlightCutoffAngle();
    float GetOuterSpotlightCutoffAngle();

    // Setters
    void SetAttenuation(float constant, float linear, float quadratic);
    void SetLightType(int type);
    void SetLightPower(float power);
    void SetSpotlightAngles(float innerCutoff, float outerCutoff);

    
    /// <summary>
    /// Show ImGUI controls for this light
    /// </summary>
    /// <param name="label">Name of light</param>
    void ShowGuiControls(const char* label);

    ~WorldLight();

private:

    // Arrays of view matrices
    XMMATRIX* viewMatrices;
    // Pointer to projection matrix
    XMMATRIX* projectionMatrices;
    // Pointer to shadow map
    ShadowMap* directionalShadowMap;
    // Pointer to texture cube shadow map
    TextureCubeShadowMaps* tCubeShadowMap;

    float lightPower; // Light power multiplied by diffuse colour
    float constantAttenuation; // Constant attenuation, typically 1
    float linearAttenuation; // Linear attenuation
    float quadraticAttenuation; // Quadratic attenuation
    int lightType; // Light type, 0 = directional, 1 = point, 2 = spot 

    // Spotlight cutoff angles. (De Vries, 2014 b)
    float innerSpotlightCutoffAngle;
    float outerSpotlightCutoffAngle;
};

