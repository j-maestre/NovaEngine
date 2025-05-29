struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

struct PS_OUT
{
    float4 out_light : SV_Target0;
    float4 out_emissive : SV_Target1;
};


Texture2D albedo_tex : register(t0);
Texture2D position_tex : register(t1);
Texture2D normal_tex : register(t2);
Texture2D gMaterial : register(t3); // RGB = metallic, roughness, ao
Texture2D gEmissive : register(t4);

Texture2D gDepth : register(t5); // Needed to reconstruct world pos


SamplerState mySampler : register(s0);

cbuffer DirectionalLightConstantBuffer : register(b0)
{
    float3 direction;
    float enabled;
    float3 light_diffuse_color;
    float specular_strength;
    float3 specular_color;
    float specular_shininess;
    float intensity;
}

cbuffer CameraDeferred : register(b1){
    
    float4x4 inv_view_proj;
    float3 cam_pos;
    float padding;
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



PS_OUT PShader(PS_INPUT input) : SV_TARGET
{
    PS_OUT out_color;

    
    const float PI = 3.14159265359;
    
    // TODO: Mira a ver, que tienes que pasarle al vertex shader el valor de los materiales
    // para poder guardar en una textura el color de la textura de albedo o el color del material a pelo,
    // igual en los otros, suerte espabilao
    const float4 texture_position = (position_tex.Sample(mySampler, input.uv));
    
    const float3 view_dir = normalize(cam_pos - texture_position.rgb);
    
    const float4 texture_color = (albedo_tex.Sample(mySampler, input.uv));
    const float4 texture_normal = (normal_tex.Sample(mySampler, input.uv));
    
    const float texture_metallic = (gMaterial.Sample(mySampler, input.uv)).r;
    const float texture_roughness = (gMaterial.Sample(mySampler, input.uv)).g;
    const float texture_ao = (gMaterial.Sample(mySampler, input.uv)).b;
    
    const float3 texture_emissive = gEmissive.Sample(mySampler, input.uv).rgb;

    const float3 V = view_dir;
    float3 F0 = float3(0.04, 0.04, 0.04);
    float3 tmp_f = lerp(F0, texture_color.rgb, texture_metallic);
    F0.r = tmp_f.r;
    F0.g = tmp_f.g;
    F0.b = tmp_f.b;
    
    float3 Lo = float3(0.0, 0.0, 0.0);

    float distance = 0.0;
    float attenuation = 1.0;
    float3 radiance = light_diffuse_color * texture_color.rgb; // * intensity;
    
    float3 L = -direction;
    float3 H = normalize(V + L);
    float3 N = texture_normal.rgb;
    
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
    
    //float3 bloom = float3(texture_color.rgb);
    color += texture_emissive;
    
    // HDR tonemapping with exposure
    color = float3(1.0, 1.0, 1.0) - exp(-color * intensity);
    
    // gamma correct
    float tmp = 1.0 / 2.2;
    color = pow(color, float3(tmp, tmp, tmp));

    out_color.out_light = float4(color, 1.0);
    out_color.out_emissive = float4(texture_emissive, 1.0); // + brithness
    
    return out_color;
    //return float4(color, 1.0);
    //return float4(N, 1.0);
}

