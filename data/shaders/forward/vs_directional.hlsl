struct VOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float2 uv : UV)
{
    VOut output;

    output.position = float4(position, 1.0f);
    output.normal = float3(1.0f, 1.0f, 1.0f);
    output.uv = float2(1.0f, 1.0f);

    return output;
}

