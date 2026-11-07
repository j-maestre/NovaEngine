#include "pbr_common.hlsli"
#include "lights_common.hlsli"

// Pase unico de luz: acumula todas las luces, el IBL y el ambiente en HDR lineal.
// El tonemapping y la gamma se aplican al final, en ps_deferred_composite.

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

struct PS_OUT
{
    float4 out_light : SV_Target0;
    float4 out_emissive : SV_Target1;
};

Texture2D albedo_tex : register(t0);
Texture2D<float> depth_tex : register(t1);
Texture2D normal_tex : register(t2);
Texture2D material_tex : register(t3); // R = metallic, G = roughness, B = ao, A = horizonte
Texture2D emissive_tex : register(t4);
TextureCube specular_tex : register(t5); // Entorno prefiltrado: mip 0 = pulido, ultimo mip = rugosidad 1
Texture2D brdf_tex : register(t6);
StructuredBuffer<GPULight> lights : register(t7);
TextureCube irradiance_tex : register(t8);
Texture2D ssao_tex : register(t9);
Texture2D layers_tex : register(t10); // RG = normal de la malla, B = clear coat | rugosidad, A = sheen | tinte
TextureCubeArray probe_tex : register(t11); // Sondas de reflexion, 6 caras cada una

SamplerState linear_sampler : register(s0);

cbuffer CameraDeferred : register(b1)
{
    float4x4 inv_view_proj;
    float3 cam_pos;
    float cubemap_max_mip_level;
};

// Debe coincidir con GPUReflectionProbe en defines.h
struct ReflectionProbe
{
    float3 box_min;
    float blend;     // Metros, desde el borde de la caja hacia dentro, en los que entra poco a poco
    float3 box_max;
    float intensity;
    float3 position; // Desde donde se capturo
    float slot;      // Que cubo del array es el suyo
};

static const uint MAX_REFLECTION_PROBES = 4;

// Ordenadas de la caja mas pequena a la mas grande: la de dentro manda sobre la que la rodea
cbuffer ReflectionProbes : register(b2)
{
    uint probe_count;
    float probe_max_mip;
    float2 probe_padding;
    ReflectionProbe probes[MAX_REFLECTION_PROBES];
};

static const float3 LUMINANCE = float3(0.2126f, 0.7152f, 0.0722f);

// Bloom: entra suave entre estos dos brillos, calculados sobre la luz total.
// Con ACES y exposicion 1, un brillo de 1 ya se ve casi blanco.
static const float BLOOM_START = 1.0f;
static const float BLOOM_END = 1.5f;
static const float BLOOM_STRENGTH = 0.3f;
// Tope: el especular de una superficie muy pulida puede valer miles en un solo pixel,
// y sin tope ese pixel pinta un halo enorme que parpadea al mover la camara.
static const float BLOOM_MAX = 64.0f;

// Deshace la proyeccion: de pixel y profundidad a coordenadas de clip, y de ahi a mundo con la inversa de view * proj
float3 WorldPositionFromDepth(float2 pixel_center, float depth)
{
    float2 size;
    depth_tex.GetDimensions(size.x, size.y);
    float2 ndc = float2(pixel_center.x / size.x * 2.0f - 1.0f, 1.0f - pixel_center.y / size.y * 2.0f);
    float4 world = mul(float4(ndc, depth, 1.0f), inv_view_proj);
    return world.xyz / world.w;
}

