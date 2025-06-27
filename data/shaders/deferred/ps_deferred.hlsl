struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : UV;
    float3 cam_pos : CAMERA_POSITION;
    
    float metallic_value : METALLIC;
    float4 color_value : COLOR;
    float roughness_value : ROUGHNESS;
    float3 emissive_value : EMISSIVE;
    float emissive_intensity : EMISSIVE_INTENSITY;
};

struct PS_OUT
{
    float4 out_albedo : SV_Target0;
    float4 out_position : SV_Target1;
    float4 out_normal : SV_Target2;
    float4 out_material : SV_Target3;
    float4 out_emissive : SV_Target4;
};


Texture2D albedo_tex : register(t0);
Texture2D normal_tex : register(t1);
Texture2D metallic_tex : register(t2);
Texture2D roughness_tex : register(t3);
Texture2D ao_tex : register(t4);
Texture2D emissive_tex : register(t5);


SamplerState mySampler : register(s0);


float3 getNormalFromMap(float3 normal_texture, float3 normal, float4 tangent)
{
    
    // Normal del mapa en espacio tangent (de [0,1] a [-1,1])
    float3 tangentNormal = normal_texture * 2.0f - 1.0f;

    float3 N = normalize(normal);
    float3 T = normalize(tangent.xyz);

    // Calcula bitangent usando el signo almacenado en tangent.w
    //float3 B = tangent.w * cross(N, T);
    float3 B = normalize(cross(N, T) * tangent.w);

    // Construye matriz TBN
    float3x3 TBN = float3x3(T, B, N);

    // Transforma la normal del mapa a espacio mundo
    return normalize(mul(tangentNormal, TBN));
}

PS_OUT PShader(PS_INPUT input) : SV_TARGET
{
    float4 out_color;
    
    const float PI = 3.14159265359;
  
    const float3 view_dir = normalize(input.cam_pos - input.world_position);
    const float4 texture_color = max((albedo_tex.Sample(mySampler, input.uv)), input.color_value);
    const float4 texture_normal = (normal_tex.Sample(mySampler, input.uv));
    
    const float texture_metallic = max((metallic_tex.Sample(mySampler, input.uv)).r, input.metallic_value);
    const float texture_roughness = max((roughness_tex.Sample(mySampler, input.uv)).r, input.roughness_value);
    
    const float texture_ao = (ao_tex.Sample(mySampler, input.uv)).r;
    const float3 texture_emissive = (emissive_tex.Sample(mySampler, input.uv)).rgb;
    
    const float3 normal_procesed = getNormalFromMap(texture_normal.rgb, input.normal, input.tangent);
   
    
    PS_OUT out_textures;
    out_textures.out_albedo = texture_color;
    out_textures.out_position = float4(input.world_position, 1.0);
    out_textures.out_normal = float4(normal_procesed, 1.0);
    out_textures.out_material = float4(texture_metallic, texture_roughness, texture_ao, 1.0); // Last parameter is for specular map or brithness
    out_textures.out_emissive = float4(max(input.emissive_value.rgb, texture_emissive), input.emissive_intensity); // Also add object's brithness

    return out_textures;
}

