struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;
};

cbuffer SpotLightConstantBuffer : register(b0)
{
    float3 direction;
    float enabled;
    float3 diffuse_color;
    float specular_strength;
    float3 specular_color;
    float specular_shininess;
}

Texture2D myTexture : register(t0);
SamplerState mySampler : register(s0);


float3 CalculeSpotLight(float3 normal, float3 view_dir, float3 color_base){
    
    
    return float3(1.0, 1.0, 1.0);

}


float4 PShader(PS_INPUT input) : SV_TARGET
{
    float4 out_color;
    
    //out_color = float4(1.0f,1.0f, 1.0f, 1.0f);
    //out_color = float4(input.normal.xyz, 1.0f);
    //return out_color;
    
    const float3 view_dir = normalize(input.cam_pos - input.world_position);
    const float4 texture_color = (myTexture.Sample(mySampler, input.uv));
    //const float3 light_color = CalculePointLight(input.normal, view_dir, texture_color.rgb);
    
    return float4(0.0, 1.0, 0.0, 1.0);

}

