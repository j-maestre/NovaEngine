#include "pbr_common.hlsli"

// Prefiltrado de una sonda: cada mip es el reflejo para una rugosidad (mip 0 = espejo, el ultimo = 1).
// Para cada texel se reparten muestras segun GGX alrededor de su direccion (importance sampling, Karis)
// y cada muestra lee el mip de la captura que cubre su trozo de esfera, para que haya poco ruido
// con pocas muestras ("filtered importance sampling", GPU Gems 3, cap. 20).

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

TextureCube source_tex : register(t0); // La captura, con todos sus mips
SamplerState linear_sampler : register(s0);

cbuffer Prefilter : register(b0)
{
    uint face;
    float roughness;
    float face_size;          // Lado de la cara en este mip
    float source_resolution;  // Lado de la captura en el mip 0
};

static const uint SAMPLES = 64;

// Direccion de un texel de una cara, con el convenio de D3D: +X, -X, +Y, -Y, +Z, -Z
float3 FaceDirection(uint index, float2 uv)
{
    float2 p = uv * 2.0f - 1.0f;
    float3 d;
    if (index == 0) d = float3(1.0f, -p.y, -p.x);
    else if (index == 1) d = float3(-1.0f, -p.y, p.x);
    else if (index == 2) d = float3(p.x, 1.0f, p.y);
    else if (index == 3) d = float3(p.x, -1.0f, -p.y);
    else if (index == 4) d = float3(p.x, -p.y, 1.0f);
    else d = float3(-p.x, -p.y, -1.0f);
    return normalize(d);
}

float2 Hammersley(uint i, uint count)
{
    return float2(float(i) / float(count), float(reversebits(i)) * 2.3283064365386963e-10f);
}

float3 ImportanceSampleGGX(float2 xi, float3 N, float alpha)
{
    float phi = 2.0f * PI * xi.x;
    float cos_theta = sqrt((1.0f - xi.y) / (1.0f + (alpha * alpha - 1.0f) * xi.y));
    float sin_theta = sqrt(1.0f - cos_theta * cos_theta);
    float3 H = float3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);

    float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

float4 PShader(PS_INPUT input) : SV_TARGET
{
    float3 N = FaceDirection(face, input.position.xy / face_size);

    if (roughness <= 0.0f)
        return float4(source_tex.SampleLevel(linear_sampler, N, 0.0f).rgb, 1.0f);

    // Se supone que se mira de frente (V = N), como en el split-sum del cubemap del cielo
    float3 V = N;
    float alpha = roughness * roughness;
    float texel_solid_angle = 4.0f * PI / (6.0f * source_resolution * source_resolution);

    float3 color = float3(0.0f, 0.0f, 0.0f);
    float weight = 0.0f;

    [loop]
    for (uint i = 0; i < SAMPLES; i++)
    {
        float3 H = ImportanceSampleGGX(Hammersley(i, SAMPLES), N, alpha);
        float3 L = 2.0f * dot(V, H) * H - V;
        float NdotL = dot(N, L);
        if (NdotL <= 0.0f)
            continue;

        float NdotH = saturate(dot(N, H));
        float a2 = alpha * alpha;
        float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
        float D = a2 / (PI * denom * denom);
        // Con V = N, pdf = D * NdotH / (4 * VdotH) = D / 4
        float pdf = D * 0.25f + 0.0001f;
        float sample_solid_angle = 1.0f / (float(SAMPLES) * pdf);
        float mip = max(0.5f * log2(sample_solid_angle / texel_solid_angle) + 1.0f, 0.0f);

        color += source_tex.SampleLevel(linear_sampler, L, mip).rgb * NdotL;
        weight += NdotL;
    }

    return float4(color / max(weight, 0.0001f), 1.0f);
}
