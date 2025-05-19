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
    float3 light_position;
    float enabled;
    float3 direction;
    float cutt_off;
    float3 diffuse_color;
    float outer_cut_off;
    float3 specular_color;
    float specular_strength;
    float specular_shininess;
    float constant_att;
    float linear_att;
    float quadratic_att;
}

Texture2D myTexture : register(t0);
SamplerState mySampler : register(s0);


float3 CalculeSpotLight(float3 normal, float3 view_dir, float3 color_base, float3 frag_pos){
    
    float3 lightDir = normalize(light_position - frag_pos);
    float cut_off = cos(cutt_off * 3.1415 / 180);
    float outer_cut_off_calculated = cos(outer_cut_off * 3.1415 / 180);
    
    

    
    float3 light_diffuse_calculated;
    float3 light_specular_calculated;
   

    float diff = max(dot(normal, direction), 0.0);
    light_diffuse_calculated = diff * diffuse_color * color_base; // * texture(u_texture, uv).rgb;

    float3 reflectDir = reflect(-direction, normal);
    float spec = pow(max(dot(view_dir, reflectDir), 0.0), specular_shininess);

    light_specular_calculated = specular_strength * spec * specular_color * color_base; // * texture(u_texture, uv).rgb;
    
    
    
    
    
    
    
    float distance = length(light_position - frag_pos);

    float k0 = linear_att;
    float k1 = linear_att;
    float k2 = quadratic_att;

    float attenuationAmount = 1.0 / (k0 + (k1 * distance) + k2 * (distance * distance));
    light_diffuse_calculated *= attenuationAmount;
    light_specular_calculated *= attenuationAmount;
    float theta = dot(lightDir, normalize(-direction)); //0.5

    float epsilon = (cut_off - outer_cut_off_calculated); // 0.9978 - 0.953 /
    float intensity = clamp((theta - outer_cut_off_calculated) / epsilon, 0.0, 1.0); //  0.5 - 0.953 / 0.9978 - 0.953
    light_diffuse_calculated *= intensity;
    light_specular_calculated *= intensity;

    return light_diffuse_calculated + light_specular_calculated;
    
    //return float3(1.0, 1.0, 1.0);

}


float4 PShader(PS_INPUT input) : SV_TARGET
{
    float4 out_color;
    
    //out_color = float4(1.0f,1.0f, 1.0f, 1.0f);
    //out_color = float4(input.normal.xyz, 1.0f);
    //return out_color;
    
    const float3 view_dir = normalize(input.cam_pos - input.world_position);
    const float4 texture_color = (myTexture.Sample(mySampler, input.uv));
    const float3 light_color = CalculeSpotLight(input.normal, view_dir, texture_color.rgb, input.world_position);
    
    return float4(light_color, 1.0);

}

