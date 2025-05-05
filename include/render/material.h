#pragma once
#include "Core/defines.h"
#include "render/texture.h"


class MaterialComponent {

public:
	MaterialComponent();
	MaterialComponent(const MaterialComponent&);
	MaterialComponent(MaterialComponent&&);
	~MaterialComponent();

	MaterialComponent& operator=(const MaterialComponent&);
	MaterialComponent& operator=(MaterialComponent&&);

	void set_texture_albedo(Texture* t) { m_texture_albedo = t; }

	inline Texture* get_albedo() { return m_texture_albedo; }
	inline Texture* get_metallic() { return m_texture_metallic; }
	inline Texture* get_roughness() { return m_texture_roughness; }
	inline Texture* get_ao() { return m_texture_ao; }
	inline Texture* get_specular() { return m_texture_specular; }

private:
	Texture* m_texture_albedo;
	Texture* m_texture_metallic;
	Texture* m_texture_roughness;
	Texture* m_texture_ao;
	Texture* m_texture_specular;
};