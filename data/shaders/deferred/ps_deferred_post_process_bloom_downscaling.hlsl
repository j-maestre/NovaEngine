

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};


Texture2D finalTexture : register(t0);
Texture2D lightTexture : register(t1);
SamplerState samp : register(s0);

cbuffer PostprocessConstants : register(b0)
{
    
    float2 texel_size; // 1.0 / texture resolution (e.g., (1.0 / 1920.0, 1.0 / 1080.0))
    float bloom_intensity; // how strong the bloom is
    int horizontal;
    int blend;
    float3 padding;
};


float4 PShader(PS_INPUT input) : SV_TARGET
{
    float2 offsets[9] =
    {
        float2(-1, -1), float2(0, -1), float2(1, -1),
        float2(-1, 0), float2(0, 0), float2(1, 0),
        float2(-1, 1), float2(0, 1), float2(1, 1)
    };

    float kernel[9] =
    {
        1.0, 2.0, 1.0,
        2.0, 4.0, 2.0,
        1.0, 2.0, 1.0
    };
    
    float weight_sum = 0.0;
    float3 color_sum = float3(0, 0, 0);
    
    
    if (horizontal == 1){

        for (int i = 0; i < 3; i++){

            float2 offset_uv = input.uv + float2(offsets[i + 3].x * texel_size.x, 0);
            float3 sample = finalTexture.Sample(samp, offset_uv).rgb;
            float weight = kernel[i + 3];
            color_sum += sample * weight;
            weight_sum += weight;
        }


        // HDR
        //color_sum = float3(1.0, 1.0, 1.0) - exp(-color_sum * 1.0);

        // Gamma
        //float3 gamma_exponent = float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2);
        //color_sum = pow(color_sum, gamma_exponent);
    } else {
        for (int i = 0; i < 3; i++){

            float2 offset_uv = input.uv + float2(0, offsets[3 * i + 1].y * texel_size.y);
            float3 sample = finalTexture.Sample(samp, offset_uv).rgb;
            float weight = kernel[3 * i + 1];
            color_sum += sample * weight;
            weight_sum += weight;
        }

        // Add final color

        // HDR
        //color_sum = float3(1.0, 1.0, 1.0) - exp(-color_sum * 1.0);

        // Gamma
        //float3 gamma_exponent = float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2);
        //color_sum = pow(color_sum, gamma_exponent);
    }

    float3 final_color = color_sum / weight_sum;
    //final_color *= finalTexture.Sample(samp, input.uv).a;
        
    if (blend == 1){
        final_color += lightTexture.Sample(samp, input.uv).rgb;
    }

    return float4(final_color, 1.0);
    
};