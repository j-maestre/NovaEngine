#include "pbr_common.hlsli"

// Pase unico de luz: acumula todas las luces, el IBL y el ambiente en HDR lineal.
// El tonemapping y la gamma se aplican al final, en ps_deferred_composite.

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

static const int LIGHT_DIRECTIONAL = 0;
static const int LIGHT_POINT = 1;
static const int LIGHT_SPOT = 2;

// Debe coincidir con GPULight en defines.h
struct GPULight
{
    float3 position;
    int type;
    float3 direction;
    float intensity;
    float3 color;
    float range;
    float cos_inner;
    float cos_outer;
    float fall_start;
    float constant_att;
    float linear_att;
    float quadratic_att;
    float2 padding;
};

Texture2D albedo_tex : register(t0);
Texture2D position_tex : register(t1);
Texture2D normal_tex : register(t2);
Texture2D material_tex : register(t3); // R = metallic, G = roughness, B = ao
Texture2D emissive_tex : register(t4);
TextureCube skybox_tex : register(t5);
Texture2D brdf_tex : register(t6);
StructuredBuffer<GPULight> lights : register(t7);
TextureCube irradiance_tex : register(t8);
Texture2D ssao_tex : register(t9);

SamplerState linear_sampler : register(s0);

cbuffer LightPassConstants : register(b0)
{
    uint light_count;
    float ambient_intensity;
    float ssao_power;   // 0 = SSAO apagado
    float ssao_direct;  // Cuanto atenua el SSAO la luz directa
};

cbuffer CameraDeferred : register(b1)
{
    float4x4 inv_view_proj;
    float3 cam_pos;
    float cubemap_max_mip_level;
};

static const float3 LUMINANCE = float3(0.2126f, 0.7152f, 0.0722f);

// El mapa de irradiancia tiene de media 2.1 veces el brillo del entorno: se normaliza para
// que una superficie blanca devuelva, de media, lo mismo que recibe.
static const float IRRADIANCE_SCALE = 0.48f;

// Bloom: entra suave entre estos dos brillos, calculados sobre la luz total
static const float BLOOM_START = 1.0f;
static const float BLOOM_END = 1.5f;
static const float BLOOM_STRENGTH = 0.3f;

// Radiancia que llega al punto P. Devuelve 0 si la luz no le afecta.
float3 LightRadiance(GPULight light, float3 P, out float3 L)
{
    if (light.type == LIGHT_DIRECTIONAL)
    {
        L = -light.direction;
        return light.color * light.intensity;
    }

    float3 to_light = light.position - P;
    float distance = length(to_light);
    L = to_light / max(distance, 0.0001f);

    if (distance >= light.range)
        return float3(0.0f, 0.0f, 0.0f);

    float attenuation = 1.0f / (light.constant_att + light.linear_att * distance + light.quadratic_att * distance * distance);
    attenuation *= 1.0f - smoothstep(light.range * light.fall_start, light.range, distance);

    if (light.type == LIGHT_SPOT)
    {
        float theta = dot(L, -light.direction);
        attenuation *= saturate((theta - light.cos_outer) / max(light.cos_inner - light.cos_outer, 0.0001f));
    }

    return light.color * light.intensity * attenuation;
}

PS_OUT PShader(PS_INPUT input)
{
    PS_OUT output;

    int3 pixel = int3(input.position.xy, 0);
    float3 N = normal_tex.Load(pixel).xyz;
    float4 emissive = emissive_tex.Load(pixel);
    emissive.rgb *= emissive.a;

    // Sin geometria (cielo): lo pinta despues draw_skybox
    if (dot(N, N) < 0.001f)
    {
        output.out_light = float4(0.0f, 0.0f, 0.0f, 1.0f);
        output.out_emissive = emissive;
        return output;
    }

    N = normalize(N);
    float3 P = position_tex.Load(pixel).xyz;
    float3 albedo = albedo_tex.Load(pixel).rgb;
    float3 material = material_tex.Load(pixel).rgb;
    float metallic = material.r;
    float roughness = material.g;
    float ao = material.b;
    float screen_ao = ssao_power > 0.0f ? pow(saturate(ssao_tex.Load(pixel).r), ssao_power) : 1.0f;

    float3 V = normalize(cam_pos - P);
    float NdotV = max(dot(N, V), 0.0f);
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

    // Luz directa
    float3 color = float3(0.0f, 0.0f, 0.0f);

    [loop]
    for (uint i = 0; i < light_count; i++)
    {
        GPULight light = lights[i];

        float3 L;
        float3 radiance = LightRadiance(light, P, L);
        float NdotL = dot(N, L);
        if (NdotL <= 0.0f || dot(radiance, radiance) <= 0.0f)
            continue;

        float3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(NdotV, NdotL, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);

        float3 specular = NDF * G * F / (4.0f * NdotV * NdotL + 0.0001f);
        float3 kD = (1.0f - F) * (1.0f - metallic);

        color += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Fisicamente el AO no afecta a la luz directa; una parte se aplica para que se vea bajo luz fuerte
    color *= lerp(1.0f, screen_ao, ssao_direct);

    // Ambiente difuso e IBL especular, una sola vez. Aqui el AO se aplica entero.
    float3 reflected = reflect(-V, N);
    float3 reflect_color = skybox_tex.SampleLevel(linear_sampler, reflected, roughness * cubemap_max_mip_level).rgb;
    float2 env_brdf = brdf_tex.SampleLevel(linear_sampler, float2(NdotV, roughness), 0.0f).rg;
    float3 F_ibl = FresnelSchlickRoughness(NdotV, F0, roughness);

    float3 kD_ibl = (1.0f - F_ibl) * (1.0f - metallic);
    float3 irradiance = irradiance_tex.SampleLevel(linear_sampler, N, 0.0f).rgb * IRRADIANCE_SCALE;

    const float occlusion = ao * screen_ao;
    color += reflect_color * (F_ibl * env_brdf.x + env_brdf.y) * occlusion * ambient_intensity;
    color += kD_ibl * albedo * irradiance * occlusion * ambient_intensity;

    // Bloom sobre la luz total
    float bright = smoothstep(BLOOM_START, BLOOM_END, dot(color, LUMINANCE));
    output.out_emissive = float4(lerp(emissive.rgb, color * (1.0f + emissive.rgb) * BLOOM_STRENGTH, bright), 1.0f);

    color += emissive.rgb;

    output.out_light = float4(color, 1.0f);
    return output;
}
