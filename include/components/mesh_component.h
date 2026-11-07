#pragma once
#include "Core/defines.h"
#include "Core/input.h"
#include "Core/engine.h"
#include "memory"


class MeshComponent {

public:
	// entt no mueve este componente al borrar otros: los punteros a el siguen siendo validos
	static constexpr bool in_place_delete = true;

	MeshComponent();
	MeshComponent(Model* model);
	MeshComponent(const MeshComponent&);
	MeshComponent(MeshComponent&&);
	~MeshComponent();

	void set_model(Model* model);
	void set_model(Model& model);
	inline Model* get_model() { return &m_model; }

	// Set the same material to all sub meshes of a fbx
	void set_material(Material mat);
	void set_pending_material(bool value);

	// Un material por slot del archivo original. Con uno solo, se aplica a todas las mallas.
	void set_materials(const std::vector<Material>& materials);

	// Guarda como propios los materiales actuales de las mallas (tras editarlos en el Inspector)
	void store_materials_from_model();

	// False si los materiales son los que traia el archivo
	bool has_own_materials() const { return !m_pending_material; }
	std::vector<Material>& get_materials() { return m_materials; }

	void set_model_path(const std::string& path) { m_model_path = path; }

	// Si las sondas de reflexion lo capturan. Una sonda ve la escena desde un solo punto y pega todo
	// lo que ve en las paredes de su caja: un objeto en medio de la habitacion aparece en reflejos donde
	// no toca. Lo normal es dejarlo solo en lo grande y estatico (paredes, suelo, muebles grandes).
	void set_visible_in_probes(bool value) { m_visible_in_probes = value; }
	bool is_visible_in_probes() const { return m_visible_in_probes; }
	const std::string& get_model_path() const { return m_model_path; }

private:

	void apply_materials();

	Model m_model;
	std::vector<Material> m_materials;
	std::string m_model_path;
	bool m_pending_material = true;
	bool m_visible_in_probes = true;

};