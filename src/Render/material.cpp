#include "render/material.h"
#include "Core/engine.h"

MaterialComponent::MaterialComponent(){

	//Engine* e = Engine::get_instance();

	//set_texture_albedo(e->get_default_albedo_texture());
}

MaterialComponent::MaterialComponent(const MaterialComponent& other){

	m_texture_albedo = other.m_texture_albedo;
	m_texture_ao = other.m_texture_ao;
	m_texture_metallic = other.m_texture_metallic;
	m_texture_roughness = other.m_texture_roughness;
	m_texture_specular = other.m_texture_specular;
}

MaterialComponent::MaterialComponent(MaterialComponent&& other){
	m_texture_albedo = other.m_texture_albedo;
	m_texture_ao = other.m_texture_ao;
	m_texture_metallic = other.m_texture_metallic;
	m_texture_roughness = other.m_texture_roughness;
	m_texture_specular = other.m_texture_specular;

	other.m_texture_albedo = nullptr;
	other.m_texture_ao = nullptr;
	other.m_texture_metallic = nullptr;
	other.m_texture_roughness = nullptr;
	other.m_texture_specular = nullptr;
}

MaterialComponent::~MaterialComponent()
{
}

MaterialComponent& MaterialComponent::operator=(const MaterialComponent& other){

	if (this != &other) {
		m_texture_albedo = other.m_texture_albedo;
		m_texture_ao = other.m_texture_ao;
		m_texture_metallic = other.m_texture_metallic;
		m_texture_roughness = other.m_texture_roughness;
		m_texture_specular = other.m_texture_specular;
	}

	return *this;
}

MaterialComponent& MaterialComponent::operator=(MaterialComponent&& other){

	if (this != &other) {
		m_texture_albedo = other.m_texture_albedo;
		m_texture_ao = other.m_texture_ao;
		m_texture_metallic = other.m_texture_metallic;
		m_texture_roughness = other.m_texture_roughness;
		m_texture_specular = other.m_texture_specular;

		other.m_texture_albedo = nullptr;
		other.m_texture_ao = nullptr;
		other.m_texture_metallic = nullptr;
		other.m_texture_roughness = nullptr;
		other.m_texture_specular = nullptr;
	}

	return *this;
}