// SSAO a la resolucion de pantalla. Si se calculo a la mitad, mezcla los 4 valores mas cercanos,
// pero cada uno pesa menos cuanto mas distinta sea su distancia a la camara: los del otro lado
// de un borde casi no cuentan y no aparecen halos alrededor de los objetos.
float SampleSSAO(float2 pixel_center, float view_distance)
{
    float2 full_size;
    float2 low_size;
    depth_tex.GetDimensions(full_size.x, full_size.y);
    ssao_tex.GetDimensions(low_size.x, low_size.y);

    if (low_size.x >= full_size.x)
        return ssao_tex.Load(int3(pixel_center, 0)).r;

    float2 pos = pixel_center * (low_size / full_size) - 0.5f;
    int2 base = int2(floor(pos));
    float2 f = pos - float2(base);

    float sum = 0.0f;
    float weight_sum = 0.0f;
    float best_ao = 1.0f;
    float best_difference = 1e30f;
    [unroll]
    for (int j = 0; j < 2; j++)
    {
        [unroll]
        for (int i = 0; i < 2; i++)
        {
            int2 q = clamp(base + int2(i, j), int2(0, 0), int2(low_size) - 1);
            float2 s = ssao_tex.Load(int3(q, 0)).rg;
            float relative = abs(s.g - view_distance) / max(view_distance, 0.001f);
            float bilinear = (i == 0 ? 1.0f - f.x : f.x) * (j == 0 ? 1.0f - f.y : f.y);
            float w = bilinear * exp(-relative * relative / (2.0f * 0.03f * 0.03f));
            sum += s.r * w;
            weight_sum += w;
            if (relative < best_difference)
            {
                best_difference = relative;
                best_ao = s.r;
            }
        }
    }

    // Ninguno es de esta superficie (un borde de un pixel): el mas parecido
    return weight_sum > 1e-3f ? sum / weight_sum : best_ao;
}

// Reflejo del entorno en P en la direccion R. Si P esta dentro de la caja de una sonda, se usa lo que
// ella vio desde dentro de la habitacion; si no, el cubemap del cielo.
// Proyeccion de caja: el cubemap supone que todo esta infinitamente lejos. Se corta el rayo con la caja
// (las paredes) y se busca en la sonda la direccion desde donde se capturo hasta ese punto.
float3 SampleEnvironment(float3 P, float3 R, float roughness)
{
    float3 result = float3(0.0f, 0.0f, 0.0f);
    float remaining = 1.0f;

    [loop]
    for (uint i = 0; i < probe_count; i++)
    {
        ReflectionProbe probe = probes[i];
        float3 inside = min(P - probe.box_min, probe.box_max - P);
        float edge = min(inside.x, min(inside.y, inside.z));
        if (edge <= 0.0f)
            continue;

        float weight = saturate(edge / max(probe.blend, 0.001f)) * remaining;

        float3 to_max = (probe.box_max - P) / R;
        float3 to_min = (probe.box_min - P) / R;
        float3 far_hit = max(to_max, to_min);
        float hit_distance = min(far_hit.x, min(far_hit.y, far_hit.z));
        float3 direction = normalize(P + R * hit_distance - probe.position);

        // Un reflejo rugoso mezcla muchas direcciones: la correccion importa menos
        direction = lerp(direction, R, roughness * roughness);

        float3 probe_color = probe_tex.SampleLevel(linear_sampler, float4(direction, probe.slot), roughness * probe_max_mip).rgb;
        result += probe_color * probe.intensity * weight;
        remaining -= weight;
        if (remaining <= 0.001f)
            return result;
    }

    // El cielo se escala con el ambiente; lo que ve una sonda ya es la luz real de la escena
    float3 sky = specular_tex.SampleLevel(linear_sampler, R, roughness * cubemap_max_mip_level).rgb * SPECULAR_SCALE * ambient_intensity;
    return result + sky * remaining;
}

// Cuanta luz devuelve el sheen de media para un angulo de vista. Aproximacion de la tabla de
// Filament para la distribucion Charlie; sirve para quitarle esa energia a la capa de debajo.
float SheenAlbedo(float NdotV, float roughness)
{
    return lerp(0.1f, 0.35f, roughness) * (0.6f + 0.4f * pow(1.0f - NdotV, 2.0f));
}

