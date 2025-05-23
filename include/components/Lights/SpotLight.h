#pragma once
#include "memory"
#include "Core/defines.h"
#include "components/Lights/Light.h"

struct SpotLightConstantBuffer {
    Vec3 position = { 0.5f, 0.5f, 0.5f };
    float enabled = 0.5f;
    Vec3 direction = { 0.5f, 0.5f, 0.5f };
    float cutt_off = 2.5f;
    Vec3 diffuse_color = { 0.5f, 0.5f, 0.5f };
    float outer_cut_off = 50.0f;
    Vec3 specular_color = { 1.0f, 1.0f, 1.0f };
    float specular_strength = 0.003f;
    float specular_shininess = 32.0f;
    float constant_att = 1.0f;
    float linear_att = 0.045f;
    float quadratic_att = 0.0075f;
    float expossure = 1.0f;
    float intensity = 1.0f;
    float light_distance = 10.0f;
    float padding;
};

class SpotLight : public Light {

public:
	SpotLight();
	SpotLight(float aspect_ratio);
	SpotLight(const SpotLight&);
	SpotLight(SpotLight&&);
	~SpotLight();


	void upload_data() override;
    

    Vec3 get_direction() { return m_buffer.direction; }
    Vec3 get_position() { return m_buffer.position; }
    float get_enabled() { return m_buffer.enabled > 0.0f; }
    Vec3  get_color() { return m_buffer.diffuse_color; }
    float get_specular_strenght() { return m_buffer.specular_strength; }
    Vec3  get_specular_color() { return m_buffer.specular_color; }
    float get_specular_shininess() { return m_buffer.specular_shininess; }
    float get_constant_attenuation() { return m_buffer.constant_att; }
    float get_linear_attenuation() { return m_buffer.linear_att; }
    float get_quadratic_attenuation() { return m_buffer.quadratic_att; }

    float get_inner_circle() { return m_buffer.cutt_off; }
    float get_outside_circle() { return m_buffer.outer_cut_off; }

    float get_expossure() { return m_buffer.expossure; }

    float get_intensity() { return m_buffer.intensity; }
    float get_distance() { return m_buffer.light_distance; }


    void set_direction(Vec3 dir);
    void set_position(Vec3 pos);
    void set_enabled(bool enabled);
    void set_color(Vec3 color);
    void set_specular_strenght(float value);
    void set_specular_color(Vec3 color);
    void set_specular_shininess(float value);
    void set_constant_attenuation(float value);
    void set_linear_attenuation(float value);
    void set_quadratic_attenuation(float value);

    void set_inner_circle(float radius);
    void set_outer_circle(float radius);

    void set_expossure(float value);
    void set_intensity(float value);
    void set_distance(float value);

private:

    void init_buffers();

    D3D11_BUFFER_DESC m_light_desc_constant_buffer;
    ID3D11Buffer* m_constant_buffer_light;
    SpotLightConstantBuffer m_buffer;

};

