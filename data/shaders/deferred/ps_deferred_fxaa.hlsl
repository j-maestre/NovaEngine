// FXAA: suaviza los dientes de sierra a partir de la imagen final, ya tonemapeada y en sRGB.
// Busca bordes por contraste de brillo, los recorre para ver donde acaban y mezcla el pixel
// con su vecino del otro lado del borde. Tambien aplica el dithering, que debe ir despues.

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

Texture2D color_texture : register(t0);
SamplerState linear_clamp : register(s0);

cbuffer FxaaConstants : register(b0)
{
    float2 texel_size;
    int fxaa_enabled;
    float padding;
};

// Contraste minimo para considerar que hay borde: absoluto y relativo al brillo de la zona
static const float EDGE_MIN = 0.0312f;
static const float EDGE_RELATIVE = 0.125f;

// Cuanto se suavizan los detalles de menos de un pixel. Mas alto emborrona mas las texturas.
static const float SUBPIXEL_AMOUNT = 0.75f;

// Pasos al recorrer el borde en cada sentido: cada vez mas largos para llegar lejos con pocas lecturas
static const int EDGE_STEPS = 10;
static const float STEP_SIZES[EDGE_STEPS] = { 1.0f, 1.5f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 4.0f, 8.0f };

float Luma(float3 color)
{
    return dot(color, float3(0.299f, 0.587f, 0.114f));
}

float LumaAt(float2 uv, float2 offset)
{
    return Luma(color_texture.SampleLevel(linear_clamp, uv + offset * texel_size, 0.0f).rgb);
}

float3 Fxaa(float2 uv, float3 center_color)
{
    const float center = Luma(center_color);
    const float north = LumaAt(uv, float2(0.0f, -1.0f));
    const float south = LumaAt(uv, float2(0.0f, 1.0f));
    const float east = LumaAt(uv, float2(1.0f, 0.0f));
    const float west = LumaAt(uv, float2(-1.0f, 0.0f));

    const float highest = max(center, max(max(north, south), max(east, west)));
    const float lowest = min(center, min(min(north, south), min(east, west)));
    const float contrast = highest - lowest;
    if (contrast < max(EDGE_MIN, highest * EDGE_RELATIVE))
        return center_color;

    const float north_east = LumaAt(uv, float2(1.0f, -1.0f));
    const float north_west = LumaAt(uv, float2(-1.0f, -1.0f));
    const float south_east = LumaAt(uv, float2(1.0f, 1.0f));
    const float south_west = LumaAt(uv, float2(-1.0f, 1.0f));

    // Mezcla por detalle de menos de un pixel: el pixel frente a la media de sus vecinos
    const float average = (2.0f * (north + south + east + west) + north_east + north_west + south_east + south_west) / 12.0f;
    const float subpixel = smoothstep(0.0f, 1.0f, saturate(abs(average - center) / contrast));
    const float subpixel_blend = subpixel * subpixel * SUBPIXEL_AMOUNT;

    // Direccion del borde: horizontal si el brillo cambia mas en vertical
    const float horizontal_change = abs(north + south - 2.0f * center) * 2.0f + abs(north_east + south_east - 2.0f * east) + abs(north_west + south_west - 2.0f * west);
    const float vertical_change = abs(east + west - 2.0f * center) * 2.0f + abs(north_east + north_west - 2.0f * north) + abs(south_east + south_west - 2.0f * south);
    const bool is_horizontal = horizontal_change >= vertical_change;

    // Lado del borde al que hay que mezclar: el de mayor diferencia de brillo
    const float luma_positive = is_horizontal ? south : east;
    const float luma_negative = is_horizontal ? north : west;
    const float gradient_positive = abs(luma_positive - center);
    const float gradient_negative = abs(luma_negative - center);
    const bool towards_positive = gradient_positive >= gradient_negative;
    const float gradient = max(gradient_positive, gradient_negative);
    const float opposite = towards_positive ? luma_positive : luma_negative;
    const float edge_luma = (center + opposite) * 0.5f;
    const float threshold = gradient * 0.25f;

    const float2 across = is_horizontal ? float2(0.0f, texel_size.y) : float2(texel_size.x, 0.0f);
    const float2 along = is_horizontal ? float2(texel_size.x, 0.0f) : float2(0.0f, texel_size.y);
    const float2 edge_uv = uv + across * (towards_positive ? 0.5f : -0.5f);

    // Recorrer el borde en los dos sentidos hasta que el brillo deja de ser el suyo
    float distance_positive = 0.0f;
    float distance_negative = 0.0f;
    float delta_positive = 0.0f;
    float delta_negative = 0.0f;
    bool done_positive = false;
    bool done_negative = false;

    for (int i = 0; i < EDGE_STEPS && !(done_positive && done_negative); i++)
    {
        if (!done_positive)
        {
            distance_positive += STEP_SIZES[i];
            delta_positive = Luma(color_texture.SampleLevel(linear_clamp, edge_uv + along * distance_positive, 0.0f).rgb) - edge_luma;
            done_positive = abs(delta_positive) >= threshold;
        }
        if (!done_negative)
        {
            distance_negative += STEP_SIZES[i];
            delta_negative = Luma(color_texture.SampleLevel(linear_clamp, edge_uv - along * distance_negative, 0.0f).rgb) - edge_luma;
            done_negative = abs(delta_negative) >= threshold;
        }
    }

    // Solo se mezcla si, en el extremo mas cercano, el borde se aleja de este pixel
    const bool positive_is_closest = distance_positive <= distance_negative;
    const float closest = positive_is_closest ? distance_positive : distance_negative;
    const float closest_delta = positive_is_closest ? delta_positive : delta_negative;
    const float edge_blend = (closest_delta < 0.0f) != (center - edge_luma < 0.0f) ? 0.5f - closest / (distance_positive + distance_negative) : 0.0f;

    const float blend = max(edge_blend, subpixel_blend);
    return color_texture.SampleLevel(linear_clamp, uv + across * (towards_positive ? blend : -blend), 0.0f).rgb;
}

float4 PShader(VS_OUT input) : SV_TARGET
{
    float3 color = color_texture.SampleLevel(linear_clamp, input.uv, 0.0f).rgb;
    if (fxaa_enabled != 0)
        color = Fxaa(input.uv, color);

    // Dithering: menos de un nivel de ruido antes de pasar a 8 bits, para que no se vean bandas
    float noise = frac(52.9829189f * frac(dot(input.position.xy, float2(0.06711056f, 0.00583715f))));
    color += (noise - 0.5f) / 255.0f;

    return float4(color, 1.0f);
}
