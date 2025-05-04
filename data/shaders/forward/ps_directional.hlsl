struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;
};

Texture2D myTexture : register(t0);
SamplerState mySampler : register(s0);


float3 CalculeDirectionalLight(float3 normal, float3 view_dir, float3 color_base){
    
    const float3 light_dir = float3(0.0,-1.0, 1.0);
    const float specular_shininess = 32.0;
    const float specular_strenght = 0.003;
    const float3 specular_color = float3(1.0, 1.0, 1.0);
    
    float3 diff = max(dot(normal, -light_dir), 0.0);
    const float3 albedo = diff * color_base;
    
    const float3 reflect_dir = normalize(reflect(-light_dir, normalize(normal)));
    float spec = pow(max(dot(normalize(view_dir), normalize(reflect_dir)), 0.0), specular_shininess);
    float3 specular = specular_strenght * spec * specular_color;
    
    return float3(albedo + specular);

}


float4 PShader(PS_INPUT input) : SV_TARGET
{
    float4 out_color;
    
    //out_color = float4(1.0f,1.0f, 1.0f, 1.0f);
    //out_color = float4(input.normal.xyz, 1.0f);
    //return out_color;
    
    const float3 view_dir = normalize(input.cam_pos - input.world_position);
    const float4 texture_color = (myTexture.Sample(mySampler, input.uv));
    const float3 light_color = CalculeDirectionalLight(input.normal, view_dir, texture_color.rgb);
    
    return float4(light_color, 1.0);

}

