struct VOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float4 PShader(VOut input) : SV_TARGET
{

    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}

