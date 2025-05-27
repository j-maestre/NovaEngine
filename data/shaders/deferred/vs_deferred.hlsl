struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 tangent : TANGENT;
};
    
struct VOut{
    float4 position : SV_POSITION;
    float3 world_pos : WORLD_POSITION;
    float3 normal : NORMAL; 
    float4 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;
    
    float metallic_value : METALLIC;
    float4 color_value : COLOR;
    float roughness_value : ROUGHNESS;
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


VOut VShader(VS_IN input){
    
    VOut output;

    float4 worldPosition = mul(float4(input.position, 1.0f), model);
    output.world_pos = worldPosition.xyz;

    // Transform to clip space
    float4 viewPos = mul(worldPosition, view);
    output.position = mul(viewPos, projection);

    output.normal = normalize(mul(input.normal, (float3x3) model));
    
    float3 tangent_world = normalize(mul(float4(input.tangent.xyz, 0.0f), model).xyz);
    output.tangent = float4(tangent_world, input.tangent.w);
    output.bitangent = cross(output.normal, output.tangent.rgb) * input.tangent.w;

    output.uv = input.uv;
    output.cam_pos = cam_pos;
    
    output.metallic_value = metallic_value;
    output.color_value = color_value;
    output.roughness_value = roughness_value;
    output.emissive_value = emissive_value;

    return output;
}

