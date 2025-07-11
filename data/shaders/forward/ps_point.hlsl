struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;
    float4 tangent : TANGENT;
    float metallic_value : METALLIC;
    float roughness_value : ROUGHNESS;
    float4 color_value : COLOR;
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
    float intensity;
    float exposure;
    float light_distance;
    float fall_off;
    float fall_start;
    float3 padding;

}

Texture2D albedo_tex : register(t0);
Texture2D normal_tex : register(t1);
Texture2D metallic_tex : register(t2);
Texture2D roughness_tex : register(t3);
Texture2D ao_tex : register(t4);

SamplerState mySampler : register(s0);


float3 CalculePointLight(float3 normal, float3 view_dir, float3 color_base, float3 frag_pos){
    
    
    float3 lightDir = normalize(light_position - frag_pos);
 
    //Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    //Specular
    float3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(view_dir, reflectDir), 0.0), specular_shininess);

    //float lightDistance = length(light_position - frag_pos);
    //float attenuation = 1.0 / (constant_att + linear_att * lightDistance + quadratic_att * (lightDistance * lightDistance));
  

    float3 difuse = diffuse_color * diff * color_base;
    //float3 specular = (specular_strength * specular_shininess * spec) * specular_color;


    //difuse *= (attenuation * attenuation);
    //specular *= attenuation;
  
    //return (difuse + specular);
    return difuse;

}

float3 getNormalFromMapOLD(float3 normal_texture, float3 normal, float4 tangent)
{
    
    // Normal del mapa en espacio tangent (de [0,1] a [-1,1])
    float3 tangentNormal = normal_texture * 2.0f - 1.0f;

    float3 N = normalize(normal);
    float3 T = normalize(tangent.xyz);

    // Calcula bitangent usando el signo almacenado en tangent.w
    //float3 B = tangent.w * cross(N, T);
    float3 B = normalize(cross(N, T) * tangent.w);

    // Construye matriz TBN
    float3x3 TBN = float3x3(T, B, N);

    // Transforma la normal del mapa a espacio mundo
    return normalize(mul(tangentNormal, TBN));
}

float DistributionGGX(float3 N, float3 H, float roughness)
{

    const float PI = 3.14159265359;
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
  
    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{

    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{

    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
float3 fresnelSchlick(float cosTheta, float3 F0)
{

    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    //return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
    
}


float4 PShader(PS_INPUT input) : SV_TARGET
{
    float4 out_color;
    
    const float PI = 3.14159265359;
    
    const float3 view_dir = normalize(input.cam_pos - input.world_position);
    const float4 texture_color = max((albedo_tex.Sample(mySampler, input.uv)), input.color_value);
    const float4 texture_normal = (normal_tex.Sample(mySampler, input.uv));
    
    const float texture_metallic = max((metallic_tex.Sample(mySampler, input.uv)).r, input.metallic_value);
    const float texture_roughness = max((roughness_tex.Sample(mySampler, input.uv)).r, input.roughness_value);
    
    //const float texture_metallic = metallic_tex.Sample(mySampler, input.uv).r;
    //const float texture_roughness = roughness_tex.Sample(mySampler, input.uv).r;
    
    const float texture_ao = (ao_tex.Sample(mySampler, input.uv)).r;
    const float3 normal_procesed = getNormalFromMapOLD(texture_normal.rgb, input.normal, input.tangent);
    
    
    
    const float3 V = view_dir;
    float3 F0 = float3(0.04, 0.04, 0.04);
    float3 tmp_f = lerp(F0, texture_color.rgb, texture_metallic);
    F0.r = tmp_f.r;
    F0.g = tmp_f.g;
    F0.b = tmp_f.b;
    
    float3 Lo = float3(0.0, 0.0, 0.0);

    //float3 radiance = CalculePointLight(normal_procesed, view_dir, texture_color.rgb, input.world_position);

    float3 L = normalize(light_position - input.world_position);
    float3 H = normalize(V + L);
    float distance = length(light_position - input.world_position);
    //float attenuation = 1.0 / (distance * distance);
    float attenuation = 1.0 / (constant_att + linear_att * distance + quadratic_att * (distance * distance));
    //float fade = saturate(1.0 - distance / light_distance);
    
    // Fade out
    //float fade = saturate(1.0 - pow(distance / light_distance, fall_off));
    
    float fade_start = light_distance * fall_start;
    float fade = smoothstep(fade_start, light_distance, distance); // 0 a 1 en el borde
    //fade = pow(fade, fall_off);
    fade = 1.0 - fade;
    attenuation *= fade;
    
    float3 radiance = diffuse_color * texture_color.rgb * attenuation * intensity;

    float3 N = normal_procesed;
    
    // Cook-Torrance BRDF specular
    float NDF = DistributionGGX(N, H, texture_roughness);
    float G = GeometrySmith(N, V, L, texture_roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f; // + 0.0001 to prevent divide by zero
    float3 specular = numerator / denominator;
    
    // kS is equal to Fresnel
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - texture_metallic; // by the moment, no reflection on metallic surfaces
    
    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);
    //if (NdotL < 0.05) return float4(0.0, 1.0, 0.0, 1.0 );
    
    // add to outgoing radiance Lo
    Lo += (kD * texture_color.rgb / PI + specular) * radiance * NdotL;
    
    float3 ambient = float3(0.01f, 0.01f, 0.01f) * texture_color.rgb * texture_ao;
    ambient *= (1.0 - texture_metallic); // prevent albedo on full metallic parts
    
    //float3 color = Lo + ambient;
    float3 color = Lo;
    
    // HDR tonemapping
    //color = color / (color + float3(1.0, 1.0, 1.0));
    
    // HDR tonemapping with exposure
    color = float3(1.0, 1.0, 1.0) - exp(-color * exposure);
    
    // gamma correct
    float tmp = 1.0 / 2.2;
    color = pow(color, float3(tmp, tmp, tmp));

    return float4(color, 1.0);

}

