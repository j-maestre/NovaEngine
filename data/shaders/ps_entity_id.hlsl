// Pase de seleccion: escribe en cada pixel el ID de la entidad visible (0 = nada)

struct VS_OUT
{
    float4 position : SV_POSITION;
};

cbuffer EntityIdConstants : register(b0)
{
    uint entity_id;
    float3 padding;
};

uint PShader(VS_OUT input) : SV_TARGET
{
    return entity_id;
}
