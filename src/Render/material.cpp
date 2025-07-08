#include "render/material.h"
#include "Core/engine.h"

Material::Material(){

	//Engine* e = Engine::get_instance();

	//set_texture_albedo(e->get_default_albedo_texture());
	m_color_value = {0.0f, 0.0f, 0.0f, 1.0f};
	m_emissive = {0.0f, 0.0f, 0.0f};
	
}

Material::Material(const Material& other){

	m_texture_albedo = other.m_texture_albedo;
	m_texture_normal = other.m_texture_normal;
	m_texture_ao = other.m_texture_ao;
	m_texture_metallic = other.m_texture_metallic;
	m_texture_roughness = other.m_texture_roughness;
	m_texture_specular = other.m_texture_specular;
	m_texture_emissive = other.m_texture_emissive;
	m_initialised = other.m_initialised;
	m_emissive = other.m_emissive;
	m_emissive_intensity = other.m_emissive_intensity;

	m_color_value = other.m_color_value;
	m_metallic_value = other.m_metallic_value;
	m_roughness_value = other.m_roughness_value;
}

Material::Material(Material&& other){
	m_texture_albedo = other.m_texture_albedo;
	m_texture_normal = other.m_texture_normal;
	m_texture_ao = other.m_texture_ao;
	m_texture_metallic = other.m_texture_metallic;
	m_texture_roughness = other.m_texture_roughness;
	m_texture_specular = other.m_texture_specular;
	m_texture_emissive = other.m_texture_emissive;
	m_emissive = other.m_emissive;
	m_emissive_intensity = other.m_emissive_intensity;

	m_color_value = other.m_color_value;
	m_metallic_value = other.m_metallic_value;
	m_roughness_value = other.m_roughness_value;

	m_initialised = other.m_initialised;

	other.m_texture_albedo = nullptr;
	other.m_texture_normal = nullptr;
	other.m_texture_ao = nullptr;
	other.m_texture_metallic = nullptr;
	other.m_texture_roughness = nullptr;
	other.m_texture_specular = nullptr;
	other.m_texture_emissive = nullptr;

	other.m_color_value = { 0.0f, 0.0f, 0.0f, 0.0f };
	other.m_metallic_value = 0.0f;
	other.m_roughness_value = 0.0f;
	other.m_initialised = false;
	other.m_emissive = {0.0f, 0.0f, 0.0f};
	other.m_emissive_intensity = 0.0f;

}

Material::~Material(){

	/*
	if (m_buffer_ptr && m_initialised) {
		m_buffer_ptr->Release();
	}
	*/
}

Material& Material::operator=(const Material& other){

	if (this != &other) {
		m_texture_albedo = other.m_texture_albedo;
		m_texture_normal = other.m_texture_normal;
		m_texture_ao = other.m_texture_ao;
		m_texture_metallic = other.m_texture_metallic;
		m_texture_roughness = other.m_texture_roughness;
		m_texture_specular = other.m_texture_specular;
		m_texture_emissive = other.m_texture_emissive;
		m_emissive = other.m_emissive;
		m_emissive_intensity = other.m_emissive_intensity;

		m_color_value = other.m_color_value;
		m_metallic_value = other.m_metallic_value;
		m_roughness_value = other.m_roughness_value;
		m_initialised = other.m_initialised;

	}

	return *this;
}

Material& Material::operator=(Material&& other){

	if (this != &other) {
		m_texture_albedo = other.m_texture_albedo;
		m_texture_normal = other.m_texture_normal;
		m_texture_ao = other.m_texture_ao;
		m_texture_metallic = other.m_texture_metallic;
		m_texture_roughness = other.m_texture_roughness;
		m_texture_specular = other.m_texture_specular;
		m_texture_emissive = other.m_texture_emissive;
		m_emissive = other.m_emissive;
		m_emissive_intensity = other.m_emissive_intensity;

		m_color_value = other.m_color_value;
		m_metallic_value = other.m_metallic_value;
		m_roughness_value = other.m_roughness_value;
		m_initialised = other.m_initialised;

		other.m_texture_albedo = nullptr;
		other.m_texture_normal = nullptr;
		other.m_texture_ao = nullptr;
		other.m_texture_metallic = nullptr;
		other.m_texture_roughness = nullptr;
		other.m_texture_specular = nullptr;
		other.m_texture_emissive = nullptr;
		other.m_emissive = {0.0f, 0.0f, 0.0f};
		other.m_emissive_intensity = 0.0f;

		other.m_color_value = { 0.0f, 0.0f, 0.0f, 0.0f };
		other.m_metallic_value = 0.0f;
		other.m_roughness_value = 0.0f;
		other.m_initialised = false;

	}

	return *this;
}

void Material::init_material(){

	Material* m = Engine::get_instance()->get_default_material();
	m_initialised = true;
	set_texture_albedo(m->get_albedo());
	set_texture_normal(m->get_normal());
	set_texture_metallic(m->get_metallic());
	set_texture_roughness(m->get_roughness());
	set_texture_ao(m->get_ao());
	set_texture_emissive(m->get_emissive());
}


