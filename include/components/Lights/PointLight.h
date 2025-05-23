#pragma once
#include "memory"
#include "Core/defines.h"
#include "components/Lights/Light.h"



struct PointLightConstantBuffer {
    Vec3 position = { 0.5f, 0.5f, 0.5f };
    float specular_strength = 0.003f;
    Vec3 diffuse_color = { 0.5f, 0.5f, 0.5f };
    float specular_shininess = 32.0f;
    Vec3 specular_color = { 1.0f, 1.0f, 1.0f };
    float constant_att = 1.0f;
    float linear_att = 0.045f;
    float quadratic_att = 0.0075f;
    float enabled = 0.5f;
    float attenuation = 1.0f;
    float intensity = 1.0f;
    float expossure = 1.0f;
    float distance = 1.0f;
    float fall_off = 7.5f; // 80 bytes
    float fall_start = 0.2f;
    Vec3 padding;

};



class PointLight : public Light{

public:
	PointLight();
	PointLight(float aspect_ratio);
	PointLight(const PointLight&);
	PointLight(PointLight&&);
	~PointLight();

	void upload_data() override;

    Vec3 get_position() { return m_buffer.position; }
    float get_enabled() { return m_buffer.enabled > 0.0f; }
    Vec3  get_color() { return m_buffer.diffuse_color; }
    float get_specular_strenght() { return m_buffer.specular_strength; }
    Vec3  get_specular_color() { return m_buffer.specular_color; }
    float get_specular_shininess() { return m_buffer.specular_shininess; }
    float get_constant_attenuation() { return m_buffer.constant_att; }
    float get_linear_attenuation() { return m_buffer.linear_att; }
    float get_quadratic_attenuation() { return m_buffer.quadratic_att; }
    float get_attenuation() { return m_buffer.attenuation; }
    float get_intensity() { return m_buffer.intensity; }
    float get_expossure() { return m_buffer.expossure; }
    float get_distance() { return m_buffer.distance; }
    float get_fall_off() { return m_buffer.fall_off; }
    float get_fall_start() { return m_buffer.fall_start; }

    void set_position(Vec3 pos);
    void set_enabled(bool enabled);
    void set_color(Vec3 color);
    void set_specular_strenght(float value);
    void set_specular_color(Vec3 color);
    void set_specular_shininess(float value);
    void set_constant_attenuation(float value);
    void set_linear_attenuation(float value);
    void set_quadratic_attenuation(float value);
    void set_attenuation(float value);
    void set_intensity(float value);
    void set_expossure(float value);
    void set_distance(float value);
    void set_fall_off(float value);
    void set_fall_start(float value);

private:

    void init_buffers();

    D3D11_BUFFER_DESC m_light_desc_constant_buffer;
    ID3D11Buffer* m_constant_buffer_light;
    PointLightConstantBuffer m_buffer;


};