PS_OUT PShader(PS_INPUT input)
{
    PS_OUT output;

    int3 pixel = int3(input.position.xy, 0);
    float3 N = normal_tex.Load(pixel).xyz;
    float4 emissive = emissive_tex.Load(pixel); // RGB: emisivo. A: cuanto entra en el bloom.
    float3 emissive_bloom = emissive.rgb * emissive.a;

    // Sin geometria (cielo): lo pinta despues draw_skybox
    if (dot(N, N) < 0.001f)
    {
        output.out_light = float4(0.0f, 0.0f, 0.0f, 1.0f);
        output.out_emissive = float4(emissive_bloom, 1.0f);
        return output;
    }

    N = normalize(N);
    float3 P = WorldPositionFromDepth(input.position.xy, depth_tex.Load(pixel));
    float3 albedo = albedo_tex.Load(pixel).rgb;
    float4 material = material_tex.Load(pixel);
    float metallic = material.r;
    float roughness = material.g;
    float ao = material.b;
    float screen_ao = ssao_power > 0.0f ? pow(saturate(SampleSSAO(input.position.xy, length(P - cam_pos))), ssao_power) : 1.0f;

    float3 V = normalize(cam_pos - P);
    float NdotV = max(dot(N, V), 0.0f);
    float3 R = reflect(-V, N);
    float alpha_light = max(roughness * roughness, 0.002f);

    // Dielectricos: F0 = 0.16 * reflectance^2 (0.5 da el 4% de siempre). Metales: su color.
    float reflectance = albedo_tex.Load(pixel).a;
    float3 F0 = lerp((0.16f * reflectance * reflectance).xxx, albedo, metallic);

    // Tabla BRDF: A + B es la energia que devuelve el especular con un solo rebote
    float2 env_brdf = brdf_tex.SampleLevel(linear_sampler, float2(NdotV, roughness), 0.0f).rg;

    // Multiscattering (Fdez-Aguera, Filament): en una superficie rugosa la luz rebota entre varias
    // microfacetas antes de salir y GGX solo cuenta el primer rebote. Sin esto los metales rugosos
    // salen mas oscuros de lo que son. Se devuelve la energia que falta, en proporcion a F0.
    float3 energy_compensation = 1.0f + F0 * (1.0f / max(env_brdf.x + env_brdf.y, 0.001f) - 1.0f);

    // Capas: clear coat (barniz transparente encima) y sheen (brillo de tela en los bordes)
    float4 layers = layers_tex.Load(pixel);
    float2 coat_params = UnpackTwo(layers.b);
    float2 sheen_params = UnpackTwo(layers.a);
    float clear_coat = coat_params.x;
    float coat_roughness = clamp(coat_params.y, 0.03f, 1.0f);
    float alpha_coat = max(coat_roughness * coat_roughness, 0.002f);
    // El barniz es liso: usa la normal de la malla, no la del mapa de normales
    float3 N_coat = clear_coat > 0.0f ? DecodeOctahedron(layers.rg) : N;
    float coat_NdotV = max(dot(N_coat, V), 0.0001f);
    float3 R_coat = reflect(-V, N_coat);
    float3 tint = albedo / max(max(albedo.r, max(albedo.g, albedo.b)), 0.0001f);
    float3 sheen_color = sheen_params.x * lerp(float3(1.0f, 1.0f, 1.0f), tint, sheen_params.y);
    float sheen_strength = max(sheen_color.r, max(sheen_color.g, sheen_color.b));
    // La energia que se lleva el sheen ya no la devuelve lo de debajo
    float sheen_scaling = 1.0f - sheen_strength * SheenAlbedo(NdotV, roughness);

    // Luz directa
    float3 color = float3(0.0f, 0.0f, 0.0f);

    [loop]
    for (uint i = 0; i < light_count; i++)
    {
        GPULight light = lights[i];

        float3 L;
        float3 L_spec;
        float2 spread;
        float3 radiance = LightRadiance(light, P, R, L, L_spec, spread);
        float NdotL = saturate(dot(N, L));
        float NdotL_spec = saturate(dot(N, L_spec));
        if ((NdotL <= 0.0f && NdotL_spec <= 0.0f) || dot(radiance, radiance) <= 0.0f)
            continue;

        float3 H = normalize(V + L_spec);
        float NdotH = saturate(dot(N, H));
        float LdotH = saturate(dot(L_spec, H));
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(NdotV, NdotL_spec, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);

        float3 specular = NDF * G * F / (4.0f * NdotV * NdotL_spec + 0.0001f) * energy_compensation;
        float3 kD = (1.0f - F) * (1.0f - metallic);

        // La difusa usa su propia direccion (el punto de la luz mas cercano a P)
        float3 H_diffuse = normalize(V + L);
        float diffuse = DiffuseBurley(NdotV, NdotL, saturate(dot(L, H_diffuse)), roughness);

        float3 lit = (kD * albedo * diffuse * NdotL + specular * NdotL_spec * AreaEnergy(alpha_light, spread)) * sheen_scaling;

        if (sheen_strength > 0.0f)
            lit += sheen_color * DistributionCharlie(NdotH, roughness) * VisibilityNeubelt(NdotV, NdotL_spec) * NdotL_spec;

        if (clear_coat > 0.0f)
        {
            // Segundo especular encima, con F0 de 0.04 (barniz) y su propia normal y punto de la luz.
            // Lo que refleja ya no llega a lo de debajo.
            float3 L_coat;
            float3 L_coat_spec;
            float2 coat_spread;
            LightRadiance(light, P, R_coat, L_coat, L_coat_spec, coat_spread);
            float3 H_coat = normalize(V + L_coat_spec);
            float coat_LdotH = saturate(dot(L_coat_spec, H_coat));
            float coat_NdotL = saturate(dot(N_coat, L_coat_spec));
            float coat_fresnel = FresnelSchlick(coat_LdotH, float3(0.04f, 0.04f, 0.04f)).x * clear_coat;
            float coat = DistributionGGX(N_coat, H_coat, coat_roughness) * VisibilityKelemen(coat_LdotH) * coat_fresnel;
            lit = lit * (1.0f - coat_fresnel) + coat * coat_NdotL * AreaEnergy(alpha_coat, coat_spread);
        }

        color += lit * radiance;
    }

    // Fisicamente el AO no afecta a la luz directa; una parte se aplica para que se vea bajo luz fuerte
    color *= lerp(1.0f, screen_ao, ssao_direct);

    // Ambiente difuso e IBL especular, una sola vez. Aqui el AO se aplica entero.
    float3 reflect_color = SampleEnvironment(P, R, roughness);
    float3 F_ibl = FresnelSchlickRoughness(NdotV, F0, roughness);

    float3 kD_ibl = (1.0f - F_ibl) * (1.0f - metallic);
    float3 irradiance = irradiance_tex.SampleLevel(linear_sampler, N, 0.0f).rgb * IRRADIANCE_SCALE;

    const float occlusion = ao * screen_ao;

    // Oclusion del especular: en un hueco, un reflejo rasante mira a la pared de al lado, no al cielo
    float alpha = roughness * roughness;
    float specular_occlusion = saturate(pow(NdotV + occlusion, pow(2.0f, -16.0f * alpha - 1.0f)) - 1.0f + occlusion);

    specular_occlusion *= material.a; // Horizonte, calculado en el G-Buffer

    float3 ambient = reflect_color * (F_ibl * env_brdf.x + env_brdf.y) * energy_compensation * specular_occlusion;
    ambient += kD_ibl * albedo * irradiance * occlusion * ambient_intensity;
    ambient *= sheen_scaling;

    if (sheen_strength > 0.0f)
        ambient += sheen_color * reflect_color * SheenAlbedo(NdotV, roughness) * occlusion;

    if (clear_coat > 0.0f)
    {
        float coat_fresnel = FresnelSchlick(coat_NdotV, float3(0.04f, 0.04f, 0.04f)).x * clear_coat;
        float3 coat_reflection = SampleEnvironment(P, R_coat, coat_roughness);
        ambient = ambient * (1.0f - coat_fresnel) + coat_reflection * coat_fresnel * specular_occlusion;
    }

    color += ambient;

    // Bloom sobre la luz total
    float luminance = dot(color, LUMINANCE);
    float bright = smoothstep(BLOOM_START, BLOOM_END, luminance);
    float3 bloom_light = color * min(1.0f, BLOOM_MAX / max(luminance, 0.0001f));
    output.out_emissive = float4(emissive_bloom + bloom_light * bright * BLOOM_STRENGTH, 1.0f);

    color += emissive.rgb;

    output.out_light = float4(color, 1.0f);
    return output;
}
