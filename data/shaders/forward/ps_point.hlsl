struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;
};

cbuffer PointLightConstantBuffer : register(b0)
{
    float3 light_position;
    float specular_strength;
    float3 diffuse_color;
    float specular_shininess;
    float3 specular_color;
    float constant_att;
    float linear_att;
    float quadratic_att;
    float enabled;
    float attenuation;
}

Texture2D myTexture : register(t0);
SamplerState mySampler : register(s0);


float3 CalculePointLight(float3 normal, float3 view_dir, float3 color_base, float3 frag_pos){
    
    
    float3 lightDir = normalize(light_position - frag_pos);
 
    //Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    //Specular
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(view_dir, reflectDir), 0.0), specular_shininess);

    float lightDistance = length(light_position - frag_pos);
    float attenuation = 1.0 / (constant_att + linear_att * lightDistance + quadratic_att * (lightDistance * lightDistance));
  

    float3 difuse = diffuse_color * diff * color_base;
    float3 specular = (specular_strength * specular_shininess * spec) * specular_color;


    difuse *= (attenuation * attenuation);
    specular *= attenuation;
  
    return (difuse + specular);

}


float4 PShader(PS_INPUT input) : SV_TARGET
{
    float4 out_color;
    
    //out_color = float4(1.0f,1.0f, 1.0f, 1.0f);
    //out_color = float4(input.normal.xyz, 1.0f);
    //return out_color;
    
    const float3 view_dir = normalize(input.cam_pos - input.world_position);
    const float4 texture_color = (myTexture.Sample(mySampler, input.uv));
    const float3 light_color = CalculePointLight(input.normal, view_dir, texture_color.rgb, input.world_position);
    
    return float4(light_color, 1.0);
    //return float4(1.0, 1.0, 0.0, 1.0);

}

