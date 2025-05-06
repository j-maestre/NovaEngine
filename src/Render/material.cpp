#include "render/material.h"
#include "Core/engine.h"

Material::Material(){

	//Engine* e = Engine::get_instance();

	//set_texture_albedo(e->get_default_albedo_texture());
}

Material::Material(const Material& other){

	m_texture_albedo = other.m_texture_albedo;
	m_texture_normal = other.m_texture_normal;
	m_texture_ao = other.m_texture_ao;
	m_texture_metallic = other.m_texture_metallic;
	m_texture_roughness = other.m_texture_roughness;
	m_texture_specular = other.m_texture_specular;
}

Material::Material(Material&& other){
	m_texture_albedo = other.m_texture_albedo;
	m_texture_normal = other.m_texture_normal;
	m_texture_ao = other.m_texture_ao;
	m_texture_metallic = other.m_texture_metallic;
	m_texture_roughness = other.m_texture_roughness;
	m_texture_specular = other.m_texture_specular;

	other.m_texture_albedo = nullptr;
	other.m_texture_normal = nullptr;
	other.m_texture_ao = nullptr;
	other.m_texture_metallic = nullptr;
	other.m_texture_roughness = nullptr;
	other.m_texture_specular = nullptr;
}

Material::~Material()
{
}

Material& Material::operator=(const Material& other){

	if (this != &other) {
		m_texture_albedo = other.m_texture_albedo;
		m_texture_normal = other.m_texture_normal;
		m_texture_ao = other.m_texture_ao;
		m_texture_metallic = other.m_texture_metallic;
		m_texture_roughness = other.m_texture_roughness;
		m_texture_specular = other.m_texture_specular;
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

		other.m_texture_albedo = nullptr;
		other.m_texture_normal = nullptr;
		other.m_texture_ao = nullptr;
		other.m_texture_metallic = nullptr;
		other.m_texture_roughness = nullptr;
		other.m_texture_specular = nullptr;
	}

	return *this;
}
