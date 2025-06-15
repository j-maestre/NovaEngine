struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};




Texture2D emissive_texture : register(t0);
Texture2D final_texture : register(t1);

SamplerState mySampler : register(s0);

cbuffer PostprocessConstants : register(b0){
    
    float2 texelSize; // 1.0 / texture resolution (e.g., (1.0 / 1920.0, 1.0 / 1080.0))
    float bloomIntensity; // how strong the bloom is
    int horizontal;
}

float4 PShader(PS_INPUT input) : SV_TARGET{
    
    float4 emissiveBloom = emissive_texture.Sample(mySampler, input.uv);
    float emissive_intensity = emissiveBloom.a;
    float3 finalColor = final_texture.Sample(mySampler, input.uv).rgb;
    float2 uv = input.uv;
    
    float2 texel_size;
    emissive_texture.GetDimensions(texel_size.x, texel_size.y);
    texel_size.x = 1.0f / texel_size.x;
    texel_size.y = 1.0f / texel_size.y;
    
    const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    
    float3 result = emissiveBloom.rgb * weight[0];
    
   
    
    if (horizontal == 1){
        for (int i = 1; i < 5; ++i){
            result += emissive_texture.Sample(mySampler, input.uv + float2(texel_size.x * i * emissive_intensity, 0.0)).rgb * weight[i];
            result += emissive_texture.Sample(mySampler, input.uv - float2(texel_size.x * i * emissive_intensity, 0.0)).rgb * weight[i];
        }
        
        // HDR
        result = float3(1.0, 1.0, 1.0) - exp(-result * 1.0);

        // Gamma
        float3 gamma_exponent = float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2);
        result = pow(result, gamma_exponent);
    }else{

        for (int i = 1; i < 5; ++i)
        {
            result += emissive_texture.Sample(mySampler, input.uv + float2(0.0, texel_size.y * i * emissive_intensity)).rgb * weight[i];
            result += emissive_texture.Sample(mySampler, input.uv - float2(0.0, texel_size.y * i * emissive_intensity)).rgb * weight[i];

        }
        
        // HDR
        result = float3(1.0, 1.0, 1.0) - exp(-result * 1.0);

        // Gamma
        float3 gamma_exponent = float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2);
        result = pow(result, gamma_exponent);
        
        result += finalColor; // Blend with light color
    }
    
    float3 prueba = float3(texel_size.r, texel_size.g, bloomIntensity) * 0.000005;
    
    
    
    return float4(result + prueba, 1.0);
   
}

