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
    float attenuation = 1.0f; // 64 bytes
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

private:

    void init_buffers();

    D3D11_BUFFER_DESC m_light_desc_constant_buffer;
    ID3D11Buffer* m_constant_buffer_light;
    PointLightConstantBuffer m_buffer;


};
