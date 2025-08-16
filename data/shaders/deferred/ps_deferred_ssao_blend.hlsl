

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

cbuffer SSAOBlendConstantBuffer : register(b0){
    float blend_intensity;
    float3 padding;
}

Texture2D ssaoTexture : register(t0);
Texture2D albedoTexture : register(t1);
SamplerState samp : register(s0);

float4 PShader(VS_OUT input) : SV_TARGET{
    
    //const float ssao_intensity = 1.0f;
    
    float ssao = ssaoTexture.Sample(samp, input.uv).r;
    ssao = lerp(1.0f, ssao, blend_intensity);
    float3 albedo = albedoTexture.Sample(samp, input.uv).rgb;
    
    return float4(albedo.rgb * ssao, 1.0);
}