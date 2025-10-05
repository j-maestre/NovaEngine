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

float3 ViewSpaceFromDepth(float depth, float2 uv)
{
    // Back to NDCs
    uv.y = 1.0f - uv.y; // Invert Y due to UV <--> NDC diff
    uv = uv * 2.0f - 1.0f;
    float4 screenPos = float4(uv, depth, 1.0f);
    // Back to view space
    float4 viewPos = mul(projection_inverse, screenPos);
    return viewPos.xyz / viewPos.w;
}

float2 UVFromViewSpacePosition(float3 viewSpacePosition)
{
    // Apply the projection matrix to the view space position then perspective divide
    float4 samplePosScreen = mul(projection, float4(viewSpacePosition, 1.0f));
    samplePosScreen.xyz /= samplePosScreen.w;
    // Adjust from NDCs to UV coords (flip the Y!)
    samplePosScreen.xy = samplePosScreen.xy * 0.5f + 0.5f;
    samplePosScreen.y = 1.0f - samplePosScreen.y;
    // Return just the UVs
    return samplePosScreen.xy;
}

float PShader(VS_OUT input) : SV_TARGET {
    
    float pixel_depth = depthTexture.Sample(samp_clamp, input.uv);
    if (pixel_depth >= 1.0f) return 1.0f;
    
    //float3 pixelPositionViewSpace = ViewSpaceFromDepth(pixel_depth * 2.0f - 1.0f, input.uv).rgb;
    float3 pixelPositionViewSpace = positionTexture.Sample(samp, input.uv).xyz;
    pixelPositionViewSpace = mul(float4(pixelPositionViewSpace, 1.0f), view).xyz;
    
    float2 aspect_ratio = float2(width / 4.0f, height / 4.0f);
    float3 random_dir = noiseTexture.Sample(samp, input.uv * aspect_ratio).xyz;
    
    float3 normal = normalTexture.Sample(samp, input.uv).xyz * 2.0f - 1.0f;
    normal = normalize(mul(normal, (float3x3)view));
    
    // TBN
    float3 tangent = normalize(random_dir - normal * dot(random_dir, normal));
    float3 bitangent = cross(tangent, normal);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    
    float occlusion = 0.0f;
    
    for (int i = 0; i < samples; i++){
        float3 samplePosView = pixelPositionViewSpace + mul(kernel_samples[i].xyz, TBN) * radius_base;
        
        float2 samplePosScreen = UVFromViewSpacePosition(samplePosView);

        
        float sampleDepth = depthTexture.SampleLevel(samp_clamp, samplePosScreen.xy, 0.0f).r;
        float sampleZ = ViewSpaceFromDepth(sampleDepth, samplePosScreen.xy).z;
        
        float rangeCheck = smoothstep(0.0f, 1.0f, radius_base / abs(pixelPositionViewSpace.z - sampleZ));
        occlusion += (sampleZ < samplePosView.z ? rangeCheck : 0.0f);
                
        
        /*
        float3 samplePosViewFromTexture = positionTexture.Sample(samp, samplePosScreen.xy).xyz;
        samplePosViewFromTexture = mul(float4(samplePosViewFromTexture, 1), view).xyz;

        float rangeCheck = smoothstep(0.0f, 1.0f, radius_base / abs(pixelPositionViewSpace.z - samplePosViewFromTexture.z));
        occlusion += (samplePosViewFromTexture.z < samplePosView.z ? rangeCheck : 0.0f);
        */        
        
    }
    
    occlusion = 1.0f - occlusion / samples_float;
    return occlusion;

}