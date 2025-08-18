struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

cbuffer SSAOConstantBuffer : register(b0)
{
    float4x4 projection;
    float4x4 view;
    //float4 kernel_samples[16];
    int samples;
    float radius_base;
    float samples_float;
    float width;
    float height;
};

Texture2D positionTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D noiseTexture : register(t2);
//Texture2D depthTexture : register(t3);
SamplerState samp : register(s0);
SamplerState samp_noise : register(s1);

float AmbientOcclusionFunction(float2 uv, float3 position, float3 normal){
    
    float3 posVector;
    float3 vec;
    float distance;
    float occlusion;

    float bias = 0.025;
    float ssao_scale = 1.0f;
    float ssao_intensity = 2.0f;
    
    // Get the position vector from the position portion of the G buffer.
    float3 position_tex = positionTexture.Sample(samp, uv).rgb;
    posVector = mul(float4(position_tex, 1.0f), view);
    
    // Subtract the input position.
    posVector = posVector - position;

    // Normalize the result to get the vector between the occluder and the pixel being occluded.
    vec = normalize(posVector);
	
    // Calculate distance of occluder.
    distance = length(posVector) * ssao_scale;

    // Calculate the ambient occlusion.
    occlusion = max(0.0, dot(normal, vec) - bias) * (1.0f / (1.0f + distance)) * ssao_intensity;

    return occlusion;
}




float PShader(VS_OUT input) : SV_TARGET {
    
    // Get data from G-Buffer
    float3 position = positionTexture.Sample(samp, input.uv).rgb;
    position = mul(float4(position, 1.0), view); // To view spacae
    float3 normal = normalTexture.Sample(samp, input.uv).rgb;
    
    // Expand normals from 0,1 to -1,1
    normal = normalize((normal * 2.0f) - 1.0f);
    
    // Setup random texture sampling coordinates for the random vector
    float2 tex_coords = float2(width / 4.0f, height / 4.0f);
    tex_coords *= input.uv;
    
    // Move random vector from the noise texture
    float2 random_vector = noiseTexture.Sample(samp_noise, tex_coords).xy;
    
    // Move random vector to -1, 1 range
    random_vector = normalize((random_vector * 2.0f) - 1.0f);
    
    // Set up 4 vectors
    float2 vector_array[4];
    vector_array[0] = float2(1.0f, 0.0f);
    vector_array[1] = float2(-1.0f, 0.0f);
    vector_array[2] = float2(0.0f, 1.0f);
    vector_array[3] = float2(0.0f, -1.0f);
    
    // Set the sample radius to take into account the depth of the pixel
    float min_radius = 0.1f;
    float max_radius = 5.0f;
    float distance_scale = -position.z;
    float max_distance = 40.0f;
    
    //float radius = radius_base / position.z;
    float radius = min_radius + (max_radius - min_radius) * saturate(distance_scale / max_distance);
    radius = clamp(radius * (width / height), min_radius, max_radius);

    
    const int count = 4;
    float occlusion = 0.0f;
    float2 tex_coord_1;
    float2 tex_coord_2;
    for (int i = 0; i < count; i++){
        
        tex_coord_1 = reflect(vector_array[i], random_vector) * radius;
        tex_coord_2 = float2(((tex_coord_1.x * 0.75f) - (tex_coord_1.y * 0.75f)), ((tex_coord_1.x * 0.75f) + (tex_coord_1.y * 0.75f)));
        
        float fj = 1.0f;
        for (int j = 1; j <= samples; j++){
            float factor = (fj / samples_float);
            occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_1 * factor), position, normal);
            occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_2 * factor), position, normal);
            //occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_1 * 0.75f), position, normal);
            //occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_2 * 1.0f), position, normal);
            fj += 1.0f;
        }
        /*
            occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_1 * 0.25f), position, normal);
            occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_2 * 0.5f), position, normal);
            occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_1 * 0.75f), position, normal);
            occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_2 * 1.0f), position, normal);
        */
        
    }
    
    // Average of the sum based on how many loops
    occlusion = occlusion / (((float) count) * (samples * 2));
    
    // Invert occlusion output
    occlusion = 1.0f - occlusion;
    
    return occlusion;
}