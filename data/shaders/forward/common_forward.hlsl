#ifndef COMMON_FORWARD
#define COMMON_FORWARD

struct VOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};


#endif