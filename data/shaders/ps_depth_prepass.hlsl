

struct VS_OUT
{
    float4 position : SV_POSITION;
    float depth : TEXCOORD0;
};


float PShader(VS_OUT input) : SV_TARGET
{
    
    float depth = input.depth * 0.5f + 0.5f;
    
    return depth;
}