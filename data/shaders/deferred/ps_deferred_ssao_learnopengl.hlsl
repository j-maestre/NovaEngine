struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

cbuffer SSAOConstantBuffer : register(b0)
{
    float4x4 projection;
    float4x4 view;
    float4 kernel_samples[64];
    float width;
    float height;
    float2 padding;
};

Texture2D positionTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D noiseTexture : register(t2);
//Texture2D depthTexture : register(t3);
SamplerState samp : register(s0);
SamplerState samp_noise : register(s1);

float4 PShader(VS_OUT input) : SV_TARGET {
  
    float3 position_tex = positionTexture.Sample(samp, input.uv).rgb;       // Position (world)
    float3 normal = normalTexture.Sample(samp, input.uv).rgb;               // Normal (world)

    
    const float2 noiseScale = float2(width / 4.0, height / 4.0);
    float3 random_vec = noiseTexture.Sample(samp_noise, input.uv * noiseScale).xyz;
    random_vec = normalize(random_vec * 2.0 - 1.0);

    // TBN
    float3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    
    
    float occlusion = 0.0;
    int kernel_size = 64;
    float radius = 0.5;
    float bias = 0.025;
    
    
    
    for (int i = 0; i < kernel_size; i++)
    {
        
        float3 sample_pos = mul(kernel_samples[i].xyz, TBN);
        sample_pos = position_tex + sample_pos * radius;
        
        float4 offset = float4(sample_pos, 1.0);
        offset = mul(offset, projection);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        float sample_depth = positionTexture.Sample(samp, offset.xy).z;
        float range_check = smoothstep(0.0, 1.0, radius / abs(position_tex.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
 
    };
    
    occlusion = 1.0 - (occlusion / kernel_size);

    return float4(occlusion, occlusion, occlusion, 1.0);
}