#ifndef COLOR_COMMON_HLSLI
#define COLOR_COMMON_HLSLI

float3 SRGBToLinear(float3 color)
{
    color = saturate(color);
    return lerp(color / 12.92f, pow((color + 0.055f) / 1.055f, 2.4f), step(0.04045f, color));
}

float3 LinearToSRGB(float3 color)
{
    color = saturate(color);
    return lerp(color * 12.92f, 1.055f * pow(color, 1.0f / 2.4f) - 0.055f, step(0.0031308f, color));
}

#endif
