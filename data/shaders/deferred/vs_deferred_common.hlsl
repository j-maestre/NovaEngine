struct VS_IN
{
    float3 position : POSITION;
    float2 uv : UV;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

VS_OUT VShader(VS_IN input)
{
    VS_OUT output;
    output.position = float4(input.position, 1.0);
    output.uv = input.uv;
    return output;
}

