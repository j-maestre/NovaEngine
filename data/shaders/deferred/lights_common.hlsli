#ifndef LIGHTS_COMMON_HLSLI
#define LIGHTS_COMMON_HLSLI

// Luces compartidas por el pase de luces y por la captura de las sondas de reflexion

static const int LIGHT_DIRECTIONAL = 0;
static const int LIGHT_POINT = 1;
static const int LIGHT_SPOT = 2;

// Debe coincidir con GPULight en defines.h
struct GPULight
{
    float3 position;
    int type;
    float3 direction;
    float intensity;
    float3 color;
    float range;
    float cos_inner;
    float cos_outer;
    float fall_start;
    float constant_att;
    float linear_att;
    float quadratic_att;
    float source_radius; // Point: radio de la esfera que emite
    float source_length; // Point: largo del tubo, a lo largo de direction
};

cbuffer LightPassConstants : register(b0)
{
    uint light_count;
    float ambient_intensity;
    float ssao_power;   // 0 = SSAO apagado
    float ssao_direct;  // Cuanto atenua el SSAO la luz directa
};

// El mapa de irradiancia tiene de media 2.1 veces el brillo del entorno: se normaliza para
// que una superficie blanca devuelva, de media, lo mismo que recibe.
static const float IRRADIANCE_SCALE = 0.48f;

// El mapa especular se horneo con la misma ganancia: de media, 2 veces el brillo del entorno
static const float SPECULAR_SCALE = 0.5f;

// Radiancia que llega al punto P. Devuelve 0 si la luz no le afecta.
// Las luces con tamano (Karis, "Real Shading in Unreal Engine 4") dan dos direcciones:
//  - L: hacia el punto de la fuente mas cercano a P. Lo usan la difusa y la caida con la distancia.
//  - L_spec: hacia el punto de la fuente mas cercano al rayo reflejado R. Es el que se ve reflejado,
//    y por eso el brillo toma la forma de la bombilla o del tubo en vez de ser un punto.
// spread: cuanto ensancha la fuente el brillo (x = tubo, y = esfera). Con AreaEnergy da cuanto baja
// su pico para que la luz total reflejada sea la misma; depende de la rugosidad de cada capa.
float3 LightRadiance(GPULight light, float3 P, float3 R, out float3 L, out float3 L_spec, out float2 spread)
{
    spread = float2(0.0f, 0.0f);
    if (light.type == LIGHT_DIRECTIONAL)
    {
        L = -light.direction;
        L_spec = L;
        return light.color * light.intensity;
    }

    float3 to_light = light.position - P;
    float3 to_spec = to_light;

    if (light.type == LIGHT_POINT && light.source_length > 0.0f)
    {
        // Tubo: segmento de L0 a L1 alrededor del centro
        float3 half_axis = light.direction * (light.source_length * 0.5f);
        float3 L0 = to_light - half_axis;
        float3 Ld = 2.0f * half_axis;
        float Ld2 = dot(Ld, Ld);

        to_light = L0 + Ld * saturate(-dot(L0, Ld) / Ld2);

        float RdotLd = dot(R, Ld);
        float t = (dot(R, L0) * RdotLd - dot(L0, Ld)) / max(Ld2 - RdotLd * RdotLd, 1e-6f);
        to_spec = L0 + Ld * saturate(t);
        spread.x = light.source_length / (2.0f * max(length(to_spec), 1e-4f));
    }

    if (light.type == LIGHT_POINT && light.source_radius > 0.0f)
    {
        // Esfera: se acerca el punto al rayo reflejado, como mucho el radio
        float3 center_to_ray = dot(to_spec, R) * R - to_spec;
        to_spec += center_to_ray * saturate(light.source_radius / max(length(center_to_ray), 1e-5f));
        spread.y = light.source_radius / (2.0f * max(length(to_spec), 1e-4f));
    }

    // Dentro de la esfera la luz no crece mas: como si estuviera en su superficie
    float distance = max(length(to_light), light.type == LIGHT_POINT ? light.source_radius : 0.0f);
    L = to_light / max(length(to_light), 0.0001f);
    L_spec = to_spec / max(length(to_spec), 0.0001f);

    if (distance >= light.range)
        return float3(0.0f, 0.0f, 0.0f);

    float attenuation = 1.0f / (light.constant_att + light.linear_att * distance + light.quadratic_att * distance * distance);
    attenuation *= 1.0f - smoothstep(light.range * light.fall_start, light.range, distance);

    if (light.type == LIGHT_SPOT)
    {
        float theta = dot(L, -light.direction);
        attenuation *= saturate((theta - light.cos_outer) / max(light.cos_inner - light.cos_outer, 0.0001f));
    }

    return light.color * light.intensity * attenuation;
}

// Normalizacion de la energia de una luz con tamano para una rugosidad alpha (= roughness^2):
// el tubo ensancha el brillo en una direccion y la esfera en dos.
float AreaEnergy(float alpha, float2 spread)
{
    float tube = alpha / saturate(alpha + spread.x);
    float sphere = alpha / saturate(alpha + spread.y);
    return tube * sphere * sphere;
}

#endif
