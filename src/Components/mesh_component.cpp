
#include "components/mesh_component.h"

MeshComponent::MeshComponent(){
	set_model(Engine::get_instance()->get_cube());
	set_material(*(Engine::get_instance()->get_default_material()));
}

MeshComponent::MeshComponent(Model* model){
	m_model = *model;
}

MeshComponent::MeshComponent(const MeshComponent&){
	printf("copia");
}

MeshComponent::MeshComponent(MeshComponent&&){

	printf("movimiento");
}

MeshComponent::~MeshComponent(){
	//if (m_model) delete m_model;
}

void MeshComponent::set_model(Model* model){

	if (m_pending_material) {
		m_model = *model;
	}else {
		// I already have the correct material, pre save it
		Material tmp = m_model.meshes[0].material;
		m_model = *model;
		set_pending_material(true);
		set_material(tmp);
		set_pending_material(false);
	}
}
void MeshComponent::set_model(Model& model){

	m_model = model;
}

void MeshComponent::set_material(Material mat){

	if (m_pending_material) {

		for (auto& meshes : m_model.meshes) {
			meshes.material = mat;
		}

	
		for (auto& meshes : m_model.meshes_copy) {
			meshes.material = mat;
		}
	}
	

	/*
	for (auto& meshes : m_model_to_reload->meshes) {
		meshes.material = mat;
	}
	
	for (auto& meshes : m_model_to_reload->meshes_copy) {
		meshes.material = mat;
	}
	*/
}

void MeshComponent::set_pending_material(bool value) {
	m_pending_material = value;
}
