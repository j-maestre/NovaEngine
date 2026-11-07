
#include "components/mesh_component.h"
#include <algorithm>

MeshComponent::MeshComponent(){
	set_model(Engine::get_instance()->get_cube());
	set_material(*(Engine::get_instance()->get_default_material()));
}

MeshComponent::MeshComponent(Model* model){
	m_model = *model;
}

MeshComponent::MeshComponent(const MeshComponent& other)
	: m_model(other.m_model), m_materials(other.m_materials), m_model_path(other.m_model_path), m_pending_material(other.m_pending_material), m_visible_in_probes(other.m_visible_in_probes) {
}

MeshComponent::MeshComponent(MeshComponent&& other)
	: m_model(std::move(other.m_model)), m_materials(std::move(other.m_materials)), m_model_path(std::move(other.m_model_path)), m_pending_material(other.m_pending_material), m_visible_in_probes(other.m_visible_in_probes) {
}

MeshComponent::~MeshComponent(){
}

void MeshComponent::set_model(Model* model){

	m_model = *model;
	if (!m_pending_material) {
		apply_materials();
	}
}

void MeshComponent::set_model(Model& model){

	m_model = model;
}

void MeshComponent::set_material(Material mat){

	m_materials.clear();
	m_materials.push_back(mat);
	apply_materials();
}

void MeshComponent::set_materials(const std::vector<Material>& materials){

	m_materials = materials;
	m_pending_material = materials.empty();
	apply_materials();
}

void MeshComponent::apply_materials(){

	if (m_materials.empty()) return;

	auto material_for = [&](const Mesh& mesh) -> const Material& {
		return mesh.material_slot < m_materials.size() ? m_materials[mesh.material_slot] : m_materials[0];
	};

	for (Mesh& mesh : m_model.meshes) {
		mesh.material = material_for(mesh);
	}

	for (Mesh& mesh : m_model.meshes_copy) {
		mesh.material = material_for(mesh);
	}
}

void MeshComponent::store_materials_from_model(){

	unsigned int slots = 0;
	for (const Mesh& mesh : m_model.meshes) {
		slots = std::max(slots, mesh.material_slot + 1);
	}

	m_materials.assign(slots, Material());
	std::vector<bool> filled(slots, false);
	for (const Mesh& mesh : m_model.meshes) {
		if (!filled[mesh.material_slot]) {
			m_materials[mesh.material_slot] = mesh.material;
			filled[mesh.material_slot] = true;
		}
	}

	// Slots sin malla: cualquier material valido
	for (unsigned int i = 0; i < slots; i++) {
		if (!filled[i]) m_materials[i] = m_model.meshes[0].material;
	}

	m_pending_material = false;
}

void MeshComponent::set_pending_material(bool value) {
	m_pending_material = value;
}
