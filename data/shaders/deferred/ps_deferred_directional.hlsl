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
TextureCube skybox_tex : register(t5);
Texture2D brdf_tex : register(t6);
//Texture2D gDepth : register(t5); // Needed to reconstruct world pos


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
    float cubemap_max_mip_level;
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

    //float a = roughness;
    //float k = (a * a) / 2.0f;
    
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

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness){

    float tmp = 1.0 - roughness;
    return F0 + (max(float3(tmp, tmp, tmp), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   



PS_OUT PShader(PS_INPUT input) : SV_TARGET
{
    PS_OUT out_color;

    
    const float PI = 3.14159265359;

    const float4 texture_position = (position_tex.Sample(mySampler, input.uv));
    
    const float3 view_dir = normalize(cam_pos - texture_position.rgb);
    
    const float4 texture_color = (albedo_tex.Sample(mySampler, input.uv));
    const float4 texture_normal = (normal_tex.Sample(mySampler, input.uv));
    
    const float texture_metallic = (gMaterial.Sample(mySampler, input.uv)).r;
    const float texture_roughness = (gMaterial.Sample(mySampler, input.uv)).g;
    //texture_roughness *= texture_roughness;
    const float texture_ao = (gMaterial.Sample(mySampler, input.uv)).b;
    
    const float4 texture_emissive = gEmissive.Sample(mySampler, input.uv);


    const float3 V = view_dir;
    float3 F0 = float3(0.04, 0.04, 0.04);
    float3 tmp_f = lerp(F0, texture_color.rgb, texture_metallic);
    F0.r = tmp_f.r;
    F0.g = tmp_f.g;
    F0.b = tmp_f.b;
    
    

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
    //float3 F = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, texture_roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f; // + 0.0001 to prevent divide by zero
    float3 specular = numerator / denominator;
    
    // kS is equal to Fresnel
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - texture_metallic; // by the moment, no reflection on metallic surfaces
    
    // Environment mapping
    float3 reflected = reflect(-view_dir, normalize(texture_normal.rgb));
    float mip_level = (texture_roughness)  * cubemap_max_mip_level;
    //float3 reflect_color = skybox_tex.Sample(mySampler, reflected, mip_level).rgb;
    float3 reflect_color = skybox_tex.SampleLevel(mySampler, reflected, mip_level);
    
    
    float NdotV = max(dot(N, V), 0.0f);
    float2 envBRDF = brdf_tex.Sample(mySampler, float2(NdotV, texture_roughness)).rg;
    float3 kS_tmp = fresnelSchlickRoughness(NdotV, F0, texture_roughness);
    float3 specularIBL = reflect_color * (kS_tmp * envBRDF.x + envBRDF.y);

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);
   
    specular += specularIBL;
    

    // add to outgoing radiance Lo
    float3 Lo = (kD * texture_color.rgb / PI + specular) * radiance * NdotL;
    //Lo += specularIBL;

    //float3 ambient_tmp_reflection = (kD * (texture_color.rgb * 0.01) + specularIBL) * texture_ao;
    
    
    float3 ambient = float3(0.01f, 0.01f, 0.01f) * texture_color.rgb * texture_ao;
    ambient *= (1.0 - texture_metallic); // prevent albedo on full metallic parts
    
    float3 color = Lo + ambient;
    //float3 color = Lo;// + ambient_tmp_reflection;
    
    
    // Calculate brightness before HDR
    
    // Emissive texture color
    out_color.out_emissive = float4(texture_emissive);
    out_color.out_emissive.rgb *= out_color.out_emissive.a;
    
    float3 ambient_tmp = ambient;// * 50.0;
    float brightness = dot(color + ambient_tmp, float3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0){
        // If the emissive is because of his brightness, just add the albedo
        float brightness_attenuation = (1.0, texture_metallic, 0.3);
        out_color.out_emissive = float4((color + (color * out_color.out_emissive.rgb)) * intensity * brightness_attenuation, 1.0); // * texture_emissive.a; // bloom intensity, 1.0f by default
    }


    // HDR tonemapping
    //color = color / (color + float3(1.0, 1.0, 1.0));
    
    //float3 bloom = float3(texture_color.rgb);
    
    // HDR tonemapping with exposure
    color = float3(1.0, 1.0, 1.0) - exp(-color * intensity);
 
    color += texture_emissive.rgb;
    
    // gamma correct
    float tmp = 1.0 / 2.2;
    color = pow(color, float3(tmp, tmp, tmp));

    out_color.out_light = float4(color, 1.0);
    
    return out_color;
}

