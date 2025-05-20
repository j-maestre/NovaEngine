struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;
};

cbuffer DirectionalLightConstantBuffer : register(b0)
{
    float3 direction;
    float enabled;
    float3 diffuse_color;
    float specular_strength;
    float3 specular_color;
    float specular_shininess;
}

Texture2D albedo_tex : register(t0);
Texture2D normal_tex : register(t1);
SamplerState mySampler : register(s0);


float3 CalculeDirectionalLight(float3 normal, float3 view_dir, float3 color_base){
    
    const float3 light_dir = direction;
    //const float specular_shininess = 32.0;
    //const float specular_strenght = 0.003;
    //const float3 specular_color = float3(1.0, 1.0, 1.0);
    
    float3 diff = max(dot(normal, -light_dir), 0.0);
    const float3 albedo = diff * color_base * diffuse_color;
    
    const float3 reflect_dir = normalize(reflect(-light_dir, normalize(normal)));
    float spec = pow(max(dot(normalize(view_dir), normalize(reflect_dir)), 0.0), specular_shininess);
    float3 specular = specular_strength * spec * specular_color;
    
    return float3(albedo + specular);

}

float3 getNormalFromMap(float3 normal_texture, float3 normal, float3 world_position, float2 uv){
    
    float3 tangentNormal = normal_texture * 2.0 - 1.0;

    
    float3 Q1 = ddx(world_position);
    float3 Q2 = ddy(world_position);
    float2 st1 = ddx(uv);
    float2 st2 = ddy(uv);

    float3 N = normalize(normal);
    float3 T = normalize(Q1 * st2.x - Q2 * st1.x);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);
    
    return normalize(mul(tangentNormal, TBN));
}

float4 PShader(PS_INPUT input) : SV_TARGET
{
    float4 out_color;
    
    //out_color = float4(1.0f,1.0f, 1.0f, 1.0f);
    //out_color = float4(input.normal.xyz, 1.0f);
    //return out_color;
    
    const float3 view_dir = normalize(input.cam_pos - input.world_position);
    const float4 texture_color = (albedo_tex.Sample(mySampler, input.uv));
    const float4 texture_normal = (normal_tex.Sample(mySampler, input.uv));
    const float3 normal_procesed = getNormalFromMap(texture_normal.rgb, input.normal, input.world_position, input.uv);
    
    const float3 light_color = CalculeDirectionalLight(normal_procesed, view_dir, texture_color.rgb);
    
    //return float4(normal_procesed, 1.0);
    return float4(light_color, texture_color.a);

}

