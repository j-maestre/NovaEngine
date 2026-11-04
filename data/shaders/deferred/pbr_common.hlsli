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

#endif
