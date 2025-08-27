struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

cbuffer SSAOConstantBuffer : register(b0)
{
    float4x4 projection;
    float4x4 view;
    float4x4 projection_inverse;
    float4 kernel_samples[64];
    int samples;
    float radius_base;
    float samples_float;
    float width;
    float height;
    float3 padding;
};

Texture2D positionTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D noiseTexture : register(t2);
Texture2D depthTexture : register(t3);

SamplerState samp : register(s0);
SamplerState samp_clamp : register(s1);


float PShader(VS_OUT input) : SV_TARGET {
    
    float depth_value = depthTexture.Sample(samp_clamp, input.uv).r;
    
    float2 noiseScale = float2(width / 4.0f, height / 4.0f);
    
    float4 fragPos = positionTexture.Sample(samp, input.uv);
    float4 fragPos_view = mul(view, fragPos);
    fragPos_view = fragPos;
    
    float3 normal = normalize(normalTexture.Sample(samp, input.uv).rgb).xyz * 2.0f - 1.0f;
    float3 normal_view = normalize(mul(view, float4(normal, 0.0f)));
    
    float3 randomVec = normalize(noiseTexture.Sample(samp, input.uv * noiseScale).rgb);
    
    float3 tangent = normalize(randomVec - normal_view * dot(randomVec, normal_view));// + 1e-5);
    float3 bitangent = cross(tangent, normal_view);
    float3x3 TBN = float3x3(tangent, bitangent, normal_view);
    
    float bias = 0.025f;
    
    float occlusion = 0.0f;
    for (int i = 0; i < samples; i++){
        
        
        //float3 position = 
        
        
        
        
        float3 tmp = mul(TBN, kernel_samples[i].xyz);
        tmp = fragPos_view.xyz + tmp * radius_base;
        
        float4 offset = float4(tmp, 1.0f);
        offset.xyz = mul(projection, offset);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5f + 0.5f; // Transform to 0,1
        float3 tmp_depth = positionTexture.Sample(samp, offset.xy).xyz; // pasar a view space ?
        tmp_depth.z = 0.0f;
        float sampleDepth = mul(view, float4(tmp_depth, 1.0f)).z;
        
        float range_check = smoothstep(0.0f, 1.0f, radius_base / abs(fragPos_view.z - sampleDepth));
        occlusion += (sampleDepth >= tmp.z + bias ? 1.0f : 0.0f) * range_check;
        
        
        
        /*
        float3 samplePos = fragPos_view.xyz + mul(kernel_samples[i].xyz, TBN) * radius_base;
        float3 dir = samplePos - fragPos_view.xyz;
        float dist = length(dir);

        // Normalizado
        float3 dirNorm = dir / dist;
        float NdotS = saturate(dot(normal_view, dirNorm));

        // Suavizado por distancia
        float attenuation = smoothstep(0.0f, radius_base, dist);

        occlusion += (1.0f - attenuation) * NdotS;
        */
    }

    occlusion = 1.0f - (occlusion / samples_float);
    //return occlusion;
    occlusion = 1.0f;
    return occlusion;
    
    
    // Normalizar
    //occlusion /= samples_float;
    //occlusion = 1.0f - occlusion; // invertir para que 1 = sin oclusión
    
    //return fragPos.x;
    //return occlusion + (fragPos.x * 0.001f);
   
}