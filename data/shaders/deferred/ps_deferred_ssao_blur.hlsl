struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

Texture2D ssao_texture : register(t0);

// Media 4x4 a resolucion completa. Coincide con el tile 4x4 del ruido del SSAO:
// cada ventana contiene todas las rotaciones, asi que el patron se cancela sin
// ensanchar la oclusion mas de 2 pixeles.
float PShader(PS_INPUT input) : SV_TARGET
{
    int2 size;
    ssao_texture.GetDimensions(size.x, size.y);
    int2 pixel = int2(input.position.xy);

    float sum = 0.0f;
    for (int y = -2; y < 2; y++)
    {
        for (int x = -2; x < 2; x++)
        {
            // Load fuera de rango devuelve 0: sin clamp los bordes se oscurecen
            int2 p = clamp(pixel + int2(x, y), int2(0, 0), size - 1);
            sum += ssao_texture.Load(int3(p, 0)).r;
        }
    }

    return sum / 16.0f;
}
