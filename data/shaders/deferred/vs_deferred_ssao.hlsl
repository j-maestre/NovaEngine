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
    output.uv = input.uv;
    
    
	// Adjust the position based on the UV
    output.position = float4(output.uv, 0.0f, 1.0f);
    output.position.x = output.position.x * 2.0f - 1.0f;
    output.position.y = output.position.y * -2.0f + 1.0f;
    
    return output;
}

