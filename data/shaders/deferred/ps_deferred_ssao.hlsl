struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

cbuffer SSAOConstantBuffer : register(b0)
{
    float4x4 projection;
    float4x4 view;
    float4x4 projection_inverse;
    
    float4 kernel_samples[64];
    int samples;
    float radius_base;
    float samples_float;
    float width;
    float height;
    float max_radius_px;
    float resolution_scale; // 1 = completa, 2 = media
    float padding;
};

Texture2D normalTexture : register(t0);
Texture2D noiseTexture : register(t1);
Texture2D depthTexture : register(t2);

SamplerState samp : register(s0);
SamplerState samp_clamp : register(s1);

// Radio maximo en pantalla. Al acercarse, un radio fijo en mundo ocupa media
// pantalla: las muestras quedan muy separadas, la cache de texturas falla y
// los fps se hunden. Con este tope el coste es estable.
static const float BIAS = 0.025f;

float3 ViewSpaceFromDepth(float depth, float2 uv)
{
    // Back to NDCs
    uv.y = 1.0f - uv.y; // Invert Y due to UV <--> NDC diff
    uv = uv * 2.0f - 1.0f;
    float4 screenPos = float4(uv, depth, 1.0f);
    // Back to view space
    float4 viewPos = mul(screenPos, projection_inverse);
    return viewPos.xyz / viewPos.w;
}

float2 UVFromViewSpacePosition(float3 viewSpacePosition)
{
    float4 samplePosScreen = mul(float4(viewSpacePosition, 1.0f), projection);
    samplePosScreen.xyz /= samplePosScreen.w;
    samplePosScreen.xy = samplePosScreen.xy * 0.5f + 0.5f;
    samplePosScreen.y = 1.0f - samplePosScreen.y;
    return samplePosScreen.xy;
}

// Lectura de profundidad SIN filtrar. Con un sampler lineal/anisotropico se
// interpolan profundidades a ambos lados de un borde y aparecen superficies
// falsas que oscurecen los laterales de los objetos.
float LoadDepth(float2 uv, float2 size)
{
    int2 p = clamp(int2(uv * size), int2(0, 0), int2(size) - 1);
    return depthTexture.Load(int3(p, 0)).r;
}

// R: oclusion (1 = nada). G: distancia a la camara del punto usado, para el upsample del pase de luces.
float2 PShader(VS_OUT input) : SV_TARGET
{
    float2 size;
    depthTexture.GetDimensions(size.x, size.y);
    int2 ssao_pixel = int2(input.position.xy);

    // A media resolucion cada pixel cubre 2x2 de la profundidad: se usa el mas cercano a la camara,
    // para que un objeto fino delante no desaparezca entre medias
    int scale = int(resolution_scale);
    int2 pixel = min(ssao_pixel * scale, int2(size) - 1);
    float pixel_depth = depthTexture.Load(int3(pixel, 0)).r;
    for (int by = 0; by < scale; by++)
    {
        for (int bx = 0; bx < scale; bx++)
        {
            int2 p = min(ssao_pixel * scale + int2(bx, by), int2(size) - 1);
            float d = depthTexture.Load(int3(p, 0)).r;
            if (d < pixel_depth)
            {
                pixel_depth = d;
                pixel = p;
            }
        }
    }
    if (pixel_depth >= 1.0f)
        return float2(1.0f, 60000.0f);

    float2 pixel_uv = (float2(pixel) + 0.5f) / size;

    // Posicion reconstruida de la misma profundidad que las muestras: asi el
    // pixel y sus muestras tienen la misma precision (el G-buffer de posicion
    // es fp16 y no coincide con la profundidad R32 -> auto-oclusion y ruido).
    float3 P = ViewSpaceFromDepth(pixel_depth, pixel_uv);

    // Ruido 4x4 por pixel, sin filtrar y sin depender del tamano de ventana
    float3 random_dir = noiseTexture.Load(int3(ssao_pixel % 4, 0)).xyz;

    float3 normal = normalTexture.Load(int3(pixel, 0)).xyz;
    normal = normalize(mul(normal, (float3x3) view));

    // Limitar el radio proyectado en pantalla
    float radius_px = radius_base * projection[1][1] * 0.5f * size.y / P.z;
    float radius = radius_base * min(1.0f, max_radius_px / radius_px);

    // TBN
    float3 tangent = normalize(random_dir - normal * dot(random_dir, normal));
    float3 bitangent = cross(tangent, normal);
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0f;

    [loop]
    for (int i = 0; i < samples; i++)
    {
        float3 samplePosView = P + mul(kernel_samples[i].xyz, TBN) * radius;
        float2 sampleUV = UVFromViewSpacePosition(samplePosView);

        // Fuera de pantalla: no hay informacion, no ocluye
        if (any(sampleUV < 0.0f) || any(sampleUV > 1.0f))
            continue;

        float sampleZ = ViewSpaceFromDepth(LoadDepth(sampleUV, size), sampleUV).z;

        float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(P.z - sampleZ));
        occlusion += (sampleZ < samplePosView.z - BIAS ? rangeCheck : 0.0f);
    }

    return float2(1.0f - occlusion / samples_float, length(P));
}