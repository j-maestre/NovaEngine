struct VS_IN
{
    float3 position : POSITION;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
};

cbuffer CameraDepthPrePass : register(b0){
    float4x4 view;
    float4x4 projection;
    float4x4 model;
}

VS_OUT VShader(VS_IN input)
{
    VS_OUT output;
    float4 world_pos = mul(float4(input.position, 1.0), model);
    float4 view_pos = mul(world_pos, view);
    output.position = mul(view_pos, projection);
    return output;
}

