#include "color_common.hlsli"

// Ultimo paso: de HDR lineal a lo que muestra el backbuffer

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

    float3 output_color = LinearToSRGB(color);

    // Dithering: menos de un nivel de ruido antes de pasar a 8 bits, para que no se vean bandas
    float noise = frac(52.9829189f * frac(dot(input.position.xy, float2(0.06711056f, 0.00583715f))));
    output_color += (noise - 0.5f) / 255.0f;

    return float4(output_color, 1.0f);
}
