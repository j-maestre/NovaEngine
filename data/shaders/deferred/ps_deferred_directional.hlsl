struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};


Texture2D albedo_tex : register(t0);
Texture2D position_tex : register(t1);
Texture2D normal_tex : register(t2);
Texture2D gMaterial : register(t3); // RGB = metallic, roughness, ao
Texture2D gEmissive : register(t4);

Texture2D gDepth : register(t5); // Needed to reconstruct world pos


SamplerState mySampler : register(s0);



cbuffer CameraDeferred : register(b0){
    
    float4x4 inv_view_proj;
    float3 cam_pos;
    float padding;
}

float4 PShader(PS_INPUT input) : SV_TARGET
{
    float4 out_color;
    
    const float PI = 3.14159265359;
    
    // TODO: Mira a ver, que tienes que pasarle al vertex shader el valor de los materiales
    // para poder guardar en una textura el color de la textura de albedo o el color del material a pelo,
    // igual en los otros, suerte espabilao
    const float4 texture_position = (position_tex.Sample(mySampler, input.uv));
    
    const float3 view_dir = normalize(cam_pos - texture_position.rgb);
    
    const float4 texture_color = (albedo_tex.Sample(mySampler, input.uv));
    const float4 texture_normal = (normal_tex.Sample(mySampler, input.uv));
    
    const float texture_metallic = (gMaterial.Sample(mySampler, input.uv)).r;
    const float texture_roughness = (gMaterial.Sample(mySampler, input.uv)).g;
    
    const float texture_ao = (gMaterial.Sample(mySampler, input.uv)).b;


    float3 color = float3(texture_position.rgb + texture_color.rgb + texture_normal.rgb + texture_metallic.rrr + texture_roughness.rrr + texture_ao.rrr * 0.01);

    return float4(color, 1.0);
}

