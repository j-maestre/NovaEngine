struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

cbuffer SSAOConstantBuffer : register(b0)
{
    float4x4 projection;
    float4x4 view;
    float4 kernel_samples[16];
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

float AmbientOcclusionFunction(float2 uv, float3 position, float3 normal){
    
    float3 posVector;
    float3 vec;
    float distance;
    float occlusion;

    float bias = 0.025;
    float ssao_scale = 1.0f;
    float ssao_intensity = 2.0f;
    
    // Get the position vector from the position portion of the G buffer.
    posVector = positionTexture.Sample(samp, uv);

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


float4 PShader(VS_OUT input) : SV_TARGET {
    
    // Get data from G-Buffer
    float3 position = positionTexture.Sample(samp, input.uv).rgb;
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
    float radius = 0.5f / position.z; // TODO: parametrize radius into cbuffer
    
    const int count = 4;
    float occlusion = 0.0f;
    float2 tex_coord_1;
    float2 tex_coord_2;
    
    for (int i = 0; i < count; i++){
        
        tex_coord_1 = reflect(vector_array[i], random_vector) * radius;
        tex_coord_2 = float2(((tex_coord_1.x * 0.75f) - (tex_coord_1.y * 0.75f)), ((tex_coord_1.x * 0.75f) + (tex_coord_1.y * 0.75f)));
        
        occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_1 * 0.25f), position, normal);
        occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_2 * 0.5f), position, normal);
        occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_1 * 0.75f), position, normal);
        occlusion += AmbientOcclusionFunction(input.uv + (tex_coord_2 * 1.0f), position, normal);
        
    }
    
    // Average of the sum based on how many loops
    occlusion = occlusion / (((float) count) * 4.0f);
    
    // Invert occlusion output
    occlusion = 1.0f - occlusion;
    
    return float4(occlusion, occlusion, occlusion, 1.0);
}