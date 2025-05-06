#include "components/Lights/DirectionalLight.h"
#include "Core/engine.h"

DirectionalLight::DirectionalLight() : Light(){
	init_buffers();
}

DirectionalLight::DirectionalLight(float aspect_ratio) : Light(aspect_ratio){
	init_buffers();
}

DirectionalLight::DirectionalLight(const DirectionalLight&)
{
}

DirectionalLight::DirectionalLight(DirectionalLight&&)
{
}

DirectionalLight::~DirectionalLight()
{
}

void DirectionalLight::upload_data(){
	Engine* engine = Engine::get_instance();
	engine->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0,1, &m_constant_buffer_light);
	engine->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_constant_buffer_light,0,nullptr,&m_buffer, 0,0);
}

void DirectionalLight::set_direction(Vec3 direction){
	m_buffer.direction = direction;
}

void DirectionalLight::set_enabled(bool enabled){
	if (enabled) {
		m_buffer.enabled = 1.0f;
	}else{
		m_buffer.enabled = 0.0f;
	}
}

void DirectionalLight::set_color(Vec3 color){
	m_buffer.diffuse_color = color;
}

void DirectionalLight::set_specular_strenght(float value){
	m_buffer.specular_strength = value;
}

void DirectionalLight::set_specular_color(Vec3 color){
	m_buffer.specular_color = color;
}

void DirectionalLight::set_specular_shininess(float value){
	m_buffer.specular_shininess = value;
}

void DirectionalLight::init_buffers(){

	m_constant_buffer_light = nullptr;
	ZeroMemory(&m_light_desc_constant_buffer, sizeof(m_light_desc_constant_buffer));
	m_light_desc_constant_buffer.Usage = D3D11_USAGE_DEFAULT;
	m_light_desc_constant_buffer.ByteWidth = sizeof(DirectionalLightConstantBuffer);
	m_light_desc_constant_buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_light_desc_constant_buffer.CPUAccessFlags = 0;
	Engine::get_instance()->get_engine_props()->deviceInterface->CreateBuffer(&m_light_desc_constant_buffer, NULL, &m_constant_buffer_light);
	
}
