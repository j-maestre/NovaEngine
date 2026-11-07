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
	// Una textura que no se pudo cargar llega como nullptr: se conserva la que hubiera
	void set_texture_albedo(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); if (!t) return; m_texture_albedo = t; m_color_value = { 0.0f, 0.0f, 0.0f, 0.0f }; }
	void set_texture_normal(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); if (!t) return; m_texture_normal = t; }
	void set_texture_metallic(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); if (!t) return; m_texture_metallic = t; m_metallic_value = 0.0f; }
	void set_texture_roughness(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); if (!t) return; m_texture_roughness = t; m_roughness_value = 0.0f; }
	void set_texture_ao(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); if (!t) return; m_texture_ao = t; }
	void set_texture_emissive(Texture* t) { assert(m_initialised && "Forget to call init_material() ?"); if (!t) return; m_texture_emissive = t; }

	void set_metallic_value(float value) { m_metallic_value = value; }
	void set_roughness_value(float value) { m_roughness_value = value; }
	void set_color_value(Vec4 value) { m_color_value = value; }
	void set_emissive_value(Vec3 value) { m_emissive = value; }
	void set_emissive_intensity(float value) { m_emissive_intensity = value; }
	// Cuanto del emisivo entra en el bloom: 0 = solo se ve el color, 1 = todo
	void set_emissive_bloom(float value) { m_emissive_bloom = value; }
	// Reflejo de frente de los dielectricos: F0 = 0.16 * reflectance^2. 0.5 = 4% (plastico, la mayoria de materiales)
	void set_reflectance(float value) { m_reflectance = value; }
	// Clear coat: barniz transparente encima (coche, madera lacada). 0 = sin barniz.
	void set_clear_coat(float value) { m_layers.x = value; }
	void set_clear_coat_roughness(float value) { m_layers.y = value; }
	// Sheen: brillo de tela en los bordes (terciopelo, algodon). El tinte lo lleva del blanco al color del albedo.
	void set_sheen(float value) { m_layers.z = value; }
	void set_sheen_tint(float value) { m_layers.w = value; }
	// Pulso del emisivo: radianes por segundo (0 = fijo) y la fraccion de intensidad en el punto mas bajo
	void set_emissive_pulse(float speed, float minimum) { m_emissive_pulse_speed = speed; m_emissive_pulse_min = minimum; }
	// Intensidad de este frame. La base (get_emissive_intensity) es la que se edita y se guarda.
	void update_emissive_pulse(float time) { m_current_emissive_intensity = m_emissive_intensity * pulse_factor(time, m_emissive_pulse_speed, m_emissive_pulse_min); }
	float get_current_emissive_intensity() const { return m_current_emissive_intensity; }
	void set_uv_scale(Vec2 value) { m_uv_scale = value; }

	inline Texture* get_albedo() { return m_texture_albedo; }
	inline Texture* get_normal() { return m_texture_normal; }
	inline Texture* get_metallic() { return m_texture_metallic; }
	inline Texture* get_roughness() { return m_texture_roughness; }
	inline Texture* get_ao() { return m_texture_ao; }
	inline Texture* get_emissive() { return m_texture_emissive; }
	inline Texture* get_specular() { return m_texture_specular; }

	float get_metallic_value() { return m_metallic_value; }
	float get_roughness_value() { return m_roughness_value; }
	Vec4 get_color_value() { return m_color_value; }
	Vec3 get_emissive_value() { return m_emissive; }
	float get_emissive_intensity() { return m_emissive_intensity; }
	float get_emissive_bloom() { return m_emissive_bloom; }
	float get_reflectance() { return m_reflectance; }
	float get_clear_coat() { return m_layers.x; }
	float get_clear_coat_roughness() { return m_layers.y; }
	float get_sheen() { return m_layers.z; }
	float get_sheen_tint() { return m_layers.w; }
	Vec4 get_layers() { return m_layers; }
	float get_emissive_pulse_speed() { return m_emissive_pulse_speed; }
	float get_emissive_pulse_min() { return m_emissive_pulse_min; }
	Vec2 get_uv_scale() { return m_uv_scale; }

	// TODO: anade emissive_intensity, guardalo en el gbuffer en el alpha de la textura de emissivo, anade imgui tambien


private:

	friend ResourceManager;
	friend Engine;

	bool m_initialised = false;
	float m_metallic_value = 0.0f;
	float m_roughness_value = 0.0f;
	Vec4 m_color_value;
	Vec3 m_emissive;
	float m_emissive_intensity = 1.0f;
	float m_emissive_bloom = 1.0f;
	float m_reflectance = 0.5f;
	Vec4 m_layers = { 0.0f, 0.1f, 0.0f, 0.5f };	// Clear coat, su rugosidad, sheen, tinte del sheen
	float m_emissive_pulse_speed = 0.0f;
	float m_emissive_pulse_min = 0.0f;
	float m_current_emissive_intensity = 0.0f;
	Vec2 m_uv_scale = { 1.0f, 1.0f };

	Texture* m_texture_albedo = nullptr;
	Texture* m_texture_normal = nullptr;
	Texture* m_texture_metallic = nullptr;
	Texture* m_texture_roughness = nullptr;
	Texture* m_texture_ao = nullptr;
	Texture* m_texture_emissive = nullptr;
	Texture* m_texture_specular = nullptr;
};