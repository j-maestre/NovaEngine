

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

Texture2D finalTexture : register(t0);
SamplerState samp : register(s0);

float4 PShader(VS_OUT input) : SV_TARGET
{
    return finalTexture.Sample(samp, input.uv);
}