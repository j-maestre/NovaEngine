#include "color_common.hlsli"

// De HDR lineal a sRGB. El FXAA y el dithering van despues, en ps_deferred_fxaa.

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

Texture2D hdr_texture : register(t0);
SamplerState samp : register(s0);

cbuffer CompositeConstants : register(b0)
{
    float exposure;
    int tonemapper; // Debe coincidir con Tonemapper en defines.h
    float2 padding;
};

static const int TONEMAP_EXPONENTIAL = 0;
static const int TONEMAP_ACES = 1;

// Aproximacion de ACES de Krzysztof Narkowicz
float3 ACES(float3 x)
{
    return saturate((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f));
}

float4 PShader(VS_OUT input) : SV_TARGET
{
    float3 color = hdr_texture.Sample(samp, input.uv).rgb;

    // Tonemapping
    color *= exposure;
    if (tonemapper == TONEMAP_ACES)
        color = ACES(color);
    else
        color = 1.0f - exp(-color);

    return float4(LinearToSRGB(color), 1.0f);
}
