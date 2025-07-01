struct PS_IN{

  float4 position : SV_POSITION;
  float3 direction : TEXCOORD0;
};


TextureCube skybox_tex : register(t0);
SamplerState mySampler : register(s0);

float4 PShader(PS_IN input) : SV_TARGET
{

  float3 uv_normalized = normalize(input.direction).rgb;
  float3 color = skybox_tex.Sample(mySampler, uv_normalized).rgb;
  //return float4(1.0, 1.0, 0.0, 1.0);
  return float4(color.rgb, 1.0);

}

