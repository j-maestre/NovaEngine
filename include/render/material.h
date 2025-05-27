#pragma once
#include "Core/defines.h"
#include "render/texture.h"


class Engine;
class Material {

public:
	Material();
	Material(const Material&);
	Material(Material&&);
	~Material();

	Material& operator=(const Material&);
	Material& operator=(Material&&);

	void init_material();
	void set_texture_albedo(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); m_texture_albedo = t; m_color_value = { 0.0f, 0.0f, 0.0f, 0.0f }; }
	void set_texture_normal(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); m_texture_normal = t; }
	void set_texture_metallic(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); m_texture_metallic = t; m_metallic_value = 0.0f; }
	void set_texture_roughness(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); m_texture_roughness = t; m_roughness_value = 0.0f; }
	void set_texture_ao(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); m_texture_ao = t; }

	void set_metallic_value(float value) { m_metallic_value = value; }
	void set_roughness_value(float value) { m_roughness_value = value; }
	void set_color_value(Vec4 value) { m_color_value = value; }
	void set_emissive_value(Vec3 value) { m_emissive = value; }

	inline Texture* get_albedo() { return m_texture_albedo; }
	inline Texture* get_normal() { return m_texture_normal; }
	inline Texture* get_metallic() { return m_texture_metallic; }
	inline Texture* get_roughness() { return m_texture_roughness; }
	inline Texture* get_ao() { return m_texture_ao; }
	inline Texture* get_specular() { return m_texture_specular; }

	float get_metallic_value() { return m_metallic_value; }
	float get_roughness_value() { return m_roughness_value; }
	Vec4 get_color_value() { return m_color_value; }
	Vec3 get_emissive_value() { return m_emissive; }


private:

	friend ResourceManager;
	friend Engine;

	bool m_initialised = false;
	float m_metallic_value = 0.0f;
	float m_roughness_value = 0.0f;
	Vec4 m_color_value;
	Vec3 m_emissive;

	Texture* m_texture_albedo;
	Texture* m_texture_normal;
	Texture* m_texture_metallic;
	Texture* m_texture_roughness;
	Texture* m_texture_ao;
	Texture* m_texture_specular;
};