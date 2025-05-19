// Tess info buffer stores information for dynamic tessellation
cbuffer TessInfo : register(b0)
{
    float2 minMaxTess;
    float2 minMaxDist;
    float4 cameraPosition;
    matrix worldMatrix;
}

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    // Calculate the tessellation factor of each point on the quad
    float4 worldPosition0 = mul(worldMatrix, inputPatch[0].position);
    float distance0 = length(cameraPosition - worldPosition0);
    float distanceFactor0 = 1 - clamp((distance0 - minMaxDist.x) / (minMaxDist.y - minMaxDist.x), 0, 1);
    float tessFactor0 = lerp(minMaxTess.x, minMaxTess.y, distanceFactor0);

    float4 worldPosition1 = mul(worldMatrix, inputPatch[1].position);
    float distance1 = length(cameraPosition - worldPosition1);
    float distanceFactor1 = 1 - clamp((distance1 - minMaxDist.x) / (minMaxDist.y - minMaxDist.x), 0, 1);
    float tessFactor1 = lerp(minMaxTess.x, minMaxTess.y, distanceFactor1);

    float4 worldPosition2 = mul(worldMatrix, inputPatch[2].position);
    float distance2 = length(cameraPosition - worldPosition2);
    float distanceFactor2 = 1 - clamp((distance2 - minMaxDist.x) / (minMaxDist.y - minMaxDist.x), 0, 1);
    float tessFactor2 = lerp(minMaxTess.x, minMaxTess.y, distanceFactor2);

    float4 worldPosition3 = mul(worldMatrix, inputPatch[3].position);
    float distance3 = length(cameraPosition - worldPosition3);
    float distanceFactor3 = 1 - clamp((distance3 - minMaxDist.x) / (minMaxDist.y - minMaxDist.x), 0, 1);
    float tessFactor3 = lerp(minMaxTess.x, minMaxTess.y, distanceFactor3);


    // For each edge, average the 2 point's tessellation factors. 
    output.edges[0] = (tessFactor0 + tessFactor1) / 2.0;
    output.edges[1] = (tessFactor1 + tessFactor2) / 2.0;
    output.edges[2] = (tessFactor2 + tessFactor3) / 2.0;
    output.edges[3] = (tessFactor3 + tessFactor0) / 2.0;

    // For the inside tessellation factor, average all points factors
    output.inside[0] = (tessFactor0 + tessFactor1 + tessFactor2 + tessFactor3) / 4.0;
    output.inside[1] = (tessFactor0 + tessFactor1 + tessFactor2 + tessFactor3) / 4.0;
    
    return output;
}

// Setup with for quad tessellation
[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Just pass through all data
    output.position = patch[pointId].position;
    output.tex = patch[pointId].tex;
    output.normal = patch[pointId].normal;
    output.tangent = patch[pointId].tangent;
    output.bitangent = patch[pointId].bitangent;

    return output;
}