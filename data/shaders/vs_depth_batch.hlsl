// Prepass con un solo draw call: toda la geometria esta en un mismo buffer y cada vertice
// lleva el indice de su objeto, para leer la matriz en un StructuredBuffer.

struct VS_IN
{
    float3 position : POSITION;
    uint object_index : OBJECT_INDEX;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
};

cbuffer CameraDepthPrePass : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

StructuredBuffer<float4x4> object_matrices : register(t0);

VS_OUT VShader(VS_IN input)
{
    VS_OUT output;
    float4 world_pos = mul(float4(input.position, 1.0f), object_matrices[input.object_index]);
    float4 view_pos = mul(world_pos, view);
    output.position = mul(view_pos, projection);
    return output;
}
