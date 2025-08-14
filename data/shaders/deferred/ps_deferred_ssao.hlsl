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
    
    
    //float4 depth_tex = depthTexture.Sample(samp, input.uv);
    
    float3 position_tex = positionTexture.Sample(samp, input.uv).rgb;       // Position (world)
    float3 position_view = mul(float4(position_tex, 1.0), view).xyz;        // Position (view)
    float3 normal = normalTexture.Sample(samp, input.uv).rgb;               // Normal (world)
    //float3 normal = normalize(normal_raw * 2.0 - 1.0);
    
    const float2 noiseScale = float2(width / 4.0, height / 4.0);
    float3 noise = noiseTexture.Sample(samp_noise, input.uv * noiseScale).xyz;
    noise = normalize(noise * 2.0 - 1.0);

    // TBN
    float3 normal_view = normalize(mul(float4(normal, 0.0), view).xyz);
    float3 tangent = normalize(noise - normal_view * dot(noise, normal_view));
    //float3 tangent = noise - normal * dot(noise, normal);
    
    /*
    
    // noise barely parralel to normal
    if (all(tangent == 0.0f)){
        // pick any vector perpendicular to normal
        tangent = abs(normal.x) < 0.999 ? normalize(cross(normal, float3(1, 0, 0))) : normalize(cross(normal, float3(0, 1, 0)));
    }else{
        tangent = normalize(tangent);
    }
    */
    
    float3 bitangent = cross(normal_view, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal_view);
    
    
    float occlusion = 0.0;
    int kernel_size = 64;
    float radius = 0.5;
    float bias = 0.025;
    
    
    
    for (int i = 0; i < kernel_size; i++)
    {
        
        // Rotate sample with TBN and translate
        //float3 sample_vec = mul(kernel_samples[i].xyz, TBN);
        float3 sample_pos_view = position_view + mul(kernel_samples[i].xyz, TBN) * radius;
        //float3 sample_pos = position_tex + sample_vec * radius;
        
        
        // Project sample_pos to clip
        float4 offset = mul(float4(sample_pos_view, 1.0), projection); // World to clip
        offset.xyz /= offset.w;
        float2 offset_uv = offset.xy * 0.5 + 0.5;
        //offset.xy = offset.xy * 0.5 + 0.5;
        
        //if (sample_uv.x < 0.0f || sample_uv.x > 1.0f || sample_uv.y < 0.0f || sample_uv.y > 1.0f)continue;
        
        /*
        float4 offset = float4(samplePos, 1.0);
        offset = mul(offset, projection);           // View to clip space
        offset.xyz /= offset.w;                     // Perpective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;
        */        
        
        float3 sample_pos_world_from_tex = positionTexture.Sample(samp, offset_uv).rgb;
        float3 sample_pos_view_from_tex = mul(float4(sample_pos_world_from_tex, 1.0), view).xyz;

        // Comparar Z en espacio de vista
        float range_check = smoothstep(0.0, 1.0, radius / abs(position_view.z - sample_pos_view_from_tex.z));
        occlusion += (sample_pos_view_from_tex.z >= sample_pos_view.z + bias ? 1.0 : 0.0) * range_check;
        
        /*

        float sample_depth = positionTexture.Sample(samp, offset_uv).z;
        float range_check = smoothstep(0.0, 1.0, radius / abs(position_tex.z - sample_depth));
        
        occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
        */
    };
    
    occlusion = 1.0 - (occlusion / kernel_size);
    //occlusion += 0.5;
    //occlusion = abs(position_view.z);
    //occlusion = 1.0;
    //return float4(1.0, 0.0, 0.0, 1.0);
    return float4(occlusion, occlusion, occlusion, 1.0);
}