#ifndef PBR_COMMON_HLSLI
#define PBR_COMMON_HLSLI

static const float PI = 3.14159265359f;

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotX, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return NdotX / (NdotX * (1.0f - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(saturate(1.0f - cosTheta), 5.0f);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max((1.0f - roughness).xxx, F0) - F0) * pow(saturate(1.0f - cosTheta), 5.0f);
}

// x^5 con multiplicaciones: pow() son un logaritmo y una exponencial
float Pow5(float x)
{
    float x2 = x * x;
    return x2 * x2 * x;
}

// Difuso de Burley (Disney), con la renormalizacion de Frostbite para no devolver mas luz de la que llega.
// Las superficies rugosas se aclaran algo en los bordes y las lisas se oscurecen; Lambert es plano.
// Ya incluye el 1/PI.
float DiffuseBurley(float NdotV, float NdotL, float LdotH, float roughness)
{
    float energy_bias = 0.5f * roughness;
    float energy_factor = lerp(1.0f, 1.0f / 1.51f, roughness);
    float f90 = energy_bias + 2.0f * LdotH * LdotH * roughness;
    float light_scatter = 1.0f + (f90 - 1.0f) * Pow5(1.0f - NdotL);
    float view_scatter = 1.0f + (f90 - 1.0f) * Pow5(1.0f - NdotV);
    return light_scatter * view_scatter * energy_factor / PI;
}

// Sheen (tela, terciopelo): distribucion "Charlie" de Estevez y Kulla. Al contrario que GGX,
// tiene el maximo con la microfaceta de lado, por eso brilla en los bordes y no en el centro.
float DistributionCharlie(float NdotH, float roughness)
{
    float inv_alpha = 1.0f / max(roughness * roughness, 0.0078125f);
    float sin2h = max(1.0f - NdotH * NdotH, 0.0078125f);
    return (2.0f + inv_alpha) * pow(sin2h, inv_alpha * 0.5f) / (2.0f * PI);
}

// Visibilidad de Neubelt y Pettineo para el sheen
float VisibilityNeubelt(float NdotV, float NdotL)
{
    return saturate(1.0f / (4.0f * (NdotL + NdotV - NdotL * NdotV)));
}

// Normal en 2 valores de [0, 1] (octaedro): cabe en dos canales sin perder casi precision
float2 EncodeOctahedron(float3 n)
{
    n /= abs(n.x) + abs(n.y) + abs(n.z);
    float2 e = n.z >= 0.0f ? n.xy : (1.0f - abs(n.yx)) * (n.xy >= 0.0f ? 1.0f : -1.0f);
    return e * 0.5f + 0.5f;
}

float3 DecodeOctahedron(float2 e)
{
    e = e * 2.0f - 1.0f;
    float3 n = float3(e, 1.0f - abs(e.x) - abs(e.y));
    float t = saturate(-n.z);
    n.xy += n.xy >= 0.0f ? -t : t;
    return normalize(n);
}

// Dos valores de 8 bits en un canal UNORM de 16 bits, sin perdida
float PackTwo(float high, float low)
{
    uint h = (uint) round(saturate(high) * 255.0f);
    uint l = (uint) round(saturate(low) * 255.0f);
    return float(h * 256u + l) / 65535.0f;
}

float2 UnpackTwo(float value)
{
    uint v = (uint) round(value * 65535.0f);
    return float2(float(v >> 8), float(v & 255u)) / 255.0f;
}

// Visibilidad de Kelemen para el clear coat: barata, y la capa es casi siempre pulida
float VisibilityKelemen(float LdotH)
{
    return 0.25f / max(LdotH * LdotH, 1e-4f);
}

#endif
