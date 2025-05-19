#include "components/Lights/SpotLight.h"
#include "Core/engine.h"

SpotLight::SpotLight() : Light(){
	init_buffers();
}

SpotLight::SpotLight(float aspect_ratio) : Light(aspect_ratio){
	init_buffers();
}

SpotLight::SpotLight(const SpotLight&)
{
}

SpotLight::SpotLight(SpotLight&&)
{
}

SpotLight::~SpotLight(){
	m_constant_buffer_light->Release();
}

void SpotLight::upload_data(){

	Engine* engine = Engine::get_instance();
	engine->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_constant_buffer_light);
	engine->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_constant_buffer_light, 0, nullptr, &m_buffer, 0, 0);

}

void SpotLight::set_direction(Vec3 dir){

	m_buffer.direction = dir;
}

void SpotLight::set_position(Vec3 pos){
	m_buffer.position = pos;
}

void SpotLight::set_enabled(bool enabled){

	m_buffer.enabled = enabled ? 1.0f : 0.0f;
}

void SpotLight::set_color(Vec3 color){
	m_buffer.diffuse_color = color;
}

void SpotLight::set_specular_strenght(float value){
	m_buffer.specular_strength = value;
}

void SpotLight::set_specular_color(Vec3 color){
	m_buffer.specular_color = color;
}

void SpotLight::set_specular_shininess(float value){
	m_buffer.specular_shininess = value;
}

void SpotLight::set_constant_attenuation(float value){
	m_buffer.constant_att = value;
}

void SpotLight::set_linear_attenuation(float value){
	m_buffer.linear_att = value;
}

void SpotLight::set_quadratic_attenuation(float value){
	m_buffer.quadratic_att = value;
}

void SpotLight::set_inner_circle(float radius){
	m_buffer.cutt_off = radius;
}

void SpotLight::set_outer_circle(float radius){
	m_buffer.outer_cut_off = radius;
}

void SpotLight::init_buffers(){

	m_constant_buffer_light = nullptr;
	ZeroMemory(&m_light_desc_constant_buffer, sizeof(m_light_desc_constant_buffer));
	m_light_desc_constant_buffer.Usage = D3D11_USAGE_DEFAULT;
	m_light_desc_constant_buffer.ByteWidth = sizeof(SpotLightConstantBuffer);
	m_light_desc_constant_buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_light_desc_constant_buffer.CPUAccessFlags = 0;
	Engine::get_instance()->get_engine_props()->deviceInterface->CreateBuffer(&m_light_desc_constant_buffer, NULL, &m_constant_buffer_light);

}
