struct VS_IN{

    float3 position : POSITION;
};

struct VS_OUT{

    float4 position : SV_POSITION;
    float3 direction : TEXCOORD0;
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
    float emissive_intensity;
}

VS_OUT VShader(VS_IN input)
{
    VS_OUT output;

    float4x4 view_no_translation = view;
    view_no_translation._41 = 0;
    view_no_translation._42 = 0;
    view_no_translation._43 = 0;

    float4 world_pos = mul(float4(input.position, 1.0), view_no_translation);
    output.position = mul(world_pos, projection);
    output.position.z = output.position.w;

    output.direction = input.position;
    
    
    return output;
}

