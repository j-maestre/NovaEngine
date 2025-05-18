#pragma once
#include "memory"
#include "Core/defines.h"
#include "components/Lights/Light.h"

// Raw values just to check if data is correctly uploaded
struct DirectionalLightConstantBuffer {
	Vec3 direction = { 0.0f, 0.5f, 0.0f };
	float enabled = 0.5f;
	Vec3 diffuse_color = { 0.5f, 0.5f, 0.5f };
	float specular_strength = 0.003f;
	Vec3 specular_color = { 1.0f, 1.0f, 1.0f };
	float specular_shininess = 32.0f; // 48 bytes

};

class DirectionalLight : public Light {

public:
	DirectionalLight();
	DirectionalLight(float aspect_ratio);
	DirectionalLight(const DirectionalLight&);
	DirectionalLight(DirectionalLight&&);
	~DirectionalLight();

	void upload_data() override;

	void set_direction(Vec3 direction);
	void set_enabled(bool enabled);
	void set_color(Vec3 color);
	void set_specular_strenght(float value);
	void set_specular_color(Vec3 color);
	void set_specular_shininess(float value);
	
	Vec3& get_direction();
	bool get_enabled();
	Vec3& get_color();
	float get_specular_strenght();
	Vec3& get_specular_color();
	float get_specular_shininess();
private:
	
	void init_buffers();

	D3D11_BUFFER_DESC m_light_desc_constant_buffer;
	ID3D11Buffer* m_constant_buffer_light;
	DirectionalLightConstantBuffer m_buffer;

};

