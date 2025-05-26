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
    float metallic_value : METALLIC;
    float roughness_value : ROUGHNESS;
    float4 color_value : COLOR;
    float3 emissive_value : EMISSIVE;
};

cbuffer CameraObject : register(b0){
    float4x4 view;
    float4x4 projection;
    float4x4 model;
    float3 cam_pos;
    float metallic_value;
    float4 color_value;
    float roughness_value;
    float3 emissive_value;
}


VOut VShader(VS_IN input)
{
    
    float4x4 vpm = mul(model, mul(view, projection));
    
    VOut output;
    
    output.metallic_value = metallic_value;
    output.roughness_value = roughness_value;
    output.color_value = color_value;
    output.emissive_value = emissive_value;

    output.position = mul(float4(input.position, 1.0f), vpm);
    output.world_position = mul(float4(input.position, 1.0f), model).xyz;
    output.normal = normalize(mul(float4(input.normal, 0.0f), model).xyz);
    
    
    float3 tangent_world = normalize(mul(float4(input.tangent.xyz, 0.0f), model).xyz);
    output.tangent = float4(tangent_world, input.tangent.w);

    
    //output.normal = mul(float4(input.normal, 0.0f), model).xyz;
    
    //float3 tangent_world = mul(float4(input.tangent.xyz, 0.0f), model).xyz;
   // output.tangent = float4(normalize(tangent_world), input.tangent.w);
    
    //output.normal = input.normal;
    output.uv = float2(input.uv);
    output.cam_pos = cam_pos;

    return output;
}

