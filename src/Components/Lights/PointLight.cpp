#include "components/Lights/PointLight.h"
#include "Core/engine.h"

PointLight::PointLight() : Light(){
	init_buffers();
}

PointLight::PointLight(float aspect_ratio) : Light(aspect_ratio){
	init_buffers();
}

PointLight::PointLight(const PointLight&)
{
}

PointLight::PointLight(PointLight&&)
{
}

PointLight::~PointLight(){
	m_constant_buffer_light->Release();
}

void PointLight::upload_data(){
	Engine* engine = Engine::get_instance();
	engine->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_constant_buffer_light);
	engine->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_constant_buffer_light, 0, nullptr, &m_buffer, 0, 0);
}

void PointLight::set_position(Vec3 pos){

	m_buffer.position = pos;
}

void PointLight::set_enabled(bool enabled){
	m_buffer.enabled = enabled;
}

void PointLight::set_color(Vec3 color){

	m_buffer.diffuse_color = color;
}

void PointLight::set_specular_strenght(float value){

	m_buffer.specular_strength = value;
}

void PointLight::set_specular_color(Vec3 color){
	m_buffer.specular_color = color;
}

void PointLight::set_specular_shininess(float value){
	m_buffer.specular_shininess = value;
}

void PointLight::set_constant_attenuation(float value){
	m_buffer.constant_att = value;
}

void PointLight::set_linear_attenuation(float value){
	m_buffer.linear_att = value;
}

void PointLight::set_quadratic_attenuation(float value){
	m_buffer.quadratic_att = value;
}

void PointLight::set_attenuation(float value){
	m_buffer.attenuation = value;
}

void PointLight::init_buffers(){

	m_constant_buffer_light = nullptr;
	ZeroMemory(&m_light_desc_constant_buffer, sizeof(m_light_desc_constant_buffer));
	m_light_desc_constant_buffer.Usage = D3D11_USAGE_DEFAULT;
	m_light_desc_constant_buffer.ByteWidth = sizeof(PointLightConstantBuffer);
	m_light_desc_constant_buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_light_desc_constant_buffer.CPUAccessFlags = 0;
	Engine::get_instance()->get_engine_props()->deviceInterface->CreateBuffer(&m_light_desc_constant_buffer, NULL, &m_constant_buffer_light);


}
