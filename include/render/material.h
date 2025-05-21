#pragma once
#include "Core/defines.h"
#include "render/texture.h"


class Material {

public:
	Material();
	Material(const Material&);
	Material(Material&&);
	~Material();

	Material& operator=(const Material&);
	Material& operator=(Material&&);

	void set_texture_albedo(Texture* t) { m_texture_albedo = t; }
	void set_texture_normal(Texture* t) { m_texture_normal = t; }
	void set_texture_metallic(Texture* t) { m_texture_metallic = t; }
	void set_texture_roughness(Texture* t) { m_texture_roughness = t; }
	void set_texture_ao(Texture* t) { m_texture_ao = t; }

	inline Texture* get_albedo() { return m_texture_albedo; }
	inline Texture* get_normal() { return m_texture_normal; }
	inline Texture* get_metallic() { return m_texture_metallic; }
	inline Texture* get_roughness() { return m_texture_roughness; }
	inline Texture* get_ao() { return m_texture_ao; }
	inline Texture* get_specular() { return m_texture_specular; }

private:
	Texture* m_texture_albedo;
	Texture* m_texture_normal;
	Texture* m_texture_metallic;
	Texture* m_texture_roughness;
	Texture* m_texture_ao;
	Texture* m_texture_specular;
};