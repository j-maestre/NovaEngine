struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};
    
struct VOut{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

//VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float2 uv : UV)
VOut VShader(VS_IN input)
{
    VOut output;

    output.position = float4(input.position, 1.0f);
    output.normal = float3(input.normal);
    output.uv = float2(input.uv);
    //output.normal = float3(1.0f, 1.0f, 1.0f);
    //output.uv = float2(1.0f, 1.0f);

    return output;
}

