struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 tangent : TANGENT;
};
    
struct VOut{
    float4 position : SV_POSITION;
    float3 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;
    float4 tangent : TANGENT;
};

cbuffer CameraObject : register(b0){
    float4x4 view;
    float4x4 projection;
    float4x4 model;
    float3 cam_pos;
}


VOut VShader(VS_IN input)
{
    
    float4x4 vpm = mul(model, mul(view, projection));
    
    VOut output;

    output.position = mul(float4(input.position, 1.0f), vpm);
    output.world_position = mul(float4(input.position, 1.0f), model).xyz;
    output.normal = mul(float4(input.normal, 0.0f), model).xyz;
    
    float3 tangent_world = mul(float4(input.tangent.xyz, 0.0f), model).xyz;
    output.tangent = float4(normalize(tangent_world), input.tangent.w);
    
    //output.normal = input.normal;
    output.uv = float2(input.uv);
    output.cam_pos = cam_pos;

    return output;
}

