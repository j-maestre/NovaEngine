#include "pbr_common.hlsli"
#include "lights_common.hlsli"

// Captura de una sonda de reflexion: dibuja la escena desde la sonda, una cara del cubo cada vez.
// Lo que se ve en un reflejo es sobre todo la luz difusa y los emisivos, asi que es un forward sencillo:
// todas las luces sin especular (un brillo depende de desde donde se mira, y la sonda no es la camara).

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;

    float metallic_value : METALLIC;
    float4 color_value : COLOR;
    float roughness_value : ROUGHNESS;
    float3 emissive_value : EMISSIVE;
    float emissive_intensity : EMISSIVE_INTENSITY;
    float emissive_bloom : EMISSIVE_BLOOM;
    float reflectance : REFLECTANCE;
    float4 layers : LAYERS;
};

Texture2D albedo_tex : register(t0);
Texture2D normal_tex : register(t1);
Texture2D metallic_tex : register(t2);
Texture2D roughness_tex : register(t3);
Texture2D ao_tex : register(t4);
Texture2D emissive_tex : register(t5);
TextureCube specular_tex : register(t6);
StructuredBuffer<GPULight> lights : register(t7);
TextureCube irradiance_tex : register(t8);

SamplerState linear_sampler : register(s0);

// El mismo buffer que el pase de luces; aqui solo se usa el numero de mips del cielo
cbuffer CameraDeferred : register(b1)
{
    float4x4 inv_view_proj;
    float3 deferred_cam_pos;
    float cubemap_max_mip_level;
};

float3 NormalFromMap(float3 normal_texture, float3 normal, float4 tangent)
{
    float3 tangent_normal = normal_texture * 2.0f - 1.0f;
    float3 N = normalize(normal);
    float3 T = normalize(tangent.xyz);
    float3 B = normalize(cross(N, T) * tangent.w);
    return normalize(mul(tangent_normal, float3x3(T, B, N)));
}

float4 PShader(PS_INPUT input) : SV_TARGET
{
    float3 albedo = max(albedo_tex.Sample(linear_sampler, input.uv), input.color_value).rgb;
    float metallic = max(metallic_tex.Sample(linear_sampler, input.uv).r, input.metallic_value);
    float roughness = max(roughness_tex.Sample(linear_sampler, input.uv).r, input.roughness_value);
    float ao = ao_tex.Sample(linear_sampler, input.uv).r;
    float3 emissive = max(input.emissive_value, emissive_tex.Sample(linear_sampler, input.uv).rgb) * input.emissive_intensity;
    float3 N = NormalFromMap(normal_tex.Sample(linear_sampler, input.uv).rgb, input.normal, input.tangent);

    float3 P = input.world_position;
    float3 V = normalize(input.cam_pos - P);
    float3 R = reflect(-V, N);

    // Un metal no tiene difusa: solo refleja. Pero la sonda no puede ver reflejos (necesitaria otra sonda),
    // y un metal saldria negro en ella. Como hace Unreal en sus capturas, aqui el metal se trata como
    // un difuso de su color: en un reflejo se ve como un objeto dorado mate en vez de un agujero negro.
    float3 diffuse_albedo = albedo;
    float3 color = float3(0.0f, 0.0f, 0.0f);

    [loop]
    for (uint i = 0; i < light_count; i++)
    {
        float3 L;
        float3 L_spec;
        float2 spread;
        float3 radiance = LightRadiance(lights[i], P, R, L, L_spec, spread);
        color += diffuse_albedo / PI * saturate(dot(N, L)) * radiance;
    }

    // Ambiente: el cielo, como en el pase de luces
    float3 irradiance = irradiance_tex.SampleLevel(linear_sampler, N, 0.0f).rgb * IRRADIANCE_SCALE;
    color += diffuse_albedo * irradiance * ao * ambient_intensity;

    return float4(color + emissive, 1.0f);
}
